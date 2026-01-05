#include "mqtt.h"

#include <WiFi.h>
#include "../include/wifi_secure_shim.h"
#include <PubSubClient.h>
#include <Preferences.h>

#include "state_machine.h"
#include "sensors.h"

#ifndef WIFI_SSID
#define WIFI_SSID "Meow Meow 5"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "meowmeow5"
#endif

#ifndef MQTT_HOST
#define MQTT_HOST "25e714c4c7914708950ce3a88dc1dd1a.s1.eu.hivemq.cloud"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "esp32-thermostat"
#endif

#ifndef MQTT_BASE_TOPIC
#define MQTT_BASE_TOPIC "thermostat"
#endif

#ifndef MQTT_USER
#define MQTT_USER "mahya"
#endif

#ifndef MQTT_PASS
#define MQTT_PASS "vikrap-weqrug-xabVe6"
#endif

#ifndef MQTT_USE_TLS
#define MQTT_USE_TLS 1
#endif
namespace
{
    WiFiClient s_wifiClient;
#if MQTT_USE_TLS
    WiFiClientSecure s_tlsClient;
    PubSubClient s_mqttClient(s_tlsClient);
#else
    PubSubClient s_mqttClient(s_wifiClient);
#endif

    unsigned long s_lastPublishMs = 0;
    const unsigned long PUBLISH_INTERVAL_MS = 1000;
    unsigned long s_nextReconnectMs = 0;
    unsigned long s_reconnectDelayMs = 1000;

    bool wifiLoadCredentials(String &ssid, String &pass)
    {
        Preferences prefs;
        if (!prefs.begin("wifi", true))
            return false;
        ssid = prefs.getString("ssid", "");
        pass = prefs.getString("pass", "");
        prefs.end();
        return ssid.length() > 0;
    }

    bool mqttLoadConfig(String &host, uint16_t &port, String &user, String &pass, String &baseTopic)
    {
        Preferences prefs;
        if (!prefs.begin("mqtt", true))
            return false;
        host = prefs.getString("host", "");
        port = (uint16_t)prefs.getUInt("port", 0);
        user = prefs.getString("user", "");
        pass = prefs.getString("pass", "");
        baseTopic = prefs.getString("base", "");
        prefs.end();
        return host.length() > 0 && port != 0;
    }

    const char *activeBaseTopic()
    {
        static String base;
        String host;
        String user;
        String pass;
        uint16_t port = 0;
        if (mqttLoadConfig(host, port, user, pass, base) && base.length() > 0)
            return base.c_str();
        return MQTT_BASE_TOPIC;
    }

