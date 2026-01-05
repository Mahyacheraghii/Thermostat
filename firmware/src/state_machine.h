#pragma once

#include <Arduino.h>

enum class ThermostatState : uint8_t
{
    IDLE = 0,
    PRESTART,
    COOLING_LOW,
    COOLING_HIGH,
    FAN_ONLY,
    OFF
};

extern ThermostatState gCurrentState;
extern float gSetPointC;
extern float gCurrentTempC;
extern float gCurrentHumidity;
extern float gHysteresisC;
extern bool gSetPointChanged;
extern bool gFanOnlyRequested;
extern bool gPowerOffRequested;
extern bool gPumpDesired;

void stateMachineInit();
void stateMachineUpdate();
// Parameter and flag API (for UI only: set flags, do not change states directly)
void setSetPoint(float celsius);
void setHysteresis(float celsius);
void requestFanOnly();
void requestPowerOff();
