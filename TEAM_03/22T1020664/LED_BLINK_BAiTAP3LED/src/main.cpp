#include <Arduino.h>

bool IsReady(unsigned long &t, uint32_t ms) {
  if (millis() - t < ms) return false;
  t = millis();
  return true;
}

// ✅ ĐÚNG THEO diagram.json
#define PIN_LED_GREEN  32
#define PIN_LED_YELLOW 33
#define PIN_LED_RED    25

const uint32_t PERIOD_MS = 1000; // 2 giây / 1 lần nháy
const uint32_t PULSE_MS  = 200;  // sáng 200ms (tùy chỉnh)

const int ledPins[3] = { PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED };

void allOff() {
  for (int i = 0; i < 3; i++) digitalWrite(ledPins[i], LOW);
}

void setup() {
  for (int i = 0; i < 3; i++) pinMode(ledPins[i], OUTPUT);
  allOff();
}

void loop() {
  static int currentLed = 0;   // 0 xanh, 1 vàng, 2 đỏ
  static int count = 0;        // số lần nháy (0..5)
  static bool isOn = false;
  static unsigned long timer = 0;

  if (!isOn) {
    // chờ phần OFF để đủ chu kỳ 2s rồi bật nháy
    uint32_t offMs = (PERIOD_MS > PULSE_MS) ? (PERIOD_MS - PULSE_MS) : 0;
    if (IsReady(timer, offMs)) {
      allOff();
      digitalWrite(ledPins[currentLed], HIGH);
      isOn = true;
      count++;
    }
  } else {
    // giữ sáng PULSE_MS rồi tắt
    if (IsReady(timer, PULSE_MS)) {
      digitalWrite(ledPins[currentLed], LOW);
      isOn = false;

      if (count >= 5) {              // đủ 5 lần -> chuyển đèn
        count = 0;
        currentLed = (currentLed + 1) % 3;
      }
    }
  }
}