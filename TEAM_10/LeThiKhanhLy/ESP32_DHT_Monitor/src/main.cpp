#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "DHT.h"

#define BLYNK_TEMPLATE_ID "TMPL6qxownBxg"
#define BLYNK_TEMPLATE_NAME "ESP32 DHT"
#define BLYNK_AUTH_TOKEN "XD1C9wEisDpB07EMKt_VF3Wkj2cjFb7U"

#include <BlynkSimpleEsp32.h>
// ===== CẤU HÌNH DHT =====
#define DHTPIN 4        // Chân DATA nối GPIO4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ===== CẤU HÌNH OLED =====
// OLED SH1106 I2C (phổ biến trên Wokwi)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  dht.begin();

  u8g2.begin();
}

// ===== LOOP =====
void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Kiểm tra lỗi đọc cảm biến
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  // In ra Serial (debug)
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" *C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  // ===== HIỂN THỊ OLED =====
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(10, 15, "ESP32 + DHT22");

  // Hiển thị nhiệt độ
  char tempStr[20];
  sprintf(tempStr, "Temp: %.1f C", temperature);
  u8g2.drawStr(10, 35, tempStr);

  // Hiển thị độ ẩm
  char humStr[20];
  sprintf(humStr, "Humi: %.1f %%", humidity);
  u8g2.drawStr(10, 55, humStr);

  u8g2.sendBuffer();

  delay(2000); // cập nhật mỗi 2 giây
}