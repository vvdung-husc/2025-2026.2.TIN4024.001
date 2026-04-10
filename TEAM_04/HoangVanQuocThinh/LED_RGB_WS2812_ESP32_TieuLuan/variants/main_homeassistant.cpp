#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// --- 1. Cấu hình WiFi & MQTT ---
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
const char* mqtt_server = "192.168.1.100";  // IP MQTT Broker
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt_user";
const char* mqtt_pass = "mqtt_pass";

// --- 2. Cấu hình MQTT Topics ---
#define MQTT_LED_SET "esp32/led/set"
#define MQTT_LED_BRIGHTNESS "esp32/led/brightness"
#define MQTT_LED_MODE "esp32/led/mode"
#define MQTT_LED_STATE "esp32/led/state"
#define MQTT_LED_RGB "esp32/led/rgb"

// --- 3. Cấu hình LED RGB (WS2812) ---
#define LED_PIN 5
#define NUM_LEDS 60
#define BRIGHTNESS 255

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- 4. Web Server ---
WebServer server(80);

// --- 5. MQTT Client ---
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- 6. Biến điều khiển LED ---
struct LEDState {
  uint8_t r = 255;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t brightness = 255;
  int mode = 0;
  int speed = 50;
  bool isOn = true;
} ledState;

enum Mode { SOLID = 0, RAINBOW = 1, PULSE = 2, FADE = 3, STROBE = 4 };
Mode currentMode = SOLID;

unsigned long lastUpdate = 0;
unsigned long lastMqttConnect = 0;
static uint16_t offset = 0;

// --- 7. Utility Functions ---
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void rainbowEffect() {
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t hue = (i * 256 / NUM_LEDS + offset) & 255;
    strip.setPixelColor(i, Wheel(hue));
  }
  strip.show();
  offset++;
}

void pulseEffect() {
  uint8_t brightness = (sin(millis() / 500.0) + 1) * 127;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(
      (ledState.r * brightness) / 255,
      (ledState.g * brightness) / 255,
      (ledState.b * brightness) / 255
    ));
  }
  strip.show();
}

void fadeEffect() {
  uint8_t brightness = (sin(millis() / 1000.0) + 1) * 127;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(
      (ledState.r * brightness) / 255,
      (ledState.g * brightness) / 255,
      (ledState.b * brightness) / 255
    ));
  }
  strip.show();
}

void strobeEffect() {
  if ((millis() / 100) % 2 == 0) {
    setSolidColor(ledState.r, ledState.g, ledState.b);
  } else {
    setSolidColor(0, 0, 0);
  }
}

