# ESP32_BleSerial
An easy-to-use BLE Serial library for Arduino ESP32.
The class inherits the Arduino Stream class so all the familiar Serial functions are supported.

## Installation
Add the BleSerial.h, BleSerial.cpp and ByteRingBuffer.h files to your project.

## Usage
See the Examples/main.cpp file.

## Configuration
The library can be set to toggle a pin when a device is connected or disconnected. This can be used for an LED indicator, for example. To enable this feature, set ENABLE_LED to 1 in the BleSerial.h file. The pin to toggle can be set as BLUE_LED in the same file.

The GATT UUID used by the BLE serial service may be changed in the BleSerial.h file.
