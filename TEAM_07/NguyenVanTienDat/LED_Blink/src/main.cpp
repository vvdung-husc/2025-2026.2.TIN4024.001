#include <Arduino.h>

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25  

void tatTatCa() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {

  // ===== XANH 7 GIÂY =====
  tatTatCa();
  digitalWrite(LED_GREEN, HIGH);
  Serial.println("DEN XANH");
  delay(7000);

  // ===== VÀNG 3 GIÂY =====
  tatTatCa();
  digitalWrite(LED_YELLOW, HIGH);
  Serial.println("DEN VANG");
  delay(3000);

  // ===== ĐỎ 5 GIÂY =====
  tatTatCa();
  digitalWrite(LED_RED, HIGH);
  Serial.println("DEN DO");
  delay(5000);
}
