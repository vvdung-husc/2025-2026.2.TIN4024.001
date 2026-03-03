#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22

// ===== I2C custom pins (theo diagram của bạn) =====
#define I2C_SDA 13
#define I2C_SCL 12

// ===== LEDs =====
const int LED_RED = 4;
const int LED_YELLOW = 2;
const int LED_GREEN = 15;

// ===== Non-blocking timing =====
const unsigned long DHT_INTERVAL_MS = 2000;  // DHT22 ~2s/lần
const unsigned long BLINK_INTERVAL_MS = 500; // nhấp nháy

// ===== Hardware objects (định nghĩa ở main.cpp) =====
extern Adafruit_SSD1306 display;
extern DHT dht;

// ===== Shared state =====
extern float h;
extern float t;
extern const char *statusText;
extern int activeLED;
extern bool ledOn;

extern unsigned long lastDhtMs;
extern unsigned long lastBlinkMs;
