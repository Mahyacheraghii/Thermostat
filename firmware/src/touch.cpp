#include "touch.h"

#include <Preferences.h>

#include "ui.h"

// ---------------------------------------------------------------------------
// Pin map (ILI9341 + XPT2046, shared SPI)
// ---------------------------------------------------------------------------
static const uint16_t kTftWidth = 320;
static const uint16_t kTftHeight = 240;
static const uint8_t kTftRotation = 3;

// Touch controller pins
static const uint8_t TOUCH_CS_PIN = 5;
static const uint8_t TOUCH_IRQ_PIN = 33;

#ifndef TOUCH_DEBUG
#define TOUCH_DEBUG 0 // Enable debug output to diagnose issues
#endif

// Last known touch state shared with LVGL
static volatile bool s_touched = false;
static volatile uint16_t s_touchX = 0;
static volatile uint16_t s_touchY = 0;

// NVS storage for calibration
static Preferences s_prefs;

static const char *kTouchPrefsNs = "touch";
static const char *kTouchPrefsKeyHasCal = "has_cal";
static const char *kTouchPrefsKeyCal = "cal";

static uint16_t s_touchCalData[5] = {0, 0, 0, 0, 0};
static bool s_touchCalValid = false;

// LVGL input device handle - IMPORTANT for registration
static lv_indev_t *s_touchIndev = nullptr;

// Load calibration from NVS
static void touchLoadCalibration()
{
    if (!s_prefs.begin(kTouchPrefsNs, true))
        return;
    bool has_cal = s_prefs.getBool(kTouchPrefsKeyHasCal, false);
    if (has_cal)
    {
        size_t read = s_prefs.getBytes(kTouchPrefsKeyCal, s_touchCalData, sizeof(s_touchCalData));
        s_touchCalValid = (read == sizeof(s_touchCalData));
    }
    s_prefs.end();
}

static void touchSaveCalibration()
{
    if (!s_prefs.begin(kTouchPrefsNs, false))
        return;
    s_prefs.putBool(kTouchPrefsKeyHasCal, true);
    s_prefs.putBytes(kTouchPrefsKeyCal, s_touchCalData, sizeof(s_touchCalData));
    s_prefs.end();
}

// Clear stored calibration (call this to force recalibration)
void touchClearCalibration()
{
    s_prefs.begin(kTouchPrefsNs, false);
    s_prefs.clear();
    s_prefs.end();
    s_touchCalValid = false;
    Serial.println("Touch: Calibration cleared. Restart to recalibrate.");
}

void touchInit()
{
    // 1. Disable touch CS initially to prevent SPI conflicts
    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);

    // 2. Load existing calibration
    touchLoadCalibration();

    TFT_eSPI &tft = uiGetTft();
    tft.setRotation(kTftRotation);

    if (s_touchCalValid)
    {
        tft.setTouch(s_touchCalData);
        Serial.println("Touch: Calibration loaded from NVS.");

#if TOUCH_DEBUG
        Serial.printf("Touch Cal Data: %d, %d, %d, %d, %d\n",
                      s_touchCalData[0], s_touchCalData[1],
                      s_touchCalData[2], s_touchCalData[3],
                      s_touchCalData[4]);
#endif
    }
    else
    {
        Serial.println("Touch: Calibration needed. Follow on-screen prompts.");

        // Clear screen and show calibration UI
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, 100);
        tft.println("Touch the corners");

        delay(1000);

        tft.calibrateTouch(s_touchCalData, TFT_WHITE, TFT_BLACK, 15);
        touchSaveCalibration();
        s_touchCalValid = true;
        tft.setTouch(s_touchCalData);

        Serial.println("Touch: Calibration complete and saved.");
    }

#if TOUCH_DEBUG
    Serial.println("Touch: Initialization complete.");
#endif
}

// Register touch input device with LVGL - CALL THIS AFTER lv_init()!
void touchRegisterLvgl()
{
    s_touchIndev = lv_indev_create();
    lv_indev_set_type(s_touchIndev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_touchIndev, touchLvglRead);

    Serial.println("Touch: LVGL input device registered.");
}

void touchUpdate()
{
    TFT_eSPI &tft = uiGetTft();
    uint16_t x = 0, y = 0;

    // Lower threshold for more sensitive touch (try 300-600)
    // Adjust based on your specific touch panel
    const uint16_t TOUCH_THRESHOLD = 400;

    if (tft.getTouch(&x, &y, TOUCH_THRESHOLD))
    {
        static uint16_t lastValidX = 0;
        static uint16_t lastValidY = 0;
        static uint32_t lastTouchTime = 0;

        uint32_t now = millis();

        // Debounce: ignore touches within 30ms (reduced from 50ms for better responsiveness)
        if (now - lastTouchTime > 30)
        {
            // Clamp coordinates to screen bounds
            x = constrain(x, 0, kTftWidth - 1);
            y = constrain(y, 0, kTftHeight - 1);

            s_touchX = x;
            s_touchY = y;
            s_touched = true;
            lastTouchTime = now;

#if TOUCH_DEBUG
            // Log significant position changes
            if (abs((int)x - (int)lastValidX) > 5 || abs((int)y - (int)lastValidY) > 5)
            {
                Serial.printf("Touch: x=%d, y=%d\n", x, y);
                lastValidX = x;
                lastValidY = y;
            }
#endif
        }
    }
    else
    {
        s_touched = false;
    }
}

void touchCalibrateStart()
{
    Serial.println("Touch: Manual calibration requested.");
    TFT_eSPI &tft = uiGetTft();
    tft.setRotation(kTftRotation);
    tft.fillScreen(TFT_BLACK);
    tft.calibrateTouch(s_touchCalData, TFT_WHITE, TFT_BLACK, 15);
    touchSaveCalibration();
    s_touchCalValid = true;
    tft.setTouch(s_touchCalData);
    Serial.println("Touch: Calibration complete.");
}

bool touchCalibrationIsActive()
{
    return false;
}

// LVGL read callback
void touchLvglRead(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev; // Unused parameter

    if (s_touched)
    {
        data->point.x = s_touchX;
        data->point.y = s_touchY;
        data->state = LV_INDEV_STATE_PRESSED;

#if TOUCH_DEBUG
        static uint32_t lastLog = 0;
        if (millis() - lastLog > 200)
        {
            Serial.printf("LVGL Touch: x=%d, y=%d, PRESSED\n", s_touchX, s_touchY);
            lastLog = millis();
        }
#endif
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// Get touch coordinates (for external use)
bool touchGetPoint(uint16_t *x, uint16_t *y)
{
    if (s_touched)
    {
        *x = s_touchX;
        *y = s_touchY;
        return true;
    }
    return false;
}