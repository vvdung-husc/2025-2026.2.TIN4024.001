#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include "main.h"
#include "ultils.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // I2C theo diagram
  Wire.begin(13, 12);

  setupLED();
  offAllLED();
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while (1);
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  String status = "READING...";
  int ledPin = -1;

  if (!isnan(t) && !isnan(h)) {
    if (t < 13) {
      status = "TOO COLD";
      ledPin = LED_GREEN;
    } else if (t < 20) {
      status = "COLD";
      ledPin = LED_GREEN;
    } else if (t < 25) {
      status = "COOL";
      ledPin = LED_YELLOW;
    } else if (t < 30) {
      status = "WARM";
      ledPin = LED_YELLOW;
    } else if (t < 35) {
      status = "HOT";
      ledPin = LED_RED;
    } else {
      status = "TOO HOT";
      ledPin = LED_RED;
    }
  }

  // OLED luôn hiển thị
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Temp: ");
  if (isnan(t)) display.println("--");
  else {
    display.print(t);
    display.println(" C");
  }

  display.setCursor(0, 15);
  display.print("Humi: ");
  if (isnan(h)) display.println("--");
  else {
    display.print(h);
    display.println(" %");
  }

  display.setCursor(0, 35);
  display.print("Status:");
  display.setCursor(0, 50);
  display.println(status);

  display.display();

  offAllLED();
  if (ledPin != -1) blinkLED(ledPin, 500);

  delay(2000);
}
