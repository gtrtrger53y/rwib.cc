#pragma once
#include <Arduino.h>
#include <time.h>
#include "LGFX_CYD.hpp"
#include "weather.h"
#include "settings.h"

enum class MenuAction
{
    None,
    WifiLocation,
    ToggleUnits,
    ToggleHour,
    ToggleBle,
    ToggleNotif,
    Back
};

namespace Display
{
    void begin(LGFX &gfx);

    // Full-screen clock + weather redraw. notifCount < 0 means "no data
    // yet / feature disabled" and the badge is hidden.
    void drawClock(LGFX &gfx, const struct tm &timeinfo, const WeatherData &wx,
                    int notifCount, const Settings &s);

    // Simple "connect to this WiFi network to finish setup" screen shown
    // while the WiFiManager captive portal is active.
    void drawPortalScreen(LGFX &gfx, const char *apName);

    // Returns true if (x,y) landed on the small gear icon drawn by
    // drawClock(), meaning the settings menu should open.
    bool gearTapped(LGFX &gfx, int x, int y);

    void drawSettingsMenu(LGFX &gfx, const Settings &s);
    MenuAction menuActionForTouch(LGFX &gfx, int x, int y);
}
