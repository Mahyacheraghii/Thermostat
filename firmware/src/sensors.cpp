#include "sensors.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>

namespace
{
    // SHT31: digital temperature/humidity sensor with better accuracy and faster response than DHT22.
    unsigned long lastUpdateMs = 0;
    bool sensorHealthy = false;
    Adafruit_SHT31 sht31;
}

bool sensorsInit()
{
    // Initialize I2C on specified pins (manual init as required)
    Wire.begin(21, 22);

    // SHT31 is a high-accuracy digital temperature/humidity sensor
    // compared to DHT22: SHT31 offers better accuracy, faster response,
    // and I2C interface (no timing-critical single-wire protocol).
    // We use Adafruit_SHT31 library to interact with the sensor.

    bool ok = sht31.begin(0x44);
    sensorHealthy = ok;
    lastUpdateMs = millis();
    gCurrentTempC = NAN;
    gCurrentHumidity = NAN;
    return ok;
}

void sensorsUpdate()
{
    const unsigned long now = millis();
    if (now - lastUpdateMs < 1000)
    {
        return;
    }

    lastUpdateMs = now;

    if (!sensorHealthy)
    {
        // Attempt to reinitialize periodically if previously failed
        if (sht31.begin(0x44))
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

    // Read temperature and humidity from SHT31
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
