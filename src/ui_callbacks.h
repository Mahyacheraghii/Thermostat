#pragma once

#include <lvgl.h>

#ifdef __cplusplus
#include "state_machine.h"
#else
typedef int ThermostatMode;
#endif
// LVGL event callbacks the UI should call. These callbacks set flags or
// parameters only and do not change states directly.
#ifdef __cplusplus
extern "C"
{
#endif

    void ui_on_setpoint_changed(lv_event_t *e);
    void ui_on_fan_button_pressed(lv_event_t *e);
    void ui_on_power_button_pressed(lv_event_t *e);
    void ui_on_mode_change_requested(lv_event_t *e); // expects ThermostatMode* as user_data
    void ui_on_calibrate_touch_pressed(lv_event_t *e);
    void ui_on_wifi_icon_pressed(lv_event_t *e);
    void ui_on_wifi_back_pressed(lv_event_t *e);
    void ui_on_wifi_connect_pressed(lv_event_t *e);
    void ui_on_wifi_clear_pressed(lv_event_t *e);
    void ui_on_wifi_ssid_focused(lv_event_t *e);
    void ui_on_wifi_pass_focused(lv_event_t *e);
    void ui_on_mode_toggle_pressed(lv_event_t *e);
    void ui_on_fan_toggle_pressed(lv_event_t *e);
    void ui_on_mqtt_save_pressed(lv_event_t *e);

    bool ui_wifi_connect_in_progress();
    uint32_t ui_wifi_connect_elapsed_ms();
    void ui_wifi_connect_finished();

#ifdef __cplusplus
}
#endif
