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

String systemInfoBuffer = ""; // Buffer for serial data
unsigned long lastSystemInfoUpdate = 0; // For non-blocking system info updates
const unsigned long SYSTEM_INFO_UPDATE_INTERVAL = 1000; // 1 second interval

void handleRoot() {
  String page = "<!DOCTYPE html><html><head><title>System Monitor</title>";
  page += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  page += "<style>body {font-family: Arial, sans-serif; text-align: center; padding: 20px;}";
  page += "button {font-size: 1.2em; padding: 10px 20px; margin: 10px; width: 80px;}";
  page += "button.off {background-color: #555; color: white;}";
  page += "button.red {background-color: red; color: white;}";
  page += "button.green {background-color: green; color: white;}";
  page += "button.blue {background-color: blue; color: white;}";
  page += "#systemInfo {display: flex; flex-direction: column; align-items: center; margin-top: 20px;}";
  page += ".info-item {display: flex; justify-content: space-between; width: 100%; max-width: 400px; padding: 8px; border-bottom: 1px solid #ddd;}";
  page += ".label {font-weight: bold;}";
  page += "</style></head><body>";
  page += "<h1>Control the RP2040 NeoPixel & Monitor System</h1>";
  page += "<button class=\"off\" onclick=\"sendColor('OFF')\">OFF</button>";
  page += "<button class=\"red\" onclick=\"sendColor('RED')\">RED</button>";
  page += "<button class=\"green\" onclick=\"sendColor('GREEN')\">GRN</button>";
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
  page += "    systemInfoDiv.innerHTML = '';";
  page += "    for (const [key, value] of Object.entries(data)) {";
  page += "      const item = document.createElement('div');";
  page += "      item.classList.add('info-item');";
  page += "      item.innerHTML = `<span class='label'>${key}:</span> <span>${value}</span>`;";
  page += "      systemInfoDiv.appendChild(item);";
  page += "    }";
  page += "  }).catch(error => console.log('Error fetching system info:', error));"; // Catch JSON parsing errors
  page += "}";
  page += "setInterval(fetchSystemInfo, 1000);";
  page += "</script></body></html>";
  server.send(200, "text/html", page);
}

void handleSetColor() {
  if (server.hasArg("color")) {
    String color = server.arg("color");
    Serial.print("Color selected: ");
    Serial.println(color);
    // Send the command over UART to RP2040 immediately
    SerialPort.print(color + "\n");
    server.send(204, "text/plain", "");
  } else {
    server.send(400, "text/plain", "Color not specified");
  }
}

void handleSystemInfo() {
  // Ensure valid JSON is sent in response
  if (systemInfo.startsWith("{") && systemInfo.endsWith("}")) {
    server.send(200, "application/json", systemInfo);
  } else {
    server.send(200, "application/json", "{}"); // Fallback empty JSON to avoid client errors
  }
}

void setup() {
  Serial.begin(115200); 
  SerialPort.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("ESP32-C3 UART Transmitter Initialized");

  WiFi.softAP(ssid, password);
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("Access Point started");
  Serial.print("Static IP address: ");
  Serial.println(local_ip);

  server.on("/", handleRoot);
  server.on("/setcolor", handleSetColor);
  server.on("/systeminfo", handleSystemInfo);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Non-blocking

  unsigned long currentMillis = millis();

  // Non-blocking serial read
  while (SerialPort.available()) {
    char receivedChar = SerialPort.read();
    if (receivedChar == '\n') {
      // End of JSON data, validate and update system info
      if (systemInfoBuffer.startsWith("{") && systemInfoBuffer.endsWith("}")) {
        systemInfo = systemInfoBuffer;
      }
      systemInfoBuffer = ""; // Clear buffer after processing
    } else {
      systemInfoBuffer += receivedChar; // Append character to buffer
    }
  }

  // Update system info only at intervals
  if (currentMillis - lastSystemInfoUpdate >= SYSTEM_INFO_UPDATE_INTERVAL) {
    lastSystemInfoUpdate = currentMillis;
    if (systemInfo.isEmpty()) {
      systemInfo = "{}"; // Ensure systemInfo is valid JSON if nothing received
    }
  }
}
