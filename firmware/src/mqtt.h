#pragma once

#include <Arduino.h>

void mqttInit();
void mqttUpdate();
void mqttSaveConfig(const char *host, uint16_t port, const char *user, const char *pass, const char *baseTopic);
