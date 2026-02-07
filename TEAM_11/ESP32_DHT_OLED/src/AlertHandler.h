#include <Arduino.h>
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

String getStatus(float t) {
  if (t < 20) return "COLD";
  if (t < 30) return "WARM";
  return "HOT";
}

int getActiveLed(float t) {
  if (t < 20) return LED_CYAN;
  if (t < 30) return LED_YELLOW;
  return LED_RED;
}

void handleLEDBloking(int activeLed) {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastLedToggle >= blinkInterval) {
    lastLedToggle = currentMillis;
    ledState = !ledState; // Đảo trạng thái LED

    // Tắt hết LED trước khi bật LED chỉ định
    digitalWrite(LED_CYAN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    if (activeLed != -1) {
      digitalWrite(activeLed, ledState);
    }
  }
}

