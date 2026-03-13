#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- THÔNG TIN MẠNG VÀ TELEGRAM BOT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOTtoken "8347100449:AAGxaeke_SLpRm7LUhF0fUDirTW4yPtO-iM"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "1900826323" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// --- ĐỊNH NGHĨA CHÂN THIẾT BỊ ---
const int motionSensor = 27; // Cảm biến chuyển động PIR
const int ledPin = 4;        // Đã sửa thành chân 4 cho khớp chuẩn với sơ đồ dây của bạn

bool motionDetected = false;

// Các biến phục vụ việc nhận tin nhắn
unsigned long lastTimeBotRan = 0;
const unsigned long botRequestDelay = 1000; // Kiểm tra tin nhắn mỗi 1 giây

// --- CÁC HÀM XỬ LÝ ---

// Hàm định dạng chuỗi
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

// Hàm ngắt (Interrupt) khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// Hàm xử lý lệnh điều khiển đèn LED từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển hệ thống:\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }
    else if (text == "/get_state") {
      if (digitalRead(ledPin) == HIGH) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

// --- SETUP & KHỞI ĐỘNG ---

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== IoT Telegram Motion Detection & LED - Starting ===");

  // 1. Cấu hình chân phần cứng
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Đảm bảo đèn tắt khi mới khởi động
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // 2. KẾT NỐI WIFI (Cấu hình IP tĩnh bypass DNS)
  Serial.printf("Connecting to WiFi: %s ", ssid);
  
  IPAddress localIP(10, 13, 37, 55);       
  IPAddress gateway(10, 13, 37, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);         
  IPAddress secondaryDNS(1, 1, 1, 1);        

  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("\n[ERROR] WiFi.config failed!");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 30000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[CRITICAL] WiFi connect FAILED after 30s! Restarting...");
    ESP.restart();  
  }

  Serial.println("\nWiFi connected SUCCESS!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

  client.setInsecure(); 

  // Wokwi cần một khoảng nghỉ dài để ổn định Gateway mạng
  Serial.println("Waiting for network full stable (Wokwi needs this)...");
  delay(10000); 

  // 3. Test DNS ngay lập tức
  IPAddress resolved;
  if (WiFi.hostByName("api.telegram.org", resolved)) {
    Serial.printf("DNS TEST SUCCESS: api.telegram.org -> %s\n", resolved.toString().c_str());
  } else {
    Serial.println("[DNS TEST FAIL] Still cannot resolve api.telegram.org!");
  }

  // 4. Báo cáo hệ thống sẵn sàng qua Telegram
  String testMsg = "Hệ thống IoT đã sẵn sàng!\n";
  testMsg += "Đã kết nối thành công điều khiển LED và Cảm biến chuyển động.";

  Serial.println("Sending welcome message to Telegram...");
  if (bot.sendMessage(GROUP_ID, testMsg, "")) {
    Serial.println("→ Test message SENT OK!");
  } else {
    Serial.println("→ Send FAILED - check network or ID");
  }

  Serial.println("=== Setup done - System Running ===");
}

// --- VÒNG LẶP CHÍNH ---

void loop() {
  static uint count_ = 0;

  // 1. Quét cảm biến chuyển động
  if(motionDetected){
    ++count_;
    Serial.print(count_); Serial.println(". MOTION DETECTED => Sending to Telegram");    
    String msg = StringFormat("%u => Motion detected! Có chuyển động lạ!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_); Serial.println(". Sent successfully to Telegram");
    motionDetected = false;
  }

  // 2. Chờ và quét tin nhắn lệnh từ Telegram
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}