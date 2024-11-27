/**
 * BLE Battery Service
 * Reports the device battery level to the connecting device.
 * 
 * Avinab Malla
 * 13 October 2024
 */


#pragma once
#include <Arduino.h>
#include <BLEServer.h>

extern BLEUUID BatteryServiceUUID;
extern BLEUUID BatteryLevelCharacteristicUUID;


class BleBatteryService
{

public:
    BleBatteryService() {};
    void begin();
    void reportBatteryPercent(uint8_t batPercent) const;
    void end();

private:
    BLEService *batteryService = nullptr;
    BLECharacteristic *batteryCharacteristic = nullptr;
    bool started = false;

};