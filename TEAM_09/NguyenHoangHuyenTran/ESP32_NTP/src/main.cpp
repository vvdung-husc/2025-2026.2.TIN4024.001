// --- ĐIỀN THÔNG TIN BLYNK VÀ TELEGRAM VÀO ĐÂY ---
#define BLYNK_TEMPLATE_ID "TMPL6UYLCgh3A"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "-8UZNaukBlqlEBgJbVAT8NnWmPi9SGq-"
#define BOTtoken "8692886537:AAEU2IsE_BAVi0gjn5im2NHNBpNaD1spAJk"
#define CHAT_ID "6131894938" 

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "time.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>

// --- CẤU HÌNH WIFI & MÚI GIỜ ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600; // GMT+7
const int   daylightOffset_sec = 0;

// --- CẤU HÌNH PHẦN CỨNG ---
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(14, DHT22);
#define LED_GREEN 25
#define LED_RED 26

// --- KHỞI TẠO BLYNK & TELEGRAM ---
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
BlynkTimer timer;

// Biến lưu trữ
float lastTemp = 0;
float lastHum = 0;
String currentTime = "00:00:00";
String currentDate = "00/00/0000";

// ================= CÁC HÀM XỬ LÝ ĐA NHIỆM =================

// 1. Hàm cập nhật thời gian từ NTP và vẽ lên OLED (Chạy mỗi 1 giây)
void updateTimeAndOLED() {
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    char timeStr[10];
    char dateStr[15];
    sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    sprintf(dateStr, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    currentTime = String(timeStr);
    currentDate = String(dateStr);
  }

  display.clearDisplay();
  
  // In Giờ
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 5);
  display.print(currentTime);
  
  // In Ngày
  display.setTextSize(1);
  display.setCursor(30, 30);
  display.print(currentDate);

  // In Nhiệt độ
  display.setCursor(0, 50);
  display.print("Temp: ");
  display.print(lastTemp);
  display.print(" C");

  display.display();
}

// 2. Hàm đọc cảm biến, gửi Blynk và cảnh báo (Chạy mỗi 2 giây)
void readSensorsAndBlynk() {
  lastTemp = dht.readTemperature();
  lastHum = dht.readHumidity();

  if (isnan(lastTemp) || isnan(lastHum)) return;

  // Cảnh báo LED đỏ nếu quá 30 độ
  if (lastTemp > 30.0) {
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(LED_RED, LOW);
  }

  // Đẩy dữ liệu lên Blynk Web & Mobile
  Blynk.virtualWrite(V1, lastTemp);
  Blynk.virtualWrite(V2, lastHum);
  Blynk.virtualWrite(V3, currentTime + " - " + currentDate);
}

// 3. Hàm kiểm tra tin nhắn Telegram (Chạy mỗi 2 giây)
void pollTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      if (text == "/start") {
        bot.sendMessage(chat_id, "Chào Công chúa! Gõ /time để xem giờ, /temp để xem nhiệt độ.", "");
      }
      else if (text == "/time") {
        bot.sendMessage(chat_id, "⏰ Bây giờ là: " + currentTime + "\n📅 Ngày: " + currentDate, "");
      }
      else if (text == "/temp") {
        bot.sendMessage(chat_id, "🌡️ Nhiệt độ: " + String(lastTemp) + " °C", "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ================= SETUP & LOOP =================
void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // 1. Kết nối WiFi có in thông báo ra Bảng đen
  Serial.println("\nĐang kết nối WiFi Wokwi-GUEST...");
  WiFi.begin(ssid, password);
  secured_client.setInsecure(); // Bỏ qua SSL cho Telegram Wokwi
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nWiFi OK! Đã hòa mạng.");

  // 2. ÉP DNS GOOGLE (8.8.8.8) - THUỐC ĐẶC TRỊ LỖI MÙ ĐƯỜNG NTP TRÊN WOKWI
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8, 8, 8, 8));

  // 3. Cấu hình NTP sau khi đã có DNS xịn
  Serial.println("Đang đồng bộ thời gian với máy chủ NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // 4. Cấu hình Blynk
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80); 

  digitalWrite(LED_GREEN, HIGH); // Sáng đèn báo hiệu sẵn sàng
  Serial.println("Hệ thống Đa nhiệm sẵn sàng hoạt động!");

  // Phân luồng công việc (Đa nhiệm)
  timer.setInterval(1000L, updateTimeAndOLED);
  timer.setInterval(2000L, readSensorsAndBlynk);
  timer.setInterval(5000L, pollTelegram);
}

void loop() {
  Blynk.run();
  timer.run();
}