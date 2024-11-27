#pragma once
#include <Arduino.h>

#include <BLEUtils.h>
#include <BLE2902.h>
#include "ByteRingBuffer.h"

#define BLE_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN // must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN
#define RX_BUFFER_SIZE 4096

#define NORDIC_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
#define NORDIC_RX_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
#define NORDIC_TX_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

typedef void (*BLE_CONNECT_CALLBACK)(bool connected);

class BleSerial : public BLECharacteristicCallbacks, public Stream
{
public:
	BleSerial();

	void begin(const char *name, int led_pin = 13);
	void begin(const char *name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, int led_pin = 13);

	void end();

	bool connected();

	int available() override;
	int peek() override;

	int read() override;
	size_t readBytes(uint8_t* buffer, size_t bufferSize) override;

	size_t write(uint8_t byte) override;
	size_t write(const uint8_t* buffer, size_t bufferSize) override;

	void flush() override;

	// Services
	BLEService *serialService;

	// Serial Characteristics
	BLECharacteristic *txCharacteristic;
	BLECharacteristic *rxCharacteristic;

	char serialServiceUuid[40] = NORDIC_SERVICE_UUID;
	char rxUuid[40] = NORDIC_RX_UUID;
	char txUuid[40] = NORDIC_TX_UUID;

	// BLECharacteristicCallbacks
	void onWrite(BLECharacteristic* pCharacteristic) override;

	void setConnectCallback(BLE_CONNECT_CALLBACK callback);

	bool isStarted() const { return started;}
	
protected:
	size_t transmitBufferLength;
	bool bleConnected;

private:
	BleSerial(BleSerial const &other) = delete;		 // disable copy constructor
	void operator=(BleSerial const &other) = delete; // disable assign constructor

	ByteRingBuffer<RX_BUFFER_SIZE> receiveBuffer;
	size_t numAvailableLines;

	unsigned long long lastFlushTime;
	uint8_t transmitBuffer[BLE_BUFFER_SIZE];

	void setupSerialService();
	bool started = false;
};
