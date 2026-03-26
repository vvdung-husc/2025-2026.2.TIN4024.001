#include <Arduino.h>

// Khai báo danh sách tất cả các chân có thể kết nối với Relay
int tatCaCacChan[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
int soLuongChan = 9;

void setup() {
  // Cài đặt tất cả các chân thành OUTPUT
  for (int i = 0; i < soLuongChan; i++) {
    pinMode(tatCaCacChan[i], OUTPUT);
  }
}

void loop() {
  // Cho tất cả các chân chớp nháy cùng một lúc
  for (int i = 0; i < soLuongChan; i++) {
    digitalWrite(tatCaCacChan[i], HIGH);
  }
  delay(500); // Cùng BẬT trong 0.5 giây
  
  for (int i = 0; i < soLuongChan; i++) {
    digitalWrite(tatCaCacChan[i], LOW);
  }
  delay(500); // Cùng TẮT trong 0.5 giây
}