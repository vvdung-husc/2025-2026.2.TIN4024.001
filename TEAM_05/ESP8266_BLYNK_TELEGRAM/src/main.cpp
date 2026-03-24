/*
THÔNG TIN NHÓM 05
1. Nguyễn Công Hiếu - Telegram: hiieucn
2. Phạm Đức Thành Đạt - Telegram: thanhdat21
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

// --- CẤU HÌNH WIFI ---
char ssid[] = "CongHieu";   
char pass[] = "hiiEu1357";  

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH CHÂN LINH KIỆN ---
// ⚠ Đổi DHTPIN cho đúng sau khi chạy scanDHTPin() để xác định chân
#define DHTPIN 14  // GPIO14 = D5 — đổi nếu scan tìm ra pin khác
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Hàm scan tự động tìm chân DATA và loại cảm biến DHT
void scanDHTPin() {
  int pins[]    = { 0,  2, 13, 14, 12, 15 };
  String names[]= {"D3","D4","D7","D5","D6","D8"};
  int count = 6;
  int types[]   = { DHT11, DHT22 };
  String typeNames[] = { "DHT11", "DHT22" };

  Serial.println("[SCAN] Thu DHT11 va DHT22 tren tat ca pin...");
  for (int t = 0; t < 2; t++) {
    Serial.println("[SCAN] --- Loai: " + typeNames[t] + " ---");
    for (int i = 0; i < count; i++) {
      DHT testDHT(pins[i], types[t]);
      testDHT.begin();
      delay(1500);
      float temp = testDHT.readTemperature();
      float humi = testDHT.readHumidity();
      if (!isnan(temp) && !isnan(humi)) {
        Serial.println(">>> [FOUND] Loai: " + typeNames[t] + " | Chan: " + names[i] + " (GPIO" + String(pins[i]) + ")");
        Serial.println("    Nhiet do: " + String(temp) + "C | Do am: " + String(humi) + "%");
        Serial.println("    => Doi: #define DHTPIN " + String(pins[i]));
        Serial.println("    => Doi: #define DHTTYPE " + typeNames[t]);
        return;
      } else {
        Serial.println("    " + typeNames[t] + " pin " + names[i] + ": NaN");
      }
    }
  }
  Serial.println("[SCAN] KHONG TIM THAY! Nguyen nhan co the:");
  Serial.println("  - Day DATA cam bien bi long/khong noi");
  Serial.println("  - Cam bien bi hong");
  Serial.println("  - Thieu dien tro pull-up 4.7k ohm");
}

//#define LED_PIN 2
#define LED_PIN 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// I2C pins cho NodeMCU ESP8266: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
#define I2C_SDA 4
#define I2C_SCL 5

BlynkTimer timer;

// Biến lưu trạng thái để Telegram và Blynk đồng bộ
float lastTemp = 0.0;
float lastHum = 0.0;
bool ledState = false;

// Hàm cập nhật nhiệt độ & độ ẩm lên OLED (dòng 2 và 3)
void updateOLED_Sensor(float t, float h) {
  display.fillRect(0, 16, 128, 24, SSD1306_BLACK); // Xóa vùng giữa
  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(t, 1);
  display.println(" C");
  display.setCursor(0, 28);
  display.print("Humi: ");
  display.print(h, 1);
  display.println(" %");
  display.display();
}

// Hàm cập nhật trạng thái LED lên OLED (dòng 4)
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

    // Cập nhật OLED hiển thị nhiệt độ & độ ẩm mới nhất
    updateOLED_Sensor(t, h);
    Serial.print("[Sensor] Temp: "); Serial.print(t);
    Serial.print(" C | Humi: "); Serial.print(h); Serial.println(" %");

    // Gửi cảnh báo Telegram nếu nhiệt độ lệch 1 độ, hoặc độ ẩm lệch 2%
    if (abs(t - lastTemp) >= 1.0 || abs(h - lastHum) >= 2.0) {
      if (lastTemp != 0.0) { // Bỏ qua lần báo rác khi mới khởi động
        String msg = "Cap nhat thoi tiet thay doi!\n";
        msg += "Nhiet do: " + String(t) + " C\n";
        msg += "Do am: " + String(h) + "%";
        bot.sendMessage(CHAT_ID, msg, "");
      }
      lastTemp = t;
      lastHum = h;
    }
  } else {
    Serial.println("[WARN] Doc DHT22 that bai (NaN)! Kiem tra chan cam bien.");
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

  // Quét tìm đúng chân DHT (xóa hoặc comment dòng này sau khi đã biết DHTPIN đúng)
  scanDHTPin();
  // Khởi tạo DHT22 với DHTPIN đã định nghĩa
  dht.begin();
  delay(2000);
  // Thử đọc ngay để kiểm tra
  float testT = dht.readTemperature();
  float testH = dht.readHumidity();
  if (isnan(testT) || isnan(testH)) {
    Serial.println();
    Serial.println("[WARN] DHT22 NaN ngay khi bat dau! Co the:");
    Serial.println("  1. Sai chan DATA: DHTPIN=" + String(DHTPIN) + " (D" + String(DHTPIN==14?5:DHTPIN==12?6:DHTPIN==13?7:0) + ")");
    Serial.println("  2. Sai loai cam bien (DHT11 vs DHT22)");
    Serial.println("  3. Thieu dien tro pull-up 4.7k tren day DATA");
  } else {
    Serial.print("[OK] DHT22 ok: ");
    Serial.print(testT); Serial.print("C, ");
    Serial.print(testH); Serial.println("%");
  }

  // Khởi tạo I2C với đúng pin của NodeMCU
  Wire.begin(I2C_SDA, I2C_SCL); // SDA=D2(GPIO4), SCL=D1(GPIO5)
  Serial.println("[OK] I2C da khoi tao (SDA=D2, SCL=D1)");

  // Khởi tạo OLED
  Serial.println("[..] Dang khoi tao OLED tai 0x3C...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[FAIL] 0x3C that bai, thu 0x3D..."));
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("[FAIL] OLED khong phan hoi ca 0x3C lan 0x3D! Kiem tra day I2C."));
    } else {
      Serial.println("[OK] OLED da khoi tao tai 0x3D!");
    }
  } else {
    Serial.println("[OK] OLED da khoi tao tai 0x3C");
  }
  // Hiển thị màn hình chào
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEAM 05 - IOT");
  display.println("Temp: --.- C");
  display.println("Humi: --.- %");
  display.println("LED: OFF");
  display.display();

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
  timer.setInterval(5000L, checkTelegram); // 5 giây/lần — tránh blocking HTTP làm hỏng timer
  Serial.println("=== KHOI DONG HOAN TAT ===");
}

void loop() {
  Blynk.run();
  timer.run();
}
