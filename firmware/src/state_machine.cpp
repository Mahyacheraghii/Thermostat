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
static const uint32_t PUMP_PRIMING_DURATION_MS = 10000; // 10 seconds
static const uint32_t SENSOR_TIMEOUT_MS = 3000; // fail-safe if no fresh data

void stateMachineInit()
{
    gCurrentState = ThermostatState::IDLE;
    gSetPointC = 22.0f;
    gHysteresisC = DEFAULT_HYSTERESIS_C;
    gSetPointChanged = false;
    gFanOnlyRequested = false;
    gPowerOffRequested = false;
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
    // Allow updating hysteresis at runtime via UI or config tools.
    // Validate value: must be finite and non-negative. Typical hysteresis
    // values are small (e.g., 0.5°C). Ignore invalid inputs.
    if (!isnan(celsius) && isfinite(celsius) && celsius >= 0.0f)
    {
        gHysteresisC = celsius;
    }
}

void requestFanOnly()
{
    gFanOnlyRequested = true;
}

void requestPowerOff()
{
    gPowerOffRequested = true;
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

    auto tempValid = []() {
        return !isnan(gCurrentTempC) && !isnan(gSetPointC);
    };

    auto desiredCoolingState = []() -> ThermostatState {
        if (gCurrentTempC >= (gSetPointC + gHysteresisC))
            return ThermostatState::COOLING_HIGH;
        if (gCurrentTempC >= gSetPointC)
            return ThermostatState::COOLING_LOW;
        return ThermostatState::IDLE;
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
            if (desiredCoolingState() != ThermostatState::IDLE)
            {
                gCurrentState = ThermostatState::PRESTART;
                gPrestartEnteredTime = millis();
                gPumpDesired = true;
            }
            gSetPointChanged = false;
            break;
        }

        if (tempValid() && desiredCoolingState() != ThermostatState::IDLE)
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

        ThermostatState desired = desiredCoolingState();
        if (desired == ThermostatState::IDLE)
        {
            gCurrentState = ThermostatState::IDLE;
            gPumpDesired = false;
        }
        else
        {
            gCurrentState = desired;
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

        if (gCurrentTempC <= (gSetPointC - gHysteresisC))
        {
            gCurrentState = ThermostatState::IDLE;
            gPumpDesired = false;
            break;
        }

        ThermostatState desired = desiredCoolingState();
        if (desired == ThermostatState::IDLE)
        {
            gCurrentState = ThermostatState::IDLE;
            gPumpDesired = false;
        }
        else
        {
            gCurrentState = desired;
            gPumpDesired = true;
        }
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
