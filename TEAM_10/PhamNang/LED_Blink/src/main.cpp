#include <Arduino.h>

// ===== NON-BLOCKING =====
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// ===== GPIO =====
#define PIN_LED_RED     23
#define PIN_LED_YELLOW  22
#define PIN_LED_GREEN   21

// ===== STATE =====
enum LedState {
  RED,
  YELLOW,
  GREEN
};

void setup() {
  printf("WELCOME IOT\n");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}

void loop() {
  static LedState state = RED;
  static unsigned long ulTimer = 0;
  static bool ledStatus = false;
  static int blinkCount = 0;

  if (IsReady(ulTimer, 500)) {   // 500ms giống mẫu 1 đèn
    ledStatus = !ledStatus;

    // Tắt tất cả trước
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    // Bật LED theo state hiện tại
    switch (state) {
      case RED:
        digitalWrite(PIN_LED_RED, ledStatus ? HIGH : LOW);
        break;

      case YELLOW:
        digitalWrite(PIN_LED_YELLOW, ledStatus ? HIGH : LOW);
        break;

      case GREEN:
        digitalWrite(PIN_LED_GREEN, ledStatus ? HIGH : LOW);
        break;
    }

    // Đếm số lần chớp (chỉ đếm khi vừa OFF)
    if (!ledStatus) {
      blinkCount++;

      if (blinkCount >= 7) {
        blinkCount = 0;

        // Chuyển sang đèn tiếp theo
        if (state == RED) state = YELLOW;
        else if (state == YELLOW) state = GREEN;
        else state = RED;
      }
    }
  }
}
