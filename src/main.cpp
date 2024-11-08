#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h> // Required for parsing JSON-like data

#define RX_PIN D6  // ESP32-C3 pin connected to RP2040's TX pin
#define TX_PIN D7  // ESP32-C3 pin connected to RP2040's RX pin

HardwareSerial SerialPort(1); // Use UART1

const char* ssid = "ESP32C3RP2040";
const char* password = ""; // No password for the hotspot

WebServer server(80);

// Variables to store system information
String systemInfo = "{}";  // Initial empty JSON string

void handleRoot() {
  String page = "<!DOCTYPE html><html><head><title>System Monitor</title>";
  page += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  page += "<style>body {font-family: Arial, sans-serif; text-align: center; padding: 20px;}";
  page += "button {font-size: 1.2em; padding: 10px 20px; margin: 10px; width: 80px;}";
  page += "button.off {background-color: #555; color: white;}";
  page += "button.red {background-color: red; color: white;}";
  page += "button.green {background-color: green; color: white;}";
  page += "button.blue {background-color: blue; color: white;}";
  page += "#systemInfo {display: flex; flex-direction: column; align-items: center; margin-top: 20px;}";  // Center-align system info
  page += ".info-item {display: flex; justify-content: space-between; width: 100%; max-width: 400px; padding: 8px; border-bottom: 1px solid #ddd;}";
  page += ".label {font-weight: bold;}";
  page += "</style></head><body>";
  page += "<h1>Control the RP2040 NeoPixel & Monitor System</h1>";
  page += "<button class=\"off\" onclick=\"sendColor('OFF')\">OFF</button>";
  page += "<button class=\"red\" onclick=\"sendColor('RED')\">RED</button>";
  page += "<button class=\"green\" onclick=\"sendColor('GRN')\">GREEN</button>";
  page += "<button class=\"blue\" onclick=\"sendColor('BLUE')\">BLUE</button>";
  page += "<h2>System Information</h2><div id=\"systemInfo\"></div>";
  page += "<script>";
  page += "function sendColor(color) {";
  page += "  var xhr = new XMLHttpRequest();";
  page += "  xhr.open('GET', '/setcolor?color=' + color, true);";
  page += "  xhr.send();";
  page += "}";
  page += "function fetchSystemInfo() {";
  page += "  fetch('/systeminfo').then(response => response.json()).then(data => {";
  page += "    const systemInfoDiv = document.getElementById('systemInfo');";
  page += "    systemInfoDiv.innerHTML = '';";  // Clear existing content";
  page += "    for (const [key, value] of Object.entries(data)) {";
  page += "      const item = document.createElement('div');";
  page += "      item.classList.add('info-item');";
  page += "      item.innerHTML = `<span class='label'>${key}:</span> <span>${value}</span>`;";
  page += "      systemInfoDiv.appendChild(item);";
  page += "    }";
  page += "  });";
  page += "}";
  page += "setInterval(fetchSystemInfo, 1000);"; // Update every second
  page += "</script></body></html>";
  server.send(200, "text/html", page);
}

void handleSetColor() {
  if (server.hasArg("color")) {
    String color = server.arg("color");
    Serial.print("Color selected: ");
    Serial.println(color);
    // Send the command over UART to RP2040
    SerialPort.print(color + "\n");
    server.send(204, "text/plain", ""); // No content response
  } else {
    server.send(400, "text/plain", "Color not specified");
  }
}

void handleSystemInfo() {
  server.send(200, "application/json", systemInfo); // Send system info JSON
}

void setup() {
  Serial.begin(115200); // Initialize USB serial console
  SerialPort.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // Initialize UART1
  Serial.println("ESP32-C3 UART Transmitter Initialized");

  // Start Wi-Fi in AP mode with a static IP
  WiFi.softAP(ssid, password);
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("Access Point started");
  Serial.print("Static IP address: ");
  Serial.println(local_ip);

  // Set up web server
  server.on("/", handleRoot);
  server.on("/setcolor", handleSetColor);
  server.on("/systeminfo", handleSystemInfo);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Listen for data from RP2040 over UART
  if (SerialPort.available()) {
    String receivedData = SerialPort.readStringUntil('\n');
    Serial.print("Received from RP2040: ");
    Serial.println(receivedData); // Print to serial monitor

    // Check if received data is valid JSON-like format
    if (receivedData.startsWith("{") && receivedData.endsWith("}")) {
        systemInfo = receivedData;
    } else {
        Serial.println("Received data is not in the expected JSON format");
    }
  }
}
