#include <Arduino.h>

#define LED_PIN 22   // Chân GPIO nối với LED

void setup() {
  pinMode(LED_PIN, OUTPUT); // Khai báo chân LED là OUTPUT
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Bật LED
  delay(500);                  // Chờ 500 ms
  digitalWrite(LED_PIN, LOW);  // Tắt LED
  delay(500);                  // Chờ 500 ms
}