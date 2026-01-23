#include <Arduino.h>

// --- CẤU HÌNH CHÂN (Theo sơ đồ bạn gửi) ---
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

void setup() {
  // Bật Serial Monitor để xem đếm giây trên màn hình
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // --- 1. ĐÈN XANH (Vừa nhấp nháy, vừa đếm giây) ---
  // Tổng thời gian: 10 giây (Lặp 10 lần, mỗi lần 1 giây)
  Serial.println("--- DEN XANH (NHAP NHAY) ---");
  
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);

  for (int i = 10; i > 0; i--) {
    Serial.print("Con lai: ");
    Serial.print(i);
    Serial.println(" giay");

    digitalWrite(LED_GREEN, HIGH); // Bật
    delay(500);                    // Sáng 0.5 giây
    digitalWrite(LED_GREEN, LOW);  // Tắt
    delay(500);                    // Tắt 0.5 giây
  }

  // --- 2. ĐÈN VÀNG (Sáng đứng 3 giây) ---
  Serial.println("--- DEN VANG ---");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  delay(3000); 
  digitalWrite(LED_YELLOW, LOW);

  // --- 3. ĐÈN ĐỎ (Sáng đứng 10 giây) ---
  Serial.println("--- DEN DO ---");
  digitalWrite(LED_RED, HIGH);
  
  // Bạn có muốn đèn đỏ cũng đếm ngược không? Code dưới đây để đếm ngược cho đèn đỏ:
  for (int i = 10; i > 0; i--) {
    Serial.print("Dung lai: ");
    Serial.print(i);
    Serial.println(" giay");
    delay(1000); // Chờ 1 giây (Đèn đỏ vẫn sáng liên tục)
  }
  
  digitalWrite(LED_RED, LOW); // Tắt đỏ để quay lại vòng lặp xanh
}