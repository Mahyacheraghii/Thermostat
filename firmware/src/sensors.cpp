#include "sensors.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>

namespace
{
    // SHT3x (SHT30/SHT31/SHT35) digital temperature/humidity sensor over I2C.
    unsigned long lastUpdateMs = 0;
    bool sensorHealthy = false;
    Adafruit_SHT31 sht31;
    static const uint8_t SHT3X_I2C_ADDR = 0x44; // ADDR/AD tied low; use 0x45 if tied high.
    static const unsigned long SHT3X_UPDATE_INTERVAL_MS = 10;
}

bool sensorsInit()
{
    // Initialize I2C on specified pins (manual init as required)
    Wire.begin(21, 22);

    // SHT3x is a high-accuracy digital temperature/humidity sensor.
    // We use Adafruit_SHT31 library to interact with the SHT3x family.

    bool ok = sht31.begin(SHT3X_I2C_ADDR);
    sensorHealthy = ok;
    lastUpdateMs = millis();
    gCurrentTempC = NAN;
    gCurrentHumidity = NAN;
    return ok;
}

void sensorsUpdate()
{
    const unsigned long now = millis();
    if (now - lastUpdateMs < SHT3X_UPDATE_INTERVAL_MS)
    {
        return;
    }

    lastUpdateMs = now;

    if (!sensorHealthy)
    {
        // Attempt to reinitialize periodically if previously failed
        if (sht31.begin(SHT3X_I2C_ADDR))
        {
            sensorHealthy = true;
        }
        else
        {
            // Leave readings unchanged but mark unhealthy
            sensorHealthy = false;
            return;
        }
    }

    // Read temperature and humidity from SHT3x
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    // Validate readings (check for NaN)
    if (isnan(t) || isnan(h))
    {
        sensorHealthy = false;
        return;
    }

    // Store valid readings (degrees C, percent RH)
    gCurrentTempC = t;
    gCurrentHumidity = h;
}

bool sensorsAvailable()
{
    return sensorHealthy;
}

unsigned long sensorsLastUpdateMs()
{
    return lastUpdateMs;
}
