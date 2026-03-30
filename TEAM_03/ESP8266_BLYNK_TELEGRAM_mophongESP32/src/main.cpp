/*
THONG TIN NHOM X
1. Ten thanh vien 1
2. Ten thanh vien 2
3. Ten thanh vien 3
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

// Chan theo diagram.json
const int DHT_PIN = 16;
const int LED1_PIN = 15; // cyan
const int LED2_PIN = 2;  // yellow
const int LED3_PIN = 4;  // magenta

float temperature = 0;
float humidity = 0;
int gasValue = 0;
unsigned long startMillis = 0;

void showWelcome() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("Welcome!");
    display.display();
}

void showOLED(float t, float h, int gas, unsigned long uptimeSec) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("IOT Team 3");

    display.setCursor(0, 12);
    display.print("Temp: ");
    display.print(t, 1);
    display.println(" C");

    display.setCursor(0, 24);
    display.print("Humi: ");
    display.print(h, 1);
    display.println(" %");

    display.setCursor(0, 36);
    display.print("Gas : ");
    display.println(gas);

    display.setCursor(0, 48);
    display.print("Up  : ");
    display.print(uptimeSec);
    display.println(" s");

    display.display();
}

void controlLED(float t, float h, int gas) {
    digitalWrite(LED1_PIN, (t > 30.0) ? HIGH : LOW);
    digitalWrite(LED2_PIN, (h > 70.0) ? HIGH : LOW);
    digitalWrite(LED3_PIN, (gas > 700) ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);

    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);

    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

    // SDA = 13, SCL = 12
    Wire.begin(13, 12);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Khong tim thay OLED!");
        while (true) {
            delay(100);
        }
    }

    showWelcome();
    delay(1500);

    startMillis = millis();
    randomSeed(micros());
}

void loop() {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    temperature = data.temperature;
    humidity = data.humidity;

    // Gia lap MQ2
    gasValue = random(300, 1000);

    unsigned long uptimeSec = (millis() - startMillis) / 1000UL;

    Serial.println("===== SENSOR DATA =====");
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" C");

    Serial.print("Humidity   : ");
    Serial.print(humidity, 1);
    Serial.println(" %");

    Serial.print("Gas value  : ");
    Serial.println(gasValue);

    Serial.print("Uptime     : ");
    Serial.print(uptimeSec);
    Serial.println(" s");
    Serial.println("=======================");

    showOLED(temperature, humidity, gasValue, uptimeSec);
    controlLED(temperature, humidity, gasValue);

    delay(2000);
}