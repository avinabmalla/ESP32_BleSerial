/**
* Bluetooth LE Serial Bridge Example
* 
* Creates a bridge between the default serial port and a Bluetooth LE Serial port.
* Data received from BLE is transferred to the serial port and 
* data receivedfrom serial port is transferred to BLE.
*
* Avinab Malla
* 28 December 2022
**/


#include <BleSerial.h>
#include <esp_attr.h>
#include <esp_task_wdt.h>
#include <driver/rtc_io.h>
#include "soc/rtc_wdt.h"

const int BUFFER_SIZE = 8192;

BleSerial SerialBT;

uint8_t unitMACAddress[6];  // Use MAC address in BT broadcast and display
char deviceName[20];        // The serial string that is broadcast.


uint8_t bleReadBuffer[BUFFER_SIZE];
uint8_t serialReadBuffer[BUFFER_SIZE];

void startBluetooth() {
  // Get unit MAC address
  esp_read_mac(unitMACAddress, ESP_MAC_WIFI_STA);
  
  // Convert MAC address to Bluetooth MAC (add 2): https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
  unitMACAddress[5] += 2;                                                          
  
  //Create device name
  sprintf(deviceName, "BleBridge-%02X%02X", unitMACAddress[4], unitMACAddress[5]); 

  //Init BLE Serial
  SerialBT.begin(deviceName);
  SerialBT.setTimeout(10);
}

//Task for reading Serial Port
void ReadSerialTask(void *e) {
  while (true) {
    if (Serial.available()) {
      auto count = Serial.readBytes(serialReadBuffer, BUFFER_SIZE);
      SerialBT.write(serialReadBuffer, count);
    }
    delay(20);
  }
}

//Task for reading BLE Serial
void ReadBtTask(void *e) {
  while (true) {
    if (SerialBT.available()) {
      auto count = SerialBT.readBytes(bleReadBuffer, BUFFER_SIZE);
      Serial.write(bleReadBuffer, count);
    }
    delay(20);
  }
}


void setup() {
  //Start Serial
  Serial.begin(115200);
  Serial.setRxBufferSize(BUFFER_SIZE);
  Serial.setTimeout(10);

  //Start BLE
  startBluetooth();

  //Disable watchdog timers
  disableCore0WDT();
  disableCore1WDT();
  disableLoopWDT();
  esp_task_wdt_delete(NULL);
  rtc_wdt_protect_off();
  rtc_wdt_disable();

  //Start tasks
  xTaskCreate(ReadSerialTask, "ReadSerialTask", 10240, NULL, 1, NULL);
  xTaskCreate(ReadBtTask, "ReadBtTask", 10240, NULL, 1, NULL);
}

void loop() {
  //This task is not used
  vTaskDelete(NULL);
}
 
