#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "state_machine.h"

void uiInit();
void uiUpdate();
TFT_eSPI &uiGetTft();
