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

  Serial.println();
  Serial.println("╔══════════════════════════════════╗");
  Serial.println("║      NANG - TRAM IOT             ║");
  Serial.println("╠══════════════════════════════════╣");
  
  Serial.print("║  Lan do: #"); 
  if(readingCount < 10) Serial.print("00");
  else if(readingCount < 100) Serial.print("0");
  Serial.print(readingCount);
  
  for(int i=0; i<20; i++) Serial.print(' ');
  Serial.println("║");
  
  Serial.println("╟──────────────────────────────────╢");

  if (isnan(t) || isnan(h)) {
    Serial.println("║  ⚠ LOI: KHONG DOC DUOC CAM BIEN  ║");
  } else {
    Serial.print("║  Nhiet do: "); 
    Serial.print(t, 2); 
    Serial.print(" *C"); 
    
    int valLen = 5; 
    if(t >= 100.0 || t <= -10.0) valLen = 6;
    else if(t < 10.0 && t >= 0) valLen = 4;
    
    int currentLen = 12 + valLen + 3; 
    int padding = 34 - currentLen;
    
    for(int i=0; i<padding; i++) Serial.print(' ');
    Serial.println("║");

    Serial.print("║  "); Serial.print(drawProgressBar(t, 50));
    for(int i=0; i<10; i++) Serial.print(' '); 
    Serial.println("║");
    
    Serial.println("║                                  ║");

    Serial.print("║  Do am:    "); 
    Serial.print(h, 2); 
    Serial.print(" %");
    
    valLen = 5;
    if(h == 100.00) valLen = 6;
    else if(h < 10.0) valLen = 4;
    
    currentLen = 12 + valLen + 2; 
    padding = 34 - currentLen;
    
    for(int i=0; i<padding; i++) Serial.print(' ');
    Serial.println("║");

    Serial.print("║  "); Serial.print(drawProgressBar(h, 100));
    for(int i=0; i<10; i++) Serial.print(' '); 
    Serial.println("║");
  }

  Serial.println("╟──────────────────────────────────╢");
  
  Serial.print("║  TRANG THAI: "); 
  Serial.print(status);
  Serial.print(' '); 
  Serial.print(icon);
  
  int iconVisualWidth = 2; 
  int usedWidth = 14 + status.length() + 1 + iconVisualWidth;
  int spaces = 34 - usedWidth;
  
  if(spaces < 0) spaces = 0;

  for(int k=0; k<spaces; k++) Serial.print(' ');
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