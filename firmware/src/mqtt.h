#pragma once

#include <Arduino.h>

#ifndef WIFI_SSID
#define WIFI_SSID "CHANGE_ME"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CHANGE_ME"
#endif

#ifndef MQTT_HOST
#define MQTT_HOST "192.168.1.17"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_USER
#define MQTT_USER ""
#endif

#ifndef MQTT_PASS
#define MQTT_PASS ""
#endif

#ifndef MQTT_BASE_TOPIC
#define MQTT_BASE_TOPIC "thermostat"
#endif

#ifndef MQTT_USE_TLS
#define MQTT_USE_TLS 0
#endif

void mqttInit();
void mqttUpdate();
void mqttSaveConfig(const char *host, uint16_t port, const char *user, const char *pass, const char *baseTopic);
