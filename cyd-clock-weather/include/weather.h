#pragma once
#include <Arduino.h>

enum class WeatherIcon
{
    Sun,
    MoonClear,
    PartlyCloudy,
    Cloudy,
    Fog,
    Drizzle,
    Rain,
    Snow,
    Thunder,
    Unknown
};

struct WeatherData
{
    bool valid = false;
    float tempC = 0;
    bool isDay = true;
    int weatherCode = 0;
    int rainChancePercent = 0;  // today's max precipitation probability
    String description = "--";
    WeatherIcon icon = WeatherIcon::Unknown;
    long utcOffsetSeconds = 0;
};

namespace Weather
{
    // Resolves a free-text city name to coordinates via Open-Meteo's
    // geocoding API. Returns true and fills lat/lon/resolvedName on success.
    bool geocodeCity(const String &city, double &lat, double &lon, String &resolvedName);

    // Fetches current conditions + today's rain chance for the given
    // coordinates. Returns true on success.
    bool fetchForecast(double lat, double lon, WeatherData &out);

    // Human-readable description + icon for a WMO weather code.
    String describeCode(int code);
    WeatherIcon iconForCode(int code, bool isDay);
}
