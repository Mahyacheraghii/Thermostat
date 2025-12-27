#include "ui.h"
#include <lvgl.h>
#include <WiFi.h>
#include "GUI.h"
#include "ui_callbacks.h"
#include "state_machine.h"

static void uiUpdateLabels()
{
    const float minSetpoint = 15.0f;
    const float maxSetpoint = 30.0f;

    if (GUI_Label__screen__currentTemperature)
    {
        char buf[8];
        if (!isnan(gCurrentTempC))
            snprintf(buf, sizeof(buf), "%.1f°", gCurrentTempC);
        else
            snprintf(buf, sizeof(buf), "--.-°");
        lv_label_set_text(GUI_Label__screen__currentTemperature, buf);
    }

    if (GUI_Label__screen__moisture)
    {
        char buf[8];
        if (!isnan(gCurrentHumidity))
            snprintf(buf, sizeof(buf), "%.0f%%", gCurrentHumidity);
        else
            snprintf(buf, sizeof(buf), "--%");
        lv_label_set_text(GUI_Label__screen__moisture, buf);
    }

    if (GUI_Label__screen__setTemperature)
    {
        char buf[8];
        if (!isnan(gSetPointC))
            snprintf(buf, sizeof(buf), "%.1f°", gSetPointC);
        else
            snprintf(buf, sizeof(buf), "--.-°");
        lv_label_set_text(GUI_Label__screen__setTemperature, buf);
    }

    if (GUI_Arc__screen__arc && !isnan(gSetPointC))
    {
        int val = (int)gSetPointC;
        if (val < (int)minSetpoint)
            val = (int)minSetpoint;
        if (val > (int)maxSetpoint)
            val = (int)maxSetpoint;
        lv_arc_set_range(GUI_Arc__screen__arc, (int)minSetpoint, (int)maxSetpoint);
        lv_arc_set_value(GUI_Arc__screen__arc, val);
    }

    if (GUI_Image__screen__fanImg)
    {
        if (gCurrentState == ThermostatState::FAN_ONLY)
            lv_obj_add_state(GUI_Image__screen__fanImg, LV_STATE_CHECKED);
        else
            lv_obj_clear_state(GUI_Image__screen__fanImg, LV_STATE_CHECKED);
    }

    if (GUI_Image__screen__fanImg)
    {
        if (gCurrentState == ThermostatState::FAN_ONLY)
            lv_obj_set_style_opa(GUI_Image__screen__fanImg, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
        else
            lv_obj_set_style_opa(GUI_Image__screen__fanImg, LV_OPA_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void uiInit()
{
    // LVGL UI callbacks are in `ui_callbacks.*` and should be attached by
    // the generated GUI code (SquareLine). Nothing else to do here for now.
}

static void uiUpdateWifiIcon(wl_status_t status)
{
    if (!GUI_Image__screen__wifiImage)
        return;

    lv_color_t color = lv_color_hex(0x94a3b8);
    uint8_t recolor_opa = 120;

    if (status == WL_CONNECTED)
    {
        color = lv_color_hex(0x22c55e);
        int rssi = WiFi.RSSI();
        if (rssi > -60)
            recolor_opa = 255; // full
        else if (rssi > -70)
            recolor_opa = 200; // medium
        else if (rssi > -80)
            recolor_opa = 140; // low
        else
            recolor_opa = 90; // weak
    }
    else if (status == WL_IDLE_STATUS)
    {
        color = lv_color_hex(0xf59e0b); // connecting
        recolor_opa = 180;
    }
    else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL)
    {
        color = lv_color_hex(0xef4444); // error
        recolor_opa = 220;
    }

    lv_obj_set_style_img_recolor(GUI_Image__screen__wifiImage, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(GUI_Image__screen__wifiImage, recolor_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void uiUpdate()
{
    // Drive LVGL timers/handlers. Do not manipulate state directly here; the
    // GUI callbacks call the state-machine API which sets flags.
    // v8/v9 compatibility: ensure the lvgl header is included. Call the
    // timer handler to run LVGL timers and tasks.
    lv_timer_handler();

    static uint32_t lastUiUpdateMs = 0;
    static uint32_t lastWifiCheckMs = 0;
    const uint32_t now = millis();

    if (now - lastUiUpdateMs >= 500)
    {
        lastUiUpdateMs = now;
        uiUpdateLabels();
    }

    if (now - lastWifiCheckMs >= 1000)
    {
        lastWifiCheckMs = now;
        wl_status_t status = WiFi.status();
        if (GUI_Image__screen__wifiImage)
            lv_obj_clear_flag(GUI_Image__screen__wifiImage, LV_OBJ_FLAG_HIDDEN);

        uiUpdateWifiIcon(status);

        if (GUI_Screen__wifi && lv_scr_act() == GUI_Screen__wifi && GUI_Label__wifi__status)
        {
            if (ui_wifi_connect_in_progress() && status != WL_CONNECTED && ui_wifi_connect_elapsed_ms() > 10000)
            {
                lv_label_set_text(GUI_Label__wifi__status, "Status: timeout");
                WiFi.disconnect(true);
                ui_wifi_connect_finished();
                return;
            }
            switch (status)
            {
            case WL_CONNECTED:
                lv_label_set_text(GUI_Label__wifi__status, "Status: connected");
                ui_wifi_connect_finished();
                break;
            case WL_CONNECT_FAILED:
                lv_label_set_text(GUI_Label__wifi__status, "Status: failed");
                ui_wifi_connect_finished();
                break;
            case WL_NO_SSID_AVAIL:
                lv_label_set_text(GUI_Label__wifi__status, "Status: no SSID");
                ui_wifi_connect_finished();
                break;
            case WL_IDLE_STATUS:
                lv_label_set_text(GUI_Label__wifi__status, "Status: connecting...");
                break;
            default:
                lv_label_set_text(GUI_Label__wifi__status, "Status: disconnected");
                break;
            }
        }
    }
}
