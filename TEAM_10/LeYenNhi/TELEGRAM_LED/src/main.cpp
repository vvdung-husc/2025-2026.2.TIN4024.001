#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WIFI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// TELEGRAM BOT TOKEN
#define BOTtoken "8235124253:AAHt8WxIYQEdb9a_EwdyQEUjPe2jKgYrpmU"

// GROUP ID
#define GROUP_ID "-4663178968"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// PIR và LED
const int motionSensor = 27;
const int ledPin = 23;

bool motionDetected = false;
bool ledState = false;

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

// ===== NGẮT PIR =====
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// ===== XỬ LÝ LỆNH TELEGRAM =====
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (chat_id != GROUP_ID) {
      bot.sendMessage(chat_id, "Không được phép truy cập", "");
      continue;
    }

    if (text == "/start") {
      String welcome = "Xin chào, Lê Yến Nhi.\n";
      welcome += "Hệ thống ESP32 + PIR + LED\n\n";
      welcome += "/led_on  : bật đèn\n";
      welcome += "/led_off : tắt đèn\n";
      welcome += "/get_state : trạng thái đèn";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED đã bật", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED đã tắt", "");
    }

    if (text == "/get_state") {
      if (ledState)
        bot.sendMessage(chat_id, "LED đang BẬT", "");
      else
        bot.sendMessage(chat_id, "LED đang TẮT", "");
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");

  bot.sendMessage(GROUP_ID, "ESP32 của Lê Yến Nhi đã khởi động", "");
}

void loop() {

  static uint count_ = 0;

  // PIR phát hiện chuyển động
  if (motionDetected) {

    count_++;

    Serial.print(count_);
    Serial.println(". MOTION DETECTED");

    // BẬT ĐÈN
    digitalWrite(ledPin, HIGH);
    ledState = true;

    bot.sendMessage(GROUP_ID, "⚠️ Phát hiện chuyển động!", "");

    delay(5000);   // đèn sáng 5 giây

    // TẮT ĐÈN
    digitalWrite(ledPin, LOW);
    ledState = false;

    motionDetected = false;
  }

  // đọc lệnh telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}