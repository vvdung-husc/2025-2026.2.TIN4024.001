#include <Arduino.h>

// Chân LED
#define PIN_LED_RED    25
#define PIN_LED_GREEN  32
#define PIN_LED_YELLOW 33

// Hàm kiểm tra thời gian non-blocking
bool IsReady(unsigned long &timer, uint32_t ms) {
  if (millis() - timer < ms) return false;
  timer = millis();
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("SEQUENTIAL BLINK: RED -> GREEN -> YELLOW");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
}

void loop() {
  static unsigned long blinkTimer = 0;
  static unsigned long phaseTimer = 0;

  static bool ledState = false;
  static int phase = 0;     // 0 = Red, 1 = Green, 2 = Yellow
  static int blinkCount = 0;

  const int maxBlink = 6;   // số lần nhấp nháy mỗi màu

  // Nhấp nháy LED mỗi 300ms
  if (IsReady(blinkTimer, 300)) {
    ledState = !ledState;

    // Tắt tất cả trước
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);

    // Bật LED theo phase
    if (phase == 0 && ledState) digitalWrite(PIN_LED_RED, HIGH);
    if (phase == 1 && ledState) digitalWrite(PIN_LED_GREEN, HIGH);
    if (phase == 2 && ledState) digitalWrite(PIN_LED_YELLOW, HIGH);

    blinkCount++;

    // Sau khi nhấp nháy đủ lần → đổi màu
    if (blinkCount >= maxBlink) {
      blinkCount = 0;
      phase = (phase + 1) % 3; // Red -> Green -> Yellow
    }
  }
}
