#pragma once

#include <Arduino.h>
#include "state_machine.h"

constexpr uint8_t FAN_HIGH_RELAY_PIN = 27;
constexpr uint8_t FAN_LOW_RELAY_PIN = 14;
constexpr uint8_t PUMP_RELAY_PIN = 25;

void outputsInit();
void outputsApplyState(ThermostatState state);
void outputsAllOff();
// Relay helpers (active-high relays)
void setFanHighRelay(bool on);
void setFanLowRelay(bool on);
void setPumpRelay(bool on);
