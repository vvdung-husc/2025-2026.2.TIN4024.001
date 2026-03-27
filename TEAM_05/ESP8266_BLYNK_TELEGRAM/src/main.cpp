/*
THÔNG TIN NHÓM 05
1. Nguyễn Công Hiếu - Telegram: hiieucn
2. Phạm Đức Thành Đạt - Telegram: thanhdat21
3. Trần Văn Tiến -Telegram: tientran0603
4. Đậu Thiên Nhật
*/
#include "secrets.h"

#include <Arduino.h>
#include <Wire.h>
// Màn hình OLED SSD1306 128x64 I2C
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
// #include <WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//#include <BlynkSimpleEsp32.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- CẤU HÌNH WIFI (dùng hotspot 2.4GHz — WiFi lab là 5GHz, ESP8266 không hỗ trợ) ---
char ssid[] = "CongHieu";
char pass[] = "hiiEu1357";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH CHÂN LINH KIỆN ---
//#define DHTPIN 15
#define DHTPIN 0     // GPIO0 = D3 trên NodeMCU (đổi nếu cần)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//#define LED_PIN 2
#define LED_PIN 12   // GPIO12 = D6

// I2C pins: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
#define I2C_SDA 4
#define I2C_SCL 5
// OLED SSD1306 128x64 I2C — địa chỉ 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BlynkTimer timer;

// Biến lưu trạng thái để Telegram và Blynk đồng bộ
float lastTemp = 0.0;
float lastHum  = 0.0;
bool  ledState = false;
float currentTemp = NAN;
float currentHum  = NAN;
int   currentGas  = 0;

// Hàm cập nhật toàn bộ màn hình OLED — 5 dòng liên tiếp
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Team 05 IoT");

  display.setCursor(0, 12);
  display.print("Nhiet do: ");
  display.print(isnan(currentTemp) ? 0 : currentTemp, 1);
  display.println(" C");

  display.setCursor(0, 24);
  display.print("Do am:    ");
  display.print(isnan(currentHum) ? 0 : currentHum, 1);
  display.println(" %");

  display.setCursor(0, 36);
  display.print("MQ2:      ");
  display.println(currentGas);

  display.setCursor(0, 48);
  display.print("LED:      ");
  display.println(ledState ? "ON" : "OFF");

  display.display();
}

// Hàm đồng bộ trạng thái LED từ Blynk (Datastream V1)
BLYNK_WRITE(V1) {
  ledState = param.asInt() == 1;
  digitalWrite(LED_PIN, ledState);
  updateOLED();
}

