#include <Arduino.h>

#define LED_RED    18
#define LED_YELLOW 5
#define LED_GREEN  17

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// Hàm nhấp nháy LED trong khoảng thời gian xác định
void blinkLED(int pin, unsigned long timeMs) {
  unsigned long start = millis();

  while (millis() - start < timeMs) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
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