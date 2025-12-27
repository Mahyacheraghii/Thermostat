#pragma once

#include <Arduino.h>

#define TOUCH_TYPE_XPT2046 1
#define TOUCH_TYPE_FT6X36 2

#ifndef TOUCH_TYPE
#define TOUCH_TYPE TOUCH_TYPE_XPT2046
#endif

#include <lvgl.h>

// Expose touch init/update and the LVGL read callback used by `lv_indev_set_read_cb`
void touchInit();
void touchUpdate();
void touchLvglRead(lv_indev_t *indev, lv_indev_data_t *data);
// Start a non-blocking touch calibration (XPT2046 only). This collects two
// corner samples and computes a linear scale/offset mapping used by touchUpdate().
// Calibration results are stored in NVS.
void touchCalibrateStart();
bool touchCalibrationIsActive();
