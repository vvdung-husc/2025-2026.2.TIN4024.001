#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi (Trên Wokwi dùng WiFi mặc định này)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Cấu hình Telegram Bot
#define BOT_TOKEN "ĐIỀN_TOKEN_CỦA_BẠN_VÀO_ĐÂY"
#define CHAT_ID "ĐIỀN_CHAT_ID_CỦA_BẠN_VÀO_ĐÂY"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// 3. Khai báo chân linh kiện
const int ledPin = 23;
const int pirPin = 27;

bool ledState = LOW;
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Biến lưu trạng thái cảm biến PIR
int pirState = LOW;
int lastPirState = LOW;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    // Kiểm tra xem tin nhắn có đến từ group hợp lệ không
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Bạn không có quyền điều khiển bot này!", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }
    else if (text == "/get_state") {
      if (ledState == HIGH) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, ledState);

  // Kết nối WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Cấp quyền SSL cho Telegram
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  // 1. Kiểm tra tin nhắn Telegram đến
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // 2. Kiểm tra cảm biến PIR (Cảnh báo chuyển động)
  pirState = digitalRead(pirPin);
  if (pirState == HIGH && lastPirState == LOW) {
    Serial.println("Phát hiện chuyển động!");
    bot.sendMessage(CHAT_ID, "CẢNH BÁO: Phát hiện có chuyển động!", "");
    lastPirState = HIGH;
  } else if (pirState == LOW && lastPirState == HIGH) {
    lastPirState = LOW;
  }
}