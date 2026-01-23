#include <Arduino.h>


#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // 1. ĐÈN ĐỎ NHẤP NHÁY 5 GIÂY (5 lần)
  // Vòng lặp chạy 5 lần, mỗi lần 1 giây
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RED, HIGH);  // Bật
    delay(500);                   // Sáng 0.5 giây
    digitalWrite(LED_RED, LOW);   // Tắt
    delay(500);                   // Tắt 0.5 giây
  }

  // 2. ĐÈN XANH NHẤP NHÁY 7 GIÂY (7 lần)
  for (int i = 0; i < 7; i++) {
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    delay(500);
  }

  // 3. ĐÈN VÀNG NHẤP NHÁY 3 GIÂY (3 lần)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
}