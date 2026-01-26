/**
 * BLE Battery Service
 * Reports the device battery level to the connecting device.
 * 
 * Avinab Malla
 * 13 October 2024
 */
#include "BleBatteryService.h"

#include <BLE2902.h>

#include "BleSerialServer.h"


BLEUUID BatteryServiceUUID((uint16_t)0x180F);
BLEUUID BatteryLevelCharacteristicUUID((uint16_t)0x2A19);

void BleBatteryService::begin()
{
    auto server = BleSerialServer::getInstance().Server;
    batteryService = server->createService(BatteryServiceUUID);

    batteryCharacteristic = batteryService->createCharacteristic(
        BatteryLevelCharacteristicUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    batteryCharacteristic->addDescriptor(new BLE2902());
    batteryCharacteristic->setNotifyProperty(true);

    batteryService->start();

    BleSerialServer::getInstance().registerBatteryService();
    started = true;
}

void BleBatteryService::reportBatteryPercent(uint8_t batPercent) const
{
    if (!started)
        return;

	batteryCharacteristic->setValue(&batPercent, 1);
	batteryCharacteristic->notify();
}

void BleBatteryService::end()
{
    batteryService->executeDelete();
    BleSerialServer::getInstance().unregisterBatteryService();
    started = false;
}
