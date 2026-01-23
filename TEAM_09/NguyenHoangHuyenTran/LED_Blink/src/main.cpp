#include <Arduino.h>

// Định nghĩa chân nối (khớp với file diagram ở trên)
#define LED_RED    25  // Đèn đỏ nối chân 23
#define LED_YELLOW 33  // Đèn vàng nối chân 22
#define LED_GREEN  32  // Đèn xanh nối chân 21

void setup() {
  Serial.begin(115200);
  
  // Cài đặt 3 chân này là chân xuất tín hiệu
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // 1. ĐÈN ĐỎ SÁNG (5 giây)
  Serial.println("LED RED ON => 5 Seconds");
  digitalWrite(LED_RED, HIGH);    // Bật Đỏ
  digitalWrite(LED_YELLOW, LOW);  // Tắt Vàng
  digitalWrite(LED_GREEN, LOW);   // Tắt Xanh
  delay(5000);                    // Chờ 5000ms = 5 giây

  // 2. ĐÈN XANH SÁNG (7 giây) - Theo đúng thứ tự trong ảnh của Công chúa
  Serial.println("LED GREEN ON => 7 Seconds");
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  delay(7000);

  // 3. ĐÈN VÀNG SÁNG (3 giây)
  Serial.println("LED YELLOW ON => 3 Seconds");
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, LOW);
  delay(3000);
}