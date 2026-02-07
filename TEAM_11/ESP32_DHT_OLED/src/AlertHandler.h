#include <Arduino.h>

// Định nghĩa chân LED theo sơ đồ Wokwi
#define LED_CYAN   15
#define LED_YELLOW 2
#define LED_RED    4

unsigned long lastLedToggle = 0;
const unsigned long blinkInterval = 300; 
bool ledState = false;

void setupAlerts() {
  pinMode(LED_CYAN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

// Trạng thái hiển thị theo bảng nhiệt độ
String getStatus(float t) {
  if (t < 13) return "TOO COLD";
  if (t < 20) return "COLD";
  if (t < 25) return "COOL";
  if (t < 30) return "WARM";
  if (t < 35) return "HOT";
  return "TOO HOT";
}

// Chọn LED đại diện theo bảng
int getActiveLed(float t) {
  if (t < 20) return LED_CYAN;   // Green/Cyan cho Cold
  if (t < 30) return LED_YELLOW; // Yellow cho Warm
  return LED_RED;                // Red cho Hot
}

void handleLEDBloking(int activeLed) {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastLedToggle >= blinkInterval) {
    lastLedToggle = currentMillis;
    ledState = !ledState; 

    // Tắt hết LED để reset
    digitalWrite(LED_CYAN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    // Chỉ nhấp nháy LED đang được kích hoạt
    if (activeLed != -1) {
      digitalWrite(activeLed, ledState);
    }
  }
}