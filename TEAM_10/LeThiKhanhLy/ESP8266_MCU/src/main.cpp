#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

/* -------- PIN -------- */
#define DHTPIN D3
#define DHTTYPE DHT22
#define MQ2_PIN A0
#define LED_PIN LED_BUILTIN

/* -------- OBJECT -------- */
DHT dht(DHTPIN, DHTTYPE);

/* OLED SH1106 */
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  u8g2.begin();
}

void loop()
{
  /* LED nhấp nháy */
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  /* Đọc DHT */
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  /* Đọc MQ2 */
  int gas = analogRead(MQ2_PIN);

  /* Serial Monitor */
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C ");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" % ");

  Serial.print("Gas: ");
  Serial.println(gas);

  /* OLED DISPLAY */
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  /* TÊN HIỂN THỊ */
  u8g2.setCursor(10,10);
  u8g2.print("ESP8266_MCU");

  u8g2.setCursor(0,30);
  u8g2.print("Temp: ");
  u8g2.print(temperature);
  u8g2.print(" C");

  u8g2.setCursor(0,45);
  u8g2.print("Hum: ");
  u8g2.print(humidity);
  u8g2.print(" %");

  u8g2.setCursor(0,60);
  u8g2.print("Gas: ");
  u8g2.print(gas);

  u8g2.sendBuffer();

  delay(2000);
}