#include "display.h"
#include <math.h>

using namespace lgfx;

namespace
{
    constexpr uint32_t COLOR_BG = 0x000000;
    constexpr uint32_t COLOR_TEXT = 0xFFFFFF;
    constexpr uint32_t COLOR_MUTED = 0x9A9A9A;
    constexpr uint32_t COLOR_ACCENT = 0x3AA0FF;
    constexpr uint32_t COLOR_RAIN = 0x5FB8FF;
    constexpr uint32_t COLOR_SUN = 0xFFC93A;
    constexpr uint32_t COLOR_CLOUD = 0xC9CED6;
    constexpr uint32_t COLOR_BADGE = 0xFF4444;

    const char *dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    // Gear icon + notification badge live in fixed corners of the 320x240
    // (landscape) screen.
    constexpr int GEAR_X = 296, GEAR_Y = 224, GEAR_R = 16;
    constexpr int BADGE_X = 300, BADGE_Y = 16, BADGE_R = 14;

    void drawCloud(LGFX &gfx, int cx, int cy, int r, uint32_t color)
    {
        gfx.fillCircle(cx - r * 0.6, cy, r * 0.55, color);
        gfx.fillCircle(cx + r * 0.6, cy, r * 0.55, color);
        gfx.fillCircle(cx, cy - r * 0.35, r * 0.65, color);
        gfx.fillRoundRect(cx - r * 1.1, cy, r * 2.2, r * 0.7, r * 0.3, color);
    }

    void drawWeatherIcon(LGFX &gfx, int cx, int cy, int r, WeatherIcon icon)
    {
        switch (icon)
        {
        case WeatherIcon::Sun:
        {
            gfx.fillCircle(cx, cy, r * 0.5, COLOR_SUN);
            for (int i = 0; i < 8; i++)
            {
                float a = i * PI / 4;
                int x0 = cx + cosf(a) * r * 0.65;
                int y0 = cy + sinf(a) * r * 0.65;
                int x1 = cx + cosf(a) * r;
                int y1 = cy + sinf(a) * r;
                gfx.drawLine(x0, y0, x1, y1, COLOR_SUN);
            }
            break;
        }
        case WeatherIcon::MoonClear:
        {
            gfx.fillCircle(cx, cy, r * 0.55, 0xE8E8E8);
            gfx.fillCircle(cx + r * 0.28, cy - r * 0.15, r * 0.5, COLOR_BG);
            break;
        }
        case WeatherIcon::PartlyCloudy:
        {
            gfx.fillCircle(cx - r * 0.3, cy - r * 0.3, r * 0.35, COLOR_SUN);
            drawCloud(gfx, cx + r * 0.15, cy + r * 0.15, r * 0.75, COLOR_CLOUD);
            break;
        }
        case WeatherIcon::Fog:
        {
            drawCloud(gfx, cx, cy - r * 0.15, r * 0.75, COLOR_MUTED);
            for (int i = 0; i < 3; i++)
            {
                int y = cy + r * 0.55 + i * 8;
                gfx.drawFastHLine(cx - r * 0.9, y, r * 1.8, COLOR_MUTED);
            }
            break;
        }
        case WeatherIcon::Drizzle:
        case WeatherIcon::Rain:
        {
            drawCloud(gfx, cx, cy - r * 0.25, r * 0.75, COLOR_CLOUD);
            int drops = (icon == WeatherIcon::Rain) ? 4 : 3;
            for (int i = 0; i < drops; i++)
            {
                int x = cx - r * 0.7 + i * (r * 1.4 / (drops - 1));
                int y0 = cy + r * 0.35;
                gfx.drawLine(x, y0, x - 4, y0 + 12, COLOR_RAIN);
            }
            break;
        }
        case WeatherIcon::Snow:
        {
            drawCloud(gfx, cx, cy - r * 0.25, r * 0.75, COLOR_CLOUD);
            for (int i = 0; i < 3; i++)
            {
                int x = cx - r * 0.6 + i * (r * 0.6);
                int y = cy + r * 0.45;
                gfx.drawLine(x - 4, y, x + 4, y, COLOR_TEXT);
                gfx.drawLine(x, y - 4, x, y + 4, COLOR_TEXT);
            }
            break;
        }
        case WeatherIcon::Thunder:
        {
            drawCloud(gfx, cx, cy - r * 0.25, r * 0.75, 0x8A8F99);
            int bx = cx, by = cy + r * 0.3;
            gfx.fillTriangle(bx, by, bx + 10, by, bx - 4, by + 16, COLOR_SUN);
            gfx.fillTriangle(bx - 4, by + 16, bx + 6, by + 16, bx, by + 30, COLOR_SUN);
            break;
        }
        default:
        {
            drawCloud(gfx, cx, cy, r * 0.75, COLOR_MUTED);
            break;
        }
        }
    }

