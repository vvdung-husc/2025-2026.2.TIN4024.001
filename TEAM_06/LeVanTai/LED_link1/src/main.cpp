#include <Arduino.h>

// Khai báo chân LED
#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // 🔴 Đèn đỏ 3 giây
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  delay(3000);

  // 🟢 Đèn xanh 4 giây
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  delay(4000);

  // 🟡 Đèn vàng 2 giây
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, LOW);
  delay(2000);
}
