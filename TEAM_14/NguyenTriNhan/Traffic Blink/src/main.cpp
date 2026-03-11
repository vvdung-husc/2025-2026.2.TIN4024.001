#include <Arduino.h>

#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

const unsigned long BLINK_TIME = 5000; // 5 giây
const unsigned long INTERVAL   = 500;  // tốc độ nhấp nháy (0.5s)

void blinkLed(int pin) {
  unsigned long startTime = millis();

  while (millis() - startTime < BLINK_TIME) {
    digitalWrite(pin, HIGH);
    delay(INTERVAL);
    digitalWrite(pin, LOW);
    delay(INTERVAL);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // Đỏ nhấp nháy 5 giây
  blinkLed(LED_RED);

  // Vàng nhấp nháy 5 giây
  blinkLed(LED_YELLOW);

  // Xanh nhấp nháy 5 giây
  blinkLed(LED_GREEN);
}