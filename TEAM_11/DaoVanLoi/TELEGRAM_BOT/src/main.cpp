#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- THÔNG TIN ĐÀO VĂN LỢI ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
#define BOTtoken "8700950437:AAEBwn5kljWzRKxArLDWtJzmsMp12h4HrHE" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int ledPin = 23; 
const int pirPin = 27; 
bool motionDetected = false;
unsigned long lastTimeBotRan;

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, LOW);
  attachInterrupt(digitalPinToInterrupt(pirPin), detectsMovement, RISING);

  WiFi.begin(ssid, password);
  client.setInsecure(); 

  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  IPAddress dns(1, 1, 1, 1);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);

  Serial.println("\nWiFi OK!");
  // Bot sẽ nhắn một câu chào vào nhóm để khẳng định chủ quyền
  bot.sendMessage("-4265908169", "Chào nhóm IoT-K45! Bot đã sẵn sàng nhận lệnh.", "");
}

void loop() {
  if (millis() > lastTimeBotRan + 2000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      String text = bot.messages[0].text;
      String chat_id = String(bot.messages[0].chat_id);
      
      Serial.print("Lệnh từ nhóm: "); Serial.println(text);

      // Chấp nhận cả lệnh ngắn và lệnh kèm tên bot
      if (text == "/led_on" || text == "/led_on@loi_1705_bot") {
        digitalWrite(ledPin, HIGH);
        bot.sendMessage(chat_id, "xác nhận: ĐÃ BẬT ĐÈN 🟢", "");
      }
      else if (text == "/led_off" || text == "/led_off@loi_1705_bot") {
        digitalWrite(ledPin, LOW);
        bot.sendMessage(chat_id, "xác nhận: ĐÃ TẮT ĐÈN 🔴", "");
      }
      
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  if (motionDetected) {
    // Cảnh báo PIR sẽ luôn gửi vào nhóm chung
    bot.sendMessage("-4265908169", "⚠️ CẢNH BÁO: Có người di chuyển!", "");
    motionDetected = false;
  }
}