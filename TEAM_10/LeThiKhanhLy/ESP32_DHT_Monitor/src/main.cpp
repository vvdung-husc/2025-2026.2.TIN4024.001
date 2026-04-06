// ===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL6TIHfwtOz"
#define BLYNK_TEMPLATE_NAME "ESP32 DHT OLED"
#define BLYNK_AUTH_TOKEN "aOK7MUu4NeC7KgKix9ic0ZYyp32xcnCK"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "DHT.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_PIN 2

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== TIMER =====
BlynkTimer timer;

// ===== HÀM GỬI DỮ LIỆU =====
void sendData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  // Debug
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" *C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  // ===== GỬI LÊN BLYNK =====
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, 1); // báo cho Blynk là LED đang ON

  // ===== OLED =====
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(10, 15, "ESP32 + DHT22");

  char tempStr[20];
  sprintf(tempStr, "Temp: %.1f C", temperature);
  u8g2.drawStr(10, 35, tempStr);

  char humStr[20];
  sprintf(humStr, "Humi: %.1f %%", humidity);
  u8g2.drawStr(10, 55, humStr);

  u8g2.sendBuffer();
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  dht.begin();
  u8g2.begin();

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Gửi mỗi 2 giây
  timer.setInterval(2000L, sendData);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // mặc định bật
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2);
}

BLYNK_WRITE(V2) {
  int value = param.asInt();

  digitalWrite(LED_PIN, value);

  if (value == 1) Serial.println("LED ON");
  else Serial.println("LED OFF");
}
// ===== LOOP =====
void loop() {
  Blynk.run();
  timer.run();
}