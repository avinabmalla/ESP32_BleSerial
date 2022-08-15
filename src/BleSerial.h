#pragma once
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "ByteRingBuffer.h"


#define BLE_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN //must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN
#define MIN_MTU 50


class BleSerial : public BLECharacteristicCallbacks, public BLEServerCallbacks, public Stream
{
public:
	BleSerial();

	void begin(const char *name, bool enable_led = false, int led_pin = 13);
	void end();
	void onWrite(BLECharacteristic *pCharacteristic);
	int available();
	int read();
	size_t readBytes(uint8_t *buffer, size_t bufferSize);
	int peek();
	size_t write(uint8_t byte);
	void flush();
	size_t write(const uint8_t *buffer, size_t bufferSize);
	size_t print(const char *value);
	void onConnect(BLEServer *pServer);
	void onDisconnect(BLEServer *pServer);

	bool connected();

	BLEServer *Server;

	BLEAdvertising *pAdvertising;
	//BLESecurity *pSecurity;

	//Services
	BLEService *SerialService;

	//Serial Characteristics
	BLECharacteristic *TxCharacteristic;
	BLECharacteristic *RxCharacteristic;

	bool enableLed = false;
	int ledPin = 13;
protected:
	size_t transmitBufferLength;
	bool bleConnected;

private:
	BleSerial(BleSerial const &other) = delete;		 // disable copy constructor
	void operator=(BleSerial const &other) = delete; // disable assign constructor

	ByteRingBuffer<BLE_BUFFER_SIZE> receiveBuffer;
	size_t numAvailableLines;

	unsigned long long lastFlushTime;
	uint8_t transmitBuffer[BLE_BUFFER_SIZE];

	int ConnectedDeviceCount;
	void SetupSerialService();

	uint16_t peerMTU;
	uint16_t maxTransferSize = BLE_BUFFER_SIZE;

	bool checkMTU();
	/*
	Bluetooth LE GATT UUIDs for the Nordic UART profile
	Change UUID here if required
	*/
	const char *BLE_SERIAL_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
	const char *BLE_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
	const char *BLE_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

	bool started = false;
};
