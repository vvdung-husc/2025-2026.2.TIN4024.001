#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Cấu hình Telegram
#define BOTtoken "8618224007:AAFkL2FH9sNNCbmfc0fsHXIYqv2Qy3KW1Yg"
#define GROUP_ID "7707133820" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; 
const int ledPin = 2;        
bool motionDetected = false;
bool ledStatus = false;      
bool autoMode = false;       

unsigned long lastTimeBotRan;
unsigned long lastMotionTime; 
int checkDelay = 1000; 
int autoOffDelay = 10000;    // 10 giây tự tắt đèn

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm gửi Bảng điều khiển (Chỉ gọi khi /start hoặc hệ thống Ready)
void sendWelcomeMessage(String chat_id) {
  String welcome = "<b>🏠 HỆ THỐNG GIÁM SÁT THÔNG MINH</b>\n";
  welcome += "----------------------------\n";
  welcome += "🤖 Chế độ: " + String(autoMode ? "TỰ ĐỘNG" : "THỦ CÔNG") + "\n";
  welcome += "💡 Đèn: " + String(ledStatus ? "BẬT 🟢" : "TẮT 🔴") + "\n\n";
  
  welcome += "👇 <b>DANH SÁCH LỆNH (CÓ GẠCH CHÂN):</b>\n";
  // Trick: Viết lệnh liền sau dấu gạch chéo và đăng ký với BotFather
  welcome += "💡 /led_on : Bật đèn\n";
  welcome += "🌑 /led_off : Tắt đèn\n";
  welcome += "🤖 /auto_on : Chế độ tự động\n";
  welcome += "🖐️ /auto_off : Chế độ thủ công\n";
  welcome += "📊 /status : Xem trạng thái\n";
  welcome += "🏠 /start : Hiện lại Menu";
  
  // Lưu ý: Đổi "Markdown" thành "HTML"
  bot.sendMessage(chat_id, welcome, "HTML");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      autoMode = false;
      ledStatus = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "💡 Đèn đã BẬT (Thủ công)", "");
    }
    else if (text == "/led_off") {
      autoMode = false;
      ledStatus = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "🌑 Đèn đã TẮT (Thủ công)", "");
    }
    else if (text == "/auto_on") {
      autoMode = true;
      bot.sendMessage(chat_id, "🤖 Đã bật chế độ TỰ ĐỘNG", "");
    }
    else if (text == "/auto_off") {
      autoMode = false;
      bot.sendMessage(chat_id, "🖐️ Đã tắt chế độ TỰ ĐỘNG", "");
    }
    else if (text == "/status") {
      String statusMsg = "📊 *TRẠNG THÁI HIỆN TẠI*\n";
      statusMsg += "- Đèn: " + String(ledStatus ? "BẬT 🟢" : "TẮT 🔴") + "\n";
      statusMsg += "- Chế độ: " + String(autoMode ? "TỰ ĐỘNG 🤖" : "THỦ CÔNG 🖐️");
      bot.sendMessage(chat_id, statusMsg, "Markdown");
    }
    // Chỉ hiện bảng Menu khi người dùng gõ đúng /start
    else if (text == "/start") {
      sendWelcomeMessage(chat_id);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // 1. Thông báo sẵn sàng
  bot.sendMessage(GROUP_ID, "🚀 Hệ thống đã sẵn sàng!", "");
  // 2. Hiện Menu lần duy nhất lúc khởi động
  sendWelcomeMessage(GROUP_ID);
}

void loop() {
  // Xử lý cảm biến chuyển động
  if (motionDetected) {
    lastMotionTime = millis(); 
    if (autoMode && !ledStatus) {
      ledStatus = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "🏃 Có người! Tự động bật đèn 💡", "");
    } else if (!autoMode) {
      bot.sendMessage(GROUP_ID, "🚨 Cảnh báo: Phát hiện chuyển động!", "");
    }
    motionDetected = false;
  }

  // Logic tự tắt đèn trong chế độ Auto
  if (autoMode && ledStatus) {
    if (millis() - lastMotionTime > autoOffDelay) {
      ledStatus = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "🌑 Không có người, tự động tắt đèn.", "");
    }
  }

  // Quét tin nhắn từ Telegram
  if (millis() > lastTimeBotRan + checkDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}