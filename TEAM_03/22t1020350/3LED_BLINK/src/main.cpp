#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

#define BLINK_DELAY 300  // thời gian bật/tắt (ms)

// Hàm nhấp nháy LED n lần
void blinkLed(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
    delay(BLINK_DELAY);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // Red nháy 3 lần
  blinkLed(LED_RED, 3);
  delay(500);

  // Yellow nháy 5 lần
  blinkLed(LED_YELLOW, 5);
  delay(500);

  // Green nháy 7 lần
  blinkLed(LED_GREEN, 7);
  delay(1000);
}
