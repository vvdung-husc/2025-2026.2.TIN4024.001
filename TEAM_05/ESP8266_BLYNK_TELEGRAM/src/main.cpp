/*
THÔNG TIN NHÓM 05
1. Nguyễn Công Hiếu - Telegram: hiieunc
2. Phạm Đức Thành Đạt
3. Trần Văn Tiến
4.
*/
#include "secrets.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
//#include <BlynkSimpleEsp32.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>

// --- CẤU HÌNH WIFI THẬT (đổi thành đúng tên WiFi và mật khẩu của bạn) ---
char ssid[] = "LAB502-02-8247";
char pass[] = "24032026";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH CHÂN LINH KIỆN ---
//#define DHTPIN 15
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//#define LED_PIN 2
#define LED_PIN 12

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
  Blynk.virtualWrite(V6, "2. Pham Duc Thanh Dat");
  Blynk.virtualWrite(V7, "3. Tran Van Tien");
  Blynk.virtualWrite(V8, "4. Dau Thien Nhat");
}

void setup() {
  Serial.begin(115200);
  delay(500); // Cho Serial ổn định trước khi in
  Serial.println("\n\n=== KHOI DONG THIET BI ===");

  // Cấu hình chứng chỉ bảo mật cho kết nối Telegram
  secured_client.setInsecure();

  // Khởi tạo chân LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[OK] LED PIN da khoi tao");

  // Khởi tạo DHT22
  dht.begin();
  Serial.println("[OK] DHT22 da khoi tao");

  // Khởi tạo OLED
  Serial.println("[..] Dang khoi tao OLED...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[FAIL] OLED khong phan hoi! Kiem tra dia chi I2C (0x3C hoac 0x3D)"));
    // Không for(;;) — tiếp tục chạy dù không có OLED để còn debug được
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("TEAM 05 - IOT");
    display.println("1. Nguyen Cong Hieu");
    display.println("2. Pham Thanh Dat");
    display.println("3. Tran Van Tien");
    display.display();
    Serial.println("[OK] OLED da khoi tao");
  }

  // Kết nối WiFi trước (có debug)
  Serial.print("[..] Dang ket noi WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[OK] WiFi da ket noi! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[FAIL] Khong ket noi duoc WiFi! Kiem tra ten/mat khau WiFi.");
    Serial.println("      => Chuong trinh se tiep tuc nhung Blynk/Telegram se khong hoat dong.");
  }

  // Kết nối Blynk (chỉ thử nếu có WiFi)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[..] Dang ket noi Blynk...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(5000); // Timeout 5 giây, không bị treo mãi
    if (Blynk.connected()) {
      Serial.println("[OK] Blynk da ket noi!");
    } else {
      Serial.println("[FAIL] Khong ket noi duoc Blynk! Kiem tra AUTH TOKEN.");
    }
  }

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1500L, checkTelegram);
  Serial.println("=== KHOI DONG HOAN TAT ===");
}

void loop() {
  Blynk.run();
  timer.run();
}