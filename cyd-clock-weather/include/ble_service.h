#pragma once
#include <Arduino.h>

// Optional BLE GATT service. Exposes:
//  - Notification Count (write, uint16 LE): a phone-side bridge
//    (automation app, shortcut, or nRF Connect for testing) writes the
//    current unread-notification count here.
//  - Status (read/notify, JSON string): reports WiFi + weather state so a
//    companion app can display it without polling the device's screen.
//
// There's no universal, permission-free way to read a phone's live
// notification count over BLE (iOS ANCS needs bonding + Apple's notification
// protocol, Android exposes nothing standard), so this firmware takes
// notification counts as input from whatever bridge you choose to run on
// the phone rather than trying to read them itself.
namespace BleService
{
    void begin();
    void end();
    void loop();

    // Latest count received from the phone-side bridge, or -1 if none has
    // connected/written yet this boot.
    int notificationCount();
}
