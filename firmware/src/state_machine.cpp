#include "state_machine.h"
#include "outputs.h"
#include "sensors.h"

ThermostatState gCurrentState = ThermostatState::IDLE;
ThermostatMode gCurrentMode = ThermostatMode::WINTER;
float gSetPointC = 22.0f;
float gCurrentTempC = NAN;
float gCurrentHumidity = NAN;
static const float DEFAULT_HYSTERESIS_C = 1.0f;
float gHysteresisC = DEFAULT_HYSTERESIS_C;
bool gSetPointChanged = false;
bool gFanOnlyRequested = false;
bool gPowerOffRequested = false;
bool gModeChanged = false;
bool gPumpDesired = false; // Decided only in PRESTART

// PRESTART state management
static uint32_t gPrestartEnteredTime = 0;
static const uint32_t PUMP_PRIMING_DURATION_MS = 10000; // 10 seconds
static const uint32_t SENSOR_TIMEOUT_MS = 3000; // fail-safe if no fresh data

void stateMachineInit()
{
    gCurrentState = ThermostatState::IDLE;
    gCurrentMode = ThermostatMode::WINTER;
    gSetPointC = 22.0f;
    gHysteresisC = DEFAULT_HYSTERESIS_C;
    gSetPointChanged = false;
    gFanOnlyRequested = false;
    gPowerOffRequested = false;
    gModeChanged = false;
    gPumpDesired = false;
    gPrestartEnteredTime = 0;

    outputsApplyState(gCurrentState);
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

void requestModeChange(ThermostatMode newMode)
{
    // Update desired mode and set mode-changed flag; state machine will handle PRESTART
    if (newMode != gCurrentMode)
    {
        gCurrentMode = newMode;
        gModeChanged = true;
    }
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
        outputsApplyState(gCurrentState);
        return;
    }

    // ============================================================================
    // GLOBAL OVERRIDE: Power OFF request
    // ============================================================================
    if (gPowerOffRequested)
    {
        gCurrentState = ThermostatState::OFF;
        gPowerOffRequested = false; // Consume the flag
        outputsApplyState(gCurrentState);
        return;
    }

    // ============================================================================
    // GLOBAL: Mode change always goes to PRESTART
    // ============================================================================
    if (gModeChanged)
    {
        // Consume the flag, decide pump based on current mode/temps, then enter PRESTART
        gModeChanged = false; // Consume the flag
        // Decide pump: mode-driven. If in SUMMER and cooling likely, pump desired.
        if (!isnan(gCurrentTempC) && !isnan(gSetPointC))
        {
            // Use hysteresis-aware "start cooling" condition:
            // start cooling when currentTemp >= setPoint + hysteresis
            if (gCurrentMode == ThermostatMode::SUMMER && (gCurrentTempC >= (gSetPointC + gHysteresisC)))
                gPumpDesired = true;
            else
                gPumpDesired = false;
        }
        else
        {
            // Sensor missing: default to pump OFF for safety
            gPumpDesired = false;
        }
        // Now enter PRESTART
        gCurrentState = ThermostatState::PRESTART;
        gPrestartEnteredTime = millis();
        outputsApplyState(gCurrentState);
        return;
    }

    // ============================================================================
    // STATE MACHINE TRANSITIONS
    // ============================================================================

    switch (gCurrentState)
    {
    case ThermostatState::IDLE:
    {
        // User requests FAN_ONLY: go to PRESTART first, then to FAN_ONLY
        if (gFanOnlyRequested)
        {
            // Fan-only requested: decide pump first (OFF), then enter PRESTART
            gPumpDesired = false;
            gCurrentState = ThermostatState::PRESTART;
            gPrestartEnteredTime = millis();
            gFanOnlyRequested = false; // Flag will be checked again in PRESTART
            // Note: gFanOnlyRequested will be re-checked in PRESTART
        }
        // SetPoint changed: go to PRESTART to evaluate conditions
        else if (gSetPointChanged)
        {
            // SetPoint changed: decide pump first, then enter PRESTART
            if (!isnan(gCurrentTempC) && !isnan(gSetPointC))
            {
                if (gCurrentMode == ThermostatMode::SUMMER && (gCurrentTempC >= (gSetPointC + gHysteresisC)))
                    gPumpDesired = true;
                else
                    gPumpDesired = false;
            }
            else
            {
                gPumpDesired = false;
            }
            gCurrentState = ThermostatState::PRESTART;
            gPrestartEnteredTime = millis();
            gSetPointChanged = false; // Consume the flag
        }
        // Otherwise stay IDLE
        break;
    }

    case ThermostatState::PRESTART:
    {
        uint32_t elapsedMs = millis() - gPrestartEnteredTime;

        // If pump was desired, perform priming for the configured duration.
        // If pump not desired, skip priming and evaluate immediately.
        if (gPumpDesired)
        {
            if (elapsedMs < PUMP_PRIMING_DURATION_MS)
            {
                // Stay in PRESTART while pump primes
                break;
            }
        }

        // Pump priming complete; now decide next state
        bool winterMode = (gCurrentMode == ThermostatMode::WINTER);
        bool summerMode = (gCurrentMode == ThermostatMode::SUMMER);
        bool tempIsValid = !isnan(gCurrentTempC) && !isnan(gSetPointC);

        // If temperature data is not valid, go to OFF (safe state)
        if (!tempIsValid)
        {
            gCurrentState = ThermostatState::OFF;
            break;
        }

        // Check if FAN_ONLY was requested during PRESTART
        if (gFanOnlyRequested)
        {
            gCurrentState = ThermostatState::FAN_ONLY;
            gFanOnlyRequested = false; // Consume the flag
            break;
        }

        // Branch based on mode and hysteresis-aware setpoint comparison
        // Heater should start when currentTemp <= setPoint - hysteresis
        if (winterMode && (gCurrentTempC <= (gSetPointC - gHysteresisC)))
        {
            gCurrentState = ThermostatState::HEATING;
        }
        // Cooler should start when currentTemp >= setPoint + hysteresis
        else if (summerMode && (gCurrentTempC >= (gSetPointC + gHysteresisC)))
        {
            gCurrentState = ThermostatState::COOLING;
        }
        else
        {
            // Condition not met; return to IDLE
            gCurrentState = ThermostatState::IDLE;
        }

        gSetPointChanged = false; // Clear flag if it was set
        break;
    }

    case ThermostatState::HEATING:
    {
        bool winterMode = (gCurrentMode == ThermostatMode::WINTER);
        bool tempIsValid = !isnan(gCurrentTempC) && !isnan(gSetPointC);

        // If temperature data is not valid, go to OFF (safe state)
        if (!tempIsValid)
        {
            gCurrentState = ThermostatState::OFF;
            break;
        }
        // HYSTERESIS: if temperature reached setPoint + hysteresis, stop heating
        if (gCurrentTempC >= (gSetPointC + gHysteresisC))
        {
            gCurrentState = ThermostatState::IDLE;
            break;
        }

        // SetPoint changed: re-evaluate whether heating should continue
        if (gSetPointChanged)
        {
            if (winterMode && (gSetPointC > gCurrentTempC))
            {
                gSetPointChanged = false;
                break; // stay HEATING
            }
            else
            {
                gCurrentState = ThermostatState::IDLE;
                gSetPointChanged = false;
                break;
            }
        }

        // User requests FAN_ONLY: ensure pump is OFF first via PRESTART
        if (gFanOnlyRequested)
        {
            gPumpDesired = false; // ensure pump OFF
            gCurrentState = ThermostatState::PRESTART;
            gPrestartEnteredTime = millis();
            gFanOnlyRequested = false; // will be consumed in PRESTART
            break;
        }

        // Otherwise remain in HEATING
        break;
    }

    case ThermostatState::COOLING:
    {
        bool summerMode = (gCurrentMode == ThermostatMode::SUMMER);
        bool tempIsValid = !isnan(gCurrentTempC) && !isnan(gSetPointC);

        // If temperature data is not valid, go to OFF (safe state)
        if (!tempIsValid)
        {
            gCurrentState = ThermostatState::OFF;
            break;
        }
        // HYSTERESIS: if temperature fallen to setPoint - hysteresis, stop cooling
        if (gCurrentTempC <= (gSetPointC - gHysteresisC))
        {
            gCurrentState = ThermostatState::IDLE;
            break;
        }

        // SetPoint changed: re-evaluate whether cooling should continue
        if (gSetPointChanged)
        {
            if (summerMode && (gSetPointC < gCurrentTempC))
            {
                gSetPointChanged = false;
                break; // stay COOLING
            }
            else
            {
                gCurrentState = ThermostatState::IDLE;
                gSetPointChanged = false;
                break;
            }
        }

        // User requests FAN_ONLY: ensure pump is turned off first via PRESTART
        if (gFanOnlyRequested)
        {
            gPumpDesired = false;
            gCurrentState = ThermostatState::PRESTART;
            gPrestartEnteredTime = millis();
            gFanOnlyRequested = false;
            break;
        }

        // Otherwise remain in COOLING
        break;
    }

    case ThermostatState::FAN_ONLY:
    {
        // User requests FAN_ONLY again or setPoint changes: stay in FAN_ONLY
        // FAN_ONLY doesn't respond to setPoint or mode changes directly,
        // except those are already handled by global overrides above

        if (gSetPointChanged)
        {
            gSetPointChanged = false; // Just consume it; stay in FAN_ONLY
        }

        if (gFanOnlyRequested)
        {
            gFanOnlyRequested = false; // Consume; already in FAN_ONLY
        }

        // Otherwise stay in FAN_ONLY
        break;
    }

    case ThermostatState::OFF:
    {
        // OFF state does not transition out by itself
        // Only exits via mode change or power restoration (handled at top of function)
        break;
    }

    default:
        break;
    }

    // Apply outputs based on the current (possibly updated) state
    outputsApplyState(gCurrentState);
}
