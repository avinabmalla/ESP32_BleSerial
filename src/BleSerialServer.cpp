#include "BleSerialServer.h"

BleSerialServer::BleSerialServer()
{
}

uint16_t BleSerialServer::getMaxTransferSize()
{
    if (maxTransferSize < MIN_MTU)
    {
        peerMTU = Server->getPeerMTU(Server->getConnId()) - 5;
        maxTransferSize = peerMTU > BLE_BUFFER_SIZE ? BLE_BUFFER_SIZE : peerMTU;
    }

    return maxTransferSize;
}

void BleSerialServer::onConnect(BLEServer *pServer)
{
    bleConnected = true;
    if (enableLed)
        digitalWrite(ledPin, HIGH);
   
    if (connectCallback !=nullptr)
	{
		connectCallback(bleConnected);
	}
}

void BleSerialServer::onDisconnect(BLEServer *pServer)
{
    bleConnected = false;
    if (enableLed)
        digitalWrite(ledPin, LOW);
    
    //Reset MTU
    peerMTU = 20;
	maxTransferSize = 20;

    Server->startAdvertising();

    if (connectCallback !=nullptr)
	{
		connectCallback(bleConnected);
	}
}

void BleSerialServer::startServer(const char *name, int led_pin)
{
    ledPin = led_pin;
    if (ledPin > 0)
    {
        enableLed = true;
        pinMode(ledPin, OUTPUT);
    }
    else
    {
        enableLed = false;
    }

    strcpy(bleDeviceName, name);

    if (started)
    {
        esp_ble_gap_set_device_name(bleDeviceName);
        return;
    }

    started = true;

    BLEDevice::init(bleDeviceName);

    Server = BLEDevice::createServer();
    Server->setCallbacks(this);
}

bool BleSerialServer::isConnected() const
{
    return Server->getConnectedCount() > 0;
}

void BleSerialServer::registerSerial(BleSerial *bleSerial)
{
    BLEDevice::stopAdvertising();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(bleSerial->serialServiceUuid);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    BLEDevice::startAdvertising();

    registeredSerialCount++;
}

void BleSerialServer::registerBatteryService()
{
    hasBatteryService = true;

    BLEDevice::stopAdvertising();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BatteryServiceUUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    BLEDevice::startAdvertising();
}

void BleSerialServer::unregisterSerial(BleSerial *bleSerial)
{
    registeredSerialCount--;
    if (registeredSerialCount <= 0)
    {
        registeredSerialCount = 0;
        BLEDevice::deinit();
        started = false;
    }
}

void BleSerialServer::unregisterBatteryService()
{
    hasBatteryService = false;
}

void BleSerialServer::setConnectCallback(BLE_CONNECT_CALLBACK callback)
{
	connectCallback = callback;
}