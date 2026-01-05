#include "outputs.h"
#include "state_machine.h"

namespace
{
    void setRelay(uint8_t pin, bool on)
    {
        digitalWrite(pin, on ? HIGH : LOW);
    }

    const char *stateName(ThermostatState state)
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

    void logOutputsIfChanged(ThermostatState state, bool fanHigh, bool fanLow, bool pump)
    {
        static bool lastFanHigh = false;
        static bool lastFanLow = false;
        static bool lastPump = false;
        static ThermostatState lastState = ThermostatState::OFF;

        if (fanHigh == lastFanHigh && fanLow == lastFanLow && pump == lastPump &&
            state == lastState)
            return;

        Serial.print("Outputs (");
        Serial.print(stateName(state));
        Serial.print("): fan_high=");
        Serial.print(fanHigh ? "ON" : "OFF");
        Serial.print(" fan_low=");
        Serial.print(fanLow ? "ON" : "OFF");
        Serial.print(" pump=");
        Serial.println(pump ? "ON" : "OFF");

        lastFanHigh = fanHigh;
        lastFanLow = fanLow;
        lastPump = pump;
        lastState = state;
    }
}

void outputsInit()
{
    // Ensure relays are driven LOW (OFF for active-HIGH relays) before switching pins to outputs
    digitalWrite(FAN_HIGH_RELAY_PIN, LOW);
    digitalWrite(FAN_LOW_RELAY_PIN, LOW);
    digitalWrite(PUMP_RELAY_PIN, LOW);

    pinMode(FAN_HIGH_RELAY_PIN, OUTPUT);
    pinMode(FAN_LOW_RELAY_PIN, OUTPUT);
    pinMode(PUMP_RELAY_PIN, OUTPUT);

    outputsAllOff(); // enforce known safe state
}

void outputsAllOff()
{
    setRelay(FAN_HIGH_RELAY_PIN, false);
    setRelay(FAN_LOW_RELAY_PIN, false);
    setRelay(PUMP_RELAY_PIN, false);
}

// Individual relay helpers (active-HIGH)
void setFanHighRelay(bool on)
{
    setRelay(FAN_HIGH_RELAY_PIN, on);
}

void setFanLowRelay(bool on)
{
    setRelay(FAN_LOW_RELAY_PIN, on);
}

void setPumpRelay(bool on)
{
    setRelay(PUMP_RELAY_PIN, on);
}

void outputsApplyState(ThermostatState state)
{
    bool fanHighOn = false;
    bool fanLowOn = false;
    bool pumpOn = false;

    switch (state)
    {
    case ThermostatState::PRESTART:
        // PRESTART: PRE-decide pump; only PRESTART controls pump.
        // Pump decision is cooling-driven and set in state_machine (`gPumpDesired`).
        pumpOn = gPumpDesired;
        break;

    case ThermostatState::COOLING_LOW:
        fanLowOn = true;
        pumpOn = true;
        break;

    case ThermostatState::COOLING_HIGH:
        fanHighOn = true;
        pumpOn = true;
        break;

    case ThermostatState::FAN_ONLY:
        fanLowOn = true;
        pumpOn = false;
        break;

    case ThermostatState::IDLE:
    case ThermostatState::OFF:
    default:
        pumpOn = false;
        break;
    }

    setRelay(FAN_HIGH_RELAY_PIN, fanHighOn);
    setRelay(FAN_LOW_RELAY_PIN, fanLowOn);
    setRelay(PUMP_RELAY_PIN, pumpOn);
    logOutputsIfChanged(state, fanHighOn, fanLowOn, pumpOn);
}
