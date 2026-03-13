#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// ===== Khai báo chân =====
#define LED_PIN D0
#define DHTPIN D3
#define MQ2_PIN A0

#define DHTTYPE DHT22

// ===== Khởi tạo cảm biến =====
DHT dht(DHTPIN, DHTTYPE);

// OLED SH1106 I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== Timer LED =====
unsigned long previousMillis = 0;
const long interval = 500;
bool ledState = false;

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    dht.begin();
    u8g2.begin();
}

void loop()
{
    // ===== LED nhấp nháy =====
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }

    // ===== Đọc DHT =====
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Failed to read from DHT sensor!");
        temperature = 0;
        humidity = 0;
    }

    // ===== Đọc MQ2 =====
    int gasValue = analogRead(MQ2_PIN);

    // ===== Serial debug =====
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" C  ");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %  ");

    Serial.print("Gas: ");
    Serial.println(gasValue);

    // ===== Hiển thị OLED =====
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.drawStr(0, 12, "ESP8266 CMU");

    char buffer[20];

    sprintf(buffer, "Temp: %.1f C", temperature);
    u8g2.drawStr(0, 30, buffer);

    sprintf(buffer, "Humi: %.1f %%", humidity);
    u8g2.drawStr(0, 45, buffer);

    sprintf(buffer, "Gas: %d", gasValue);
    u8g2.drawStr(0, 60, buffer);

    u8g2.sendBuffer();

    delay(2000);
}