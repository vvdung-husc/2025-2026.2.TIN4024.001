#include <Arduino.h> 

// --- CẤU HÌNH CHÂN (Theo sơ đồ bạn gửi) ---
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {

  /* ========= 1. ĐÈN XANH – NHẤP NHÁY 7s ========= */
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);

  for (int i = 0; i < 7; i++) {
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    delay(500);
  }
  Serial.println("Den xanh da nhay 7s");

  /* ========= 2. ĐÈN VÀNG – NHẤP NHÁY 3s ========= */
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
  Serial.println("Den vang da nhay 3s");

  /* ========= 3. ĐÈN ĐỎ – NHẤP NHÁY 5s ========= */
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);

  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RED, HIGH);
    delay(500);
    digitalWrite(LED_RED, LOW);
    delay(500);
  }
  Serial.println("Den do da nhay 5s");
}
