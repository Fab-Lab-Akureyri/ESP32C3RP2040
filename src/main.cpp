#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

#define RX_PIN D6  // ESP32-C3 pin connected to RP2040's TX pin
#define TX_PIN D7  // ESP32-C3 pin connected to RP2040's RX pin

HardwareSerial SerialPort(1); // Use UART1

const char* ssid = "ESP32_Hotspot";
const char* password = ""; // No password for the hotspot

WebServer server(80);

void handleRoot() {
  String page = "<!DOCTYPE html><html><head><title>NeoPixel Control</title></head><body>";
  page += "<h1>Control the RP2040 NeoPixel</h1>";
  page += "<form action=\"/setcolor\" method=\"get\">";
  page += "<label for=\"color\">Choose a color:</label>";
  page += "<select name=\"color\" id=\"color\">";
  page += "<option value=\"OFF\">OFF</option>";
  page += "<option value=\"RED\">RED</option>";
  page += "<option value=\"GREEN\">GREEN</option>";
  page += "<option value=\"BLUE\">BLUE</option>";
  page += "</select>";
  page += "<input type=\"submit\" value=\"Set Color\">";
  page += "</form></body></html>";
  server.send(200, "text/html", page);
}

void handleSetColor() {
  if (server.hasArg("color")) {
    String color = server.arg("color");
    Serial.print("Color selected: ");
    Serial.println(color);
    // Send the command over UART to RP2040
    SerialPort.print(color + "\n");
    server.sendHeader("Location", "/"); // Redirect back to root
    server.send(303);
  } else {
    server.send(400, "text/plain", "Color not specified");
  }
}

void setup() {
  Serial.begin(115200); // Initialize USB serial console
  SerialPort.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // Initialize UART1
  Serial.println("ESP32-C3 UART Transmitter Initialized");

  // Start Wi-Fi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up web server
  server.on("/", handleRoot);
  server.on("/setcolor", handleSetColor);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Optional: Handle any UART responses from RP2040
  if (SerialPort.available()) {
    String receivedData = SerialPort.readStringUntil('\n');
    Serial.print("Received from RP2040: ");
    Serial.println(receivedData);
  }
}
