#include <Arduino.h>

// Hàm kiểm tra đã đủ thời gian chưa (không dùng delay)
bool IsReady(unsigned long &timer, uint32_t ms) {
  if (millis() - timer < ms) return false; // chưa đủ ms
  timer = millis();                        // đủ rồi -> cập nhật mốc
  return true;
}

#define PIN_LED_RED 23

void setup() {
  Serial.begin(115200);
  Serial.println("WELCOME IOT");

  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW); // ban đầu tắt
}

void loop() {
  static unsigned long timer = 0; // mốc thời gian lần trước
  static bool led = false;        // trạng thái LED

  if (IsReady(timer, 500)) {      // mỗi 500ms chạy 1 lần
    led = !led;                   // đảo trạng thái
    digitalWrite(PIN_LED_RED, led ? HIGH : LOW);
    // Serial.println(led ? "LED ON" : "LED OFF"); // muốn xem log thì mở dòng này
  }
}
