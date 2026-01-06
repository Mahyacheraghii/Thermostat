#include "state_machine.h"
#include "outputs.h"
#include "sensors.h"

ThermostatState gCurrentState = ThermostatState::IDLE;
float gSetPointC = 22.0f;
float gCurrentTempC = NAN;
float gCurrentHumidity = NAN;
static const float DEFAULT_HYSTERESIS_C = 1.0f;
float gHysteresisC = DEFAULT_HYSTERESIS_C;
bool gSetPointChanged = false;
bool gFanOnlyRequested = false;
bool gPowerOffRequested = false;
bool gPowerOnRequested = false;
bool gPumpDesired = false; // Decided only in PRESTART

static ThermostatState s_lastState = ThermostatState::OFF;

static const char *stateName(ThermostatState state)
{
    switch (state)
    {
    case ThermostatState::IDLE:
        return "IDLE";
    case ThermostatState::PRESTART:
        return "PRESTART";
    case ThermostatState::COOLING_LOW:
        return "COOLING_LOW";
    case ThermostatState::COOLING_HIGH:
        return "COOLING_HIGH";
    case ThermostatState::FAN_ONLY:
        return "FAN_ONLY";
    case ThermostatState::OFF:
        return "OFF";
    default:
        return "UNKNOWN";
    }
}

static void logStateChangeIfNeeded()
{
    if (gCurrentState != s_lastState)
    {
        Serial.print("State -> ");
        Serial.println(stateName(gCurrentState));
        s_lastState = gCurrentState;
    }
}

// PRESTART state management
static uint32_t gPrestartEnteredTime = 0;
static const uint32_t PUMP_PRIMING_DURATION_MS = 3000; // 3 seconds
static const uint32_t SENSOR_TIMEOUT_MS = 3000; // fail-safe if no fresh data

void stateMachineInit()
{
    gCurrentState = ThermostatState::IDLE;
    gSetPointC = 22.0f;
    gHysteresisC = DEFAULT_HYSTERESIS_C;
    gSetPointChanged = false;
    gFanOnlyRequested = false;
    gPowerOffRequested = false;
    gPowerOnRequested = false;
    gPumpDesired = false;
    gPrestartEnteredTime = 0;

    outputsApplyState(gCurrentState);
    s_lastState = gCurrentState;
}

// -------------------------
// Parameter / Flag API
// UI code should call these to request actions. They only set flags
// or update decision parameters; the state machine handles transitions.
// -------------------------
void setSetPoint(float celsius)
{
    if (!isnan(celsius))
    {
        gSetPointC = celsius;
        gSetPointChanged = true;
    }
}

void setHysteresis(float celsius)
{
    // Hysteresis is fixed at 1.0°C for this project.
    (void)celsius;
    gHysteresisC = DEFAULT_HYSTERESIS_C;
}

void requestFanOnly()
{
    gFanOnlyRequested = true;
}

void requestPowerOff()
{
    gPowerOffRequested = true;
}

void requestPowerOn()
{
    gPowerOnRequested = true;
}


