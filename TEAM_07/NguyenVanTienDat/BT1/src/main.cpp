#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHTesp dht;

// ===== PIN =====
const int LED_GREEN  = 15;
const int LED_YELLOW = 2;
const int LED_RED    = 4;
const int DHT_PIN    = 16;

// ===== BIẾN TOÀN CỤC =====
float temp = 0;
float hum  = 0;
String statusText = "WAIT...";
int activeLED = 0;
int blinkInterval = 800;

unsigned long previousMillis = 0;
unsigned long lastDHTRead = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(13, 12);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
}

void loop() {
  unsigned long currentMillis = millis();

  // ===== ĐỌC DHT MỖI 2 GIÂY =====
  if (currentMillis - lastDHTRead >= 2000) {
    lastDHTRead = currentMillis;

    TempAndHumidity data = dht.getTempAndHumidity();

    // Chỉ cập nhật nếu đọc hợp lệ
    if (!isnan(data.temperature) && !isnan(data.humidity)) {
      temp = data.temperature;
      hum  = data.humidity;

      // PHÂN LOẠI NHIỆT ĐỘ
      if (temp < 13) {
        statusText = "TOO COLD";
        activeLED = 1;
        blinkInterval = 400;
      }
      else if (temp < 20) {
        statusText = "COLD";
        activeLED = 1;
        blinkInterval = 800;
      }
      else if (temp < 25) {
        statusText = "COOL";
        activeLED = 2;
        blinkInterval = 400;
      }
      else if (temp < 30) {
        statusText = "WARM";
        activeLED = 2;
        blinkInterval = 800;
      }
      else if (temp < 35) {
        statusText = "HOT";
        activeLED = 3;
        blinkInterval = 800;
      }
      else {
        statusText = "TOO HOT";
        activeLED = 3;
        blinkInterval = 400;
      }

      Serial.print("Temp: "); Serial.println(temp);
     
    }

    // ===== OLED CẬP NHẬT =====
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0,0);
    display.print("Temp: ");
    display.print(temp,1);
    display.println(" C");

    display.setCursor(0,12);
    display.print("Hum : ");
    display.print(hum,1);
    display.println(" %");

    display.setCursor(0,26);
    display.print("Status:");

    display.setTextSize(2);
    display.setCursor(0,40);
    display.print(statusText);

    display.display();
  }

  // ===== LED BLINK ĐỘC LẬP =====
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;

    digitalWrite(LED_GREEN,  (activeLED == 1 && ledState));
    digitalWrite(LED_YELLOW, (activeLED == 2 && ledState));
    digitalWrite(LED_RED,    (activeLED == 3 && ledState));
  }
}
