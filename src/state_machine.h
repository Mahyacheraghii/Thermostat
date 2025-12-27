#pragma once

#include <Arduino.h>

enum class ThermostatState : uint8_t
{
    IDLE = 0,
    PRESTART,
    HEATING,
    COOLING,
    FAN_ONLY,
    OFF
};

enum class ThermostatMode : uint8_t
{
    WINTER = 0,
    SUMMER
};

extern ThermostatState gCurrentState;
extern ThermostatMode gCurrentMode;
extern float gSetPointC;
extern float gCurrentTempC;
extern float gCurrentHumidity;
extern float gHysteresisC;
extern bool gSetPointChanged;
extern bool gFanOnlyRequested;
extern bool gPowerOffRequested;
extern bool gModeChanged;
extern bool gPumpDesired;

void stateMachineInit();
void stateMachineUpdate();
// Parameter and flag API (for UI only: set flags, do not change states directly)
void setSetPoint(float celsius);
void setHysteresis(float celsius);
void requestFanOnly();
void requestPowerOff();
void requestModeChange(ThermostatMode newMode);