    void drawGear(LGFX &gfx)
    {
        gfx.fillCircle(GEAR_X, GEAR_Y, GEAR_R * 0.5, COLOR_MUTED);
        for (int i = 0; i < 8; i++)
        {
            float a = i * PI / 4;
            int x0 = GEAR_X + cosf(a) * GEAR_R * 0.55;
            int y0 = GEAR_Y + sinf(a) * GEAR_R * 0.55;
            int x1 = GEAR_X + cosf(a) * GEAR_R;
            int y1 = GEAR_Y + sinf(a) * GEAR_R;
            gfx.drawLine(x0, y0, x1, y1, COLOR_MUTED);
        }
        gfx.fillCircle(GEAR_X, GEAR_Y, GEAR_R * 0.22, COLOR_BG);
    }
}

void Display::begin(LGFX &gfx)
{
    gfx.init();
    gfx.setRotation(1); // landscape, 320x240
    gfx.setBrightness(180);
    gfx.fillScreen(COLOR_BG);
}

void Display::drawClock(LGFX &gfx, const struct tm &timeinfo, const WeatherData &wx,
                         int notifCount, const Settings &s)
{
    gfx.startWrite();
    gfx.fillScreen(COLOR_BG);

    // --- Date ---
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%s, %s %d",
              dayNames[timeinfo.tm_wday], monthNames[timeinfo.tm_mon], timeinfo.tm_mday);
    gfx.setFont(&fonts::Font4);
    gfx.setTextDatum(top_center);
    gfx.setTextColor(COLOR_MUTED, COLOR_BG);
    gfx.drawString(dateBuf, gfx.width() / 2, 10);

    // --- Big time (7-segment style digits) ---
    int hour = timeinfo.tm_hour;
    bool pm = hour >= 12;
    if (s.hour12)
    {
        hour = hour % 12;
        if (hour == 0)
            hour = 12;
    }
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hour, timeinfo.tm_min, timeinfo.tm_sec);

    gfx.setFont(&fonts::Font7);
    gfx.setTextDatum(top_center);
    gfx.setTextColor(COLOR_TEXT, COLOR_BG);
    gfx.drawString(timeBuf, gfx.width() / 2, 52);

    if (s.hour12)
    {
        gfx.setFont(&fonts::Font4);
        gfx.setTextDatum(top_left);
        gfx.setTextColor(COLOR_ACCENT, COLOR_BG);
        gfx.drawString(pm ? "PM" : "AM", gfx.width() / 2 + 118, 60);
    }

    // --- Weather ---
    int iconCx = 46, iconCy = 158, iconR = 34;
    if (wx.valid)
    {
        drawWeatherIcon(gfx, iconCx, iconCy, iconR, wx.icon);

        int displayTemp = s.useFahrenheit ? (int)roundf(wx.tempC * 9.0f / 5.0f + 32.0f)
                                           : (int)roundf(wx.tempC);
        char tempBuf[16];
        snprintf(tempBuf, sizeof(tempBuf), "%d%s", displayTemp, s.useFahrenheit ? "F" : "C");

        gfx.setFont(&fonts::Font6);
        gfx.setTextDatum(top_left);
        gfx.setTextColor(COLOR_TEXT, COLOR_BG);
        gfx.drawString(tempBuf, iconCx + iconR + 16, 132);

        gfx.setFont(&fonts::Font2);
        gfx.setTextColor(COLOR_MUTED, COLOR_BG);
        gfx.drawString(wx.description, iconCx + iconR + 16, 170);

        char rainBuf[32];
        snprintf(rainBuf, sizeof(rainBuf), "Rain today: %d%%", wx.rainChancePercent);
        gfx.setTextColor(wx.rainChancePercent >= 40 ? COLOR_RAIN : COLOR_MUTED, COLOR_BG);
        gfx.drawString(rainBuf, iconCx + iconR + 16, 190);
    }
    else
    {
        gfx.setFont(&fonts::Font2);
        gfx.setTextDatum(top_left);
        gfx.setTextColor(COLOR_MUTED, COLOR_BG);
        gfx.drawString("Weather unavailable", iconCx + iconR + 16, 150);
    }

    // --- Notification badge ---
    if (s.notifEnabled && notifCount >= 0)
    {
        gfx.fillCircle(BADGE_X, BADGE_Y, BADGE_R, notifCount > 0 ? COLOR_BADGE : 0x2A2A2A);
        gfx.setFont(&fonts::Font2);
        gfx.setTextDatum(middle_center);
        gfx.setTextColor(COLOR_TEXT, notifCount > 0 ? COLOR_BADGE : 0x2A2A2A);
        char nbuf[8];
        snprintf(nbuf, sizeof(nbuf), "%d", notifCount > 99 ? 99 : notifCount);
        gfx.drawString(nbuf, BADGE_X, BADGE_Y);
    }

    drawGear(gfx);
    gfx.endWrite();
}

