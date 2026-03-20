#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- WIFI CHO MẠCH ẢO WOKWI ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- THÔNG TIN BOT TELEGRAM CỦA BẠN ---
// (Bạn phải lấy mã này từ @BotFather trên Telegram)
#define BOT_TOKEN "8733232857:AAH08BXjF1BrrHRoLBA5ghhANBL8vr5pTls" 

// --- CẤU HÌNH CHÂN (PINS) ---
const int ledPin = 23; 
const int pirPin = 27; // Chân nối với cảm biến chuyển động

bool ledState = LOW;
int pirState = LOW;             // Trạng thái hiện tại của cảm biến
int lastPirState = LOW;         // Trạng thái trước đó để chống gửi tin nhắn liên tục

// Biến lưu Chat ID để ESP32 biết phải gửi tin nhắn báo động cho ai
String currentChatId = ""; 

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_MTBS = 1000; 
unsigned long bot_lasttime = 0;

void handleNewMessages(int numNewMessages) {
  Serial.print("Có tin nhắn mới: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (from_name == "") from_name = "Guest";

    // LƯU LẠI CHAT ID: Khi bạn nhắn tin cho Bot, ESP32 sẽ nhớ ID của bạn
    // để tí nữa cảm biến phát hiện người thì nó biết gửi cảnh báo đi đâu.
    currentChatId = chat_id; 

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Mình là ESP32 Ảo đây!\n\n";
      welcome += "Gửi /led_on để bật đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để xem trạng thái đèn\n";
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
      if (ledState) {
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
  pinMode(pirPin, INPUT); // Cài đặt chân cảm biến là đầu vào để đọc tín hiệu
  
  digitalWrite(ledPin, ledState);

  // Kết nối WiFi ảo
  Serial.print("Đang kết nối WiFi ảo Wokwi-GUEST ");
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Bắt buộc cho ESP32 kết nối HTTPS

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi đã kết nối thành công!");
}

void loop() {
  // 1. KIỂM TRA LỆNH TỪ TELEGRAM (Mỗi 1 giây)
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  // 2. ĐỌC CẢM BIẾN & CHỦ ĐỘNG GỬI TÍN HIỆU LÊN TELEGRAM
  pirState = digitalRead(pirPin);
  
  // Nếu trạng thái thay đổi từ LOW (không có người) sang HIGH (có người)
  if (pirState == HIGH && lastPirState == LOW) {
    Serial.println("Phát hiện chuyển động!");
    
    // Nếu bạn đã chat với Bot (ESP32 đã lưu Chat ID của bạn) thì nó sẽ gửi tin nhắn
    if (currentChatId != "") {
      bot.sendMessage(currentChatId, "⚠️ Cảnh báo: Cảm biến phát hiện có người!", "");
    } else {
      Serial.println("Chưa có Chat ID. Hãy nhắn /start cho bot trên Telegram trước.");
    }
    lastPirState = HIGH; // Cập nhật trạng thái để không bị gửi liên tục
  } 
  // Nếu trạng thái thay đổi từ HIGH về LOW (hết chuyển động)
  else if (pirState == LOW && lastPirState == HIGH) {
    lastPirState = LOW; // Reset lại để chờ lần chuyển động tiếp theo
  }
}