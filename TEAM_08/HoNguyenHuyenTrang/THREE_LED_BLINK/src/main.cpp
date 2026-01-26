#include <Arduino.h>

bool IsReady(unsigned long &t, uint32_t ms) {
  if (millis() - t < ms) return false;
  t = millis();
  return true;
}

#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

int leds[3] = {LED_YELLOW, LED_RED, LED_GREEN}; // thứ tự chạy
int durations[3] = {3000, 5000, 7000};         // thời gian mỗi LED

void setup() {
  for (int i = 0; i < 3; i++)
    pinMode(leds[i], OUTPUT);
}

void loop() {
  static unsigned long tBlink = 0;
  static unsigned long tChange = 0;
  static bool status = false;
  static int state = 0;

  // Nhấp nháy mỗi 1 giây
  if (IsReady(tBlink, 1000)) {
    status = !status;
    digitalWrite(leds[state], status);
  }

  // Đổi LED theo thời gian riêng
  if (IsReady(tChange, durations[state])) {
    digitalWrite(leds[state], LOW); // tắt LED cũ
    state = (state + 1) % 3;
    status = false;                // reset cho LED mới
  }
}
