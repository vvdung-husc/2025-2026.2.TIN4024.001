#include <Arduino.h>

// Khai báo chân LED
#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

// Hàm cho LED nhấp nháy trong thời gian (ms)
void blinkLed(int pin, int durationMs) {
  int elapsed = 0;
  while (elapsed < durationMs) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
    elapsed += 1000;
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // 🔴 Đèn đỏ nhấp nháy 5 giây
  blinkLed(LED_RED, 5000);

  // 🟡 Đèn vàng nhấp nháy 3 giây
  blinkLed(LED_YELLOW, 3000);

  // 🟢 Đèn xanh nhấp nháy 7 giây
  blinkLed(LED_GREEN, 7000);
}
