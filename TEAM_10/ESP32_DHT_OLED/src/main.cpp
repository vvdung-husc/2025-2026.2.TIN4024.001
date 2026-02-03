/*
1. Trần Thị Quỳnh Anh
2. Lê Yến Nhi
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_GREEN  15
#define LED_YELLOW 2
#define LED_RED    4

unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(13, 12); // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true);
  }

  dht.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) return;

  // ===== Xác định trạng thái =====
  String status = "";
  int ledColor = 0; // 1=GREEN, 2=YELLOW, 3=RED

  if (t < 13) {
    status = "TOO COLD";
    ledColor = 1;
  } else if (t < 20) {
    status = "COLD";
    ledColor = 1;
  } else if (t < 25) {
    status = "COOL";
    ledColor = 2;
  } else if (t < 30) {
    status = "WARM";
    ledColor = 2;
  } else if (t < 35) {
    status = "HOT";
    ledColor = 3;
  } else {
    status = "TOO HOT";
    ledColor = 3;
  }

  // ===== LED chớp nháy =====
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    ledState = !ledState;

    digitalWrite(LED_GREEN,  ledColor == 1 ? ledState : LOW);
    digitalWrite(LED_YELLOW, ledColor == 2 ? ledState : LOW);
    digitalWrite(LED_RED,    ledColor == 3 ? ledState : LOW);
  }

  // ===== OLED =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Weather Monitor");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.print(t);
  display.println(" C");

  display.setCursor(0, 28);
  display.print("Humidity: ");
  display.print(h);
  display.println(" %");

  display.setCursor(0, 44);
  display.print("Status: ");
  display.println(status);

  display.display();
}
