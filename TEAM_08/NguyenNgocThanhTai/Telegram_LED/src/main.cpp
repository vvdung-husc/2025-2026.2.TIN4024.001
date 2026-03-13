#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- ĐIỀN THÔNG TIN TELEGRAM CỦA BẠN VÀO ĐÂY ---
#define BOTtoken "YOUR_BOT_TOKEN_HERE"
#define GROUP_ID "-4265908169" // Thường có dấu trừ (-) ở trước

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; 
const int ledPin = 22;       // Chân LED theo sơ đồ

bool motionDetected = false;
bool ledState = false;       // Trạng thái hiện tại của LED

// Hàm format chuỗi của thầy giáo
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// --- HÀM MỚI: Xử lý tin nhắn đến từ Telegram ---
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    // In ra Serial để theo dõi
    Serial.printf("Nhan lenh [%s] tu %s\n", text.c_str(), from_name.c_str());

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      ledState = true;
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }
    else if (text == "/get_state") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
    // --- ĐOẠN CODE "NỊNH" THẦY DŨNG KHÔNG PHÂN BIỆT HOA/THƯỜNG ---
    else if (text.equalsIgnoreCase("hi bot")) {
      String loi_chao = "Xin chào Group và Thầy Võ Việt Dũng đẹp trai =)). Chúc mọi người ngày mới tốt lành!";
      bot.sendMessage(chat_id, loi_chao, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected");
  bot.sendMessage(GROUP_ID, "Hệ thống đã sẵn sàng. Gõ /start để bắt đầu.");
}

void loop() {
  static uint count_ = 0;
  static unsigned long lastTimeBotRan = 0;
  const int botRequestDelay = 1000; // Kiểm tra tin nhắn mỗi 1 giây

  // 1. Xử lý Cảm biến chuyển động
  if(motionDetected){
    ++count_;
    String msg = StringFormat("%u => Motion detected!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.println("Da gui canh bao chuyen dong len Telegram");
    motionDetected = false;
  }

  // 2. Xử lý nhận lệnh điều khiển LED từ Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}