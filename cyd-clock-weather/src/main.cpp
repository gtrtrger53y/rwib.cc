#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "LGFX_CYD.hpp"
#include "display.h"
#include "settings.h"
#include "weather.h"
#include "wifi_setup.h"
#include "ble_service.h"
#include "config.h"

namespace
{
    enum class Screen
    {
        Clock,
        Portal,
        Settings
    };

    LGFX gfx;
    Screen currentScreen = Screen::Clock;
    WeatherData currentWeather;
    unsigned long lastWeatherFetchMs = 0;
    unsigned long lastNtpSyncMs = 0;
    int lastDrawnSecond = -1;
    bool touchDown = false;

    void syncTime(long utcOffsetSeconds)
    {
        configTime(utcOffsetSeconds, 0, "pool.ntp.org", "time.nist.gov");
        lastNtpSyncMs = millis();
    }

    void refreshWeather()
    {
        WeatherData wx;
        if (Weather::fetchForecast(settings.lat, settings.lon, wx))
        {
            currentWeather = wx;
            if (wx.utcOffsetSeconds != settings.utcOffsetSeconds)
            {
                settings.utcOffsetSeconds = wx.utcOffsetSeconds;
                settings.save();
            }
            syncTime(wx.utcOffsetSeconds);
        }
        lastWeatherFetchMs = millis();
    }

    void onPortalStart(const char *apName)
    {
        currentScreen = Screen::Portal;
        Display::drawPortalScreen(gfx, apName);
    }

    void handleTouch(int x, int y)
    {
        if (currentScreen == Screen::Clock)
        {
            if (Display::gearTapped(gfx, x, y))
            {
                currentScreen = Screen::Settings;
                Display::drawSettingsMenu(gfx, settings);
            }
            return;
        }

        if (currentScreen != Screen::Settings)
            return;

        MenuAction action = Display::menuActionForTouch(gfx, x, y);
        switch (action)
        {
        case MenuAction::WifiLocation:
            // Blocks until the portal is submitted or times out.
            WifiSetup::startConfigPortal();
            currentScreen = Screen::Clock;
            lastDrawnSecond = -1;
            lastWeatherFetchMs = 0; // force a refresh in case location changed
            if (WiFi.status() == WL_CONNECTED)
                syncTime(settings.utcOffsetSeconds);
            break;
        case MenuAction::ToggleUnits:
            settings.useFahrenheit = !settings.useFahrenheit;
            settings.save();
            Display::drawSettingsMenu(gfx, settings);
            break;
        case MenuAction::ToggleHour:
            settings.hour12 = !settings.hour12;
            settings.save();
            Display::drawSettingsMenu(gfx, settings);
            break;
        case MenuAction::ToggleBle:
            settings.bleEnabled = !settings.bleEnabled;
            settings.save();
            if (settings.bleEnabled)
                BleService::begin();
            else
                BleService::end();
            Display::drawSettingsMenu(gfx, settings);
            break;
        case MenuAction::ToggleNotif:
            settings.notifEnabled = !settings.notifEnabled;
            settings.save();
            Display::drawSettingsMenu(gfx, settings);
            break;
        case MenuAction::Back:
            currentScreen = Screen::Clock;
            lastDrawnSecond = -1;
            break;
        default:
            break;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    settings.load();

    Display::begin(gfx);

    WifiSetup::setPortalStartCallback(onPortalStart);
    bool connected = WifiSetup::autoConnect();
    currentScreen = Screen::Clock;

    if (connected)
    {
        // Apply the last-known UTC offset immediately so the clock isn't
        // stuck at UTC until the first weather fetch completes.
        syncTime(settings.utcOffsetSeconds);
        refreshWeather();
    }

    if (settings.bleEnabled)
    {
        BleService::begin();
    }
}

void loop()
{
    unsigned long now = millis();

    if (WiFi.status() == WL_CONNECTED)
    {
        if (lastWeatherFetchMs == 0 || now - lastWeatherFetchMs >= WEATHER_REFRESH_MS)
        {
            refreshWeather();
        }
        if (now - lastNtpSyncMs >= NTP_RESYNC_MS)
        {
            syncTime(settings.utcOffsetSeconds);
        }
    }

    BleService::loop();

    int32_t tx, ty;
    bool touched = gfx.getTouch(&tx, &ty);
    if (touched && !touchDown)
    {
        touchDown = true;
        handleTouch(tx, ty);
    }
    else if (!touched)
    {
        touchDown = false;
    }

    if (currentScreen == Screen::Clock)
    {
        time_t t = time(nullptr);
        struct tm timeinfo;
        localtime_r(&t, &timeinfo);
        if (timeinfo.tm_sec != lastDrawnSecond)
        {
            lastDrawnSecond = timeinfo.tm_sec;
            int notifCount = settings.notifEnabled ? BleService::notificationCount() : -1;
            Display::drawClock(gfx, timeinfo, currentWeather, notifCount, settings);
        }
    }

    delay(20);
}
