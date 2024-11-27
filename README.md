# ESP32_BleSerial
An easy-to-use BLE Serial library for Arduino ESP32.
The class inherits the Arduino Stream class so all the familiar Serial functions are supported.

## Installation
Add the files in the src folder to your project, or install using the Arduino library manager.

## Features
- Create multiple BLE Serial ports with custom service and characteristics.
- A BLE Battery service to report battery level

## Usage
The following examples are provided:
- BleSerial_Hello: Creates a BLE Serial and echoes "Hello!" every second.
- BleSerial_Callback: Shows how to use the connect/disconnect callback.
- BleSerial_Bridge: Creates a serial bridge between the ESP32 Serial port and Bluetooth LE.
- BleSerial_Multiple: Creates multiple BLE Serial ports using a user provided service UUID. Echoes to both serial port every 2 seconds, and reads both BLE serial ports. Also includes a battery service.

## Configuration
The library can be set to toggle a pin when a device is connected or disconnected. This can be used for an LED indicator, for example. To enable this feature, pass the LED pin to the begin() function.

The library uses the Nordic BLE Serial profile by default. This can be changed by passing the service, RX and TX characteristic UUIDs to the begin() function. Make sure to not create multiple BLE serials with the same UUIDs.
