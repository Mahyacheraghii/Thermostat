#include "touch.h"

// Target display dimensions (landscape 320x240)
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// Include SPI/Wire for underlying platform APIs
#include <SPI.h>
#include <Wire.h>
#include <Preferences.h>

// Use wrapper that includes real drivers when available or provides shims
#include "touch_drivers.h"

// Pins from rules: T_CS -> GPIO15, T_IRQ -> GPIO33, SPI uses standard MOSI/MISO/SCK
#define T_CS_PIN 15
#define T_IRQ_PIN 33

#if TOUCH_TYPE == TOUCH_TYPE_XPT2046
static XPT2046_Touchscreen ts(T_CS_PIN, T_IRQ_PIN);
#elif TOUCH_TYPE == TOUCH_TYPE_FT6X36
static Adafruit_FT6206 ftTouch = Adafruit_FT6206();
#endif

// Internal state for last known touch coordinates
static volatile bool s_touched = false;
static volatile uint16_t s_touchX = 0;
static volatile uint16_t s_touchY = 0;

// NVS storage for calibration values
static Preferences s_prefs;

// Calibration factors (raw -> pixel): x' = raw * s_cal_scale_x + s_cal_off_x
static float s_cal_scale_x = 0.0f;
static float s_cal_off_x = 0.0f;
static float s_cal_scale_y = 0.0f;
static float s_cal_off_y = 0.0f;

enum CalState
{
    CAL_IDLE = 0,
    CAL_WAIT_TL_PRESS,
    CAL_WAIT_TL_RELEASE,
    CAL_WAIT_BR_PRESS,
    CAL_WAIT_BR_RELEASE
};

static volatile CalState s_cal_state = CAL_IDLE;
static TS_Point s_cal_p1;
static TS_Point s_cal_p2;

static void touchLoadCalibration()
{
    if (!s_prefs.begin("touch", true))
        return;
    bool has_cal = s_prefs.getBool("has_cal", false);
    if (has_cal)
    {
        s_cal_scale_x = s_prefs.getFloat("scale_x", 0.0f);
        s_cal_off_x = s_prefs.getFloat("off_x", 0.0f);
        s_cal_scale_y = s_prefs.getFloat("scale_y", 0.0f);
        s_cal_off_y = s_prefs.getFloat("off_y", 0.0f);
    }
    s_prefs.end();
}

static void touchSaveCalibration()
{
    if (!s_prefs.begin("touch", false))
        return;
    s_prefs.putBool("has_cal", true);
    s_prefs.putFloat("scale_x", s_cal_scale_x);
    s_prefs.putFloat("off_x", s_cal_off_x);
    s_prefs.putFloat("scale_y", s_cal_scale_y);
    s_prefs.putFloat("off_y", s_cal_off_y);
    s_prefs.end();
}

void touchInit()
{
#if TOUCH_TYPE == TOUCH_TYPE_XPT2046
    SPI.begin();
    ts.begin();
    touchLoadCalibration();
#elif TOUCH_TYPE == TOUCH_TYPE_FT6X36
    Wire.begin();
    ftTouch.begin(40); // sensitivity
#endif
}

static void touchCalSetState(CalState next)
{
    s_cal_state = next;
    if (next == CAL_WAIT_TL_PRESS)
    {
        Serial.println("Starting touch calibration.");
        Serial.println("Tap and hold the top-left corner...");
    }
    else if (next == CAL_WAIT_BR_PRESS)
    {
        Serial.println("Tap and hold the bottom-right corner...");
    }
}

