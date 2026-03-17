/*
THONG TIN NHOM 04
1. Lê Văn Thuần
2. ...
3. ...
4. ...
5. ...
*/

#define BLYNK_TEMPLATE_ID   "TMPL6scT0Job2"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN    "RC4iydZWVV92FAZFII_Zf_MDGsbC5pSb"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ===================== WIFI =====================
char ssid[] = "abc";
char pass[] = "12345678";

// ===================== TELEGRAM =====================
#define BOT_TOKEN "8513235724:AAGfkRD5BLb7zPl6uA6lPWX5Bzevu-o2fSo"
#define CHAT_ID   "-5275641584"

// ===================== PIN CONFIG =====================
#define DHTPIN      D3
#define DHTTYPE     DHT22

// Relay/den cua ban test hoat dong voi GPIO2
#define RELAY_PIN   2
#define MQ2_PIN     A0

#define OLED_SDA    D2
#define OLED_SCL    D1

// ===================== TEAM INFO =====================
const char* TEAM_NAME = "Team 04";

// ===================== OBJECTS =====================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display(128, 64, &Wire, -1);
BlynkTimer timer;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ===================== GLOBAL VARIABLES =====================
float temperature = 0.0f;
float humidity = 0.0f;
int gasValue = 0;
bool relayState = false;

unsigned long lastTelegramCheck = 0;

float lastSentTemp = -999.0f;
float lastSentHum = -999.0f;
int lastSentGas = -1;

bool useRandomGas = false;

// ===================== HELPER =====================
String formatUptime(unsigned long ms) {
  unsigned long totalSec = ms / 1000;
  unsigned long days = totalSec / 86400;
  unsigned long hours = (totalSec % 86400) / 3600;
  unsigned long mins = (totalSec % 3600) / 60;
  unsigned long secs = totalSec % 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%luD %02lu:%02lu:%02lu", days, hours, mins, secs);
  return String(buf);
}

void setRelay(bool state) {
  relayState = state;

  // Theo test cua ban: LOW = BAT, HIGH = TAT
  digitalWrite(RELAY_PIN, state ? LOW : HIGH);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, state ? 1 : 0);
  }
}

String getSensorText() {
  String msg;
  msg += "Nhiet do: " + String(temperature, 1) + " C\n";
  msg += "Do am: " + String(humidity, 1) + " %\n";
  msg += "Khi gas: " + String(gasValue) + "\n";
  msg += "Den: " + String(relayState ? "BAT" : "TAT") + "\n";
  msg += "Uptime: " + formatUptime(millis()) + "\n";
  msg += TEAM_NAME;
  return msg;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Dang ket noi WiFi");
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Khong ket noi duoc WiFi.");
  }
}

void reconnectIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (WiFi.status() == WL_CONNECTED && !Blynk.connected()) {
    Serial.println("Dang ket noi lai Blynk...");
    Blynk.connect(5000);
  }
}

// ===================== SENSOR =====================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) {
    temperature = t;
  } else {
    Serial.println("Loi doc nhiet do DHT!");
  }

  if (!isnan(h)) {
    humidity = h;
  } else {
    Serial.println("Loi doc do am DHT!");
  }

  if (useRandomGas) {
    gasValue = random(200, 900);
  } else {
    gasValue = analogRead(MQ2_PIN);
  }
}

void printSerialData() {
  Serial.println("=================================");
  Serial.printf("Uptime    : %s\n", formatUptime(millis()).c_str());
  Serial.printf("Nhiet do  : %.1f C\n", temperature);
  Serial.printf("Do am     : %.1f %%\n", humidity);
  Serial.printf("Khi gas   : %d\n", gasValue);
  Serial.printf("Relay     : %s\n", relayState ? "BAT" : "TAT");
}

// ===================== OLED =====================
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);
  display.println("ESP8266 Blynk Tele");

  display.setCursor(0, 12);
  display.printf("Nhiet do: %.1f C", temperature);

  display.setCursor(0, 24);
  display.printf("Do am  : %.1f %%", humidity);

  display.setCursor(0, 36);
  display.printf("Khi gas: %d", gasValue);

  display.setCursor(0, 48);
  display.printf("Den: %s", relayState ? "BAT" : "TAT");

  display.setCursor(80, 56);
  display.print(TEAM_NAME);

  display.display();
}