// --- 8. MQTT Functions ---
void publishLEDState() {
  if (!mqttClient.connected()) return;
  
  // Publish state
  mqttClient.publish(MQTT_LED_STATE, ledState.isOn ? "on" : "off");
  
  // Publish brightness
  mqttClient.publish(MQTT_LED_BRIGHTNESS, String(ledState.brightness).c_str());
  
  // Publish RGB as hex
  char rgbHex[7];
  sprintf(rgbHex, "%02X%02X%02X", ledState.r, ledState.g, ledState.b);
  mqttClient.publish(MQTT_LED_RGB, rgbHex);
  
  Serial.printf("LED State Published: %s, RGB=%s, Brightness=%d\n", 
                ledState.isOn ? "on" : "off", rgbHex, ledState.brightness);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char message[256];
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  
  Serial.printf("MQTT Message - Topic: %s, Message: %s\n", topic, message);
  
  if (strcmp(topic, MQTT_LED_SET) == 0) {
    // Format: rrggbb
    if (length >= 6) {
      ledState.r = strtol(String(message).substring(0, 2).c_str(), NULL, 16);
      ledState.g = strtol(String(message).substring(2, 4).c_str(), NULL, 16);
      ledState.b = strtol(String(message).substring(4, 6).c_str(), NULL, 16);
      currentMode = SOLID;
      ledState.isOn = true;
      publishLEDState();
      Serial.printf("Color set to RGB(%d, %d, %d)\n", ledState.r, ledState.g, ledState.b);
    }
  }
  else if (strcmp(topic, MQTT_LED_BRIGHTNESS) == 0) {
    ledState.brightness = atoi(message);
    strip.setBrightness(ledState.brightness);
    ledState.isOn = (ledState.brightness > 0);
    publishLEDState();
    Serial.printf("Brightness set to %d\n", ledState.brightness);
  }
  else if (strcmp(topic, MQTT_LED_MODE) == 0) {
    currentMode = (Mode)atoi(message);
    ledState.isOn = true;
    publishLEDState();
    Serial.printf("Mode set to %d\n", currentMode);
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP32-LED-" + WiFi.macAddress();
    
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" connected");
      
      // Subscribe to topics
      mqttClient.subscribe(MQTT_LED_SET);
      mqttClient.subscribe(MQTT_LED_BRIGHTNESS);
      mqttClient.subscribe(MQTT_LED_MODE);
      
      // Publish initial state
      publishLEDState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// --- 9. HTTP Handlers ---
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED RGB Control - Home Assistant Ready</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .container {
      background: white;
      border-radius: 20px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      padding: 40px;
      max-width: 500px;
      width: 100%;
    }
    h1 {
      text-align: center;
      color: #333;
      margin-bottom: 10px;
      font-size: 28px;
    }
    .subtitle {
      text-align: center;
      color: #999;
      margin-bottom: 30px;
      font-size: 12px;
    }
    .control-section {
      margin-bottom: 30px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #555;
      font-weight: 600;
      font-size: 14px;
    }
    input[type="color"] {
      width: 100%;
      height: 60px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
    }
    input[type="range"] {
      width: 100%;
      height: 6px;
      border-radius: 3px;
      background: #ddd;
      outline: none;
      -webkit-appearance: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
    }
    .mode-buttons {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-bottom: 20px;
    }
    button {
      padding: 12px 20px;
      border: none;
      border-radius: 10px;
      background: #667eea;
      color: white;
      cursor: pointer;
      font-weight: 600;
      transition: all 0.3s;
      font-size: 14px;
    }
    button:hover {
      background: #764ba2;
      transform: translateY(-2px);
    }
    .status {
      text-align: center;
      padding: 15px;
      background: #f0f0f0;
      border-radius: 10px;
      margin-top: 20px;
      font-size: 14px;
      color: #666;
    }
    .status.success {
      background: #d4edda;
      color: #155724;
    }
    .mqtt-info {
      background: #e3f2fd;
      border-left: 4px solid #2196F3;
      padding: 12px;
      border-radius: 5px;
      margin-bottom: 20px;
      font-size: 12px;
      color: #1565c0;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎨 LED RGB Control</h1>
    <p class="subtitle">Connected to Home Assistant via MQTT</p>
    
    <div class="mqtt-info">
      📡 MQTT Active - Available in Home Assistant
    </div>
    
    <div class="control-section">
      <label>Chế độ hiển thị</label>
      <div class="mode-buttons">
        <button onclick="setMode(0)">Solid</button>
        <button onclick="setMode(1)">Rainbow</button>
        <button onclick="setMode(2)">Pulse</button>
        <button onclick="setMode(3)">Fade</button>
        <button onclick="setMode(4)">Strobe</button>
        <button onclick="setMode(-1)">Tắt</button>
      </div>
    </div>

    <div class="control-section" id="color-section">
      <label>Chọn màu sắc</label>
      <input type="color" id="colorPicker" value="#ff0000" onchange="updateColor()">
    </div>

    <div class="control-section">
      <label>Độ sáng: <span id="brightnessValue">255</span></label>
      <input type="range" id="brightness" min="0" max="255" value="255" oninput="updateBrightness()">
    </div>

    <div class="status" id="status">Ready...</div>
  </div>

  <script>
    const API_URL = '/api';
    
    function setMode(mode) {
      const payload = mode === -1 
        ? {} 
        : { mode: mode };
      
      const endpoint = mode === -1 ? '/off' : '/mode';
      
      fetch(API_URL + endpoint, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
    }

    function updateColor() {
      const color = document.getElementById('colorPicker').value;
      const r = parseInt(color.substr(1,2), 16);
      const g = parseInt(color.substr(3,2), 16);
      const b = parseInt(color.substr(5,2), 16);
      
      fetch(API_URL + '/color', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ r: r, g: g, b: b })
      });
    }

    function updateBrightness() {
      const brightness = document.getElementById('brightness').value;
      document.getElementById('brightnessValue').textContent = brightness;
      
      fetch(API_URL + '/brightness', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ brightness: parseInt(brightness) })
      });
    }
  </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleAPIColor() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    ledState.r = doc["r"];
    ledState.g = doc["g"];
    ledState.b = doc["b"];
    currentMode = SOLID;
    ledState.isOn = true;
    
    publishLEDState();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPIBrightness() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    ledState.brightness = doc["brightness"];
    strip.setBrightness(ledState.brightness);
    ledState.isOn = (ledState.brightness > 0);
    
    publishLEDState();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPIMode() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    currentMode = (Mode)doc["mode"];
    ledState.isOn = true;
    
    publishLEDState();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPIOff() {
  if (server.method() == HTTP_POST) {
    strip.clear();
    strip.show();
    ledState.isOn = false;
    
    publishLEDState();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

// --- 10. Setup ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 LED RGB Control with MQTT ===");
  Serial.println("Compatible with Home Assistant");
  
  // Khởi tạo LED strip
  strip.begin();
  strip.show();
  strip.setBrightness(ledState.brightness);
  
  // Kết nối WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  Serial.println("MQTT Configured");

  // Setup HTTP routes
  server.on("/", handleRoot);
  server.on("/api/color", handleAPIColor);
  server.on("/api/brightness", handleAPIBrightness);
  server.on("/api/mode", handleAPIMode);
  server.on("/api/off", handleAPIOff);
  
  server.begin();
  Serial.println("HTTP server started");
  
  // Hiệu ứng khởi động
  setSolidColor(0, 255, 0);
  delay(500);
  setSolidColor(0, 0, 0);
}

// --- 11. Loop ---
void loop() {
  server.handleClient();
  
  // Reconnect MQTT if needed
  if (!mqttClient.connected()) {
    if (millis() - lastMqttConnect > 5000) {
      reconnectMQTT();
      lastMqttConnect = millis();
    }
  } else {
    mqttClient.loop();
  }
  
  // Cập nhật LED theo mode
  if (millis() - lastUpdate > (260 - ledState.speed)) {
    if (ledState.isOn) {
      switch (currentMode) {
        case SOLID:
          setSolidColor(ledState.r, ledState.g, ledState.b);
          break;
        case RAINBOW:
          rainbowEffect();
          break;
        case PULSE:
          pulseEffect();
          break;
        case FADE:
          fadeEffect();
          break;
        case STROBE:
          strobeEffect();
          break;
      }
    }
    lastUpdate = millis();
  }
}
