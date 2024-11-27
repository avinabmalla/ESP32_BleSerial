#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "BleSerial.h"
#include "BleBatteryService.h"

#define MIN_MTU 50 //Minimum required MTU for BLE Serial to work

#define BLE_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN // must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN

class BleSerialServer : public BLEServerCallbacks
{

public:
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

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

    bool isConnected() const;

    BLEServer *Server = nullptr;

    uint16_t getMaxTransferSize();

    void setConnectCallback(BLE_CONNECT_CALLBACK callback);

private:
    BleSerialServer();
    BleSerialServer(BleSerialServer const &other) = delete; // disable copy constructor
    void operator=(BleSerialServer const &other) = delete;  // disable assign constructor

    BLEAdvertising *pAdvertising = nullptr;
    BLE_CONNECT_CALLBACK connectCallback = nullptr;

    bool bleConnected = false;

    uint16_t peerMTU = 20;
    uint16_t maxTransferSize = 20;

    bool enableLed = false;
    int ledPin = 13;

    char bleDeviceName[80] = "BleSerial";

    bool started = false;

    int registeredSerialCount = 0;
    bool hasBatteryService = false;
};