
//* BÁO CÁO THỰC HÀNH IOT
//* NHÓM 14:
//* 1. Phan Đại Tấn
//* 2. Nguyễn Trí Nhân

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẤU HÌNH PHẦN CỨNG ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHT_PIN 15
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

#define PIN_LED_GREEN 5
#define PIN_LED_YELLOW 18
#define PIN_LED_RED 19

// --- BIẾN TOÀN CỤC ---
unsigned long previousMillis = 0;
const long interval = 1000; // Đọc dữ liệu mỗi 1000ms (1 giây)

void setup()
{
  Serial.begin(115200);

  // Cấu hình LED
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // Khởi động DHT
  dht.begin();

  // Khởi động OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("Lỗi: Không tìm thấy OLED!"));
    for (;;)
      ;
  }

  // Màn hình khởi động (Intro)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("--- NHOM 14 ---"));
  display.setCursor(10, 35);
  display.println(F("Dang khoi tao..."));
  display.display();
  delay(1500);
}

// Hàm điều khiển LED bật/tắt
// Nếu blink = true thì nhấp nháy (mô phỏng delay nhỏ để mắt thấy)
void batDen(int pin, bool blink)
{
  // Tắt hết trước
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  if (blink)
  {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
  else
  {
    digitalWrite(pin, HIGH);
  }
}

// Hàm hiển thị giao diện chính
void hienThiOLED(float t, float h, String msg)
{
  display.clearDisplay();

  // Vẽ khung viền
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);

  // Hiển thị Nhiệt độ & Độ ẩm
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.setTextSize(1);
  display.println(F("NHIET DO   DO AM"));

  display.setCursor(5, 20);
  display.setTextSize(2); // Số to
  display.print((int)t);  // Ép kiểu int cho gọn
  display.print("C");

  display.setCursor(70, 20);
  display.print((int)h);
  display.print("%");

  // Hiển thị Status Message (Nền trắng chữ đen cho nổi bật)
  display.fillRect(2, 45, 124, 16, SSD1306_WHITE); // Vẽ hộp trắng
  display.setTextColor(SSD1306_BLACK);             // Chữ đen

  // Căn giữa chữ
  int16_t x1, y1;
  uint16_t w, h_text;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h_text);
  display.setCursor((128 - w) / 2, 49);
  display.setTextSize(1);
  display.println(msg);

  display.display();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Sử dụng millis() để không chặn luồng chương trình
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    float hum = dht.readHumidity();
    float temp = dht.readTemperature();

    // Kiểm tra lỗi cảm biến
    if (isnan(hum) || isnan(temp))
    {
      Serial.println(F("Lỗi đọc DHT!"));
      display.clearDisplay();
      display.setCursor(10, 20);
      display.setTextColor(SSD1306_WHITE);
      display.println(F("CHECK SENSOR!"));
      display.display();
      return;
    }

    String trangThai = "";
    int ledSelect = -1;

    // Logic so sánh nhiệt độ (Theo yêu cầu đề bài)
    if (temp < 13)
    {
      trangThai = "TOO COLD";
      batDen(PIN_LED_GREEN, true); // Nháy đèn
    }
    else if (temp >= 13 && temp < 20)
    {
      trangThai = "COLD";
      batDen(PIN_LED_GREEN, true);
    }
    else if (temp >= 20 && temp < 25)
    {
      trangThai = "COOL";
      batDen(PIN_LED_YELLOW, true);
    }
    else if (temp >= 25 && temp < 30)
    {
      trangThai = "WARM";
      batDen(PIN_LED_YELLOW, true);
    }
    else if (temp >= 30 && temp <= 35)
    {
      trangThai = "HOT";
      batDen(PIN_LED_RED, true);
    }
    else
    { // temp > 35
      trangThai = "TOO HOT";
      batDen(PIN_LED_RED, true);
    }

    // Cập nhật màn hình
    hienThiOLED(temp, hum, trangThai);

    // Debug ra Serial để kiểm tra
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" | Hum: ");
    Serial.print(hum);
    Serial.print(" | Msg: ");
    Serial.println(trangThai);
  }
}