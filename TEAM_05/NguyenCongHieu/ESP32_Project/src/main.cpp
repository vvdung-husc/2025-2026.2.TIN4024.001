#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// Pin mapping for NodeMCU 1.0 (ESP8266)
constexpr uint8_t LED_PIN = LED_BUILTIN;
constexpr uint8_t DHT_PIN = D3;
constexpr uint8_t MQ2_PIN = A0;

// Change to DHT22 if your module is DHT22/AM2302
constexpr uint8_t DHT_TYPE = DHT11;

constexpr unsigned long LED_BLINK_MS = 500;
constexpr unsigned long SENSOR_READ_MS = 2000;

DHT dht(DHT_PIN, DHT_TYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

unsigned long lastBlinkMs = 0;
unsigned long lastReadMs = 0;
bool ledState = false;

float temperatureC = NAN;
float humidity = NAN;
int mq2Raw = 0;

void readSensors() {
  humidity = dht.readHumidity();
  temperatureC = dht.readTemperature();
  mq2Raw = analogRead(MQ2_PIN);

  Serial.print("Nhiet do hien tai: ");
  if (isnan(temperatureC)) {
    Serial.println("N/A C");
  } else {
    Serial.print(temperatureC, 1);
    Serial.println(" C");
  }

  Serial.print("Do am hien tai: ");
  if (isnan(humidity)) {
    Serial.println("N/A %");
  } else {
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  Serial.print("MQ2 raw: ");
  Serial.println(mq2Raw);
  Serial.println();
}

void drawOled() {
  char line1[24];
  char line2[24];
  char line3[24];

  if (isnan(temperatureC) || isnan(humidity)) {
    snprintf(line1, sizeof(line1), "DHT: Read error");
    line2[0] = '\0';
  } else {
    snprintf(line1, sizeof(line1), "Nhiet do: %.1f C", temperatureC);
    snprintf(line2, sizeof(line2), "Do am: %.1f %%", humidity);
  }

  const float mq2Percent = (mq2Raw / 1023.0f) * 100.0f;
  snprintf(line3, sizeof(line3), "MQ2: %4d (%.0f%%)", mq2Raw, mq2Percent);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 12, "ESP8266 Controller");
  u8g2.drawLine(0, 15, 127, 15);
  u8g2.drawStr(0, 32, line1);
  u8g2.drawStr(0, 46, line2);
  u8g2.drawStr(0, 60, line3);
  u8g2.sendBuffer();
}

void updateBlink() {
  const unsigned long now = millis();
  if (now - lastBlinkMs >= LED_BLINK_MS) {
    lastBlinkMs = now;
    ledState = !ledState;
    // LED_BUILTIN on ESP8266 is active LOW
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("Khoi dong ESP8266 Controller...");

  Wire.begin();
  dht.begin();
  u8g2.begin();

  readSensors();
  drawOled();
}

void loop() {
  updateBlink();

  const unsigned long now = millis();
  if (now - lastReadMs >= SENSOR_READ_MS) {
    lastReadMs = now;
    readSensors();
    drawOled();
  }
}