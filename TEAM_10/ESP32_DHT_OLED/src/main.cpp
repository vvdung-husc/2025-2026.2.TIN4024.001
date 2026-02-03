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

int readingCount = 0;
float lastTemp = -999.0;
float lastHum = -999.0;

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

String drawProgressBar(float value, float maxVal) {
  String bar = "[";
  int totalBars = 20;
  int numBars = map((long)value, 0, (long)maxVal, 0, totalBars);
  
  if (numBars > totalBars) numBars = totalBars;
  
  for (int i = 0; i < totalBars; i++) {
    if (i < numBars) bar += "|";
    else bar += ".";
  }
  bar += "]";
  return bar;
}

void logToSerialdashboard(float t, float h, String status) {
  readingCount++;

  String icon = "";
  if (status == "TOO COLD") icon = "❄️";
  else if (status == "COLD") icon = "☂️";
  else if (status == "COOL") icon = "☁️";
  else if (status == "WARM") icon = "☀️";
  else if (status == "HOT") icon = "🌞";
  else if (status == "TOO HOT") icon = "🔥";

  Serial.println("\n");
  Serial.println("╔══════════════════════════════════╗");
  Serial.println("║      NHOM 10 - TRAM IOT          ║");
  Serial.println("╠══════════════════════════════════╣");
  
  Serial.print("║  Lan do: #"); 
  if(readingCount < 10) Serial.print("00");
  else if(readingCount < 100) Serial.print("0");
  Serial.print(readingCount);
  Serial.println("                          ║");
  
  Serial.println("╟──────────────────────────────────╢");

  if (isnan(t) || isnan(h)) {
    Serial.println("║  ⚠ LOI: KHONG DOC DUOC CAM BIEN  ║");
  } else {
    Serial.print("║  Nhiet do: "); Serial.print(t, 2); Serial.println(" °C             ║");
    Serial.print("║  "); Serial.print(drawProgressBar(t, 50));
    Serial.println("        ║");
    
    Serial.println("║                                  ║");

    Serial.print("║  Do am:    "); Serial.print(h, 2); Serial.println(" %              ║");
    Serial.print("║  "); Serial.print(drawProgressBar(h, 100));
    Serial.println("        ║");
  }

  Serial.println("╟──────────────────────────────────╢");
  
  Serial.print("║  TRANG THAI: "); Serial.print(status); Serial.print(" "); Serial.print(icon);
  
  int padding = 19 - status.length();
  if (icon != "") padding -= 2;
  for(int k=0; k<padding; k++) Serial.print(" ");
  Serial.println("║");

  Serial.println("╚══════════════════════════════════╝");
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

    if (temp != lastTemp || hum != lastHum) {
      displayInfo(temp, hum, currentStatus);
      logToSerialdashboard(temp, hum, currentStatus);
      
      lastTemp = temp;
      lastHum = hum;
    }
  }

  offAllLED();
  if (activeLed != -1) {
    blinkLED(activeLed, 1000); 
  }

  delay(2000);
}