#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

// secrets.h must be included BEFORE BlynkSimpleEsp32.h
// because it defines BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME, BLYNK_AUTH_TOKEN
#include "secrets.h"

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

constexpr uint8_t DHT_PIN = 4;      // diagram.json: esp:D4 -> dht1:SDA
constexpr uint8_t RELAY_PIN = 23;   // diagram.json: esp:D23 -> relay1:IN
constexpr uint8_t DHT_TYPE = DHT22;

constexpr float FAN_ON_TEMP_C = 30.0f;
constexpr float FAN_OFF_TEMP_C = 28.0f;

constexpr unsigned long SENSOR_INTERVAL_MS = 500;
constexpr unsigned long THINGSPEAK_INTERVAL_MS = 20000;
constexpr unsigned long TELEGRAM_POLL_MS = 5000;

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure telegramClient;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, telegramClient);

bool fanState = false;
bool lastFanState = false;
bool sensorErrorNotified = false;
bool manualOverride = false;
float currentTemp = NAN;
float currentHum = NAN;
uint32_t sampleCount = 0;
unsigned long lastSensorRead = 0;
unsigned long lastThingSpeakSend = 0;
unsigned long lastTelegramPoll = 0;

void printTerminalStatus() {
  Serial.println("Đã kết nối Wi-Fi Wowki-GUEST!");
  Serial.println("Nhiệt độ hiện tại: " + String(currentTemp, 1) + "°C");
  Serial.println("Độ ẩm hiện tại: " + String(currentHum, 1) + " %");
  Serial.println("Quạt: " + String(fanState ? "đang bật" : "đang tắt"));
}

void printTerminalAlert() {
  Serial.println("Nhiệt độ hiện tại: " + String(currentTemp, 1) + "°C");
  Serial.println("Độ ẩm hiện tại: " + String(currentHum, 1) + " %");
  Serial.println("Cảnh báo vượt ngưỡng - Quạt đang bật");
}

void setFan(bool on) {
  fanState = on;
  digitalWrite(RELAY_PIN, fanState ? HIGH : LOW);
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(250);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Status line is printed in printTerminalStatus() after sensor read.
  } else {
    Serial.println("Chưa kết nối được Wi-Fi, hệ thống sẽ thử lại...");
  }
}

void connectBlynk() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!Blynk.connected()) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(5000);
  }
}

void sendToBlynk() {
  if (!Blynk.connected()) return;

  Blynk.virtualWrite(V0, currentTemp);            // Gauge nhiet do
  Blynk.virtualWrite(V1, currentHum);             // Do am
  Blynk.virtualWrite(V2, millis() / 1000UL);      // Thoi gian hoat dong (giay)
  Blynk.virtualWrite(V3, fanState ? 1 : 0);       // Cong tac quat
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V3);
}

BLYNK_WRITE(V3) {
  const int cmd = param.asInt();
  manualOverride = true;
  setFan(cmd == 1);
  sendToBlynk();
}

void sendToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) return;

  const unsigned long now = millis();
  if (now - lastThingSpeakSend < THINGSPEAK_INTERVAL_MS) return;
  lastThingSpeakSend = now;

  const String url = "https://api.thingspeak.com/update";
  String payload = String("api_key=") + THINGSPEAK_WRITE_API_KEY;
  payload += "&field1=" + String(currentTemp, 2);
  payload += "&field2=" + String(fanState ? 1 : 0);
  payload += "&field3=" + String(currentHum, 2);
  payload += "&field4=" + String(millis() / 1000UL);

  WiFiClientSecure httpsClient;
  httpsClient.setInsecure();
  HTTPClient http;
  http.setTimeout(12000);

  if (!http.begin(httpsClient, url)) {
    return;
  }

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  const int httpCode = http.POST(payload);
  if (httpCode > 0) {
    const String response = http.getString();
    (void)response;
  }
  http.end();
}

void notifyTelegramStatus(const char* reason) {
  if (WiFi.status() != WL_CONNECTED) return;
  String msg = String("Cap nhat (") + reason + ")\n";
  msg += "Nhiệt độ: " + String(currentTemp, 1) + " C\n";
  msg += "Độ ẩm: " + String(currentHum, 1) + " %\n";
  msg += "Quạt: " + String(fanState ? "Bật" : "Tắt");
  bot.sendMessage(TELEGRAM_CHAT_ID, msg, "");
}