    bool mqttConnectWithConfig()
    {
        String host;
        String user;
        String pass;
        String base;
        uint16_t port = 0;

        if (mqttLoadConfig(host, port, user, pass, base))
        {
            s_mqttClient.setServer(host.c_str(), port);
            const char *user_c = user.length() ? user.c_str() : nullptr;
            const char *pass_c = pass.length() ? pass.c_str() : nullptr;
            return s_mqttClient.connect(MQTT_CLIENT_ID, user_c, pass_c, (String(activeBaseTopic()) + "/telemetry/status").c_str(), 1, true, "offline");
        }

        return s_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER[0] ? MQTT_USER : nullptr, MQTT_PASS[0] ? MQTT_PASS : nullptr,
                                    (String(MQTT_BASE_TOPIC) + "/telemetry/status").c_str(), 1, true, "offline");
    }

    bool wifiEnsureConnected()
    {
        if (WiFi.status() == WL_CONNECTED)
            return true;

        String ssid;
        String pass;
        if (!wifiLoadCredentials(ssid, pass))
            return false;

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - start) < 5000)
        {
            delay(100);
        }
        return WiFi.status() == WL_CONNECTED;
    }

    const char *stateToString(ThermostatState state)
    {
        switch (state)
        {
        case ThermostatState::IDLE:
            return "idle";
        case ThermostatState::PRESTART:
            return "prestart";
        case ThermostatState::COOLING_LOW:
            return "cooling_low";
        case ThermostatState::COOLING_HIGH:
            return "cooling_high";
        case ThermostatState::FAN_ONLY:
            return "fan_only";
        case ThermostatState::OFF:
            return "off";
        default:
            return "unknown";
        }
    }

    void mqttPublish(const char *topic, const String &payload)
    {
        if (s_mqttClient.connected())
        {
            s_mqttClient.publish(topic, payload.c_str(), true);
        }
    }

    void publishState()
    {
        String base = String(activeBaseTopic());
        mqttPublish((base + "/telemetry/temp_c").c_str(), String(gCurrentTempC, 2));
        mqttPublish((base + "/telemetry/humidity").c_str(), String(gCurrentHumidity, 2));
        mqttPublish((base + "/telemetry/state").c_str(), stateToString(gCurrentState));
        mqttPublish((base + "/telemetry/setpoint_c").c_str(), String(gSetPointC, 2));
        mqttPublish((base + "/telemetry/hysteresis_c").c_str(), String(gHysteresisC, 2));
        mqttPublish((base + "/telemetry/pump_desired").c_str(), gPumpDesired ? "1" : "0");
        mqttPublish((base + "/telemetry/sensor_ok").c_str(), sensorsAvailable() ? "1" : "0");
    }

    void handleCommand(const String &topic, const String &payload)
    {
        String base = String(activeBaseTopic()) + "/cmd/";
        if (!topic.startsWith(base))
            return;

        String cmd = topic.substring(base.length());
        if (cmd == "setpoint")
        {
            setSetPoint(payload.toFloat());
        }
        else if (cmd == "hysteresis")
        {
            setHysteresis(payload.toFloat());
        }
        else if (cmd == "fan_only")
        {
            if (payload == "1" || payload == "true" || payload == "on")
                requestFanOnly();
        }
        else if (cmd == "power")
        {
            if (payload == "0" || payload == "off" || payload == "false")
                requestPowerOff();
        }
    }

    void mqttCallback(char *topic, byte *payload, unsigned int length)
    {
        String t = String(topic);
        String p;
        p.reserve(length);
        for (unsigned int i = 0; i < length; ++i)
            p += (char)payload[i];
        handleCommand(t, p);
    }

    bool mqttEnsureConnected()
    {
        if (s_mqttClient.connected())
            return true;

        if (!wifiEnsureConnected())
            return false;

        s_mqttClient.setServer(MQTT_HOST, MQTT_PORT);
        s_mqttClient.setCallback(mqttCallback);
        if (millis() < s_nextReconnectMs)
            return false;

        if (mqttConnectWithConfig())
        {
            String base = String(activeBaseTopic()) + "/cmd/#";
            s_mqttClient.subscribe(base.c_str());
            s_mqttClient.publish((String(activeBaseTopic()) + "/telemetry/status").c_str(), "online", true);
            s_reconnectDelayMs = 1000;
            return true;
        }

        s_nextReconnectMs = millis() + s_reconnectDelayMs;
        if (s_reconnectDelayMs < 30000)
            s_reconnectDelayMs *= 2;
        return false;
    }
}

void mqttInit()
{
    WiFi.mode(WIFI_STA);
#if MQTT_USE_TLS
    s_tlsClient.setInsecure();
#endif
    s_mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    s_mqttClient.setCallback(mqttCallback);
    wifiEnsureConnected();
}

void mqttSaveConfig(const char *host, uint16_t port, const char *user, const char *pass, const char *baseTopic)
{
    Preferences prefs;
    if (!prefs.begin("mqtt", false))
        return;
    if (host)
        prefs.putString("host", host);
    if (port)
        prefs.putUInt("port", port);
    if (user)
        prefs.putString("user", user);
    if (pass)
        prefs.putString("pass", pass);
    if (baseTopic)
        prefs.putString("base", baseTopic);
    prefs.end();
}

void mqttUpdate()
{
    if (!mqttEnsureConnected())
        return;

    s_mqttClient.loop();

    unsigned long now = millis();
    if (now - s_lastPublishMs >= PUBLISH_INTERVAL_MS)
    {
        s_lastPublishMs = now;
        publishState();
    }
}
