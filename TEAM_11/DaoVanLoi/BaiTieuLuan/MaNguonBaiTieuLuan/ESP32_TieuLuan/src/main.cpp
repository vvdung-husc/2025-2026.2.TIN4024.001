#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "config.h"

WebServer server(80);

bool ledState = false;
bool fanState = false;

float temperatureC = 27.0;
float humidity = 60.0;
int ldrValue = 0;
int pirValue = 0;

unsigned long lastSensorRead = 0;
unsigned long lastTempAlertTime = 0;
unsigned long lastMotionAlertTime = 0;
unsigned long lastHttpActionTime = 0;

const unsigned long sensorInterval = 2500;

// Hang doi Telegram
const int TELEGRAM_QUEUE_SIZE = 10;
String telegramQueue[TELEGRAM_QUEUE_SIZE];
int telegramHead = 0;
int telegramTail = 0;
int telegramCount = 0;

unsigned long lastTelegramSend = 0;
const unsigned long telegramGapMs = 4000;
const unsigned long httpQuietBeforeTelegramMs = 3000;

String urlEncode(const String &str) {
  String encoded = "";
  char c;
  char code0;
  char code1;

  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);

    if (isalnum((unsigned char)c)) {
      encoded += c;
    } else if (c == ' ') {
      encoded += "%20";
    } else if (c == '\n') {
      encoded += "%0A";
    } else {
      code1 = (c & 0x0f) + '0';
      if ((c & 0x0f) > 9) code1 = (c & 0x0f) - 10 + 'A';
      c = (c >> 4) & 0x0f;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

void enqueueTelegramMessage(const String &msg) {
  if (telegramCount >= TELEGRAM_QUEUE_SIZE) {
    telegramHead = (telegramHead + 1) % TELEGRAM_QUEUE_SIZE;
    telegramCount--;
  }

  telegramQueue[telegramTail] = msg;
  telegramTail = (telegramTail + 1) % TELEGRAM_QUEUE_SIZE;
  telegramCount++;
}

bool dequeueTelegramMessage(String &msg) {
  if (telegramCount == 0) return false;
  msg = telegramQueue[telegramHead];
  telegramHead = (telegramHead + 1) % TELEGRAM_QUEUE_SIZE;
  telegramCount--;
  return true;
}

void sendTelegramMessage(const String &text) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.setTimeout(1500);

  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) +
               "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) +
               "&text=" + urlEncode(text);

  if (https.begin(client, url)) {
    https.GET();
    https.end();
  }
}

void processTelegramQueue() {
  if (telegramCount == 0) return;
  if (millis() - lastTelegramSend < telegramGapMs) return;
  if (millis() - lastHttpActionTime < httpQuietBeforeTelegramMs) return;

  String msg;
  if (dequeueTelegramMessage(msg)) {
    sendTelegramMessage(msg);
    lastTelegramSend = millis();
  }
}

void applyOutputs() {
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
}

float generateRandomTemperature() {
  return 25.0 + (random(0, 100) / 10.0); // 25.0 -> 34.9
}

float generateRandomHumidity() {
  return 50.0 + (random(0, 300) / 10.0); // 50.0 -> 79.9
}

