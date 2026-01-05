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
    Serial.println("System Initializing...");

    // ابتدا خروجی‌ها را برای امنیت در حالت OFF ست می‌کنیم
    outputsInit();

    wifiConfigEnsureStored();
    sensorsInit();

    // ترتیب مهم است: ابتدا UI (که LVGL را اینیت می‌کند) و سپس Touch
    uiInit();
    touchInit();

    mqttInit();
    stateMachineInit();

    Serial.println("System Ready.");
}

void loop()
{
    // ۱. خواندن داده‌های سنسور
    sensorsUpdate();

    // ۲. خواندن مختصات لمسی از سخت‌افزار
    touchUpdate();

    // ۳. پردازش منطق سیستم (State Machine)
    stateMachineUpdate();

    // ۴. مدیریت ارتباطات
    mqttUpdate();

    // ۵. آپدیت لایه گرافیکی (اگر تابعی برای آپدیت متغیرهای نمایشی دارید)
    uiUpdate();

    // ۶. حیاتی‌ترین بخش برای LVGL: پردازش رویدادها و کلیک‌ها
    // این تابع باعث می‌شود دکمه‌ها واکنش نشان دهند
    lv_timer_handler();

    // ۷. تاخیر کوتاه برای جلوگیری از اشغال کامل CPU و پایداری Wi-Fi
    delay(1);
}