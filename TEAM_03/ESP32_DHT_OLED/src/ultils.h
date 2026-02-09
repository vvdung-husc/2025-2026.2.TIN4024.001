#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// Cập nhật trạng thái theo nhiệt độ
void updateStatusFromTemp(float tempC, const char *&statusText, int &activeLED,
                          int ledGreen, int ledYellow, int ledRed);

// Vẽ OLED
void drawOLED(Adafruit_SSD1306 &display, float t, float h, const char *statusText);

// Tắt hết LED
void setAllLEDsLow(int ledRed, int ledYellow, int ledGreen);

