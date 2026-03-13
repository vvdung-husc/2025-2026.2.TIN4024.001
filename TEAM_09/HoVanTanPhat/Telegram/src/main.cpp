#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Thông tin Telegram từ bạn cung cấp
#define BOTtoken "8692886537:AAEU2IsE_BAVi0gjn5im2NHNBpNaD1spAJk"
#define CHAT_ID "6131894938" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// 3. Chân cắm khớp với diagram.json của bạn
const int ledPin = 23;      // LED đỏ
const int motionSensor = 27; // Cảm biến PIR

bool ledState = LOW;
volatile bool motionDetected = false;
unsigned long lastTimeBotRan;
int bot_delay = 1000; 

// Hàm ngắt khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm xử lý tin nhắn từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue; 

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Chào " + from_name + "!\n";
      welcome += "Hệ thống điều khiển LED.\n\n";
      welcome += "Gửi /led_on : Bật đèn LED\n";
      welcome += "Gửi /led_off : Tắt đèn LED\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }

    if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED tắt", "");
    }

    if (text == "/get_state") {
      String msg = (digitalRead(ledPin)) ? "Đèn đang BẬT" : "Đèn đang TẮT";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // --- FIX DNS: CẤU HÌNH MẠNG TĨNH CHO WOKWI ---
  IPAddress local_IP(10, 13, 37, 2); 
  IPAddress gateway(10, 13, 37, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8); // DNS của Google
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure Static IP");
  }

  Serial.print("Connecting Wifi: ");
  WiFi.begin(ssid, password);
  
  // FIX SSL: Bỏ qua kiểm tra chứng chỉ để tránh lỗi kết nối
 // FIX SSL: Cấp chứng chỉ bảo mật chuẩn của Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
  
}

void loop() {
  // 1. Kiểm tra lệnh từ Telegram mỗi 1 giây
  if (millis() > lastTimeBotRan + bot_delay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  if (motionDetected) {
    Serial.println("Phat hien chuyen dong!");
    bot.sendMessage(CHAT_ID, "Cảnh báo: Phát hiện chuyển động!", "");
    motionDetected = false; 
  }
}