String buildStatusJson() {
  String json = "{";
  json += "\"led\":" + String(ledState ? "true" : "false") + ",";
  json += "\"fan\":" + String(fanState ? "true" : "false") + ",";
  json += "\"temperature\":" + String(temperatureC, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"ldr\":" + String(ldrValue) + ",";
  json += "\"pir\":" + String(pirValue) + ",";
  json += "\"lightText\":\"" + String(ldrValue < LDR_DARK_THRESHOLD ? "TOI" : "SANG") + "\"";
  json += "}";
  return json;
}

String htmlPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Home Control</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f4f6f9; margin: 0; padding: 20px; }
    .box { max-width: 980px; margin: auto; background: white; padding: 20px; border-radius: 16px; box-shadow: 0 4px 16px rgba(0,0,0,0.12); }
    h1 { text-align: center; margin-bottom: 24px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 16px; }
    .card { padding: 18px; border-radius: 14px; background: #fafafa; border: 1px solid #ddd; }
    .btn { display: inline-block; padding: 10px 16px; margin: 6px 6px 0 0; border: none; border-radius: 10px; color: white; cursor: pointer; font-size: 16px; }
    .on { background: #28a745; }
    .off { background: #dc3545; }
    .refresh { background: #007bff; }
    .state-on { color: #28a745; font-weight: bold; }
    .state-off { color: #dc3545; font-weight: bold; }
    .sensor { font-size: 18px; margin: 8px 0; }
  </style>
</head>
<body>
  <div class="box">
    <h1>Dieu khien thiet bi gia dung qua Wi-Fi voi ESP32</h1>

    <div class="grid">
      <div class="card">
        <h3>Den LED</h3>
        <p>Trang thai: <span id="ledState" class="state-off">DANG TAT</span></p>
        <button class="btn on" onclick="controlDevice('/api/led/on', 'led', true)">Bat den</button>
        <button class="btn off" onclick="controlDevice('/api/led/off', 'led', false)">Tat den</button>
      </div>

      <div class="card">
        <h3>Quat</h3>
        <p>Trang thai: <span id="fanState" class="state-off">DANG TAT</span></p>
        <button class="btn on" onclick="controlDevice('/api/fan/on', 'fan', true)">Bat quat</button>
        <button class="btn off" onclick="controlDevice('/api/fan/off', 'fan', false)">Tat quat</button>
      </div>

      <div class="card">
        <h3>Nhiet do - Do am (mo phong)</h3>
        <div class="sensor">Nhiet do: <span id="tempText">Dang doc...</span></div>
        <div class="sensor">Do am: <span id="humText">Dang doc...</span></div>
      </div>

      <div class="card">
        <h3>Cac cam bien khac</h3>
        <div class="sensor">PIR: <span id="pirText">Dang doc...</span></div>
        <div class="sensor">LDR: <span id="ldrText">Dang doc...</span></div>
      </div>
    </div>

    <div style="margin-top:16px;">
      <button class="btn refresh" onclick="loadStatus()">Tai lai trang thai</button>
    </div>
  </div>

<script>
function setDeviceStateUI(id, isOn) {
  const el = document.getElementById(id === 'led' ? 'ledState' : 'fanState');
  el.textContent = isOn ? 'DANG BAT' : 'DANG TAT';
  el.className = isOn ? 'state-on' : 'state-off';
}

async function fetchWithTimeout(url, ms = 1000) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), ms);
  try {
    return await fetch(url, { signal: controller.signal, cache: 'no-store' });
  } finally {
    clearTimeout(timer);
  }
}

async function loadStatus() {
  try {
    const res = await fetchWithTimeout('/status', 1000);
    const data = await res.json();

    setDeviceStateUI('led', data.led);
    setDeviceStateUI('fan', data.fan);

    document.getElementById('tempText').innerHTML = data.temperature + ' &deg;C';
    document.getElementById('humText').textContent = data.humidity + ' %';
    document.getElementById('pirText').textContent = data.pir ? 'CO CHUYEN DONG' : 'KHONG CO';
    document.getElementById('ldrText').textContent = data.ldr + ' (' + data.lightText + ')';
  } catch (e) {
    console.log('Khong tai duoc /status', e);
  }
}

async function controlDevice(url, id, isOn) {
  setDeviceStateUI(id, isOn);
  try {
    await fetchWithTimeout(url, 500);
    setTimeout(loadStatus, 100);
  } catch (e) {
    console.log('Khong goi duoc lenh dieu khien', e);
  }
}

loadStatus();
setInterval(loadStatus, 8000);
</script>
</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", htmlPage());
}

void handleLedOn() {
  ledState = true;
  applyOutputs();
  lastHttpActionTime = millis();
  enqueueTelegramMessage("Thong bao: Den LED da BAT");
  server.send(200, "application/json", "{\"ok\":true,\"device\":\"led\",\"state\":true}");
}

void handleLedOff() {
  ledState = false;
  applyOutputs();
  lastHttpActionTime = millis();
  enqueueTelegramMessage("Thong bao: Den LED da TAT");
  server.send(200, "application/json", "{\"ok\":true,\"device\":\"led\",\"state\":false}");
}

void handleFanOn() {
  fanState = true;
  applyOutputs();
  lastHttpActionTime = millis();
  enqueueTelegramMessage("Thong bao: Quat da BAT");
  server.send(200, "application/json", "{\"ok\":true,\"device\":\"fan\",\"state\":true}");
}

void handleFanOff() {
  fanState = false;
  applyOutputs();
  lastHttpActionTime = millis();
  enqueueTelegramMessage("Thong bao: Quat da TAT");
  server.send(200, "application/json", "{\"ok\":true,\"device\":\"fan\",\"state\":false}");
}

void handleStatusJson() {
  server.send(200, "application/json", buildStatusJson());
}

void readSensors() {
  ldrValue = analogRead(LDR_PIN);
  pirValue = digitalRead(PIR_PIN);

  temperatureC = generateRandomTemperature();
  humidity = generateRandomHumidity();
}

void checkAlerts() {
  unsigned long now = millis();

  if (temperatureC > TEMP_ALERT_THRESHOLD &&
      (now - lastTempAlertTime > ALERT_COOLDOWN_MS)) {
    enqueueTelegramMessage("CANH BAO: Nhiet do vuot nguong! Nhiet do hien tai: " + String(temperatureC, 1) + " *C");
    lastTempAlertTime = now;
  }

  if (pirValue == HIGH &&
      (now - lastMotionAlertTime > ALERT_COOLDOWN_MS)) {
    enqueueTelegramMessage("CANH BAO: Phat hien chuyen dong trong khu vuc giam sat");
    lastMotionAlertTime = now;
  }
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/api/led/on", handleLedOn);
  server.on("/api/led/off", handleLedOff);
  server.on("/api/fan/on", handleFanOn);
  server.on("/api/fan/off", handleFanOff);
  server.on("/status", handleStatusJson);
  server.begin();
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());

  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  applyOutputs();
  connectWiFi();
  setupWebServer();
  readSensors();

  enqueueTelegramMessage("ESP32 da khoi dong thanh cong. He thong HTTP dieu khien da san sang.");
}

void loop() {
  server.handleClient();
  processTelegramQueue();

  unsigned long now = millis();
  if (now - lastSensorRead >= sensorInterval) {
    lastSensorRead = now;
    readSensors();
    checkAlerts();
  }
}