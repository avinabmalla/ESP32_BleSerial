#include "BleSerial.h"
using namespace std;

bool BleSerial::connected()
{
	return Server->getConnectedCount() > 0;
}

void BleSerial::onConnect(BLEServer *pServer)
{
	bleConnected = true;
	if (enableLed)
		digitalWrite(ledPin, HIGH);
}

void BleSerial::onDisconnect(BLEServer *pServer)
{
	bleConnected = false;
	if (enableLed)
		digitalWrite(ledPin, LOW);
	Server->startAdvertising();
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
		buffer[i] = (uint8_t) this->receiveBuffer.pop();
		i++;
	}
	return i;
}

int BleSerial::peek()
{
	if (this->receiveBuffer.getLength() == 0)
		return -1;
	return this->receiveBuffer.get(0);
}

int BleSerial::available()
{
	return this->receiveBuffer.getLength();
}

size_t BleSerial::print(const char *str)
{
	if (Server->getConnectedCount() <= 0)
	{
		return 0;
	}

	size_t written = 0;
	for (size_t i = 0; str[i] != '\0'; i++)
	{
		written += this->write(str[i]);
	}
	flush();
	return written;
}

size_t BleSerial::write(const uint8_t *buffer, size_t bufferSize)
{
	if (Server->getConnectedCount() <= 0)
	{
		return 0;
	}

	if (maxTransferSize < MIN_MTU)
	{
		int oldTransferSize = maxTransferSize;
		peerMTU = Server->getPeerMTU(Server->getConnId()) - 5;
		maxTransferSize = peerMTU > BLE_BUFFER_SIZE ? BLE_BUFFER_SIZE : peerMTU;

		if (maxTransferSize != oldTransferSize)
		{
			log_e("Max BLE transfer size set to %u", maxTransferSize);
		}
	}

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

size_t BleSerial::write(uint8_t byte)
{
	if (Server->getConnectedCount() <= 0)
	{
		return 0;
	}
	this->transmitBuffer[this->transmitBufferLength] = byte;
	this->transmitBufferLength++;
	if (this->transmitBufferLength == maxTransferSize)
	{
		flush();
	}
	return 1;
}

void BleSerial::flush()
{
	if (this->transmitBufferLength > 0)
	{
		TxCharacteristic->setValue(this->transmitBuffer, this->transmitBufferLength);
		this->transmitBufferLength = 0;
	}
	this->lastFlushTime = millis();
	TxCharacteristic->notify(true);
}

void BleSerial::begin(const char *name, bool enable_led, int led_pin)
{
	enableLed = enable_led;
	ledPin = led_pin;

	if (enableLed)
	{
		pinMode(ledPin, OUTPUT);
	}

	ConnectedDeviceCount = 0;
	BLEDevice::init(name);

	Server = BLEDevice::createServer();
	Server->setCallbacks(this);

	SetupSerialService();

	pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(BLE_SERIAL_SERVICE_UUID);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06);
	pAdvertising->setMinPreferred(0x12);
	pAdvertising->start();
}

void BleSerial::end()
{
	BLEDevice::deinit();
}

void BleSerial::onWrite(BLECharacteristic *pCharacteristic)
{
	if (pCharacteristic->getUUID().toString() == BLE_RX_UUID)
	{
		auto value = pCharacteristic->getValue();

		for (int i = 0; i < value.length(); i++)
			receiveBuffer.add(value[i]);
	}
}

void BleSerial::SetupSerialService()
{
	// characteristic property is what the other device does.

	SerialService = Server->createService(BLE_SERIAL_SERVICE_UUID);

	RxCharacteristic = SerialService->createCharacteristic(
		BLE_RX_UUID, BLECharacteristic::PROPERTY_WRITE);

	TxCharacteristic = SerialService->createCharacteristic(
		BLE_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);

	TxCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED);
	RxCharacteristic->setAccessPermissions(ESP_GATT_PERM_WRITE_ENCRYPTED);

	TxCharacteristic->addDescriptor(new BLE2902());
	RxCharacteristic->addDescriptor(new BLE2902());

	TxCharacteristic->setReadProperty(true);
	RxCharacteristic->setWriteProperty(true);
	RxCharacteristic->setCallbacks(this);
	SerialService->start();
}

BleSerial::BleSerial()
{
}
