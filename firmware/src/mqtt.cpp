#include "mqtt.h"

#include <WiFi.h>
#include "../include/wifi_secure_shim.h"
#include <PubSubClient.h>
#include <Preferences.h>

#include "state_machine.h"
#include "sensors.h"
#include <nvs_flash.h>

#ifndef WIFI_SSID
#define WIFI_SSID "Meow Meow 5"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "meowmeow5"
#endif

#ifndef MQTT_HOST
#define MQTT_HOST "192.168.1.27"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "esp32-thermostat"
#endif

#ifndef MQTT_BASE_TOPIC
#define MQTT_BASE_TOPIC "thermostat"
#endif

#ifndef MQTT_USER
#define MQTT_USER ""
#endif

#ifndef MQTT_PASS
#define MQTT_PASS ""
#endif

#ifndef MQTT_USE_TLS
#define MQTT_USE_TLS 0
#endif
namespace
{
    WiFiClient s_wifiClient;
    PubSubClient s_mqttClient(s_wifiClient);

    static bool s_isConnectingNow = false;
    unsigned long s_lastPublishMs = 0;
    const unsigned long PUBLISH_INTERVAL_MS = 3000;
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
        static String cachedBase = "";
        static bool loaded = false;

        if (!loaded)
        {
            String host, user, pass, base;
            uint16_t port = 0;
            if (mqttLoadConfig(host, port, user, pass, base) && base.length() > 0)
            {
                cachedBase = base;
            }
            else
            {
                cachedBase = MQTT_BASE_TOPIC;
            }
            loaded = true;
        }
        return cachedBase.c_str();
    }

    bool mqttConnectWithConfig()
    {
        String host;
        uint16_t port;
        String user;
        String pass;
        String base;

        // اگر در تنظیمات حافظه چیزی بود، از آن استفاده کن
        if (mqttLoadConfig(host, port, user, pass, base) && host.length() > 0)
        {
            s_mqttClient.setServer(host.c_str(), port);
            return s_mqttClient.connect(MQTT_CLIENT_ID,
                                        user.length() ? user.c_str() : nullptr,
                                        pass.length() ? pass.c_str() : nullptr,
                                        (String(base) + "/telemetry/status").c_str(), 1, true, "offline");
        }

        // در غیر این صورت، از مقادیر پیش‌فرض (لپ‌تاپ خودت) استفاده کن
        Serial.println("[MQTT] Using Default Local Config...");
        s_mqttClient.setServer(MQTT_HOST, MQTT_PORT);
        return s_mqttClient.connect(MQTT_CLIENT_ID, nullptr, nullptr,
                                    (String(MQTT_BASE_TOPIC) + "/telemetry/status").c_str(), 1, true, "offline");
    }

    // داخل namespace در بخش wifiEnsureConnected:
    bool wifiEnsureConnected()
    {
        wl_status_t status = WiFi.status();

        if (status == WL_CONNECTED)
        {
            static bool ipPrinted = false;
            if (!ipPrinted)
            {
                Serial.print("WiFi: Connected! IP address: ");
                Serial.println(WiFi.localIP());
                ipPrinted = true;
            }
            return true;
        }

        static unsigned long lastAttemptTime = 0;
        static bool isConnecting = false;

        if (isConnecting)
        {
            if (status == WL_DISCONNECTED || status == WL_IDLE_STATUS)
            {
                if (millis() - lastAttemptTime < 15000)
                    return false;
                else
                {
                    Serial.println("WiFi: Connection timeout.");
                    isConnecting = false;
                }
            }
            else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL)
            {
                isConnecting = false;
            }
        }

        if (millis() - lastAttemptTime < 10000)
            return false;

        String ssid, pass;
        if (!wifiLoadCredentials(ssid, pass))
            return false;

        Serial.printf("WiFi: Connecting to %s...\n", ssid.c_str());

        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        lastAttemptTime = millis();
        isConnecting = true;
        // ریست کردن فلگ چاپ IP برای اتصال جدید
        // ipPrinted = false;

        return false;
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
            if (s_mqttClient.publish(topic, payload.c_str(), true))
            {
                Serial.printf("[MQTT] Sent: [%s] -> %s\n", topic, payload.c_str());
            }
            else
            {
                Serial.printf("[MQTT] Failed to publish to %s\n", topic);
            }
        }
    }

    void publishState()
    {
        // ۱. متغیرهای استاتیک برای حفظ وضعیت قبلی (برای تشخیص تغییرات)
        static bool s_firstPublish = true;
        static float s_lastTempC = -999.0f;
        static float s_lastHumidity = -999.0f;
        static ThermostatState s_lastState = ThermostatState::OFF;
        static float s_lastSetPointC = -999.0f;
        static float s_lastHysteresisC = -999.0f;
        static bool s_lastPumpDesired = false;
        static bool s_lastSensorOk = false;

        // ۲. کش کردن Base Topic برای جلوگیری از باز کردن مکرر NVS و ری‌بوت شدن
        static String cachedBaseTopic = "";
        if (cachedBaseTopic == "")
        {
            cachedBaseTopic = String(activeBaseTopic());
        }

        // ۳. زمان‌بندی برای ارسال اجباری (Heartbeat) - هر ۵ دقیقه یکبار
        static unsigned long s_lastForcePublishMs = 0;
        const unsigned long FORCE_PUBLISH_INTERVAL = 300000;

        // اگر دیسکانکت هستیم، فلگ اولین انتشار را ست کن و خارج شو
        if (!s_mqttClient.connected())
        {
            s_firstPublish = true;
            return;
        }

        const float kFloatEps = 0.01f;
        unsigned long now = millis();

        // شرط ارسال: یا اولین بار است، یا ۵ دقیقه گذشته، یا مقادیر تغییر کرده‌اند
        bool force = s_firstPublish || (now - s_lastForcePublishMs >= FORCE_PUBLISH_INTERVAL);

        // شروع چک کردن و ارسال پارامترها

        // دمای فعلی
        if (force || fabsf(gCurrentTempC - s_lastTempC) > kFloatEps)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/temp_c").c_str(), String(gCurrentTempC, 2));
            s_lastTempC = gCurrentTempC;
        }

        // رطوبت
        if (force || fabsf(gCurrentHumidity - s_lastHumidity) > kFloatEps)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/humidity").c_str(), String(gCurrentHumidity, 2));
            s_lastHumidity = gCurrentHumidity;
        }

        // وضعیت ماشین حالت
        if (force || gCurrentState != s_lastState)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/state").c_str(), stateToString(gCurrentState));
            s_lastState = gCurrentState;
        }

        // نقطه هدف (SetPoint)
        if (force || fabsf(gSetPointC - s_lastSetPointC) > kFloatEps)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/setpoint_c").c_str(), String(gSetPointC, 2));
            s_lastSetPointC = gSetPointC;
        }

        // هیسترزیس
        if (force || fabsf(gHysteresisC - s_lastHysteresisC) > kFloatEps)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/hysteresis_c").c_str(), String(gHysteresisC, 2));
            s_lastHysteresisC = gHysteresisC;
        }

        // وضعیت پمپ
        if (force || gPumpDesired != s_lastPumpDesired)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/pump_desired").c_str(), gPumpDesired ? "1" : "0");
            s_lastPumpDesired = gPumpDesired;
        }

        // وضعیت سلامت سنسور
        bool sensorOk = sensorsAvailable();
        if (force || sensorOk != s_lastSensorOk)
        {
            mqttPublish((cachedBaseTopic + "/telemetry/sensor_ok").c_str(), sensorOk ? "1" : "0");
            s_lastSensorOk = sensorOk;
        }

        // آپدیت زمان آخرین ارسال اجباری و اتمام وضعیت شروع
        if (force)
        {
            s_lastForcePublishMs = now;
        }
        s_firstPublish = false;
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
            else if (payload == "1" || payload == "on" || payload == "true")
                requestPowerOn();
        }
    }

    void mqttCallback(char *topic, byte *payload, unsigned int length)
    {
        String p;
        for (unsigned int i = 0; i < length; ++i)
            p += (char)payload[i];

        // لاگ طلایی برای پیام‌های ورودی
        Serial.printf("\n[MQTT] Message Arrived! \nTopic: [%s] \nPayload: %s\n", topic, p.c_str());

        handleCommand(String(topic), p);
    }

    // این را بالای تابع mqttEnsureConnected تعریف کنید
    static unsigned long s_wifiStableTimer = 0;

    bool mqttEnsureConnected()
    {
        if (s_mqttClient.connected())
            return true;

        if (WiFi.status() != WL_CONNECTED)
        {
            s_wifiStableTimer = 0; // ریست کردن تایمر
            return false;
        }

        // اگر تازه وصل شده، ۵ ثانیه کامل صبر کن (برای اطمینان از پایداری سوکت‌ها)
        if (s_wifiStableTimer == 0)
        {
            s_wifiStableTimer = millis();
            return false;
        }

        if (millis() - s_wifiStableTimer < 5000)
            return false;

        if (millis() < s_nextReconnectMs)
            return false;

        Serial.printf("[MQTT] Connecting to %s...\n", MQTT_HOST);

        // ساخت کلاینت آیدی رندوم برای جلوگیری از تداخل
        String clientId = "ESP32_Thermostat_" + String(random(1000, 9999));

        // استفاده از مقادیر ثابت برای تست اولیه
        if (s_mqttClient.connect(clientId.c_str()))
        {
            Serial.println("[MQTT] Connected! ✅");
            s_mqttClient.subscribe("thermostat/cmd/#");
            return true;
        }
        else
        {
            Serial.printf("[MQTT] Failed, rc=%d\n", s_mqttClient.state());
            s_nextReconnectMs = millis() + 5000;
            return false;
        }
    }

}

void mqttInit()
{
    WiFi.mode(WIFI_STA);
    s_mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    s_mqttClient.setCallback(mqttCallback);
    s_mqttClient.setBufferSize(512);

    xTaskCreatePinnedToCore(
        [](void *p)
        {
            for (;;)
            {
                // شرط بسیار سخت‌گیرانه برای شروع: وضعیت متصل + داشتن IP
                if (WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0)
                {
                    mqttUpdate();
                }
                else
                {
                    if (s_mqttClient.connected())
                        s_mqttClient.disconnect();
                }
                // افزایش زمان تاخیر برای پایداری بیشتر
                vTaskDelay(pdMS_TO_TICKS(150));
            }
        },
        "MqttTask", 10240, NULL, 1, NULL, 1); // تغییر هسته از 0 به 1

    Serial.println("[MQTT] Task Started on Core 1");
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
    yield();

    unsigned long now = millis();
    if (now - s_lastPublishMs >= PUBLISH_INTERVAL_MS)
    {
        s_lastPublishMs = now;
        publishState();
    }
}