void notifyTelegramText(const String& msg) {
  if (WiFi.status() != WL_CONNECTED) return;
  bot.sendMessage(TELEGRAM_CHAT_ID, msg, "");
}

void handleTelegramCommand(const String& text, const String& chatId) {
  if (chatId != TELEGRAM_CHAT_ID) {
    bot.sendMessage(chatId, "Unauthorized chat id.", "");
    return;
  }

  if (text == "/start") {
    String welcome = "SmartFan ESP32 commands:\n";
    welcome += "/status - xem trạng thái\n";
    welcome += "/on hoac /fan_on - bật quạt\n";
    welcome += "/off hoac /fan_off - tắt quạt\n";
    welcome += "/auto - chuyển về chế độ tự động";
    bot.sendMessage(chatId, welcome, "");
  } else if (text == "/status") {
    String modeText = manualOverride ? "MANUAL" : "AUTO";
    String msg = "SmartFan status\n";
    msg += "Mode: " + modeText + "\n";
    msg += "Temp: " + String(currentTemp, 1) + " C\n";
    msg += "Hum: " + String(currentHum, 1) + " %\n";
    msg += "Fan: " + String(fanState ? "ON" : "OFF");
    bot.sendMessage(chatId, msg, "");
  } else if (text == "/fan_on" || text == "/on") {
    manualOverride = true;
    setFan(true);
    bot.sendMessage(chatId, "Đã bật quạt", "");
    sendToBlynk();
  } else if (text == "/fan_off" || text == "/off") {
    manualOverride = true;
    setFan(false);
    bot.sendMessage(chatId, "Đã tắt quạt", "");
    sendToBlynk();
  } else if (text == "/auto") {
    manualOverride = false;
    bot.sendMessage(chatId, "Đã chuyển về chế độ tự động", "");
    sendToBlynk();
  }
}

void pollTelegram() {
  if (WiFi.status() != WL_CONNECTED) return;

  const unsigned long now = millis();
  if (now - lastTelegramPoll < TELEGRAM_POLL_MS) return;
  lastTelegramPoll = now;

  const int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    handleTelegramCommand(bot.messages[i].text, bot.messages[i].chat_id);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(RELAY_PIN, OUTPUT);
  setFan(false);
  dht.begin();

  telegramClient.setInsecure();
  telegramClient.setTimeout(12000);

  connectWiFi();
  connectBlynk();
  if (Blynk.connected()) {
    Blynk.virtualWrite(V4, "SmartFan online");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    connectBlynk();
  }

  if (Blynk.connected()) {
    Blynk.run();
  } else {
    connectBlynk();
  }

  const unsigned long now = millis();
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    sampleCount++;

    const float t = dht.readTemperature();
    const float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("Lỗi cảm biến! (NaN)");

      setFan(false);
      sendToBlynk();

      if (!sensorErrorNotified) {
        notifyTelegramText("Lỗi cảm biến: DHT22 trả về NaN. Quạt đã tắt!");
        sensorErrorNotified = true;
      }
      return;
    }

    if (sensorErrorNotified) {
      notifyTelegramText("Cảm biến DHT22 đã phục hồi. Hệ thống trở lại giám sát bình thường.");
      sensorErrorNotified = false;
    }

    currentTemp = t;
    currentHum = h;

    const bool wasFanOn = fanState;

    // Hysteresis avoids rapid relay switching near threshold.
    if (!manualOverride) {
      if (!fanState && currentTemp >= FAN_ON_TEMP_C) {
        setFan(true);
      } else if (fanState && currentTemp <= FAN_OFF_TEMP_C) {
        setFan(false);
      }
    }

    if (!manualOverride && currentTemp >= FAN_ON_TEMP_C && fanState) {
      printTerminalAlert();
    } else {
      printTerminalStatus();
    }

    sendToBlynk();
    sendToThingSpeak();

    if (!manualOverride && !wasFanOn && fanState) {
      String alert = "Quạt đã bật. Nhiệt độ hiện tại: ";
      alert += String(currentTemp, 1);
      alert += " C (tự động).";
      notifyTelegramText(alert);
    }

    if (fanState != lastFanState) {
      notifyTelegramStatus(manualOverride ? "Thay đổi trạng thái quạt (thủ công)" : "Thay đổi trạng thái quạt (tự động)");
      lastFanState = fanState;
    }
  }

  pollTelegram();
}
