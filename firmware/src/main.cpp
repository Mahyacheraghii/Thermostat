#include <Arduino.h>
#include <lvgl.h> // اضافه کردن کتابخانه اصلی برای دسترسی به مدیریت تایمر
#include "state_machine.h"
#include "sensors.h"
#include "outputs.h"
#include "ui.h"
#include "touch.h"
#include "mqtt.h"
#include "wifi_config.h"

void setup()
{
    Serial.begin(115200);
    delay(1000); // زمان برای بالا آمدن سریال مانیتور
    Serial.println("System Initializing...");

    // --- حل مشکل NVS ---
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Serial.println("NVS: Erasing and re-init...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ------------------

    outputsInit();

    // حالا Preferences به درستی کار خواهد کرد
    wifiConfigEnsureStored();
    sensorsInit();

    uiInit();
    touchInit();

    mqttInit();
    stateMachineInit();

    Serial.println("System Ready.");
}

void loop()
{
    // ۱. این دو باید با بیشترین سرعت ممکن اجرا شوند
    touchUpdate();
    lv_timer_handler();

    // ۲. بقیه کارها را دسته بندی کنید تا وقت CPU را مدام نگیرند
    static uint32_t lastSlowTask = 0;
    if (millis() - lastSlowTask > 100)
    { // هر ۱۰۰ میلی‌ثانیه یکبار (۱۰ بار در ثانیه)
        sensorsUpdate();
        stateMachineUpdate();
        uiUpdate(); // آپدیت اعداد و لیبل‌های روی صفحه
        lastSlowTask = millis();
    }

    // ۳. مدیریت شبکه (معمولاً خودش زمان‌بندی داخلی دارد اما بهتر است جدا باشد)
    static uint32_t lastNetworkTask = 0;
    if (millis() - lastNetworkTask > 500)
    { // هر نیم ثانیه یکبار
        mqttUpdate();
        lastNetworkTask = millis();
    }

    delay(2); // زمان تنفس برای سیستم عامل و جلوگیری از داغ شدن
}