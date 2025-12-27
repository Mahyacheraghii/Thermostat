#include <Arduino.h>
#include "state_machine.h"
#include "sensors.h"
#include "outputs.h"
#include "ui.h"
#include "touch.h"
#include "mqtt.h"
#include "wifi_config.h"

void setup()
{
    Serial.begin(115200);
    wifiConfigEnsureStored();
    sensorsInit();
    outputsInit();
    uiInit();
    touchInit();
    mqttInit();
    stateMachineInit();
}

void loop()
{
    sensorsUpdate();
    mqttUpdate();
    stateMachineUpdate();
    uiUpdate();
    touchUpdate();
    delay(10);
}
