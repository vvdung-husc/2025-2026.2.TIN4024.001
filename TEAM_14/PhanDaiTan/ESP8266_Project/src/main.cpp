#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>



#define LED_PIN D4
#define DHT_PIN D3
#define MQ2_PIN A0

#define DHT_TYPE DHT11



DHT dht(DHT_PIN, DHT_TYPE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);



float temperature = 0;
float humidity = 0;
int gasValue = 0;

unsigned long previousMillis = 0;
const long interval = 2000;



void readDHT()
{
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("DHT read failed!");
        return;
    }
}

void readMQ2()
{
    gasValue = analogRead(MQ2_PIN);
}

void displayOLED()
{
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.setCursor(0,15);
    u8g2.print("Temperature:");
    u8g2.setCursor(0,30);
    u8g2.print(temperature);
    u8g2.print(" C");

    u8g2.setCursor(0,45);
    u8g2.print("Humidity:");
    u8g2.setCursor(0,60);
    u8g2.print(humidity);
    u8g2.print(" %");

    u8g2.sendBuffer();
}

void displayGas()
{
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.setCursor(0,20);
    u8g2.print("Gas Sensor");

    u8g2.setCursor(0,40);
    u8g2.print("Value:");
    u8g2.print(gasValue);

    u8g2.sendBuffer();
}

void blinkLED()
{
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
}

/*================ SETUP =================*/

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(MQ2_PIN, INPUT);

    dht.begin();

    Wire.begin();
    u8g2.begin();

    Serial.println("ESP8266 Sensor System Start");
}



void loop()
{
    unsigned long currentMillis = millis();

    blinkLED();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        readDHT();
        readMQ2();

        Serial.println("------ Sensor Data ------");
        Serial.print("Temperature: ");
        Serial.println(temperature);

        Serial.print("Humidity: ");
        Serial.println(humidity);

        Serial.print("Gas Value: ");
        Serial.println(gasValue);

        displayOLED();
        delay(3000);

        displayGas();
        delay(3000);
    }
}