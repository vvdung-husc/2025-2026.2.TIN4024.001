#include <Arduino.h>

#define LED_GREEN 32
#define LED_YELLOW 33
#define LED_RED 25

unsigned long previousMillis = 0;
const unsigned long interval = 1000; // 1 giây

int state = 0; // 0: xanh, 1: đỏ, 2: vàng
int blinkCount = 0;
bool ledState = false;

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void turnOffAll() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ledState = !ledState; // đảo trạng thái bật/tắt

    switch (state) {
      case 0: // LED xanh
        turnOffAll();
        digitalWrite(LED_GREEN, ledState);

        if (!ledState) blinkCount++;

        if (blinkCount >= 7) {
          blinkCount = 0;
          state = 2; // chuyển sang LED vàng
        }
        break;

      case 2: // LED vàng
        turnOffAll();
        digitalWrite(LED_YELLOW, ledState);

        if (!ledState) blinkCount++;

        if (blinkCount >= 3) {
          blinkCount = 0;
          state = 1; // quay lại LED đỏ
        }
        break;

      case 1: // LED đỏ
        turnOffAll();
        digitalWrite(LED_RED, ledState);

        if (!ledState) blinkCount++;

        if (blinkCount >= 5) {
          blinkCount = 0;
          state = 0; // chuyển sang LED xanh
        }
        break;
    }
  }
}
 