#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

// ===== PIN =====
#define LED_GREEN   15
#define LED_YELLOW  2
#define LED_RED     4
#define DHT_PIN     16
#define OLED_SDA    13
#define OLED_SCL    12

// ===== TIME =====
#define DHT_INTERVAL   2000UL
#define BLINK_INTERVAL 1000UL

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ===== OBJECT =====
extern Adafruit_SSD1306 display;
extern DHTesp dht;

// ===== DATA =====
extern float temperature;
extern float humidity;
extern String statusText;
extern int activeLED;

// ===== API =====
void systemInit();
void systemLoop();

#endif
