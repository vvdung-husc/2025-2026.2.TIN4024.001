#include <Arduino.h>

// Định nghĩa các chân cắm dựa theo diagram.json
const int RED_LED = 25;
const int YELLOW_LED = 33;
const int GREEN_LED = 32;

void setup() {
  // Thiết lập các chân LED là đầu ra (OUTPUT)
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Khởi tạo Serial để theo dõi trạng thái trên màn hình
  Serial.begin(115200);
  Serial.println("He thong den giao thong dang khoi dong...");
}

void loop() {
  // 1. ĐÈN XANH SÁNG (7 GIÂY)
  Serial.println("XANH - Di chuyen (7s)");
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(7000); // 7000ms = 7s

  // 2. ĐÈN VÀNG SÁNG (3 GIÂY)
  Serial.println("VANG - Di cham lai (3s)");
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(3000); // 3000ms = 3s

  // 3. ĐÈN ĐỎ SÁNG (5 GIÂY)
  Serial.println("DO - Dung lai (5s)");
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(5000); // 5000ms = 5s
}