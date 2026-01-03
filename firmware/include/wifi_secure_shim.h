#pragma once

// Prefer the real WiFiClientSecure header when available; otherwise provide
// a minimal shim so editors can parse the file without full Arduino includes.

#if defined(__has_include)
#if __has_include(<WiFiClientSecure.h>)
#include <WiFiClientSecure.h>
#define WIFI_SECURE_HAVE_REAL 1
#endif
#endif

#ifndef WIFI_SECURE_HAVE_REAL
#include <WiFiClient.h>
#include <Client.h>

class WiFiClientSecure : public WiFiClient
{
public:
    void setInsecure() {}
    void setCACert(const char *) {}
};
#endif
