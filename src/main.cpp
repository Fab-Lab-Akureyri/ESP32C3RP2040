#include <Arduino.h>
#include <Wire.h>

#include <HardwareSerial.h>

#define RX_PIN D6  // ESP32-C3 pin connected to RP2040's TX pin
#define TX_PIN D7  // ESP32-C3 pin connected to RP2040's RX pin

HardwareSerial SerialPort(1); // Use UART1 (UART_NUM_1)

void setup() {
  Serial.begin(115200);        // Initialize USB serial console
  SerialPort.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);  // Initialize UART1
  Serial.println("ESP32-C3 UART Receiver Initialized");
}

void loop() {
  if (SerialPort.available()) {
    String receivedData = SerialPort.readStringUntil('\n');
    Serial.print("Received Data: ");
    Serial.println(receivedData);
  }
}
