#pragma once

#include <Arduino.h>
#include "state_machine.h"

bool sensorsInit();
void sensorsUpdate();
bool sensorsAvailable();
unsigned long sensorsLastUpdateMs();
