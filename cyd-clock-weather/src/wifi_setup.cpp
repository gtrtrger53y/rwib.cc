#include "wifi_setup.h"
#include "settings.h"
#include "weather.h"
#include "config.h"
#include <WiFiManager.h>
#include <string.h>

namespace
{
    volatile bool portalIsActive = false;
    void (*portalStartCallback)(const char *) = nullptr;

    WiFiManagerParameter *pCity = nullptr;
    WiFiManagerParameter *pUnits = nullptr;
    WiFiManagerParameter *pHour12 = nullptr;
    WiFiManagerParameter *pBle = nullptr;
    WiFiManagerParameter *pNotif = nullptr;

    void applySavedParams()
    {
        if (pCity && strlen(pCity->getValue()) > 0)
        {
            String newCity = pCity->getValue();
            if (newCity != settings.city)
            {
                double lat, lon;
                String resolved;
                if (Weather::geocodeCity(newCity, lat, lon, resolved))
                {
                    settings.city = resolved;
                    settings.lat = lat;
                    settings.lon = lon;
                }
                else
                {
                    // Keep old coordinates but remember what the user typed.
                    settings.city = newCity;
                }
            }
        }

        if (pUnits)
        {
            String v = pUnits->getValue();
            v.toLowerCase();
            if (v.startsWith("c"))
                settings.useFahrenheit = false;
            else if (v.startsWith("f"))
                settings.useFahrenheit = true;
        }

        if (pHour12)
        {
            String v = pHour12->getValue();
            if (v.indexOf("24") >= 0)
                settings.hour12 = false;
            else if (v.indexOf("12") >= 0)
                settings.hour12 = true;
        }

        if (pBle)
        {
            String v = pBle->getValue();
            v.toLowerCase();
            if (v.startsWith("off") || v == "0")
                settings.bleEnabled = false;
            else if (v.startsWith("on") || v == "1")
                settings.bleEnabled = true;
        }

        if (pNotif)
        {
            String v = pNotif->getValue();
            v.toLowerCase();
            if (v.startsWith("off") || v == "0")
                settings.notifEnabled = false;
            else if (v.startsWith("on") || v == "1")
                settings.notifEnabled = true;
        }

        settings.save();
    }

    void saveParamsCallback()
    {
        applySavedParams();
    }

    void configModeCallback(WiFiManager *wm)
    {
        portalIsActive = true;
        if (portalStartCallback)
            portalStartCallback(WIFI_PORTAL_SSID);
    }

    void setupPortalParams(WiFiManager &wm)
    {
        static char cityBuf[64];
        settings.city.toCharArray(cityBuf, sizeof(cityBuf));

        static char unitsBuf[8];
        strlcpy(unitsBuf, settings.useFahrenheit ? "F" : "C", sizeof(unitsBuf));

        static char hourBuf[8];
        strlcpy(hourBuf, settings.hour12 ? "12" : "24", sizeof(hourBuf));

        static char bleBuf[8];
        strlcpy(bleBuf, settings.bleEnabled ? "on" : "off", sizeof(bleBuf));

        static char notifBuf[8];
        strlcpy(notifBuf, settings.notifEnabled ? "on" : "off", sizeof(notifBuf));

        pCity = new WiFiManagerParameter("city", "Weather city (e.g. Denver, CO)", cityBuf, 63);
        pUnits = new WiFiManagerParameter("units", "Units: F or C", unitsBuf, 7);
        pHour12 = new WiFiManagerParameter("hour12", "Clock: 12 or 24", hourBuf, 7);
        pBle = new WiFiManagerParameter("ble", "Bluetooth: on or off", bleBuf, 7);
        pNotif = new WiFiManagerParameter("notif", "Notification badge: on or off", notifBuf, 7);

        wm.addParameter(pCity);
        wm.addParameter(pUnits);
        wm.addParameter(pHour12);
        wm.addParameter(pBle);
        wm.addParameter(pNotif);

        wm.setSaveParamsCallback(saveParamsCallback);
        wm.setAPCallback(configModeCallback);
        wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_S);
        wm.setBreakAfterConfig(true);
        wm.setTitle("CYD Clock Setup");
    }
}

void WifiSetup::setPortalStartCallback(void (*cb)(const char *))
{
    portalStartCallback = cb;
}

bool WifiSetup::autoConnect()
{
    WiFiManager wm;
    setupPortalParams(wm);

    bool ok = wm.autoConnect(WIFI_PORTAL_SSID);
    portalIsActive = false;
    return ok;
}

bool WifiSetup::startConfigPortal()
{
    WiFiManager wm;
    setupPortalParams(wm);

    bool ok = wm.startConfigPortal(WIFI_PORTAL_SSID);
    portalIsActive = false;
    return ok;
}

bool WifiSetup::portalActive()
{
    return portalIsActive;
}