static void touchCalibrationTick()
{
#if TOUCH_TYPE == TOUCH_TYPE_XPT2046
    bool touched = ts.touched();
    switch (s_cal_state)
    {
    case CAL_WAIT_TL_PRESS:
        if (touched)
        {
            s_cal_p1 = ts.getPoint();
            s_cal_state = CAL_WAIT_TL_RELEASE;
        }
        break;
    case CAL_WAIT_TL_RELEASE:
        if (!touched)
        {
            Serial.print("Raw TL: ");
            Serial.print(s_cal_p1.x);
            Serial.print(",");
            Serial.println(s_cal_p1.y);
            touchCalSetState(CAL_WAIT_BR_PRESS);
        }
        break;
    case CAL_WAIT_BR_PRESS:
        if (touched)
        {
            s_cal_p2 = ts.getPoint();
            s_cal_state = CAL_WAIT_BR_RELEASE;
        }
        break;
    case CAL_WAIT_BR_RELEASE:
        if (!touched)
        {
            Serial.print("Raw BR: ");
            Serial.print(s_cal_p2.x);
            Serial.print(",");
            Serial.println(s_cal_p2.y);

            float raw_dx = (float)(s_cal_p2.x - s_cal_p1.x);
            float raw_dy = (float)(s_cal_p2.y - s_cal_p1.y);
            if (raw_dx != 0.0f)
            {
                s_cal_scale_x = (float)(TFT_WIDTH - 1) / raw_dx;
                s_cal_off_x = -s_cal_p1.x * s_cal_scale_x;
            }
            if (raw_dy != 0.0f)
            {
                s_cal_scale_y = (float)(TFT_HEIGHT - 1) / raw_dy;
                s_cal_off_y = -s_cal_p1.y * s_cal_scale_y;
            }

            touchSaveCalibration();
            Serial.println("Calibration complete.");
            Serial.print("scale_x=");
            Serial.println(s_cal_scale_x, 6);
            Serial.print("off_x=");
            Serial.println(s_cal_off_x, 2);
            Serial.print("scale_y=");
            Serial.println(s_cal_scale_y, 6);
            Serial.print("off_y=");
            Serial.println(s_cal_off_y, 2);

            s_cal_state = CAL_IDLE;
        }
        break;
    default:
        break;
    }
#endif
}

void touchUpdate()
{
    // Poll and update cached coordinates; LVGL will call touchLvglRead to consume them
#if TOUCH_TYPE == TOUCH_TYPE_XPT2046
    if (s_cal_state != CAL_IDLE)
    {
        touchCalibrationTick();
        s_touched = false;
        return;
    }
    if (ts.touched())
    {
        TS_Point p = ts.getPoint();
        // Raw X/Y need mapping depending on wiring; try simple mapping and clip.
        // XPT library returns values in display orientation; map to 0..TFT_WIDTH/HEIGHT
        int rawx = p.x;
        int rawy = p.y;
        if (rawx < 0)
            rawx = 0;
        if (rawy < 0)
            rawy = 0;
        if (rawx > 4095)
            rawx = 4095;
        if (rawy > 4095)
            rawy = 4095;
        // If calibration is set, apply linear transform, otherwise use default map.
        if (s_cal_scale_x != 0.0f)
        {
            int32_t xx = (int32_t)(rawx * s_cal_scale_x + s_cal_off_x + 0.5f);
            if (xx < 0)
                xx = 0;
            if (xx >= TFT_WIDTH)
                xx = TFT_WIDTH - 1;
            s_touchX = (uint16_t)xx;
        }
        else
        {
            s_touchX = map(rawx, 0, 4095, 0, TFT_WIDTH - 1);
        }

        if (s_cal_scale_y != 0.0f)
        {
            int32_t yy = (int32_t)(rawy * s_cal_scale_y + s_cal_off_y + 0.5f);
            if (yy < 0)
                yy = 0;
            if (yy >= TFT_HEIGHT)
                yy = TFT_HEIGHT - 1;
            s_touchY = (uint16_t)yy;
        }
        else
        {
            s_touchY = map(rawy, 0, 4095, 0, TFT_HEIGHT - 1);
        }
        s_touched = true;
    }
    else
    {
        s_touched = false;
    }
#elif TOUCH_TYPE == TOUCH_TYPE_FT6X36
    if (ftTouch.touched())
    {
        TS_Point p = ftTouch.getPoint();
        s_touchX = p.x;
        s_touchY = p.y;
        // Adafruit FT library returns coords already in pixels for common setups
        if (s_touchX >= TFT_WIDTH)
            s_touchX = TFT_WIDTH - 1;
        if (s_touchY >= TFT_HEIGHT)
            s_touchY = TFT_HEIGHT - 1;
        s_touched = true;
    }
    else
    {
        s_touched = false;
    }
#endif
}

// Start non-blocking calibration. Calibration results are persisted to NVS.
void touchCalibrateStart()
{
#if TOUCH_TYPE == TOUCH_TYPE_XPT2046
    touchCalSetState(CAL_WAIT_TL_PRESS);
#else
    Serial.println("Touch calibration not required for this driver.");
#endif
}

bool touchCalibrationIsActive()
{
    return s_cal_state != CAL_IDLE;
}

// LVGL read callback
void touchLvglRead(lv_indev_t *indev, lv_indev_data_t *data)
{
    // Update cached sample before reading
    touchUpdate();

    if (!s_touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = s_touchX;
        data->point.y = s_touchY;
    }
}
