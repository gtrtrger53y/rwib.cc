#include "ble_service.h"
#include <NimBLEDevice.h>

namespace
{
    const char *SERVICE_UUID = "5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0001";
    const char *CHAR_NOTIF_COUNT_UUID = "5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0002";
    const char *CHAR_STATUS_UUID = "5b2c0000-6f4a-4a3e-9a8a-9c3f4c9f0003";

    NimBLEServer *server = nullptr;
    NimBLECharacteristic *notifCountChar = nullptr;
    NimBLECharacteristic *statusChar = nullptr;
    volatile int lastNotifCount = -1;
    bool running = false;

    class NotifCallbacks : public NimBLECharacteristicCallbacks
    {
        void onWrite(NimBLECharacteristic *chr) override
        {
            std::string v = chr->getValue();
            if (v.size() >= 2)
            {
                // Little-endian uint16.
                lastNotifCount = (uint8_t)v[0] | ((uint8_t)v[1] << 8);
            }
            else if (v.size() == 1)
            {
                lastNotifCount = (uint8_t)v[0];
            }
        }
    };

    class ServerCallbacks : public NimBLEServerCallbacks
    {
        void onConnect(NimBLEServer *srv) override
        {
            NimBLEDevice::startAdvertising();
        }
        void onDisconnect(NimBLEServer *srv) override
        {
            NimBLEDevice::startAdvertising();
        }
    };

    NotifCallbacks notifCallbacks;
    ServerCallbacks serverCallbacks;
}

void BleService::begin()
{
    if (running)
        return;

    NimBLEDevice::init("CYD Clock");
    server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCallbacks);

    NimBLEService *service = server->createService(SERVICE_UUID);

    notifCountChar = service->createCharacteristic(
        CHAR_NOTIF_COUNT_UUID,
        NIMBLE_PROPERTY::WRITE);
    notifCountChar->setCallbacks(&notifCallbacks);

    statusChar = service->createCharacteristic(
        CHAR_STATUS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    statusChar->setValue("{}");

    service->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    NimBLEDevice::startAdvertising();

    running = true;
}

void BleService::end()
{
    if (!running)
        return;
    NimBLEDevice::deinit(true);
    server = nullptr;
    notifCountChar = nullptr;
    statusChar = nullptr;
    running = false;
}

void BleService::loop()
{
    // NimBLE runs its own task; nothing to pump here today. Present for
    // symmetry with the other modules and future use (e.g. periodic
    // status-characteristic updates).
}

int BleService::notificationCount()
{
    return lastNotifCount;
}
