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

#include "BleBufferedSerial.h"
#include "BleSerialServer.h"
#include <BLE2902.h>
using namespace std;

BleBufferedSerial::BleBufferedSerial(const int txTimeout, const int notifyDelay, int sendQueueSize)
	: txTimeout(txTimeout), notifyDelay(notifyDelay)
{
	writeMutex = xSemaphoreCreateMutex();
	flushMutex = xSemaphoreCreateMutex();
	sendQueue = xQueueCreate(sendQueueSize, sizeof(BleTxPacket));
}


void BleBufferedSerial::begin(const char* name, int led_pin)
{
	if (started)return;
	BleSerialServer::getInstance().startServer(name, led_pin);
	setupSerialService();
	BleSerialServer::getInstance().registerSerial(serialServiceUuid);
	started = true;

	char _taskName[20]; // Store random task name
	snprintf(_taskName, sizeof(_taskName), "BLEFlushTask_%04X", esp_random() & 0xFFFF);
	xTaskCreate(flushTaskEntry, _taskName, 4096, this, 1, nullptr);
}

void BleBufferedSerial::begin(const char* name, const char* service_uuid, const char* rx_uuid, const char* tx_uuid,
                              int led_pin)
{
	if (started)return;
	strcpy(rxUuid, rx_uuid);
	strcpy(txUuid, tx_uuid);
	strcpy(serialServiceUuid, service_uuid);

	BleSerialServer::getInstance().startServer(name, led_pin);
	setupSerialService();
	BleSerialServer::getInstance().registerSerial(serialServiceUuid);
	started = true;

	char _taskName[20]; // Store random task name
	snprintf(_taskName, sizeof(_taskName), "BLEFlushTask_%04X", esp_random() & 0xFFFF);
	xTaskCreate(flushTaskEntry, _taskName, 4096, this, 1, nullptr);
}

void BleBufferedSerial::end()
{
	started = false; // This stops the flush task
	while (flushTaskRunning)
		vTaskDelay(1); // Wait for the flush task to stop
	serialService->executeDelete();
	BleSerialServer::getInstance().unregisterSerial();
}

bool BleBufferedSerial::connected()
{
	return started && BleSerialServer::getInstance().isConnected();
}

int BleBufferedSerial::available()
{
	return (int)receiveBuffer.getLength();
}


int BleBufferedSerial::peek()
{
	if (receiveBuffer.getLength() == 0)
		return -1;
	return receiveBuffer.get(0);
}

int BleBufferedSerial::read()
{
	int result = receiveBuffer.pop();
	if (result == '\n')
	{
		numAvailableLines--;
	}
	return result;
}

size_t BleBufferedSerial::readBytes(uint8_t* buffer, size_t bufferSize)
{
	int i = 0;
	while (i < bufferSize && available())
	{
		buffer[i] = (uint8_t)receiveBuffer.pop();
		i++;
	}
	return i;
}


int BleBufferedSerial::sendByte(uint8_t byte)
{
	transmitBuffer.add(byte);

	if (transmitBuffer.getLength() == maxBleTransferSize())
	{
		flush();
	}
	return 1;
}


size_t BleBufferedSerial::write(uint8_t byte)
{
	if (!connected())
		return 0;

	if (xSemaphoreTake(writeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
	{
		sendByte(byte);
		xSemaphoreGive(writeMutex);
	}

	return 1;
}

size_t BleBufferedSerial::write(const uint8_t* buffer, size_t bufferSize)
{
	if (!connected())
		return 0;

	if (maxBleTransferSize() < MIN_MTU)
	{
		return 0;
	}

	size_t written = 0;
	if (xSemaphoreTake(writeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
	{
		for (int i = 0; i < bufferSize; i++)
		{
			written += sendByte(buffer[i]);
		}
		xSemaphoreGive(writeMutex);
	}

	return written;
}


void BleBufferedSerial::onWrite(BLECharacteristic* pCharacteristic)
{
	if (!started)
		return;
	if (pCharacteristic->getUUID().toString() == rxUuid)
	{
		auto value = pCharacteristic->getValue();

		for (int i = 0; i < value.length(); i++)
			receiveBuffer.add(value[i]);
	}
}

void BleBufferedSerial::setupSerialService()
{
	serialService = BleSerialServer::getInstance().Server->createService(serialServiceUuid);

	// characteristic property is what the other device does.
	rxCharacteristic = serialService->createCharacteristic(
		rxUuid, BLECharacteristic::PROPERTY_WRITE);

	txCharacteristic = serialService->createCharacteristic(
		txUuid, BLECharacteristic::PROPERTY_NOTIFY);

	txCharacteristic->addDescriptor(new BLE2902());
	rxCharacteristic->addDescriptor(new BLE2902());

	txCharacteristic->setReadProperty(true);
	rxCharacteristic->setWriteProperty(true);

	txCharacteristic->setWriteNoResponseProperty(true);
	rxCharacteristic->setWriteNoResponseProperty(true);

	rxCharacteristic->setCallbacks(this);

	serialService->start();
}


void BleBufferedSerial::setConnectCallback(BLE_CONNECT_CALLBACK callback)
{
	BleSerialServer::getInstance().setConnectCallback(callback);
}

void BleBufferedSerial::flush()
{
	if (!connected())return;

	if (transmitBuffer.getLength() <= 0)return;

	if (xSemaphoreTake(flushMutex, portMAX_DELAY) == pdTRUE)
	{
		BleTxPacket packet;
		while (packet.length < maxBleTransferSize() && transmitBuffer.getLength() > 0)
		{
			packet.data[packet.length] = (uint8_t)transmitBuffer.pop();
			packet.length++;
		}

		if (packet.length > 0)
		{
			if (xQueueSend(sendQueue, &packet, 0) != pdTRUE)
				log_e("BLE queue full, dropping packet");
		}

		xSemaphoreGive(flushMutex);
	}
}


void BleBufferedSerial::flushTaskEntry(void* pvParams)
{
	BleBufferedSerial* self = static_cast<BleBufferedSerial*>(pvParams);
	self->flushTaskRunning = true;
	self->flushTask();
	self->flushTaskRunning = false;
	vTaskDelete(NULL);
}

void BleBufferedSerial::flushTask()
{
	BleTxPacket item;
	while (started)
	{
		if (!connected())
		{
			delay(txTimeout);
			continue;
		}

		if (xQueueReceive(sendQueue, &item, pdMS_TO_TICKS(txTimeout)) == pdTRUE)
		{
			txCharacteristic->setValue(item.data, item.length);
			txCharacteristic->notify(true);
			delay(notifyDelay);
		}
		else
		{
			if (transmitBuffer.getLength() > 0)
			{
				flush();
			}
		}
	}
}
