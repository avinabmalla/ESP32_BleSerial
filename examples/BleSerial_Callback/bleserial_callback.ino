/**
* Bluetooth LE Serial Connect/Disconnect Callback Example
* 
* Outputs the text "Hello!" to the Bluetooth LE Serial port every second.
* Outputs "Connected!" or "Disconnected" on the UART serial port
*
* Avinab Malla
* 27 November 2024
**/

#include <BleSerial.h>

BleSerial ble;

void bleConnectCallback(bool connected){
    if(connected){
        Serial.println("Connected!");
    }
    else{
        Serial.println("Disconnected!");
    }
}

void setup()
{
    //Start Serial
    Serial.begin(115200);

	//Start the BLE Serial
	//Enter the Bluetooth name here
	ble.begin("BleSerialTest");

    //Set Callback
    ble.setConnectCallback(bleConnectCallback);
}

void loop()
{
	//The usage is similar to Serial
	ble.println("Hello!");
	delay(1000);
}