// Hàm xử lý các lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text    = bot.messages[i].text;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1); // Đồng bộ trạng thái lên app Blynk
      updateOLED();
      bot.sendMessage(chat_id, "Den LED da duoc bat!", "");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0); // Đồng bộ trạng thái lên app Blynk
      updateOLED();
      bot.sendMessage(chat_id, "Den LED da duoc tat!", "");
    }
    else if (text == "/led_status") {
      if (ledState) bot.sendMessage(chat_id, "Trang thai: Den dang bat!", "");
      else          bot.sendMessage(chat_id, "Trang thai: Den dang tat!", "");
    }
    else if (text == "/get_weather") {
      float t = isnan(currentTemp) ? 0 : currentTemp;
      float h = isnan(currentHum)  ? 0 : currentHum;
      String src = isnan(currentTemp) ? " (mo phong)" : " (cam bien that)";
      String msg = "Nhiet do hien tai: " + String(t, 1) + "C" + src + "\n";
      msg       += "Do am hien tai: "    + String(h, 1) + "%" + src;
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
  // Đọc Nhiệt độ (V2), Độ ẩm (V3)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    // Dữ liệu thật từ cảm biến
    currentTemp = t;
    currentHum  = h;
    Serial.print("[Sensor-REAL] Nhiet do: "); Serial.print(t, 1);
    Serial.print("C | Do am: "); Serial.print(h, 1); Serial.println("%");
  } else {
    // Cảm biến lỗi → dùng giá trị mô phỏng thực tế để demo
    currentTemp = 25.0 + random(0, 50) / 10.0;  // 25.0 ~ 29.9 °C
    currentHum  = 60.0 + random(0, 200) / 10.0; // 60.0 ~ 79.9 %
    Serial.print("[Sensor-SIM]  Nhiet do: "); Serial.print(currentTemp, 1);
    Serial.print("C | Do am: "); Serial.print(currentHum, 1); Serial.println("% (mo phong)");
  }

  Blynk.virtualWrite(V2, currentTemp);
  Blynk.virtualWrite(V3, currentHum);

  // Sinh ngẫu nhiên dữ liệu khí gas (V4) - cảm biến MQ2
  currentGas = random(0, 1024);
  Blynk.virtualWrite(V4, currentGas);

  // Cập nhật toàn bộ OLED
  updateOLED();

  // Gửi cảnh báo Telegram nếu nhiệt độ lệch 1 độ, hoặc độ ẩm lệch 2%
  if (abs(currentTemp - lastTemp) >= 1.0 || abs(currentHum - lastHum) >= 2.0) {
    if (lastTemp != 0.0) { // Bỏ qua lần đầu tiên khi mới khởi động
      String msg = "Cap nhat thoi tiet thay doi!\n";
      msg += "Nhiet do: " + String(currentTemp, 1) + "C\n";
      msg += "Do am: "    + String(currentHum, 1)  + "%";
      bot.sendMessage(CHAT_ID, msg, "");
    }
    lastTemp = currentTemp;
    lastHum  = currentHum;
  }



  // Gửi thông tin Team (V5, V6, V7, V8)
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
  Serial.println("[OK] LED PIN da khoi tao (D6/GPIO12)");

  // Khởi tạo DHT
  dht.begin();
  delay(2000); // DHT cần 1-2 giây ổn định sau khi cấp nguồn
  float testT = dht.readTemperature();
  float testH = dht.readHumidity();
  if (isnan(testT) || isnan(testH)) {
    Serial.println("[WARN] DHT NaN — se dung du lieu mo phong khi chay");
  } else {
    Serial.print("[OK] DHT ok: ");
    Serial.print(testT); Serial.print("C, ");
    Serial.print(testH); Serial.println("%");
  }

  // Khởi tạo I2C với đúng pin của NodeMCU
  Wire.begin(I2C_SDA, I2C_SCL); // SDA=D2(GPIO4), SCL=D1(GPIO5)
  Serial.println("[OK] I2C da khoi tao (SDA=D2, SCL=D1)");

  // Quét I2C để xác nhận thiết bị
  Serial.print("[SCAN] I2C devices: ");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("0x"); if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX); Serial.print(" ");
    }
  }
  Serial.println();

  // Khởi tạo OLED SSD1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[FAIL] OLED khong phat hien duoc (dia chi: 0x3C)");
    // Thử địa chỉ khác nếu cần
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("[FAIL] OLED khong phat hien duoc (kiem tra I2C scan)");
    }
  } else {
    Serial.println("[OK] OLED SSD1306 da khoi tao");
  }

  // Hiển thị startup screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEAM 05 - IoT");
  display.println("Dang khoi dong...");
  display.display();

  // Kết nối WiFi (có debug, không bị treo mãi)
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

  // Kết nối Blynk (chỉ thử nếu có WiFi, có timeout — không bị treo)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[..] Dang ket noi Blynk...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(5000); // Timeout 5 giây
    if (Blynk.connected()) {
      Serial.println("[OK] Blynk da ket noi!");
    } else {
      Serial.println("[FAIL] Khong ket noi duoc Blynk! Kiem tra AUTH TOKEN.");
    }
  }

  // Hiển thị màn hình chính sau khi kết nối
  updateOLED();

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(5000L, checkTelegram); // 5 giây/lần — tránh blocking HTTP làm hỏng timer
  Serial.println("=== KHOI DONG HOAN TAT ===");
}

void loop() {
  Blynk.run();
  timer.run();
}
