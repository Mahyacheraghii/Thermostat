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
        kScreenBufferPixels = kMaxScreenWidth * kMaxScreenHeight / 30
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
        if (gFanSpeed != FanSpeed::OFF)
            lv_obj_add_state(GUI_Image__screen__fanImg, LV_STATE_CHECKED);
        else
            lv_obj_clear_state(GUI_Image__screen__fanImg, LV_STATE_CHECKED);
    }

    if (GUI_Image__screen__fanImg)
    {
        if (gFanSpeed == FanSpeed::FAST)
            lv_obj_set_style_opa(GUI_Image__screen__fanImg, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
        else if (gFanSpeed == FanSpeed::SLOW)
            lv_obj_set_style_opa(GUI_Image__screen__fanImg, LV_OPA_70, LV_PART_MAIN | LV_STATE_DEFAULT);
        else
            lv_obj_set_style_opa(GUI_Image__screen__fanImg, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
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
}

static void uiUpdateWifiIcon(wl_status_t status)
{
    if (!GUI_Image__screen__wifiImage)
        return;

    uint8_t icon_opa = 140;

    if (status == WL_CONNECTED)
    {
        int rssi = WiFi.RSSI();
        if (rssi > -60)
            icon_opa = 255; // full
        else if (rssi > -70)
            icon_opa = 200; // medium
        else if (rssi > -80)
            icon_opa = 160; // low
        else
            icon_opa = 120; // weak
    }
    else if (status == WL_IDLE_STATUS)
    {
        icon_opa = 180; // connecting
    }
    else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL)
    {
        icon_opa = 120; // error
    }

    lv_obj_set_style_img_recolor_opa(GUI_Image__screen__wifiImage, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(GUI_Image__screen__wifiImage, icon_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
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
