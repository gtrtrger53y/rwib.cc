#pragma once
#include <Arduino.h>

// Wraps WiFiManager to provide both WiFi credential setup and the
// weather-location / display settings, all from one captive-portal web
// form (no on-screen keyboard needed).
namespace WifiSetup
{
    // Invoked (with the portal's AP name) the moment the captive portal
    // opens, so the UI can swap to a "connect to this WiFi" screen.
    void setPortalStartCallback(void (*cb)(const char *apName));

    // Called once at boot. Tries the last-saved WiFi network; if that
    // fails (or none is saved), opens a "CYD-Clock-Setup" access point
    // with a config page and blocks until the user submits it or it
    // times out. Returns true if WiFi ends up connected.
    bool autoConnect();

    // Re-opens the config portal on demand (e.g. from the on-screen
    // settings menu) to change WiFi network and/or weather location.
    // Blocking, same as autoConnect's portal.
    bool startConfigPortal();

    // True while the captive portal AP is active, so the UI can show
    // "connect to CYD-Clock-Setup to configure" instead of the clock.
    bool portalActive();
}
