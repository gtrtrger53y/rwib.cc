#pragma once

// Preferences (NVS) namespace + keys.
#define PREFS_NAMESPACE "cydclock"
#define PREF_KEY_CITY "city"
#define PREF_KEY_LAT "lat"
#define PREF_KEY_LON "lon"
#define PREF_KEY_UNITS "units"        // "f" or "c"
#define PREF_KEY_HOUR12 "hour12"      // "1" or "0"
#define PREF_KEY_BLE_EN "ble_en"      // "1" or "0"
#define PREF_KEY_NOTIF_EN "notif_en"  // "1" or "0"
#define PREF_KEY_UTC_OFFSET "utcoff"  // seconds, cached from last weather fetch

// Timing.
#define WEATHER_REFRESH_MS (15UL * 60UL * 1000UL)  // 15 minutes
#define NTP_RESYNC_MS (60UL * 60UL * 1000UL)        // 1 hour

#define WIFI_PORTAL_SSID "CYD-Clock-Setup"
#define WIFI_PORTAL_TIMEOUT_S 180