// ===================== BLYNK =====================
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  setRelay(value == 1);
}

void updateBlynk() {
  if (!Blynk.connected()) return;

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, gasValue);
  Blynk.virtualWrite(V4, formatUptime(millis()));
  Blynk.virtualWrite(V5, TEAM_NAME);
}

// ===================== TELEGRAM =====================
void sendTelegramMessage(const String& text) {
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, text, "");
  }
}

void notifySensorChange() {
  if (WiFi.status() != WL_CONNECTED) return;

  bool tempChanged = fabs(temperature - lastSentTemp) >= 0.5f;
  bool humChanged  = fabs(humidity - lastSentHum) >= 1.0f;

  if (tempChanged || humChanged) {
    String msg = "THONG BAO TU THIET BI\n";
    msg += "Nhiet do: " + String(temperature, 1) + " C\n";
    msg += "Do am: " + String(humidity, 1) + " %\n";
    msg += TEAM_NAME;
    sendTelegramMessage(msg);

    lastSentTemp = temperature;
    lastSentHum = humidity;
  }

  if (lastSentGas < 0 || abs(gasValue - lastSentGas) >= 50) {
    String msg = "CANH BAO KHI GAS\n";
    msg += "Gia tri MQ2: " + String(gasValue) + "\n";
    msg += TEAM_NAME;
    sendTelegramMessage(msg);
    lastSentGas = gasValue;
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Telegram message: " + text);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Khong duoc phep su dung bot nay.", "");
      continue;
    }

    if (text == "/start") {
      String welcome = "Xin chao " + from_name + "\n";
      welcome += "Lenh ho tro:\n";
      welcome += "/led_on - Bat den\n";
      welcome += "/led_off - Tat den\n";
      welcome += "/led_status - Xem trang thai den\n";
      welcome += "/get_weather - Xem nhiet do, do am hien tai\n";
      welcome += "/get_all - Xem toan bo du lieu\n";
      welcome += TEAM_NAME;
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      setRelay(true);
      bot.sendMessage(chat_id, "Den da BAT", "");
    }
    else if (text == "/led_off") {
      setRelay(false);
      bot.sendMessage(chat_id, "Den da TAT", "");
    }
    else if (text == "/led_status") {
      bot.sendMessage(chat_id, String("Trang thai den: ") + (relayState ? "BAT" : "TAT"), "");
    }
    else if (text == "/get_weather") {
      String msg = "Nhiet do: " + String(temperature, 1) + " C\n";
      msg += "Do am: " + String(humidity, 1) + " %\n";
      msg += TEAM_NAME;
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/get_all") {
      bot.sendMessage(chat_id, getSensorText(), "");
    }
  }
}

void checkTelegram() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Telegram: mat WiFi");
    return;
  }

  if (millis() - lastTelegramCheck < 2000) return;
  lastTelegramCheck = millis();

  Serial.println("Telegram: dang kiem tra...");
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  Serial.printf("Telegram: %d tin nhan moi\n", numNewMessages);

  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===================== TASKS =====================
void taskReadData() {
  readSensors();
  printSerialData();
  updateOLED();
}

void taskCloudUpdate() {
  updateBlynk();
}

void taskTelegramNotify() {
  notifySensorChange();
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(RELAY_PIN, OUTPUT);

  // Theo test cua ban: HIGH = TAT luc khoi dong
  digitalWrite(RELAY_PIN, HIGH);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(0x3C, true)) {
    Serial.println("Khong tim thay OLED SH1106!");
    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Dang khoi dong...");
  display.display();

  randomSeed(analogRead(A0));

  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi da ket noi");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi chua ket noi");
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.connect(10000);
  }

  secured_client.setInsecure();

  setRelay(false);
  readSensors();
  updateOLED();
  updateBlynk();

  timer.setInterval(2000L, taskReadData);
  timer.setInterval(3000L, taskCloudUpdate);
  timer.setInterval(5000L, taskTelegramNotify);

  Serial.println("He thong san sang.");
  sendTelegramMessage(String("ESP8266 da khoi dong thanh cong\n") + TEAM_NAME);
}

// ===================== LOOP =====================
void loop() {
  reconnectIfNeeded();
  Blynk.run();
  timer.run();
  checkTelegram();
}