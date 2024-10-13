#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "BleSerial.h"
#include "BleBatteryService.h"

#define MIN_MTU 50

#define BLE_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN // must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN

class BleSerialServer : public BLEServerCallbacks
{

public:
    void onConnect(BLEServer *pServer);
    void onDisconnect(BLEServer *pServer);

    static BleSerialServer &getInstance()
    {
        static BleSerialServer instance;
        return instance;
    }

    void registerSerial(BleSerial *bleSerial);
    void registerBatteryService();

    void unregisterSerial(BleSerial *bleSerial);
    void unregisterBatteryService();

    void startServer(const char *name, int led_pin = -1);

    bool isConnected();

    BLEServer *Server;

    uint16_t getMaxTransferSize();

private:
    BleSerialServer();
    BleSerialServer(BleSerialServer const &other) = delete; // disable copy constructor
    void operator=(BleSerialServer const &other) = delete;  // disable assign constructor

    BLEAdvertising *pAdvertising;

    bool bleConnected;

    uint16_t peerMTU;
    uint16_t maxTransferSize = BLE_BUFFER_SIZE;

    bool enableLed = false;
    int ledPin = 13;

    char bleDeviceName[80] = "BleSerial";

    bool started = false;

    int registeredSerialCount = 0;
    bool hasBatteryService = false;
};