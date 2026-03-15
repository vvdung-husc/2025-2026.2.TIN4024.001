#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- CẤU HÌNH WIFI ---
const char* ssid = "Wokwi-GUEST";    
const char* password = "";    

// cấu hình bảo mật cho Telegram
#define BOT_TOKEN "8788575952:AAHwdpNf1fBiKfcp048uM-V1QFI4mRrZRoI"

const int ledPin = 23; 
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    // Phản hồi lệnh /start
    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + " (Nhật).\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
      welcome += "/led_on : để Bật sáng đèn\n";
      welcome += "/led_off : để Tắt đèn\n";
      welcome += "/get_state : để kiểm tra trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }

    // Phản hồi lệnh /led_on
    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED bật sáng", "");
      Serial.println("Lệnh: Bật LED");
    }

    // Phản hồi lệnh /led_off
    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
      Serial.println("Lệnh: Tắt LED");
    }

    // Phản hồi lệnh /get_state
    if (text == "/get_state") {
      if (digitalRead(ledPin)) {
        bot.sendMessage(chat_id, "LED bật sáng", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Mặc định tắt LED khi khởi động

  // Kết nối WiFi
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  // Cấu hình bảo mật cho Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Kiểm tra tin nhắn từ Telegram mỗi 1 giây
  if (millis() > lastTimeBotRan + 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}