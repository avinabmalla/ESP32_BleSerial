#include "BleSerial.h"
#include "BleSerialServer.h"
using namespace std;

void BleSerial::begin(const char *name, int led_pin)
{
	BleSerialServer::getInstance().startServer(name, led_pin);
	setupSerialService();
	BleSerialServer::getInstance().registerSerial(this);

	started = true;
}

void BleSerial::begin(const char *name, const char *service_uuid, const char *rx_uuid, const char *tx_uuid, int led_pin)
{
	strcpy(rxUuid, rx_uuid);
	strcpy(txUuid, tx_uuid);
	strcpy(serialServiceUuid, service_uuid);

	BleSerialServer::getInstance().startServer(name, led_pin);
	setupSerialService();
	BleSerialServer::getInstance().registerSerial(this);

	started = true;
}

void BleSerial::end()
{
	serialService->executeDelete();
	BleSerialServer::getInstance().unregisterSerial(this);
	started = false;
}

bool BleSerial::connected()
{
	return started && BleSerialServer::getInstance().isConnected();
}

int BleSerial::available()
{
	return this->receiveBuffer.getLength();
}


int BleSerial::peek()
{
	if (this->receiveBuffer.getLength() == 0)
		return -1;
	return this->receiveBuffer.get(0);
}

int BleSerial::read()
{
	int result = this->receiveBuffer.pop();
	if (result == '\n')
	{
		this->numAvailableLines--;
	}
	return result;
}

size_t BleSerial::readBytes(uint8_t *buffer, size_t bufferSize)
{
	int i = 0;
	while (i < bufferSize && available())
	{
		buffer[i] = (uint8_t)this->receiveBuffer.pop();
		i++;
	}
	return i;
}


size_t BleSerial::write(uint8_t byte)
{
	if (!connected())
		return 0;

	this->transmitBuffer[this->transmitBufferLength] = byte;
	this->transmitBufferLength++;

	int maxTransferSize = BleSerialServer::getInstance().getMaxTransferSize();
	if (this->transmitBufferLength == maxTransferSize)
	{
		flush();
	}
	return 1;
}

size_t BleSerial::write(const uint8_t *buffer, size_t bufferSize)
{
	if (!connected())
		return 0;

	int maxTransferSize = BleSerialServer::getInstance().getMaxTransferSize();
	if (maxTransferSize < MIN_MTU)
	{
		return 0;
	}

	size_t written = 0;
	for (int i = 0; i < bufferSize; i++)
	{
		written += this->write(buffer[i]);
	}
	flush();
	return written;
}


void BleSerial::flush()
{
	if (!connected())
		return;

	if (this->transmitBufferLength > 0)
	{
		txCharacteristic->setValue(this->transmitBuffer, this->transmitBufferLength);
		this->transmitBufferLength = 0;
	}
	this->lastFlushTime = millis();
	txCharacteristic->notify(true);
}

void BleSerial::onWrite(BLECharacteristic *pCharacteristic)
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

void BleSerial::setupSerialService()
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

BleSerial::BleSerial()
{
}

void BleSerial::setConnectCallback(BLE_CONNECT_CALLBACK callback)
{
	BleSerialServer::getInstance().setConnectCallback(callback);
}