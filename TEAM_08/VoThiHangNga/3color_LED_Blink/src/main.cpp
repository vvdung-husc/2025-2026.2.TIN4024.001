#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// Hàm cho 1 đèn nháy trong X giây
void blinkLED(int ledPin, int totalTimeMs) {
  int blinkDelay = 500; // 0.5s sáng, 0.5s tắt
  int elapsed = 0;

  while (elapsed < totalTimeMs) {
    digitalWrite(ledPin, HIGH);
    delay(blinkDelay);
    digitalWrite(ledPin, LOW);
    delay(blinkDelay);
    elapsed += blinkDelay * 2;
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Tắt hết lúc bắt đầu
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void loop() {
  // 🔴 Đèn đỏ nháy 10s
  blinkLED(LED_RED, 10000);

  // 🟡 Đèn vàng nháy 10s
  blinkLED(LED_YELLOW, 10000);

  // 🟢 Đèn xanh nháy 10s
  blinkLED(LED_GREEN, 10000);
}
