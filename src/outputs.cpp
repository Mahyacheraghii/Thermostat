#include "outputs.h"
#include "state_machine.h"

namespace
{
    void setRelay(uint8_t pin, bool on)
    {
        digitalWrite(pin, on ? HIGH : LOW);
    }
}

void outputsInit()
{
    // Ensure relays are driven LOW (OFF for active-HIGH relays) before switching pins to outputs
    digitalWrite(HEATER_RELAY_PIN, LOW);
    digitalWrite(COOLER_RELAY_PIN, LOW);
    digitalWrite(FAN_RELAY_PIN, LOW);
    digitalWrite(PUMP_RELAY_PIN, LOW);

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    pinMode(COOLER_RELAY_PIN, OUTPUT);
    pinMode(FAN_RELAY_PIN, OUTPUT);
    pinMode(PUMP_RELAY_PIN, OUTPUT);

    outputsAllOff(); // enforce known safe state
}

void outputsAllOff()
{
    setRelay(HEATER_RELAY_PIN, false);
    setRelay(COOLER_RELAY_PIN, false);
    setRelay(FAN_RELAY_PIN, false);
    setRelay(PUMP_RELAY_PIN, false);
}

// Individual relay helpers (active-HIGH)
void setHeaterRelay(bool on)
{
    setRelay(HEATER_RELAY_PIN, on);
}

void setCoolerRelay(bool on)
{
    setRelay(COOLER_RELAY_PIN, on);
}

void setFanRelay(bool on)
{
    setRelay(FAN_RELAY_PIN, on);
}

void setPumpRelay(bool on)
{
    setRelay(PUMP_RELAY_PIN, on);
}

void outputsApplyState(ThermostatState state)
{
    switch (state)
    {
    case ThermostatState::PRESTART:
        // PRESTART: PRE-decide pump; only PRESTART controls pump.
        setRelay(HEATER_RELAY_PIN, false);
        setRelay(COOLER_RELAY_PIN, false);
        setRelay(FAN_RELAY_PIN, false);
        // Pump decision is mode-driven and set in state_machine (`gPumpDesired`).
        if (gPumpDesired)
            setRelay(PUMP_RELAY_PIN, true);
        else
            setRelay(PUMP_RELAY_PIN, false);
        break;

    case ThermostatState::HEATING:
        // HEATING: Heater + Fan. Pump MUST be OFF before/when entering HEATING
        setRelay(HEATER_RELAY_PIN, true);
        setRelay(COOLER_RELAY_PIN, false);
        setRelay(FAN_RELAY_PIN, true);
        break;

    case ThermostatState::COOLING:
        // COOLING: Cooler + Fan, pump decision handled in PRESTART (do not toggle pump here)
        setRelay(HEATER_RELAY_PIN, false);
        setRelay(COOLER_RELAY_PIN, true);
        setRelay(FAN_RELAY_PIN, true);
        break;

    case ThermostatState::FAN_ONLY:
        setRelay(HEATER_RELAY_PIN, false);
        setRelay(COOLER_RELAY_PIN, false);
        setRelay(FAN_RELAY_PIN, true);
        break;

    case ThermostatState::IDLE:
    case ThermostatState::OFF:
    default:
        outputsAllOff();
        break;
    }
}
