#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Cấu hình WiFi & Telegram
const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define BOTtoken "8618224007:AAFkL2FH9sNNCbmfc0fsHXIYqv2Qy3KW1Yg"
#define GROUP_ID "7707133820" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// 2. Định nghĩa chân cắm & Biến trạng thái
const int motionSensor = 27; 
const int ledPin = 2;        
bool motionDetected = false;
bool ledStatus = false;      
bool autoMode = false;       
bool securityMode = false;   

unsigned long lastTimeBotRan;
unsigned long lastMotionTime; 
unsigned long ledTimerEnd = 0;  
int checkDelay = 800; 
int autoOffDelay = 10000;    

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// 3. Hàm gửi Bảng điều khiển (Dạng text clickable)
void sendWelcomeMessage(String chat_id) {
  String message = "🛠 <b>BẢNG ĐIỀU KHIỂN TRUNG TÂM</b>\n";
  message += "----------------------------\n";
  message += "💡 Đèn: " + String(ledStatus ? "BẬT 🟢" : "TẮT 🔴") + "\n";
  message += "🤖 Tự động: " + String(autoMode ? "BẬT ✅" : "TẮT ❌") + "\n";
  message += "🛡 An ninh: " + String(securityMode ? "KÍCH HOẠT ⚠️" : "TẮT 😴") + "\n";
  
  float temp = temperatureRead();
  message += "🌡 Nhiệt độ chip: " + String(temp, 1) + "°C\n";
  message += "----------------------------\n\n";
  
  message += "👇 <b>NHẤN VÀO LỆNH ĐỂ ĐIỀU KHIỂN:</b>\n";
  message += "💡 /led_on : Bật đèn\n";
  message += "🌑 /led_off : Tắt đèn\n";
  message += "🤖 /toggle_auto : Bật/Tắt Tự động\n";
  message += "🛡 /toggle_sec : Bật/Tắt An ninh\n";
  message += "⏳ /timer_1m : Hẹn giờ 1 phút\n";
  message += "📊 /status : Làm mới trạng thái\n";
  message += "🏠 /start : Hiện lại bảng này";

  bot.sendMessage(chat_id, message, "HTML");
}

// 4. Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    // Phản hồi xác nhận ngay lập tức để tránh người dùng nhấn nhiều lần
    if (text != "/start" && text != "/status") {
       bot.sendMessage(chat_id, "🔄 Đang xử lý: " + text + "...", "");
    }

    if (text == "/led_on") {
      ledStatus = true; autoMode = false; ledTimerEnd = 0;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "✅ Đèn đã BẬT!", "");
    }
    else if (text == "/led_off") {
      ledStatus = false; autoMode = false; ledTimerEnd = 0;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "✅ Đèn đã TẮT!", "");
    }
    else if (text == "/toggle_auto") {
      autoMode = !autoMode;
      String st = autoMode ? "BẬT ✅" : "TẮT ❌";
      bot.sendMessage(chat_id, "🤖 Chế độ Tự động: " + st, "");
    }
    else if (text == "/toggle_sec") {
      securityMode = !securityMode;
      String st = securityMode ? "KÍCH HOẠT ⚠️" : "TẮT 😴";
      bot.sendMessage(chat_id, "🛡 Chế độ An ninh: " + st, "");
    }
    else if (text == "/timer_1m") {
      ledStatus = true;
      digitalWrite(ledPin, HIGH);
      ledTimerEnd = millis() + 60000; 
      bot.sendMessage(chat_id, "⏳ Đã hẹn giờ: Đèn sẽ tắt sau 1 phút.", "");
    }
    else if (text == "/status" || text == "/start") {
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
  
  // Khởi động xong báo về Group
  bot.sendMessage(GROUP_ID, "🚀 <b>Hệ thống quản gia đã Sẵn Sàng!</b>", "HTML");
  sendWelcomeMessage(GROUP_ID);
}

void loop() {
  // Logic cảm biến chuyển động
  if (motionDetected) {
    lastMotionTime = millis();
    if (securityMode) {
      bot.sendMessage(GROUP_ID, "⚠️ <b>CẢNH BÁO:</b> Có chuyển động trong khu vực an ninh!", "HTML");
    }
    if (autoMode && !ledStatus) {
      ledStatus = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "🏃 Có người, tự động bật đèn.", "");
    }
    motionDetected = false;
  }

  // Tự động tắt đèn sau 10s không có người (Auto Mode)
  if (autoMode && ledStatus && (millis() - lastMotionTime > autoOffDelay)) {
    ledStatus = false;
    digitalWrite(ledPin, LOW);
    bot.sendMessage(GROUP_ID, "🌑 Không còn người, đèn tự tắt.", "");
  }

  // Tắt đèn theo hẹn giờ 1 phút
  if (ledTimerEnd > 0 && millis() > ledTimerEnd) {
    ledStatus = false;
    digitalWrite(ledPin, LOW);
    bot.sendMessage(GROUP_ID, "⏳ Hết 1 phút hẹn giờ, đèn đã tắt.", "");
    ledTimerEnd = 0;
  }

  // Kiểm tra tin nhắn mới
  if (millis() > lastTimeBotRan + checkDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}