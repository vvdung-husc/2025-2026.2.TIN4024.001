#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

#define LED_PIN LED_BUILTIN
#define DHTPIN D3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

float temperature = 0;
float humidity = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    dht.begin();
    u8g2.begin();

    delay(2000);
}

void loop()
{
    /* LED BLINK */

    digitalWrite(LED_PIN, LOW);
    delay(500);

    digitalWrite(LED_PIN, HIGH);
    delay(500);

    /* READ SENSOR */

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Doc cam bien that bai!");
        return;
    }

    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Do am: ");
    Serial.print(humidity);
    Serial.println(" %");

    /* OLED DISPLAY */

    u8g2.clearBuffer();

   u8g2.setFont(u8g2_font_6x10_tr);

    // Ten hien thi tren cung
    u8g2.drawStr(10,12,"Bui Khac Hieu");

    // Nhiet do
    u8g2.drawStr(0,35,"Nhiet do:");

    char tempBuffer[10];
    dtostrf(temperature,4,1,tempBuffer);

    u8g2.drawStr(80,35,tempBuffer);
    u8g2.drawStr(110,35,"C");

    // Do am
    u8g2.drawStr(0,55,"Do am:");

    char humBuffer[10];
    dtostrf(humidity,4,1,humBuffer);

    u8g2.drawStr(80,55,humBuffer);
    u8g2.drawStr(110,55,"%");

    u8g2.sendBuffer();
}