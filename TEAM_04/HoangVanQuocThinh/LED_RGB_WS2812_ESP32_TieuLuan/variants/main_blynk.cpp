#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// --- 1. Cấu hình Blynk ---
char auth[] = "YourBlynkAuthToken";  // Lấy từ Blynk App
char ssid[] = "Your_SSID";
char pass[] = "Your_Password";

// --- 2. Cấu hình LED RGB (WS2812) ---
#define LED_PIN 5
#define NUM_LEDS 60
#define BRIGHTNESS 255

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- 3. Web Server ---
WebServer server(80);

// --- 4. Biến điều khiển LED ---
struct LEDState {
  uint8_t r = 255;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t brightness = 255;
  int mode = 0;
  int speed = 50;
} ledState;

enum Mode { SOLID = 0, RAINBOW = 1, PULSE = 2, FADE = 3, STROBE = 4 };
Mode currentMode = SOLID;

unsigned long lastUpdate = 0;
static uint16_t offset = 0;

// --- 5. Blynk Virtual Pins ---
// V0: RGB Color Selector
// V1: Brightness Slider (0-255)
// V2: Mode Selector (0-4)
// V3: Speed Slider (10-200)
// V4: Status Display

// --- 6. Utility Functions ---
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

// --- 7. Blynk Virtual Pin Handlers ---

// Blynk Virtual Pin V0 - Color Selector
BLYNK_WRITE(V0) {
  String colorHex = param[0].asStr();  // Ví dụ: "FF0000"
  
  ledState.r = strtol(colorHex.substring(0, 2).c_str(), NULL, 16);
  ledState.g = strtol(colorHex.substring(2, 4).c_str(), NULL, 16);
  ledState.b = strtol(colorHex.substring(4, 6).c_str(), NULL, 16);
  currentMode = SOLID;
  
  Blynk.virtualWrite(V4, "Color set to RGB(" + String(ledState.r) + ", " + 
                         String(ledState.g) + ", " + String(ledState.b) + ")");
}

// Blynk Virtual Pin V1 - Brightness
BLYNK_WRITE(V1) {
  ledState.brightness = param.asInt();
  strip.setBrightness(ledState.brightness);
  Blynk.virtualWrite(V4, "Brightness: " + String(ledState.brightness));
}

// Blynk Virtual Pin V2 - Mode
BLYNK_WRITE(V2) {
  int mode = param.asInt();
  currentMode = (Mode)mode;
  
  String modeNames[] = {"Solid Color", "Rainbow", "Pulse", "Fade", "Strobe"};
  Blynk.virtualWrite(V4, "Mode: " + modeNames[mode]);
}

// Blynk Virtual Pin V3 - Speed
BLYNK_WRITE(V3) {
  ledState.speed = param.asInt();
  Blynk.virtualWrite(V4, "Speed: " + String(ledState.speed));
}

// Blynk Button V5 - Off
BLYNK_WRITE(V5) {
  if (param.asInt()) {
    strip.clear();
    strip.show();
    Blynk.virtualWrite(V4, "LED turned OFF");
  }
}

// Twitter Integration (if connected)
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V4, "LED System Online");
}

// --- 8. HTTP Handlers (for Web Interface) ---
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED RGB Control</title>
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
      margin-bottom: 30px;
      font-size: 28px;
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
    input[type="range"]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
    }
    .value-display {
      text-align: center;
      color: #667eea;
      font-weight: bold;
      margin-top: 8px;
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
    button.active {
      background: #764ba2;
      box-shadow: 0 5px 15px rgba(118, 75, 162, 0.4);
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
    .info-box {
      background: #e7f3ff;
      border-left: 4px solid #2196F3;
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
      font-size: 13px;
      color: #1565c0;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎨 LED RGB Control</h1>
    
    <div class="info-box">
      💡 Được điều khiển bằng Web Interface hoặc Blynk App
    </div>
    
    <div class="control-section">
      <label>Chế độ hiển thị</label>
      <div class="mode-buttons">
        <button onclick="setMode(0)" id="btn-solid">Solid Color</button>
        <button onclick="setMode(1)" id="btn-rainbow">Rainbow</button>
        <button onclick="setMode(2)" id="btn-pulse">Pulse</button>
        <button onclick="setMode(3)" id="btn-fade">Fade</button>
        <button onclick="setMode(4)" id="btn-strobe">Strobe</button>
        <button onclick="setMode(-1)" id="btn-off">Tắt</button>
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

    <div class="control-section" id="speed-section" style="display:none;">
      <label>Tốc độ: <span id="speedValue">50</span></label>
      <input type="range" id="speed" min="10" max="200" value="50" oninput="updateSpeed()">
    </div>

    <div class="status" id="status">Ready...</div>
  </div>

  <script>
    const API_URL = '/api';
    
    function setMode(mode) {
      if (mode === -1) {
        fetch(API_URL + '/off', { method: 'POST' })
          .then(() => updateStatus('LED tắt', 'success'))
          .catch(() => updateStatus('Lỗi: Không thể kết nối'));
        return;
      }
      
      fetch(API_URL + '/mode', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mode: mode })
      })
      .then(() => {
        updateStatus('Thay đổi chế độ thành công', 'success');
        document.getElementById('color-section').style.display = mode === 0 ? 'block' : 'none';
        document.getElementById('speed-section').style.display = mode > 0 ? 'block' : 'none';
      })
      .catch(() => updateStatus('Lỗi: Không thể kết nối'));
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
      })
      .then(() => updateStatus('Màu sắc được cập nhật', 'success'))
      .catch(() => updateStatus('Lỗi: Không thể kết nối'));
    }

    function updateBrightness() {
      const brightness = document.getElementById('brightness').value;
      document.getElementById('brightnessValue').textContent = brightness;
      
      fetch(API_URL + '/brightness', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ brightness: parseInt(brightness) })
      })
      .catch(() => updateStatus('Lỗi: Không thể kết nối'));
    }

    function updateSpeed() {
      const speed = document.getElementById('speed').value;
      document.getElementById('speedValue').textContent = speed;
      
      fetch(API_URL + '/speed', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ speed: parseInt(speed) })
      })
      .catch(() => updateStatus('Lỗi: Không thể kết nối'));
    }

    function updateStatus(msg, type = 'info') {
      const status = document.getElementById('status');
      status.textContent = msg;
      status.className = 'status ' + type;
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
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPIMode() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    currentMode = (Mode)doc["mode"];
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPIOff() {
  if (server.method() == HTTP_POST) {
    strip.clear();
    strip.show();
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleAPISpeed() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    ledState.speed = doc["speed"];
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

// --- 9. Setup ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== LED RGB Control System with Blynk ===");
  
  // Khởi tạo LED strip
  strip.begin();
  strip.show();
  strip.setBrightness(ledState.brightness);
  
  // Khởi tạo Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Setup HTTP routes
  server.on("/", handleRoot);
  server.on("/api/color", handleAPIColor);
  server.on("/api/brightness", handleAPIBrightness);
  server.on("/api/mode", handleAPIMode);
  server.on("/api/off", handleAPIOff);
  server.on("/api/speed", handleAPISpeed);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Waiting for Blynk connection...");
  
  // Hiệu ứng khởi động
  setSolidColor(0, 255, 0);
  delay(500);
  setSolidColor(0, 0, 0);
}

// --- 10. Loop ---
void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  
  server.handleClient();
  
  // Cập nhật LED theo mode
  if (millis() - lastUpdate > (260 - ledState.speed)) {
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
    lastUpdate = millis();
  }
}
