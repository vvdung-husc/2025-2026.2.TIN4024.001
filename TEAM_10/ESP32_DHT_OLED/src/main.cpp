/* THONG TIN NHOM 10
1. Le Thi Khanh Ly
2. Ngo Thi Cam Ly
3. Pham Nang
4. Tran Thi Quynh Anh
5. Le Yen Nhi
*/
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

int getSystemStatus(float temp, String &msg) {
  if (temp < 13) {
    msg = "TOO COLD";
    return LED_GREEN;
  } 
  if (temp < 20) {
    msg = "COLD";
    return LED_GREEN;
  } 
  if (temp < 25) {
    msg = "COOL";
    return LED_YELLOW;
  } 
  if (temp < 30) {
    msg = "WARM";
    return LED_YELLOW;
  } 
  if (temp < 35) {
    msg = "HOT";
    return LED_RED;
  } 
  
  msg = "TOO HOT";
  return LED_RED;
}

void displayInfo(float t, float h, String statusMsg) {
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
  display.println(statusMsg);

  display.display();
}

void setup() {
  Serial.begin(115200);

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
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  String currentStatus = "READING...";
  int activeLed = -1;

  if (!isnan(temp) && !isnan(hum)) {
    activeLed = getSystemStatus(temp, currentStatus);
  }

  displayInfo(temp, hum, currentStatus);

  offAllLED();
  if (activeLed != -1) {
    blinkLED(activeLed, 1000); 
  }

  delay(2000);
}