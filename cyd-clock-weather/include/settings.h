#pragma once
#include <Arduino.h>

// Thin wrapper around Preferences (NVS) for all user-configurable state.
// WiFi credentials themselves are managed by WiFiManager/ESP32 WiFi stack,
// not stored here.
struct Settings
{
    String city = "New York";
    double lat = 40.7128;
    double lon = -74.0060;
    bool useFahrenheit = true;
    bool hour12 = true;
    bool bleEnabled = true;
    bool notifEnabled = true;
    long utcOffsetSeconds = 0;

    void load();
    void save() const;
};

extern Settings settings;
