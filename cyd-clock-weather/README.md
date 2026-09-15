# CYD Clock & Weather

Firmware for the "Cheap Yellow Display" (Sunton ESP32-2432S028R — an
ESP32-WROOM-32 with a built-in 2.8" 320x240 ILI9341 touchscreen). Turns it
into a desk clock that shows:

- Time in 12-hour format with seconds, in a large 7-segment-style font
- Today's date
- Current weather conditions and temperature for a city you choose
- Chance of rain today
- An optional notification-count badge, fed over Bluetooth LE
- An on-screen settings menu (tap the gear icon) plus a phone-friendly
  web form for WiFi and location, no keyboard needed

Built with [PlatformIO](https://platformio.org) + Arduino framework,
[LovyanGFX](https://github.com/lovyan03/LovyanGFX) for the display/touch,
[WiFiManager](https://github.com/tzapu/WiFiManager) for setup, and
[NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) for BLE.
Weather comes from [Open-Meteo](https://open-meteo.com), which is free
and doesn't need an API key.

> **Not build-verified in this sandbox.** The sandbox this was written in
> blocks outbound access to platformio.org and github.com, so `pio run`
> could not be exercised against the real ESP32 toolchain here. The code
> is written against well-established, current APIs for each library, but
> please run a build yourself (see below) before flashing, and open an
> issue/PR-comment with any compiler errors you hit — they're likely a
> small API drift in one of the pinned library versions.

## Hardware

Tested pin assignments are for the common CYD variant with a resistive
XPT2046 touch panel (the board silkscreened `ESP32-2432S028R`). If your
board uses the capacitive (CST820) touch variant, or wires things
differently, edit `include/LGFX_CYD.hpp`.

| Function      | GPIO |
|---------------|------|
| TFT SCLK      | 14   |
| TFT MOSI      | 13   |
| TFT MISO      | 12   |
| TFT CS        | 15   |
| TFT DC        | 2    |
| TFT Backlight | 21   |
| Touch CS      | 33   |
| Touch IRQ     | 36   |

## Build & flash

```sh
pip install platformio   # if you don't already have it
cd cyd-clock-weather
pio run                  # build
pio run -t upload        # flash over USB
pio device monitor       # optional: serial log at 115200 baud
```

## First-time setup (WiFi + weather location)

On first boot (or whenever WiFi isn't connected), the device opens its
own WiFi access point called **`CYD-Clock-Setup`** and shows that on
screen. From your phone or laptop:

1. Connect to the `CYD-Clock-Setup` WiFi network.
2. A setup page should open automatically (captive portal); if not,
   browse to `192.168.4.1`.
3. Pick "Configure WiFi", enter your home network's SSID/password, and
   fill in:
   - **Weather city** — e.g. `Denver, CO` (looked up via Open-Meteo's
     geocoding API)
   - **Units** — `F` or `C`
   - **Clock** — `12` or `24`
   - **Bluetooth** — `on` or `off`
   - **Notification badge** — `on` or `off`
4. Save. The device reboots onto the clock screen once connected.

You can reopen this same form any time from the device itself: tap the
small gear icon in the bottom-right corner of the clock screen, then
**"WiFi & Location Setup"**. The other rows in that menu (units, clock
format, Bluetooth, notification badge) toggle instantly with a tap and
don't require the portal.

## Notification badge over BLE

There's no universal, permission-free way for a random BLE peripheral to
read "how many notifications does my phone have" — iOS gates that behind
ANCS (needs a paired/bonded companion, not just any device), and Android
doesn't expose it over BLE at all without an app granted notification
access. So instead of faking that, this firmware exposes a small GATT
service that any phone-side bridge can write a count into:

- Service UUID: `5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0001`
- **Notification Count** (write): `5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0002`
  — write 1–2 bytes, little-endian unsigned integer, e.g. `0x05 0x00` for 5
- **Status** (read/notify): `5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0003`
  — reserved for future use

Device advertises as **"CYD Clock"**. Options for the phone side:

- **Quick test:** use [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile)
  (iOS/Android), connect to "CYD Clock", write bytes to the Notification
  Count characteristic, and watch the badge update.
- **Android automation:** Tasker (with the AutoNotification + BLE
  plugins) or MacroDroid can watch for new notifications and write the
  current count to that characteristic on each change.
- **iOS:** there's no no-code path; you'd need a small companion app (or
  a Shortcuts + BLE-capable helper app) that reads local notification
  counts and performs the same GATT write.

Turn this off entirely from the on-screen settings menu ("Bluetooth" or
"Notification badge" rows) if you don't want to bother with a bridge —
the clock and weather work fine without it.

## Notes & known limitations

- Weather API calls skip TLS certificate validation (`setInsecure()`) to
  avoid bundling/rotating root CA certs on the device. No credentials are
  sent over that connection, so this is a reasonable trade-off for a
  hobby device, but worth knowing.
- Timezone/DST is derived from Open-Meteo's `utc_offset_seconds` for your
  location (refreshed every 15 minutes), rather than a full IANA
  timezone database — simpler, and only risks a brief clock error right
  at a DST transition.
- Touch calibration (`x_min`/`x_max`/`y_min`/`y_max` in
  `include/LGFX_CYD.hpp`) uses commonly-seen defaults for this board; if
  taps land off-target, adjust those values (or run LovyanGFX's touch
  calibration example once and copy the results in).
- Weather refreshes every 15 minutes; adjust `WEATHER_REFRESH_MS` in
  `include/config.h` if you want it more/less frequent.
