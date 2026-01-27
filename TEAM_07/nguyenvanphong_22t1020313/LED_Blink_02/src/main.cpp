#include <Arduino.h> // <--- DÒNG NÀY SẼ SỬA TOÀN BỘ LỖI CỦA BẠN

// Khai báo chân (Khớp với file diagram.json 3 đèn)
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25  

// Thời gian nghỉ giữa các giai đoạn chuyển màu (500ms)
#define DELAY_GIAI_DOAN 500 

// --- Hàm xử lý nhấp nháy ---
void nhapNhay(int pin, int soLan) {
  for(int i = 0; i < soLan; i++) {
    digitalWrite(pin, HIGH); // Bật
    delay(700);              // Sáng 700ms
    digitalWrite(pin, LOW);  // Tắt
    delay(700);              // Tắt 700ms
  }
}

void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  
}

void loop() {
  // 1. XANH: Nhấp nháy 7 lần
  
  nhapNhay(LED_GREEN, 7);
  delay(DELAY_GIAI_DOAN); 

  // 2. VÀNG: Nhấp nháy 3 lần
  
  nhapNhay(LED_YELLOW, 3);
  delay(DELAY_GIAI_DOAN);

  // 3. ĐỎ: Nhấp nháy 5 lần
  
  nhapNhay(LED_RED, 5);
  delay(DELAY_GIAI_DOAN);
}