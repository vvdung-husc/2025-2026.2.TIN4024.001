#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define DHTPIN D3
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

const int ledPin = LED_BUILTIN; 

void setup() {
  pinMode(ledPin, OUTPUT);
  dht.begin();
  u8g2.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    t = 0.0; h = 0.0;
  }

  // BẬT ĐÈN 
  digitalWrite(ledPin, LOW); 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 15, "ESP6288_CMU");
  u8g2.setCursor(0, 35);
  u8g2.print("Temp: "); u8g2.print(t); u8g2.print(" C");
  u8g2.setCursor(0, 50);
  u8g2.print("Humid: "); u8g2.print(h); u8g2.print(" %");
  u8g2.drawStr(0, 64, "LED Status: ON");
  u8g2.sendBuffer();
  delay(1000);

  // TẮT ĐÈN
  digitalWrite(ledPin, HIGH); 
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "ESP6288_CMU");
  u8g2.setCursor(0, 35);
  u8g2.print("Temp: "); u8g2.print(t); u8g2.print(" C");
  u8g2.setCursor(0, 50);
  u8g2.print("Humid: "); u8g2.print(h); u8g2.print(" %");
  u8g2.drawStr(0, 64, "LED Status: OFF");
  u8g2.sendBuffer();
  delay(1000);
}