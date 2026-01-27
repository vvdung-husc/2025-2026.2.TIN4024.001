#include <Arduino.h>

// CẤU HÌNH CHÂN
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// Hàm nhấp nháy + đếm giây
void blinkLed(int ledPin, int seconds, const char* label) {
  Serial.println(label);

  for (int i = seconds; i > 0; i--) {
    Serial.print("Con lai: ");
    Serial.print(i);
    Serial.println(" giay");

    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // ĐÈN XANH
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  blinkLed(LED_GREEN, 10, "--- DEN XANH (NHAP NHAY) ---");

  // ĐÈN VÀNG
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  blinkLed(LED_YELLOW, 3, "--- DEN VANG (NHAP NHAY) ---");

  // ĐÈN ĐỎ
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  blinkLed(LED_RED, 10, "--- DEN DO (NHAP NHAY) ---");
}
