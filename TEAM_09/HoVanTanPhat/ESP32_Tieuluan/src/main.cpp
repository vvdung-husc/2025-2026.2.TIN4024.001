/*
 * Đồ án: Giám sát năng lượng tiêu thụ với ESP32
 * Chức năng: Giả lập ACS712, tính công suất, giám sát qua Blynk (Web & Mobile) và cảnh báo Telegram
 */
#define BLYNK_TEMPLATE_ID "TMPL6UYLCgh3A"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "-8UZNaukBlqlEBgJbVAT8NnWmPi9SGq-"


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- CẤU HÌNH WIFI (Wokwi mặc định) ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- CẤU HÌNH TELEGRAM ---
// Tạo bot qua @BotFather trên Telegram để lấy Token
#define BOTtoken "8692886537:AAEU2IsE_BAVi0gjn5im2NHNBpNaD1spAJk"
#define CHAT_ID "6131894938" 

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);

// --- CẤU HÌNH PHẦN CỨNG ---
const int ledPin = 2;        // Đèn LED cảnh báo
const float VOLTAGE = 220.0; // Điện áp lưới giả định

// --- BIẾN ĐẾM THỜI GIAN ---
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 5000; // Cập nhật dữ liệu 5 giây/lần

unsigned long lastTelegramAlert = 0;
const unsigned long telegramCooldown = 60000; // Cooldown 60s để tránh spam tin nhắn Telegram
bool isFirstAlert = true; // Cờ đánh dấu cho phép gửi Telegram ngay lần đầu tiên

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println("Đang kết nối WiFi...");
  
  // Cấu hình chứng chỉ bảo mật cho Telegram
  secured_client.setInsecure(); 

  // Kết nối Blynk và WiFi
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("\nKhởi động hệ thống thành công!");
  
  // Gửi tin nhắn chào mừng lên Telegram
  bot.sendMessage(CHAT_ID, "🟢 Hệ thống Giám sát năng lượng ESP32 đã khởi động!", "");
}

void loop() {
  Blynk.run();

  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();

    // 1. Giả lập đọc cảm biến ACS712 (0A - 5A)
    float current = random(0, 500) / 100.0; 
    float power = current * VOLTAGE;

    // 2. In ra Terminal (Đã thêm \r\n để sửa lỗi chữ bị thụt lùi thành hình bậc thang)
    Serial.printf("Dòng điện: %.2f A | Công suất: %.2f W\r\n", current, power);

    // 3. Gửi dữ liệu lên Blynk Cloud
    Blynk.virtualWrite(V1, current);
    Blynk.virtualWrite(V2, power);

    // 4. Xử lý logic cảnh báo quá tải (> 4.0A)
    if (current > 4.0) {
      Serial.println("⚠️ CẢNH BÁO: Quá tải dòng điện!");
      digitalWrite(ledPin, HIGH); // Bật LED

      Blynk.logEvent("current_warning", "Cảnh báo: Dòng điện vượt ngưỡng 4A!");

      // Gửi tin nhắn Telegram ngay lần đầu, các lần sau phải cách nhau 60s
      if (isFirstAlert || millis() - lastTelegramAlert >= telegramCooldown) {
        String alertMsg = "⚠️ **BÁO ĐỘNG QUÁ TẢI** ⚠️\n";
        alertMsg += "Dòng điện hiện tại: " + String(current, 2) + " A\n";
        alertMsg += "Công suất: " + String(power, 2) + " W\n";
        alertMsg += "Vui lòng kiểm tra thiết bị ngay lập tức!";
        
        bot.sendMessage(CHAT_ID, alertMsg, "Markdown");
        
        lastTelegramAlert = millis();
        isFirstAlert = false; // Đã gửi lần đầu, tắt cờ
        Serial.println("✅ Đã gửi tin nhắn cảnh báo qua Telegram.");
      } else {
        Serial.println("⏳ Đang trong thời gian chờ (cooldown 60s), bỏ qua gửi Telegram để chống spam.");
      }
    } else {
      digitalWrite(ledPin, LOW); // Tắt LED nếu an toàn
    }
  }
}