void Display::drawPortalScreen(LGFX &gfx, const char *apName)
{
    gfx.startWrite();
    gfx.fillScreen(COLOR_BG);
    gfx.setTextDatum(middle_center);

    gfx.setFont(&fonts::Font4);
    gfx.setTextColor(COLOR_ACCENT, COLOR_BG);
    gfx.drawString("Setup mode", gfx.width() / 2, 60);

    gfx.setFont(&fonts::Font2);
    gfx.setTextColor(COLOR_TEXT, COLOR_BG);
    gfx.drawString("Connect your phone to WiFi:", gfx.width() / 2, 110);

    gfx.setFont(&fonts::Font4);
    gfx.setTextColor(COLOR_SUN, COLOR_BG);
    gfx.drawString(apName, gfx.width() / 2, 140);

    gfx.setFont(&fonts::Font2);
    gfx.setTextColor(COLOR_MUTED, COLOR_BG);
    gfx.drawString("Then follow the page that opens", gfx.width() / 2, 180);
    gfx.endWrite();
}

bool Display::gearTapped(LGFX &gfx, int x, int y)
{
    int dx = x - GEAR_X, dy = y - GEAR_Y;
    return (dx * dx + dy * dy) <= (GEAR_R + 8) * (GEAR_R + 8);
}

namespace
{
    struct MenuRow
    {
        int y, h;
        MenuAction action;
        const char *label;
    };

    // Kept in sync with drawSettingsMenu()/menuActionForTouch() below.
    const MenuRow menuRows[] = {
        {30, 40, MenuAction::WifiLocation, "WiFi & Location Setup"},
        {75, 40, MenuAction::ToggleUnits, "Units"},
        {120, 40, MenuAction::ToggleHour, "Clock format"},
        {165, 40, MenuAction::ToggleBle, "Bluetooth"},
        {210, 40, MenuAction::ToggleNotif, "Notification badge"},
    };
}

void Display::drawSettingsMenu(LGFX &gfx, const Settings &s)
{
    gfx.startWrite();
    gfx.fillScreen(COLOR_BG);

    gfx.setFont(&fonts::Font4);
    gfx.setTextDatum(top_center);
    gfx.setTextColor(COLOR_TEXT, COLOR_BG);
    gfx.drawString("Settings", gfx.width() / 2, 4);

    gfx.setFont(&fonts::Font2);
    for (const auto &row : menuRows)
    {
        gfx.drawFastHLine(10, row.y, gfx.width() - 20, 0x2A2A2A);
        gfx.setTextDatum(middle_left);
        gfx.setTextColor(COLOR_TEXT, COLOR_BG);
        gfx.drawString(row.label, 16, row.y + row.h / 2);

        String value;
        switch (row.action)
        {
        case MenuAction::WifiLocation:
            value = s.city;
            break;
        case MenuAction::ToggleUnits:
            value = s.useFahrenheit ? "Fahrenheit" : "Celsius";
            break;
        case MenuAction::ToggleHour:
            value = s.hour12 ? "12-hour" : "24-hour";
            break;
        case MenuAction::ToggleBle:
            value = s.bleEnabled ? "On" : "Off";
            break;
        case MenuAction::ToggleNotif:
            value = s.notifEnabled ? "On" : "Off";
            break;
        default:
            break;
        }
        gfx.setTextDatum(middle_right);
        gfx.setTextColor(COLOR_ACCENT, COLOR_BG);
        gfx.drawString(value, gfx.width() - 16, row.y + row.h / 2);
    }
    gfx.drawFastHLine(10, 210 + 40, gfx.width() - 20, 0x2A2A2A);

    gfx.setFont(&fonts::Font2);
    gfx.setTextDatum(bottom_center);
    gfx.setTextColor(COLOR_MUTED, COLOR_BG);
    gfx.drawString("Tap here to go back", gfx.width() / 2, gfx.height() - 6);
    gfx.endWrite();
}

MenuAction Display::menuActionForTouch(LGFX &gfx, int x, int y)
{
    for (const auto &row : menuRows)
    {
        if (y >= row.y && y < row.y + row.h)
            return row.action;
    }
    if (y >= gfx.height() - 30)
        return MenuAction::Back;
    return MenuAction::None;
}
