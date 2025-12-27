#include "wifi_config.h"

#include <Preferences.h>

namespace
{
    const char kDefaultSsid[] = "Meow Meow 2.4";
    const char kDefaultPass[] = "meowmeowmeow2.4";

    bool wifiHasCredentials()
    {
        Preferences prefs;
        if (!prefs.begin("wifi", true))
            return false;
        String ssid = prefs.getString("ssid", "");
        prefs.end();
        return ssid.length() > 0;
    }

    void wifiSaveCredentials(const char *ssid, const char *pass)
    {
        Preferences prefs;
        if (!prefs.begin("wifi", false))
            return;
        prefs.putString("ssid", ssid);
        prefs.putString("pass", pass);
        prefs.end();
    }
}

void wifiConfigEnsureStored()
{
    if (!wifiHasCredentials())
        wifiSaveCredentials(kDefaultSsid, kDefaultPass);
}
