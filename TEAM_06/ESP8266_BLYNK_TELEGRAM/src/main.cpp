/*
THÔNG TIN NHÓM 6
1. [Điền tên thành viên 1]
2. [Điền tên thành viên 2]
3. [Điền tên thành viên 3]
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>        // Dùng thư viện WiFi của ESP8266
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẤU HÌNH BLYNK ---
#define BLYNK_TEMPLATE_ID "TMPL6kLn4Q90b"
#define BLYNK_TEMPLATE_NAME "ESP8266 LED"
#define BLYNK_AUTH_TOKEN "5Fm1PX1fbEFIhICnvcuaUzkDGnl0iai5"
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h> // Dùng thư viện Blynk của ESP8266

// --- CẤU HÌNH WIFI THẬT ---
// BẮT BUỘC ĐỔI THÀNH WIFI NHÀ BẠN HOẶC WIFI TỪ ĐIỆN THOẠI PHÁT RA
const char* ssid = "TEN_WIFI_CUA_BAN";
const char* password = "MAT_KHAU_WIFI_CUA_BAN";

// --- CẤU HÌNH TELEGRAM ---
#define BOT_TOKEN "8425397338:AAGVueSsBC0ja8T2zSIwtBaJhgITeoyo6Y0"
#define CHAT_ID "7406381600"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// --- CẤU HÌNH CHÂN PHẦN CỨNG (CHO NODE MCU ESP8266) ---
#define LED_PIN D4     // Chân cắm LED
#define DHTPIN D3      // Chân cắm cảm biến DHT
#define DHTTYPE DHT11  // Mạch thật thường dùng DHT11 (Màu xanh dương). Nếu màu trắng thì đổi thành DHT22
DHT dht(DHTPIN, DHTTYPE);

// Màn hình OLED I2C tự động nhận chân: D1 (SCL) và D2 (SDA)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BlynkTimer timer;

float lastTemp = 0;
float lastHum = 0;

// --- HÀM CẬP NHẬT MÀN HÌNH OLED ---
void updateOLED(float t, float h, int gas) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0, 0);
  display.print("NHOM 6 - IOT");
  
  display.setCursor(0, 15);
  display.printf("Nhiet do: %.1f C", t);
  
  display.setCursor(0, 30);
  display.printf("Do am: %.1f %%", h);

  display.setCursor(0, 45);
  display.printf("Khi gas: %d ppm", gas);
  
  display.display();
}

// --- HÀM ĐỌC CẢM BIẾN ---
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    return;
  }

  int gasValue = random(300, 800); // Random khí gas mô phỏng

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, "Team 6 - IoT");

  updateOLED(t, h, gasValue);

  if (abs(t - lastTemp) > 1.0 || abs(h - lastHum) > 5.0) {
    String msg = "⚠️ Cảnh báo thời tiết thay đổi:\n";
    msg += "Nhiệt độ: " + String(t) + " °C\n";
    msg += "Độ ẩm: " + String(h) + " %";
    bot.sendMessage(CHAT_ID, msg, "");
    
    lastTemp = t;
    lastHum = h;
  }
}

void sendUptime() {
  Blynk.virtualWrite(V0, millis() / 1000);
}

// --- BLYNK ĐIỀU KHIỂN LED ---
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  digitalWrite(LED_PIN, pinValue);
}

// --- TELEGRAM XỬ LÝ LỆNH ---
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
      bot.sendMessage(chat_id, "💡 LED đã BẬT", "");
    } 
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0);
      bot.sendMessage(chat_id, "🌑 LED đã TẮT", "");
    } 
    else if (text == "/led_status") {
      if (digitalRead(LED_PIN) == HIGH) {
        bot.sendMessage(chat_id, "Trạng thái: LED đang BẬT 💡", "");
      } else {
        bot.sendMessage(chat_id, "Trạng thái: LED đang TẮT 🌑", "");
      }
    }
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "🌤️ Thời tiết hiện tại:\n";
      msg += "Nhiệt độ: " + String(t) + " °C\n";
      msg += "Độ ẩm: " + String(h) + " %";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  dht.begin();
  
  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khởi tạo OLED thất bại"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Dang ket noi WiFi...");
  display.display();

  // Cấu hình mạng
  WiFi.begin(ssid, password);
  client.setInsecure(); // Bắt buộc cho ESP8266 khi dùng Telegram
  Blynk.config(BLYNK_AUTH_TOKEN);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, "🚀 Hệ thống MẠCH THẬT - Team 6 đã khởi động!", "");
  }

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(3000L, sendSensorData);
}

// --- LOOP ---
void loop() {
  Blynk.run();
  timer.run();

  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}