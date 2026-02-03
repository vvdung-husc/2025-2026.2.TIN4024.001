#include <Arduino.h>

// Định nghĩa chân cắm theo sơ đồ của bạn
const int LED_RED = 25;    // r1
const int LED_YELLOW = 33; // r3
const int LED_GREEN = 32;  // r2

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200); // Mở Terminal để xem giây đếm ngược
}

// Hàm giúp đèn nhấp nháy theo giây
void playLight(int pin, int seconds, String colorName) {
  for (int i = seconds; i > 0; i--) {
    Serial.printf("Den %s con: %d giay\n", colorName.c_str(), i);
    
    digitalWrite(pin, HIGH); // Bật đèn
    delay(500);              // Sáng 0.5 giây
    digitalWrite(pin, LOW);  // Tắt đèn
    delay(500);              // Tắt 0.5 giây -> Tổng cộng là 1 giây nhấp nháy
  }
}

void loop() {
  // 1. Đèn Xanh nhấp nháy 6 giây
  playLight(LED_GREEN, 6, "XANH");

  // 2. Đèn Vàng nhấp nháy 2 giây
  playLight(LED_YELLOW, 2, "VANG");

  // 3. Đèn Đỏ nhấp nháy 5 giây
  playLight(LED_RED, 5, "DO");
}