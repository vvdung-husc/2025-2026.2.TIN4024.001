#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram BOT
#define BOTtoken "8330590252:AAGkTIwR8ebV2Yry1uA7qRmt7WzKqB-pu3M"
#define GROUP_ID "-1003709550009"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Pin
const int motionSensor = 27;
const int ledPin = 26;

bool motionDetected = false;
bool ledState = false;

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

// PIR interrupt
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// xử lý lệnh telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println(text);

    if (text == "/start") {

      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "/led_on bật sáng đèn\n";
      welcome += "/led_off để tắt đèn\n";
      welcome += "/get_state để yêu cầu trạng thái đèn hiện tại\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;

      String msg = from_name + " đã bật LED";
      bot.sendMessage(chat_id, msg, "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;

      String msg = from_name + " đã tắt LED";
      bot.sendMessage(chat_id, msg, "");
    }

    if (text == "/get_state") {

      if (ledState)
        bot.sendMessage(chat_id, "LED is ON", "");
      else
        bot.sendMessage(chat_id, "LED is OFF", "");
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected");

  bot.sendMessage(GROUP_ID, "ESP32 IoT BOT Started", "");
}

void loop() {

  // PIR phát hiện chuyển động
  if (motionDetected) {

    bot.sendMessage(GROUP_ID, "⚠ Phát Hiện Chuyển Động!", "");

    Serial.println("Motion detected");

    // LED nhấp nháy 3 lần chậm
    for(int i = 0; i < 3; i++){
      digitalWrite(ledPin, HIGH);
      delay(700);
      digitalWrite(ledPin, LOW);
      delay(700);
    }

    // Sau đó LED sáng luôn
    digitalWrite(ledPin, HIGH);
    ledState = true;

    motionDetected = false;
  }

  // kiểm tra lệnh Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}