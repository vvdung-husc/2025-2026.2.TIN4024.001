#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- THÔNG TIN CẤU HÌNH ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

#define BOTtoken "8616391327:AAHWondY_PiW20JwGq4Pa3-_P5GstMnxJSU"
#define GROUP_ID "-5254059042"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; 
const int ledPin = 23;       
volatile bool motionDetected = false; // Dùng volatile cho biến ngắt

// Tối ưu tốc độ: Kiểm tra tin nhắn mỗi 200ms
unsigned long lastTimeBotRan;
int botRequestDelay = 200; 

// --- HÀM GỬI MENU NÚT BẤM ---
void sendWelcomeMessage(String chat_id, String from_name) {
  String welcome = "Chào " + from_name + " (Năng), hệ thống IoT đã sẵn sàng!\n";
  welcome += "Sử dụng các nút bên dưới để điều khiển.";
  
  // Tạo bàn phím nút bấm (Reply Keyboard)
  String keyboardJson = "[[\"/led_on\", \"/led_off\"], [\"/get_state\"]]";
  bot.sendMessageWithReplyKeyboard(chat_id, welcome, "", keyboardJson, true);
}

// --- HÀM XỬ LÝ TIN NHẮN ---
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; 

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "Năng ơi, LED đã BẬT sáng rồi nhé! ✅", "");
    }
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "Năng ơi, LED đã TẮT rồi nhé! 🌑", "");
    }
    else if (text == "/get_state") {
      String status = digitalRead(ledPin) ? "Đang BẬT 💡" : "Đang TẮT ⚫";
      bot.sendMessage(GROUP_ID, "Trạng thái hiện tại: " + status, "");
    }
    else if (text == "/start") {
      sendWelcomeMessage(chat_id, "Năng");
    }
  }
}

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Cấu hình DNS Google để truy cập Telegram nhanh hơn
  IPAddress dns(8, 8, 8, 8); 
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns);

  Serial.println("\nWiFi connected");
  client.setInsecure(); 
  
  // Gửi thông báo khởi động và hiện nút bấm luôn
  sendWelcomeMessage(GROUP_ID, "Năng");
}

void loop() {
  // 1. Phản ứng cực nhanh với cảm biến
  if (motionDetected) {
    bot.sendMessage(GROUP_ID, "⚠️ Cảnh báo: Có chuyển động trong khu vực của Năng!", "");
    motionDetected = false;
  }

  // 2. Kiểm tra tin nhắn Telegram (tần suất cao hơn)
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}