#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ===== CẤU HÌNH =====
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

#define BOT_TOKEN  "8704175386:AAE4P2yNqZm1Zvy_PDDGk0PdHdg0HFukciA" 
#define CHAT_ID    "8765032080" 

#define LED_PIN 23   // Sửa lại cho đúng diagram.json
#define PIR_PIN 27   // Chân cảm biến PIR

// ===== KHỞI TẠO =====
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

bool ledState = false;
unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000; 

// ===== XỬ LÝ TIN NHẮN =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text    = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Bạn không có quyền!", "");
      continue;
    }

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để xem trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED đã BẬT", "");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED đã TẮT", "");
    }
    else if (text == "/get_state") {
      bot.sendMessage(chat_id, ledState ? "LED đang ON" : "LED đang OFF", "");
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT); // Khai báo chân PIR là đầu vào
  digitalWrite(LED_PIN, LOW);

  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  client.setInsecure(); 
}

// ===== LOOP =====
void loop() {
  // 1. Kiểm tra tin nhắn từ Telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // 2. Kiểm tra cảm biến PIR (Cảnh báo về Telegram)
  if (digitalRead(PIR_PIN) == HIGH) {
    Serial.println("Phát hiện chuyển động!");
    bot.sendMessage(CHAT_ID, "⚠️ Cảnh báo: Có người xâm nhập!", "");
    delay(3000); // Tạm dừng 5 giây để tránh spam tin nhắn
  }
}