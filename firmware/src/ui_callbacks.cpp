#include "ui_callbacks.h"
#include "state_machine.h"
#include "touch.h"
#include "GUI.h"
#include <WiFi.h>
#include <Preferences.h>
#include <cstring>
#include "mqtt.h"

static void wifiSetStatus(const char *text)
{
    if (GUI_Label__wifi__status)
        lv_label_set_text(GUI_Label__wifi__status, text);
}

static void wifiSaveCredentials(const char *ssid, const char *pass)
{
    Preferences prefs;
    if (!prefs.begin("wifi", false))
        return;
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

static void wifiClearCredentials()
{
    Preferences prefs;
    if (!prefs.begin("wifi", false))
        return;
    prefs.clear();
    prefs.end();
}

static void wifiLoadCredentials(String &ssid, String &pass)
{
    Preferences prefs;
    if (!prefs.begin("wifi", true))
        return;
    ssid = prefs.getString("ssid", "");
    pass = prefs.getString("pass", "");
    prefs.end();
}

static uint32_t s_wifiConnectStartMs = 0;
static bool s_wifiConnectInProgress = false;

bool ui_wifi_connect_in_progress()
{
    return s_wifiConnectInProgress;
}

uint32_t ui_wifi_connect_elapsed_ms()
{
    if (!s_wifiConnectInProgress)
        return 0;
    return millis() - s_wifiConnectStartMs;
}

void ui_wifi_connect_finished()
{
    s_wifiConnectInProgress = false;
    s_wifiConnectStartMs = 0;
}
// Slider or similar widget changed value -> update setpoint flag
void ui_on_setpoint_changed(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (!obj)
        return;

    if (obj == GUI_Arc__screen__arc)
    {
        int val = lv_arc_get_value(GUI_Arc__screen__arc);
        setSetPoint((float)val);
        if (GUI_Label__screen__setTemperature)
        {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", val);
            lv_label_set_text(GUI_Label__screen__setTemperature, buf);
        }
        return;
    }

    // Try slider first
    if (lv_obj_has_class(obj, &lv_slider_class))
    {
        int val = lv_slider_get_value(obj);
        setSetPoint((float)val);
        return;
    }

    // Fallback: read numeric label or text if present
    lv_obj_t *child = (lv_obj_t *)lv_obj_get_child(obj, 0);
    if (child)
    {
        const char *txt = lv_label_get_text(child);
        if (txt)
        {
            float v = atof(txt);
            setSetPoint(v);
        }
    }
}

// Power-off request button
void ui_on_power_button_pressed(lv_event_t *e)
{
    Serial.println("Power Button Clicked!"); // اضافه کردن لاگ برای تست لمس

    if (gCurrentState == ThermostatState::OFF)
    {
        Serial.println("Action: Requesting ON");
        requestPowerOn();
    }
    else
    {
        Serial.println("Action: Requesting OFF");
        requestPowerOff();
    }
}

// Calibration button pressed in UI -> run touch calibration routine.
void ui_on_calibrate_touch_pressed(lv_event_t *e)
{
    (void)e;
    Serial.println("UI: calibration requested");
    touchCalibrateStart();
}

void ui_on_mqtt_save_pressed(lv_event_t *e)
{
    (void)e;
    mqttSaveConfig(MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS, MQTT_BASE_TOPIC);
    Serial.println("UI: MQTT config saved");
}

void ui_on_wifi_icon_pressed(lv_event_t *e)
{
    (void)e;
    if (GUI_Screen__wifi)
    {
        lv_screen_load(GUI_Screen__wifi);
        if (GUI_Keyboard__wifi__keyboard)
            lv_obj_add_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);

        if (GUI_TextArea__wifi__ssid && GUI_TextArea__wifi__pass)
        {
            String ssid;
            String pass;
            wifiLoadCredentials(ssid, pass);
            if (ssid.length() == 0)
                ssid = WIFI_SSID;
            if (pass.length() == 0)
                pass = WIFI_PASSWORD;
            lv_textarea_set_text(GUI_TextArea__wifi__ssid, ssid.c_str());
            lv_textarea_set_text(GUI_TextArea__wifi__pass, pass.c_str());
        }
    }
}

void ui_on_wifi_back_pressed(lv_event_t *e)
{
    (void)e;
    if (GUI_Screen__screen)
        lv_screen_load(GUI_Screen__screen);
    if (GUI_Keyboard__wifi__keyboard)
        lv_obj_add_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);
}

void ui_on_wifi_connect_pressed(lv_event_t *e)
{
    (void)e;
    if (!GUI_TextArea__wifi__ssid || !GUI_TextArea__wifi__pass)
        return;

    const char *ssid = lv_textarea_get_text(GUI_TextArea__wifi__ssid);
    const char *pass = lv_textarea_get_text(GUI_TextArea__wifi__pass);
    if (!ssid || ssid[0] == '\0')
    {
        wifiSetStatus("Status: enter SSID");
        return;
    }

    if (GUI_Keyboard__wifi__keyboard)
        lv_obj_add_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);

    Serial.printf("UI: WiFi connect requested (ssid=\"%s\", pass_len=%u)\n",
                  ssid, pass ? (unsigned)strlen(pass) : 0U);
    wifiSaveCredentials(ssid, pass ? pass : "");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.begin(ssid, pass);
    wifiSetStatus("Status: connecting...");
    s_wifiConnectStartMs = millis();
    s_wifiConnectInProgress = true;
}

void ui_on_wifi_clear_pressed(lv_event_t *e)
{
    (void)e;
    wifiClearCredentials();
    if (GUI_TextArea__wifi__ssid)
        lv_textarea_set_text(GUI_TextArea__wifi__ssid, "");
    if (GUI_TextArea__wifi__pass)
        lv_textarea_set_text(GUI_TextArea__wifi__pass, "");
    WiFi.disconnect(true);
    ui_wifi_connect_finished();
    wifiSetStatus("Status: cleared");
}

void ui_on_wifi_ssid_focused(lv_event_t *e)
{
    (void)e;
    if (!GUI_Keyboard__wifi__keyboard || !GUI_TextArea__wifi__ssid)
        return;
    lv_keyboard_set_textarea(GUI_Keyboard__wifi__keyboard, GUI_TextArea__wifi__ssid);
    lv_obj_clear_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);
}

void ui_on_wifi_pass_focused(lv_event_t *e)
{
    (void)e;
    if (!GUI_Keyboard__wifi__keyboard || !GUI_TextArea__wifi__pass)
        return;
    lv_keyboard_set_textarea(GUI_Keyboard__wifi__keyboard, GUI_TextArea__wifi__pass);
    lv_obj_clear_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);
}

void ui_on_wifi_textarea_defocused(lv_event_t *e)
{
    (void)e;
    if (!GUI_Keyboard__wifi__keyboard)
        return;
    lv_keyboard_set_textarea(GUI_Keyboard__wifi__keyboard, NULL);
    lv_obj_add_flag(GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN);
}

void ui_on_wifi_keyboard_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_READY && code != LV_EVENT_CANCEL)
        return;
    ui_on_wifi_textarea_defocused(e);
}