void stateMachineUpdate()
{
    // ============================================================================
    // SAFETY: Sensor fail-safe (missing or stale data forces safe OFF)
    // ============================================================================
    const uint32_t now = millis();
    if (!sensorsAvailable() || (now - sensorsLastUpdateMs()) > SENSOR_TIMEOUT_MS)
    {
        gCurrentState = ThermostatState::OFF;
        gPumpDesired = false;
        outputsApplyState(gCurrentState);
        logStateChangeIfNeeded();
        return;
    }

    // ============================================================================
    // GLOBAL OVERRIDE: Power OFF request
    // ============================================================================
    if (gPowerOffRequested)
    {
        gCurrentState = ThermostatState::OFF;
        gPowerOffRequested = false; // Consume the flag
        gPumpDesired = false;
        outputsApplyState(gCurrentState);
        logStateChangeIfNeeded();
        return;
    }

    // ============================================================================
    // GLOBAL: Power ON request (leave OFF and resume normal control)
    // ============================================================================
    if (gPowerOnRequested)
    {
        gPowerOnRequested = false;
        gPumpDesired = false;
        gCurrentState = ThermostatState::IDLE;
        gSetPointChanged = false;
        gFanOnlyRequested = false;
        outputsApplyState(gCurrentState);
        logStateChangeIfNeeded();
        return;
    }

    auto tempValid = []() {
        return !isnan(gCurrentTempC) && !isnan(gSetPointC);
    };

    auto coolingShouldStart = []() {
        return gCurrentTempC >= (gSetPointC + gHysteresisC);
    };

    auto coolingShouldStop = []() {
        return gCurrentTempC <= (gSetPointC - gHysteresisC);
    };

    auto coolingSpeedState = []() -> ThermostatState {
        return (gCurrentTempC - gSetPointC) >= 8.0f
            ? ThermostatState::COOLING_HIGH
            : ThermostatState::COOLING_LOW;
    };

    switch (gCurrentState)
    {
    case ThermostatState::IDLE:
    {
        if (gFanOnlyRequested)
        {
            gCurrentState = ThermostatState::FAN_ONLY;
            gFanOnlyRequested = false;
            gPumpDesired = false;
            break;
        }

        if (gSetPointChanged && tempValid())
        {
            if (coolingShouldStart())
            {
                gCurrentState = ThermostatState::PRESTART;
                gPrestartEnteredTime = millis();
                gPumpDesired = true;
            }
            gSetPointChanged = false;
            break;
        }

        if (tempValid() && coolingShouldStart())
        {
            gCurrentState = ThermostatState::PRESTART;
            gPrestartEnteredTime = millis();
            gPumpDesired = true;
        }
        break;
    }

    case ThermostatState::PRESTART:
    {
        gPumpDesired = true;

        if (gFanOnlyRequested)
        {
            gCurrentState = ThermostatState::FAN_ONLY;
            gFanOnlyRequested = false;
            gPumpDesired = false;
            break;
        }

        uint32_t elapsedMs = millis() - gPrestartEnteredTime;
        if (elapsedMs < PUMP_PRIMING_DURATION_MS)
            break;

        if (!tempValid())
        {
            gCurrentState = ThermostatState::OFF;
            gPumpDesired = false;
            break;
        }

        if (coolingShouldStop())
        {
            gCurrentState = ThermostatState::IDLE;
            gPumpDesired = false;
        }
        else
        {
            gCurrentState = coolingSpeedState();
            gPumpDesired = true;
        }
        gSetPointChanged = false;
        break;
    }

    case ThermostatState::COOLING_LOW:
    case ThermostatState::COOLING_HIGH:
    {
        if (!tempValid())
        {
            gCurrentState = ThermostatState::OFF;
            gPumpDesired = false;
            break;
        }

        if (gFanOnlyRequested)
        {
            gCurrentState = ThermostatState::FAN_ONLY;
            gFanOnlyRequested = false;
            gPumpDesired = false;
            break;
        }

        if (coolingShouldStop())
        {
            gCurrentState = ThermostatState::IDLE;
            gPumpDesired = false;
            break;
        }

        const float deltaC = gCurrentTempC - gSetPointC;
        if (gCurrentState == ThermostatState::COOLING_HIGH)
        {
            if (deltaC < 8.0f)
                gCurrentState = ThermostatState::COOLING_LOW;
        }
        else
        {
            if (deltaC >= 8.0f)
                gCurrentState = ThermostatState::COOLING_HIGH;
        }

        gPumpDesired = true;
        gSetPointChanged = false;
        break;
    }

    case ThermostatState::FAN_ONLY:
    {
        gPumpDesired = false;
        if (gSetPointChanged)
            gSetPointChanged = false;
        if (gFanOnlyRequested)
            gFanOnlyRequested = false;
        break;
    }

    case ThermostatState::OFF:
    default:
        gPumpDesired = false;
        break;
    }

    outputsApplyState(gCurrentState);
    logStateChangeIfNeeded();
}
