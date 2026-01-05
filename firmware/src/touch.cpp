#include "touch.h"

#include <Preferences.h>

#include "ui.h"

// ---------------------------------------------------------------------------
// Pin map (ILI9341 + XPT2046, shared SPI). Power pins (VCC/GND) are hardware.
// These are informational; TFT_eSPI uses its own configuration for SPI pins.
// ---------------------------------------------------------------------------
static const uint16_t kTftWidth = 320;
static const uint16_t kTftHeight = 240;
static const uint8_t kTftRotation = 3;

// Shared SPI bus pins.
static const uint8_t SPI_SCK_PIN = 18;
static const uint8_t SPI_MISO_PIN = 19;
static const uint8_t SPI_MOSI_PIN = 23;

// Display control pins.
// Backlight LED is tied to 3.3V on your module (no GPIO control).
static const uint8_t TFT_CS_PIN = 15;
static const uint8_t TFT_DC_PIN = 2;
static const uint8_t TFT_RST_PIN = 4;

// Touch controller pins.
static const uint8_t TOUCH_CS_PIN = 5;
static const uint8_t TOUCH_IRQ_PIN = 33;

#ifndef TOUCH_DEBUG
#define TOUCH_DEBUG 0
#endif

// Last known touch state shared with LVGL.
static volatile bool s_touched = false;
static volatile uint16_t s_touchX = 0;
static volatile uint16_t s_touchY = 0;

// NVS storage for calibration values.
static Preferences s_prefs;

static const char *kTouchPrefsNs = "touch";
static const char *kTouchPrefsKeyHasCal = "has_cal";
static const char *kTouchPrefsKeyCal = "cal";

static uint16_t s_touchCalData[5] = {0, 0, 0, 0, 0};
static bool s_touchCalValid = false;

// Load previously stored calibration from NVS (if any).
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

void touchInit()
{
    // ۱. مدیریت پین‌های کنترلی برای جلوگیری از تداخل در زمان بوت
    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH); // غیرفعال کردن اولیه چیپ لمسی

    // ۲. بخش موقت برای عیب‌یابی: پاک کردن حافظه جهت اجبار به کالیبراسیون مجدد
    // نکته: بعد از اینکه یک بار کالیبراسیون موفق داشتید، می‌توانید ۳ خط زیر را حذف کنید.
    // s_prefs.begin(kTouchPrefsNs, false);
    // s_prefs.clear();
    // s_prefs.end();

    // ۳. تلاش برای بارگذاری کالیبراسیون از حافظه
    touchLoadCalibration();

    TFT_eSPI &tft = uiGetTft();
    tft.setRotation(kTftRotation); // حتما چرخش نمایشگر قبل از کالیبره تنظیم شود

    if (s_touchCalValid)
    {
        // اگر کالیبراسیون معتبر موجود باشد
        tft.setTouch(s_touchCalData);
        Serial.println("Touch: Calibration loaded from NVS.");
    }
    else
    {
        // اگر کالیبراسیون موجود نباشد (یا در مرحله ۲ پاک شده باشد)
        Serial.println("Touch calibration needed (first boot or forced).");
        Serial.println("Follow the on-screen prompts.");

        // اجرای فرآیند کالیبراسیون روی نمایشگر
        // در این مرحله باید ۴ نقطه در گوشه‌های صفحه را لمس کنید
        tft.calibrateTouch(s_touchCalData, TFT_WHITE, TFT_BLACK, 15);

        // ذخیره داده‌های جدید در NVS
        touchSaveCalibration();
        s_touchCalValid = true;
        tft.setTouch(s_touchCalData);

        Serial.println("Calibration complete and saved.");
    }

#if TOUCH_DEBUG
    Serial.println("Touch init: TFT_eSPI touch ready.");
#endif
}

// void touchUpdate()
// {
//     // Poll and update cached coordinates; LVGL will call touchLvglRead to consume them.
//     TFT_eSPI &tft = uiGetTft();
//     uint16_t x = 0;
//     uint16_t y = 0;
//     bool touched = tft.getTouch(&x, &y);
// #if TOUCH_DEBUG
//     static uint32_t last_log_ms = 0;
//     const uint32_t now = millis();
//     if (now - last_log_ms >= 50)
//     {
//         Serial.print("touch xy ");
//         Serial.print(x);
//         Serial.print(",");
//         Serial.print(y);
//         Serial.print(" touched=");
//         Serial.println(touched ? "1" : "0");
//         last_log_ms = now;
//     }
// #endif
//     if (touched)
//     {
//         s_touchX = x;
//         s_touchY = y;
//         s_touched = true;
//     }
//     else
//     {
//         s_touched = false;
//     }
// }

void touchUpdate()
{
    TFT_eSPI &tft = uiGetTft();
    uint16_t x_raw = 0, y_raw = 0;
    uint16_t tempX = 0, tempY = 0;

    // خواندن مقدار خام
    if (tft.getTouchRaw(&x_raw, &y_raw))
    {
        // فیلتر کردن نویزهای شدید (4095 یا 0 معمولا خطا هستند)
        if (x_raw > 0 && x_raw < 4090 && y_raw > 0 && y_raw < 4090)
        {

            // چاپ مقادیر برای کالیبره کردن دستی در صورت نیاز
            Serial.printf("Valid Raw: X=%d, Y=%d | ", x_raw, y_raw);

            if (tft.getTouch(&tempX, &tempY))
            {
                s_touchX = tempX;
                s_touchY = tempY;
                s_touched = true;
                Serial.printf("Mapped to Screen: %d, %d\n", tempX, tempY);
            }
            else
            {
                Serial.println("Waiting for Calibration...");
                s_touched = false;
            }
        }
    }
    else
    {
        s_touched = false;
    }
}

// Start calibration. Calibration results are persisted to NVS.
void touchCalibrateStart()
{
    Serial.println("Calibration requested.");
    Serial.println("Follow the on-screen prompts.");
    TFT_eSPI &tft = uiGetTft();
    tft.setRotation(kTftRotation);
    tft.calibrateTouch(s_touchCalData, TFT_WHITE, TFT_BLACK, 15);
    touchSaveCalibration();
    s_touchCalValid = true;
    tft.setTouch(s_touchCalData);
    Serial.println("Calibration complete.");
}

bool touchCalibrationIsActive()
{
    return false;
}

// LVGL read callback
void touchLvglRead(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (s_touched)
    {
        data->point.x = s_touchX;
        data->point.y = s_touchY;
        data->state = LV_INDEV_STATE_PR;
        // Serial.println("LVGL is reading a TOUCH!"); // اگر این چاپ نشود، یعنی مرحله ۱ مشکل دارد
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}
