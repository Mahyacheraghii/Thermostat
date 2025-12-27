#include "../GUI.h"

#ifdef GUI_EXTERNAL_CUSTOM_CALLBACK_FUNCTION_FILE
 #include GUI_EXTERNAL_CUSTOM_CALLBACK_FUNCTION_FILE
#endif

// Include UI callbacks (C-linkage) from project src
#include "../ui_callbacks.h"

void GUI_event__Arc__screen__arc__Clicked (lv_event_t* event) {
    ui_on_setpoint_changed(event);
}


void GUI_event__Image__screen__moodImg__Clicked (lv_event_t* event) {
    ui_on_mode_toggle_pressed(event);
}


void GUI_event__Image__screen__fanImg__Clicked (lv_event_t* event) {
    ui_on_fan_toggle_pressed(event);
}


void GUI_event__Image__screen__pumpImg__Clicked (lv_event_t* event) {
    ui_on_calibrate_touch_pressed(event);
}


void GUI_event__Image__screen__powerImg__Clicked (lv_event_t* event) {
    ui_on_power_button_pressed(event);
}

void GUI_event__Image__screen__wifiImage__Clicked (lv_event_t* event) {
    ui_on_wifi_icon_pressed(event);
}

void GUI_event__Button__wifi__backBtn__Clicked (lv_event_t* event) {
    ui_on_wifi_back_pressed(event);
}

void GUI_event__Button__wifi__connectBtn__Clicked (lv_event_t* event) {
    ui_on_wifi_connect_pressed(event);
}

void GUI_event__Button__wifi__clearBtn__Clicked (lv_event_t* event) {
    ui_on_wifi_clear_pressed(event);
}

void GUI_event__TextArea__wifi__ssid__Focused (lv_event_t* event) {
    ui_on_wifi_ssid_focused(event);
}

void GUI_event__TextArea__wifi__pass__Focused (lv_event_t* event) {
    ui_on_wifi_pass_focused(event);
}
