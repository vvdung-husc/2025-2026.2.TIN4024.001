#include <Arduino.h>

#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// Hàm nhấp nháy LED theo thời gian (ms)
void blinkLED(int pin, unsigned long durationMs) {
  unsigned long startTime = millis();

  while (millis() - startTime < durationMs) {
    digitalWrite(pin, HIGH);
    delay(500);              // bật 0.5s
    digitalWrite(pin, LOW);
    delay(500);              // tắt 0.5s
  }
}

void loop() {
  // 🔴 Đỏ nhấp nháy 5 giây
  blinkLED(LED_RED, 5000);

  // 🟡 Vàng nhấp nháy 3 giây
  blinkLED(LED_YELLOW, 3000);

  // 🟢 Xanh nhấp nháy 7 giây
  blinkLED(LED_GREEN, 7000);
}