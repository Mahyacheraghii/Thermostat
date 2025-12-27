#pragma once

#include <Arduino.h>
#include "state_machine.h"

constexpr uint8_t HEATER_RELAY_PIN = 26;
constexpr uint8_t COOLER_RELAY_PIN = 27;
constexpr uint8_t FAN_RELAY_PIN = 14;
constexpr uint8_t PUMP_RELAY_PIN = 25;

void outputsInit();
void outputsApplyState(ThermostatState state);
void outputsAllOff();
// Relay helpers (active-high relays)
void setHeaterRelay(bool on);
void setCoolerRelay(bool on);
void setFanRelay(bool on);
void setPumpRelay(bool on);
