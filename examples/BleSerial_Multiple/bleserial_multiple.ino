
/**
 * Bluetooth LE Multiple Serial Example
 *
 * Periodically transmits lines to two BLE serial ports.
 * Receives data from two BLE serial ports and writes to Serial.
 * Also includes a BLE Battery Service.
 * 
 * 
 * 
 * Avinab Malla
 * 13 October 2024
 **/

#include <BleSerial.h>
#include <BleBatteryService.h>
#include <esp_attr.h>
#include <esp_task_wdt.h>
#include <driver/rtc_io.h>
#include <esp_mac.h>

const int BUFFER_SIZE = 8192;
const int STACK_SIZE = 8192;

const char *BLE_SERIAL2_SERVICE_UUID = "7e400001-b5a3-f393-e0a9-e50e24dcca9e";
const char *BLE_RX2_UUID = "7e400002-b5a3-f393-e0a9-e50e24dcca9e";
const char *BLE_TX2_UUID = "7e400003-b5a3-f393-e0a9-e50e24dcca9e";

BleSerial SerialBT1;
BleSerial SerialBT2;
BleBatteryService BtBattery;

uint8_t unitMACAddress[6]; // Use MAC address in BT broadcast and display
char deviceName[20];	   // The serial string that is broadcast.

uint8_t bleReadBuffer[BUFFER_SIZE];
uint8_t serialReadBuffer[BUFFER_SIZE];

void startBluetooth()
{
	// Get unit MAC address
	esp_read_mac(unitMACAddress, ESP_MAC_WIFI_STA);

	// Convert MAC address to Bluetooth MAC (add 2): https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
	unitMACAddress[5] += 2;

	// Create device name
	sprintf(deviceName, "BleExample-%02X%02X", unitMACAddress[4], unitMACAddress[5]);

	// Init BLE Serials. The device name does not have to match for both serials. The device name of the last serial port is used.
	SerialBT1.begin(deviceName);
	SerialBT1.setTimeout(10);

	SerialBT2.begin(deviceName, BLE_SERIAL2_SERVICE_UUID, BLE_RX2_UUID, BLE_TX2_UUID);
	SerialBT2.setTimeout(10);

    //Init the battery service
	BtBattery.begin();
}

// Task for reading BLE Serial
void ReadBtTask(void *e)
{
	while (true)
	{
		if (SerialBT1.available())
		{
			auto count = SerialBT1.readBytes(bleReadBuffer, BUFFER_SIZE);
			Serial.print("BT1> ");
			Serial.write(bleReadBuffer, count);
		}

		if (SerialBT2.available())
		{
			auto count = SerialBT2.readBytes(bleReadBuffer, BUFFER_SIZE);
			Serial.print("BT2> ");
			Serial.write(bleReadBuffer, count);
		}
		delay(20);
	}
}

void setup()
{
	// Start Serial
	Serial.begin(115200);
	Serial.setRxBufferSize(BUFFER_SIZE);
	Serial.setTimeout(10);

	// Start BLE
	startBluetooth();

	// Disable watchdog timers
	disableCore0WDT();
	disableCore1WDT();
	disableLoopWDT();
	esp_task_wdt_delete(NULL);

	// Start tasks
	xTaskCreate(ReadBtTask, "ReadBtTask", STACK_SIZE, NULL, 1, NULL);
}

int battery = 100;
void loop()
{
	SerialBT1.print("Hello from Serial1: ");
	SerialBT1.println(millis());

	SerialBT2.print("Hello from Serial2: ");
	SerialBT2.println(millis());

	BtBattery.reportBatteryPercent(battery);
	battery--;
	if (battery < 0)
		battery = 100;

	delay(2000);
}
