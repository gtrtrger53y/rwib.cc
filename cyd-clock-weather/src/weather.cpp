#include "weather.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ctype.h>

namespace
{
    // Open-Meteo does not require an API key. We skip certificate
    // validation to avoid bundling/rotating root CAs on a hobby device;
    // acceptable here since no credentials are exchanged over this link.
    String httpGet(const String &url)
    {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        String body;
        if (http.begin(client, url))
        {
            http.setTimeout(8000);
            int code = http.GET();
            if (code == HTTP_CODE_OK)
            {
                body = http.getString();
            }
            http.end();
        }
        return body;
    }
}

namespace
{
    String urlEncodeImpl(const String &s)
    {
        String out;
        out.reserve(s.length() * 3);
        const char *hex = "0123456789ABCDEF";
        for (size_t i = 0; i < s.length(); i++)
        {
            uint8_t c = (uint8_t)s[i];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            {
                out += (char)c;
            }
            else if (c == ' ')
            {
                out += '+';
            }
            else
            {
                out += '%';
                out += hex[(c >> 4) & 0xF];
                out += hex[c & 0xF];
            }
        }
        return out;
    }
}

bool Weather::geocodeCity(const String &cityIn, double &lat, double &lon, String &resolvedName)
{
    String url = "https://geocoding-api.open-meteo.com/v1/search?count=1&language=en&format=json&name=";
    url += urlEncodeImpl(cityIn);

    String body = httpGet(url);
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
        return false;

    JsonArray results = doc["results"].as<JsonArray>();
    if (results.isNull() || results.size() == 0)
        return false;

    JsonObject first = results[0];
    lat = first["latitude"].as<double>();
    lon = first["longitude"].as<double>();

    String name = first["name"].as<const char *>() ? first["name"].as<String>() : cityIn;
    const char *admin1 = first["admin1"] | nullptr;
    const char *country = first["country"] | nullptr;
    if (admin1)
    {
        name += ", ";
        name += admin1;
    }
    else if (country)
    {
        name += ", ";
        name += country;
    }
    resolvedName = name;
    return true;
}

bool Weather::fetchForecast(double lat, double lon, WeatherData &out)
{
    char url[256];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code,is_day"
             "&daily=precipitation_probability_max"
             "&timezone=auto&forecast_days=1",
             lat, lon);

    String body = httpGet(String(url));
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
        return false;

    if (!doc["current"].is<JsonObject>())
        return false;

    JsonObject current = doc["current"];
    out.tempC = current["temperature_2m"].as<float>();
    out.weatherCode = current["weather_code"].as<int>();
    out.isDay = current["is_day"].as<int>() == 1;

    JsonArray rainProb = doc["daily"]["precipitation_probability_max"].as<JsonArray>();
    out.rainChancePercent = (rainProb.size() > 0) ? rainProb[0].as<int>() : 0;

    out.utcOffsetSeconds = doc["utc_offset_seconds"] | 0;
    out.description = describeCode(out.weatherCode);
    out.icon = iconForCode(out.weatherCode, out.isDay);
    out.valid = true;
    return true;
}

String Weather::describeCode(int code)
{
    switch (code)
    {
    case 0:
        return "Clear sky";
    case 1:
        return "Mainly clear";
    case 2:
        return "Partly cloudy";
    case 3:
        return "Overcast";
    case 45:
    case 48:
        return "Fog";
    case 51:
    case 53:
    case 55:
        return "Drizzle";
    case 56:
    case 57:
        return "Freezing drizzle";
    case 61:
    case 63:
    case 65:
        return "Rain";
    case 66:
    case 67:
        return "Freezing rain";
    case 71:
    case 73:
    case 75:
        return "Snow";
    case 77:
        return "Snow grains";
    case 80:
    case 81:
    case 82:
        return "Rain showers";
    case 85:
    case 86:
        return "Snow showers";
    case 95:
        return "Thunderstorm";
    case 96:
    case 99:
        return "Thunderstorm + hail";
    default:
        return "Unknown";
    }
}

WeatherIcon Weather::iconForCode(int code, bool isDay)
{
    switch (code)
    {
    case 0:
    case 1:
        return isDay ? WeatherIcon::Sun : WeatherIcon::MoonClear;
    case 2:
    case 3:
        return WeatherIcon::PartlyCloudy;
    case 45:
    case 48:
        return WeatherIcon::Fog;
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
        return WeatherIcon::Drizzle;
    case 61:
    case 63:
    case 65:
    case 66:
    case 67:
    case 80:
    case 81:
    case 82:
        return WeatherIcon::Rain;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return WeatherIcon::Snow;
    case 95:
    case 96:
    case 99:
        return WeatherIcon::Thunder;
    default:
        return WeatherIcon::Cloudy;
    }
}
