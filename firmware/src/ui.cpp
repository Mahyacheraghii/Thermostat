#include "ui.h"
#include <lvgl.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "GUI.h"
#include "ui_callbacks.h"
#include "state_machine.h"
#include "touch.h"

namespace
{
    static const uint16_t kMaxScreenWidth = 320;
    static const uint16_t kMaxScreenHeight = 240;
    enum
    {
        kScreenBufferPixels = kMaxScreenWidth * kMaxScreenHeight / 20
    };
    static lv_color_t s_buf[kScreenBufferPixels];
    static TFT_eSPI s_tft = TFT_eSPI();

    void uiDispFlush(lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
    {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        s_tft.startWrite();
        s_tft.setAddrWindow(area->x1, area->y1, w, h);
        s_tft.pushPixels(pixelmap, w * h);
        s_tft.endWrite();

        lv_disp_flush_ready(disp);
    }

    uint32_t uiTickGetCb()
    {
        return millis();
    }
}

static void uiUpdateLabels()
{
    const float minSetpoint = 15.0f;
    const float maxSetpoint = 30.0f;

    if (GUI_Label__screen__currentTemperature)
    {
        char buf[24];
        if (!isnan(gCurrentTempC))
            snprintf(buf, sizeof(buf), "current Temp: %.1f", gCurrentTempC);
        else
            snprintf(buf, sizeof(buf), "current Temp: --");
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
            snprintf(buf, sizeof(buf), "%d", (int)gSetPointC);
        else
            snprintf(buf, sizeof(buf), "--");
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
        const lv_image_dsc_t *fanIcon = &fan_off;
        if (gCurrentState == ThermostatState::COOLING_HIGH)
            fanIcon = &fan_fast;
        else if (gCurrentState == ThermostatState::COOLING_LOW || gCurrentState == ThermostatState::FAN_ONLY)
            fanIcon = &fan_slow;
        lv_image_set_src(GUI_Image__screen__fanImg, fanIcon);
    }

    if (GUI_Image__screen__pumpImg)
    {
        lv_image_set_src(GUI_Image__screen__pumpImg, gPumpDesired ? &pump_on : &pump_off);
    }

    if (GUI_Image__screen__powerImg)
    {
        const bool powerOff = (gCurrentState == ThermostatState::OFF);
        lv_image_set_src(GUI_Image__screen__powerImg, powerOff ? &power_off : &power_on);
    }
}

void uiInit()
{
    lv_init();

    s_tft.begin();
    s_tft.setRotation(3);
    s_tft.setSwapBytes(true);

    const uint16_t screenWidth = s_tft.width();
    const uint16_t screenHeight = s_tft.height();
    lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_buffers(disp, s_buf, NULL, kScreenBufferPixels * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, uiDispFlush);

    lv_tick_set_cb(uiTickGetCb);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchLvglRead);

    GUI_loadContent();

    if (GUI_Image__screen__moodImg)
        lv_obj_add_flag(GUI_Image__screen__moodImg, LV_OBJ_FLAG_HIDDEN);
}

TFT_eSPI &uiGetTft()
{
    return s_tft;
}

static void uiUpdateWifiIcon(wl_status_t status)
{
    if (!GUI_Image__screen__wifiImage)
        return;

    const lv_image_dsc_t *wifiIcon = &wifi_low;
    if (status == WL_CONNECTED)
    {
        int rssi = WiFi.RSSI();
        if (rssi > -60)
            wifiIcon = &wifi_full;
        else if (rssi > -70)
            wifiIcon = &wifi_midum;
        else
            wifiIcon = &wifi_low;
    }
    lv_image_set_src(GUI_Image__screen__wifiImage, wifiIcon);
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

    if (now - lastUiUpdateMs >= 1)
    {
        lastUiUpdateMs = now;
        uiUpdateLabels();
    }

    if (now - lastWifiCheckMs >= 1000)
    {
        lastWifiCheckMs = now;
        wl_status_t status = WiFi.status();
        if (GUI_Image__screen__wifiImage)
        {
            if (status == WL_CONNECTED)
            {
                lv_obj_clear_flag(GUI_Image__screen__wifiImage, LV_OBJ_FLAG_HIDDEN);
                uiUpdateWifiIcon(status);
            }
            else
            {
                lv_obj_add_flag(GUI_Image__screen__wifiImage, LV_OBJ_FLAG_HIDDEN);
            }
        }

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
