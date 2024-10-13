/**
 * BLE Battery Service
 * Reports the device battery level to the connecting device.
 * 
 * Avinab Malla
 * 13 October 2024
 */


#pragma once
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "ByteRingBuffer.h"

extern BLEUUID BatteryServiceUUID;
extern BLEUUID BatteryLevelCharacteristicUUID;


class BleBatteryService
{

public:
    BleBatteryService() {};
    void begin();
    void reportBatteryPercent(uint8_t batPercent);
    void end();

private:
    BLEService *batteryService;
    BLECharacteristic *batteryCharacteristic;
    bool started = false;

};