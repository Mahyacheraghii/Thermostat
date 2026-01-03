#pragma once

#include <Arduino.h>

#ifndef MQTT_HOST
#define MQTT_HOST "25e714c4c7914708950ce3a88dc1dd1a.s1.eu.hivemq.cloud"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif

#ifndef MQTT_USER
#define MQTT_USER "mahya"
#endif

#ifndef MQTT_PASS
#define MQTT_PASS "vikrap-weqrug-xabVe6"
#endif

#ifndef MQTT_BASE_TOPIC
#define MQTT_BASE_TOPIC "thermostat"
#endif

#ifndef MQTT_USE_TLS
#define MQTT_USE_TLS 1
#endif

void mqttInit();
void mqttUpdate();
void mqttSaveConfig(const char *host, uint16_t port, const char *user, const char *pass, const char *baseTopic);
