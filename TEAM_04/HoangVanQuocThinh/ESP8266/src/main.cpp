/**
 * PROJECT SUMMARY: Hệ thống giám sát Nhiệt độ & Độ ẩm
 * -------------------------------------------------
 * Board: ESP8266 (NodeMCU v2/v3)
 * Cảm biến: DHT22 (Kết nối chân D3)
 * Hiển thị: OLED SH1106 I2C (SDA -> D2, SCL -> D1)
 * Chức năng: 
 * 1. Đọc dữ liệu môi trường từ DHT22.
 * 2. Hiển thị nhiệt độ/độ ẩm lên màn hình OLED và Serial Monitor.
 * 3. Nháy LED báo hiệu trạng thái hệ thống hoạt động.
 */

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Cấu hình chân cắm (Đã chuẩn hóa cho ESP8266)
#define DHTPIN D3
#define DHTTYPE DHT22
#define LED_PIN D4   // LED tích hợp trên board NodeMCU thường là D4

// Khởi tạo cảm biến và màn hình
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  // Khởi tạo I2C: D2 là SDA, D1 là SCL trên ESP8266
  Wire.begin(D2, D1);

  // Khởi tạo OLED
  if (!display.begin(0x3C, true)) {
    Serial.println(F("Không tìm thấy màn hình OLED SH1106"));
    for (;;); // Dừng chương trình nếu không có màn hình
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.display();
}

void loop() {
  // Đọc dữ liệu
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Kiểm tra nếu cảm biến bị lỗi
  if (isnan(temp) || isnan(hum)) {
    Serial.println(F("Lỗi: Không thể đọc từ cảm biến DHT!"));
    return;
  }

  // Xuất ra Serial Monitor
  Serial.printf("Nhiệt độ: %.2f °C | Độ ẩm: %.2f %%\n", temp, hum);

  // Hiển thị lên OLED
  display.clearDisplay();
  
  display.setCursor(0, 5);
  display.println("HE THONG GIAM SAT");
  display.drawLine(0, 15, 127, 15, SH110X_WHITE);

  display.setCursor(0, 25);
  display.printf("Temp: %.1f C", temp);

  display.setCursor(0, 45);
  display.printf("Hum:  %.1f %%", hum);

  display.display();

  // Blink LED báo hiệu chu kỳ hoạt động
  digitalWrite(LED_PIN, LOW);  // ESP8266 LED thường sáng khi ở mức LOW
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(1500); // Đợi 2 giây trước lần đọc tiếp theo (DHT22 cần ít nhất 2s)
}