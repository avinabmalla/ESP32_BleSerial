/**
* Bluetooth LE Serial Simple Example
* 
* Outputs the text "Hello!" to the Bluetooth LE Serial port every second.
*
* Avinab Malla
* 24 July 2022
**/

#include <BleSerial.h>

BleSerial ble;

void setup()
{
	//Start the BLE Serial
	//Enter the Bluetooth name here
	ble.begin("BleSerialTest");
}

void loop()
{
	//The usage is similar to Serial
	ble.println("Hello!");
	delay(1000);
}