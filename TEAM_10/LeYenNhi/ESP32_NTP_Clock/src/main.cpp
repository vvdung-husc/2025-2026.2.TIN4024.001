#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";   
const char* password = "";

// ===== NTP =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// biến toàn cục
bool daThongBao = false;

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi!");
    while(true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nDa ket noi WiFi!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Loi lay thoi gian!");
    return;
  }

  // ===== THÔNG BÁO 10:15 =====
  if(timeinfo.tm_hour == 10 && timeinfo.tm_min == 15 && !daThongBao){
    Serial.println("Den gio hoc!");
    daThongBao = true;
  }

  // reset khi qua phút 15
  if(timeinfo.tm_min != 15){
    daThongBao = false;
  }

  display.clearDisplay();

  // ===== HIỂN THỊ GIỜ =====
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.printf("%02d:%02d:%02d",
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec);

  // ===== HIỂN THỊ NGÀY =====
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.printf("%02d/%02d/%04d",
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900);

  // ===== HIỂN THỊ THÔNG BÁO TRÊN OLED =====
  if(timeinfo.tm_hour == 10 && timeinfo.tm_min == 15){
    display.setCursor(0, 55);
    display.setTextSize(1);
    display.println("Den gio hoc!");
  }

  display.display();

  delay(1000);
}