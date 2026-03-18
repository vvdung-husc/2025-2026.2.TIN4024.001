/*
THÔNG TIN NHÓM 05
1. Nguyễn Công Hiếu
2. Phạm Đức Thành Đạt
3. Trần Văn Tiến
4.
*/
#include "secrets.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>      

// --- CẤU HÌNH WIFI WOKWI ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH CHÂN LINH KIỆN ---
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BlynkTimer timer;

// Biến lưu trạng thái để Telegram và Blynk đồng bộ
float lastTemp = 0.0;
float lastHum = 0.0;
bool ledState = false;

// Hàm cập nhật chữ LED trên màn hình OLED
void updateOLED_LED() {
  display.fillRect(0, 40, 128, 24, SSD1306_BLACK); // Xóa vùng dưới của OLED
  display.setCursor(0, 45);
  display.print("LED: ");
  display.println(ledState ? "ON" : "OFF");
  display.display();
}

// Hàm đồng bộ trạng thái LED từ Blynk (Datastream V1)
BLYNK_WRITE(V1) {
  ledState = param.asInt() == 1; 
  digitalWrite(LED_PIN, ledState);
  updateOLED_LED();
}

// Hàm xử lý các lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1); // Đồng bộ trạng thái lên app Blynk
      updateOLED_LED();
      bot.sendMessage(chat_id, "Đèn LED đã được bật", "");
    } 
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0); // Đồng bộ trạng thái lên app Blynk
      updateOLED_LED();
      bot.sendMessage(chat_id, "Đèn LED đã được tắt!", "");
    } 
    else if (text == "/led_status") {
      if (ledState) bot.sendMessage(chat_id, "Trạng thái: Đèn đang bật!", "");
      else bot.sendMessage(chat_id, "Trạng thái: Đèn đang tắt!", "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "Nhiệt độ hiện tại: " + String(t) + "°C\n";
      msg += "Độ ẩm hiện tại: " + String(h) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// Hàm kiểm tra tin nhắn mới trên Telegram
void checkTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// Hàm gửi Uptime mỗi 1 giây
void sendUptime() {
  Blynk.virtualWrite(V0, millis() / 1000);
}

// Hàm đọc cảm biến và gửi lên Blynk mỗi 2 giây
void sendSensorData() {
  // Đọc và gửi Nhiệt độ (V2), Độ ẩm (V3)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);

    // Gửi cảnh báo Telegram nếu nhiệt độ lệch 1 độ, hoặc độ ẩm lệch 2%
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 2.0) {
      if (lastTemp != 0.0) { // Bỏ qua lần báo rác khi mới khởi động
        String msg = "Cập nhật thời tiết thay đổi!\n";
        msg += "Nhiệt độ: " + String(t) + "°C\n";
        msg += "Độ ẩm: " + String(h) + "%";
        bot.sendMessage(CHAT_ID, msg, "");
      }
      lastTemp = t;
      lastHum = h;
    }
  }

  // Sinh ngẫu nhiên dữ liệu khí ga (V4) - Do Wokwi không có MQ2
  int randomGas = random(0, 1024); 
  Blynk.virtualWrite(V4, randomGas);

  // Gửi thông tin Team (V5, V6, V7)
  Blynk.virtualWrite(V5, "1. Nguyen Cong Hieu");
  Blynk.virtualWrite(V7, "3. Tran Van Tien");
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chứng chỉ bảo mật cho kết nối Telegram
  secured_client.setInsecure();

  // Khởi tạo chân LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Khởi tạo DHT22
  dht.begin();

  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khởi tạo OLED thất bại!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEAM 05 - IOT");
  display.println("1. Nguyen Cong Hieu");
  display.println("2. Pham Thanh Dat");
  display.println("3. Tran Van Tien");
  display.display(); 

  // Kết nối WiFi và Blynk
  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(500L, sendUptime);
  timer.setInterval(2500L, sendSensorData);
  timer.setInterval(200L, checkTelegram); 
}

void loop() {
  Blynk.run();
  timer.run();
}