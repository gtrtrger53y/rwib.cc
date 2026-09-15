#include "settings.h"
#include "config.h"
#include <Preferences.h>

Settings settings;

void Settings::load()
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    city = prefs.getString(PREF_KEY_CITY, city);
    lat = prefs.getDouble(PREF_KEY_LAT, lat);
    lon = prefs.getDouble(PREF_KEY_LON, lon);
    useFahrenheit = prefs.getString(PREF_KEY_UNITS, "f") == "f";
    hour12 = prefs.getString(PREF_KEY_HOUR12, "1") == "1";
    bleEnabled = prefs.getString(PREF_KEY_BLE_EN, "1") == "1";
    notifEnabled = prefs.getString(PREF_KEY_NOTIF_EN, "1") == "1";
    utcOffsetSeconds = prefs.getLong(PREF_KEY_UTC_OFFSET, 0);
    prefs.end();
}

void Settings::save() const
{
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString(PREF_KEY_CITY, city);
    prefs.putDouble(PREF_KEY_LAT, lat);
    prefs.putDouble(PREF_KEY_LON, lon);
    prefs.putString(PREF_KEY_UNITS, useFahrenheit ? "f" : "c");
    prefs.putString(PREF_KEY_HOUR12, hour12 ? "1" : "0");
    prefs.putString(PREF_KEY_BLE_EN, bleEnabled ? "1" : "0");
    prefs.putString(PREF_KEY_NOTIF_EN, notifEnabled ? "1" : "0");
    prefs.putLong(PREF_KEY_UTC_OFFSET, utcOffsetSeconds);
    prefs.end();
}
