#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

/*
=====================================================
ESP32 + PIR Motion Sensor + Telegram Alert
Tác giả: Thuần Lê
Chức năng:
1. Khi cảm biến PIR phát hiện chuyển động -> bật đèn + gửi Telegram
2. Người dùng có thể bật tắt đèn bằng Telegram
=====================================================
*/

// ================= WIFI =================
// Wokwi chỉ dùng mạng này
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
#define BOTtoken "8724216274:AAFGbKFDk64HYaDLbqqn27PXZxEdgatFOJg"
#define CHAT_ID "8583007319"

// tạo đối tượng bot
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ================= PIN =================
const int pirPin = 27;   // chân OUT của cảm biến PIR
const int ledPin = 23;   // chân LED

// ================= BIẾN =================
bool ledState = false;
int motionCount = 0;

// kiểm tra tin nhắn telegram mỗi 1 giây
unsigned long lastTimeBotRan;
int bot_delay = 1000;

// =====================================================
// HÀM XỬ LÝ LỆNH TELEGRAM
// =====================================================
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Telegram command: " + text);

    // menu hướng dẫn
    if (text == "/start") {

      String msg = "ESP32 Alarm System\n\n";

      msg += "Lenh dieu khien:\n";
      msg += "/led_on  - Bat den\n";
      msg += "/led_off - Tat den\n";
      msg += "/status  - Trang thai den\n\n";

      msg += "He thong se tu dong gui canh bao khi PIR phat hien chuyen dong.";

      bot.sendMessage(chat_id, msg);
    }

    // bật LED
    if (text == "/led_on") {

      digitalWrite(ledPin, HIGH);
      ledState = true;

      bot.sendMessage(chat_id, "Den da BAT");
    }

    // tắt LED
    if (text == "/led_off") {

      digitalWrite(ledPin, LOW);
      ledState = false;

      bot.sendMessage(chat_id, "Den da TAT");
    }

    // kiểm tra trạng thái
    if (text == "/status") {

      if (ledState)
        bot.sendMessage(chat_id, "Den dang BAT");
      else
        bot.sendMessage(chat_id, "Den dang TAT");
    }
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.println("Khoi dong ESP32...");

  // kết nối wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setInsecure();

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // gửi tin nhắn khi hệ thống bắt đầu
  bot.sendMessage(CHAT_ID, "ESP32 Alarm System Started\nGo /start de xem huong dan");
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ================= PIR SENSOR =================
  // PIR sẽ trả HIGH khi phát hiện chuyển động

  int motion = digitalRead(pirPin);

  if (motion == HIGH) {

    Serial.println("Phat hien chuyen dong!");

    motionCount++;

    digitalWrite(ledPin, HIGH);

    String msg = "🚨 Phat hien chuyen dong!\nSo lan: ";
    msg += motionCount;

    bot.sendMessage(CHAT_ID, msg);

    delay(3000);

    digitalWrite(ledPin, LOW);
  }

  // ================= TELEGRAM =================

  if (millis() > lastTimeBotRan + bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}