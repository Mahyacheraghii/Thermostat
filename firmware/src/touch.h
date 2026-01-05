#pragma once

#include <Arduino.h>

#define TOUCH_TYPE_XPT2046 1

#ifndef TOUCH_TYPE
#define TOUCH_TYPE TOUCH_TYPE_XPT2046
#endif

#include <lvgl.h>

// Expose touch init/update and the LVGL read callback used by `lv_indev_set_read_cb`
void touchInit();
void touchUpdate();
void touchLvglRead(lv_indev_t *indev, lv_indev_data_t *data);
// Start touch calibration using the TFT_eSPI helper. This blocks until the
// user completes the on-screen prompts. Calibration results are stored in NVS.
void touchCalibrateStart();
bool touchCalibrationIsActive();
