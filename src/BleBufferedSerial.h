/**
* BLE Buffered Serial
*
* A buffered, thread-safe BLE Serial implementation
* that groups outgoing data into larger packets and
* avoids multiple BLE notify calls.
*
* Can be used as a drop-in replacement for BleSerial,
* but has a higher memory footprint.
*
* Avinab Malla
* 6 October 2025
*/

#pragma once
#include <Arduino.h>

#include "BleSerialConstants.h"
#include "ByteRingBuffer.h"

struct BleTxPacket
{
	uint8_t data[BLE_MAX_PACKET_SIZE];
	size_t length = 0;
};

class BleBufferedSerial : public BLECharacteristicCallbacks, public Stream
{
public:
	BleBufferedSerial(int txTimeout = DEFAULT_BLE_TX_TIMEOUT, int notifyDelay = DEFAULT_BLE_TX_NOTIFY_DELAY, int sendQueueSize = DEFAULT_BLE_SEND_QUEUE_SIZE );

	void begin(const char *name, int led_pin = -1);
	void begin(const char *name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, int led_pin = -1);

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
	BLEService *serialService = nullptr;

	// Serial Characteristics
	BLECharacteristic *txCharacteristic = nullptr;
	BLECharacteristic *rxCharacteristic = nullptr;

	char serialServiceUuid[40] = NORDIC_SERVICE_UUID;
	char rxUuid[40] = NORDIC_RX_UUID;
	char txUuid[40] = NORDIC_TX_UUID;

	// BLECharacteristicCallbacks
	void onWrite(BLECharacteristic* pCharacteristic) override;

	void setConnectCallback(BLE_CONNECT_CALLBACK callback);

	bool isStarted() const { return started;}

	//Settings
	uint16_t txTimeout = DEFAULT_BLE_TX_TIMEOUT; //Flushes buffer with existing contents after this
	uint16_t notifyDelay = DEFAULT_BLE_TX_NOTIFY_DELAY; //Delay interval between TX notify

private:
	BleBufferedSerial(BleBufferedSerial const &other) = delete;		 // disable copy constructor
	void operator=(BleBufferedSerial const &other) = delete; // disable assign constructor

	ByteRingBuffer<BLE_RX_BUFFER_SIZE> receiveBuffer;
	size_t numAvailableLines = 0;

	ByteRingBuffer<BLE_MAX_PACKET_SIZE + 1> transmitBuffer;

	void setupSerialService();
	bool started = false;
	bool flushTaskRunning = false;

	int sendByte(uint8_t byte);

	QueueHandle_t sendQueue;
	void flushTask();
	static void flushTaskEntry(void *pvParams);

	SemaphoreHandle_t writeMutex;
	SemaphoreHandle_t flushMutex;
};
