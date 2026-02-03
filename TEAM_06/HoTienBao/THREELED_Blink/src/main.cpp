#include <Arduino.h>

// --- 1. CẤU HÌNH CHÂN ĐÈN (Theo diagram.json) ---
const int RED_LED = 25;
const int YELLOW_LED = 33;
const int GREEN_LED = 32;

// --- 2. CẤU HÌNH THỜI GIAN (Mili giây) ---
const int TIME_RED = 5000;         // Đỏ sáng 5 giây
const int TIME_YELLOW = 3000;      // Vàng sáng 3 giây
const int TIME_GREEN_FULL = 5000;  // Xanh sáng đứng 5 giây
const int BLINK_COUNT = 4;         // Số lần nháy đèn xanh trước khi tắt

void setup() {
  // Thiết lập các chân là đầu ra (OUTPUT)
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Tắt hết đèn khi khởi động để tránh lỗi
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  // Khởi tạo Serial
  Serial.begin(115200);
  Serial.println(">>> HE THONG DEN GIAO THONG KHOI DONG <<<");
}

void loop() {
  // ==========================================
  // GIAI ĐOẠN 1: ĐÈN XANH (Đi)
  // ==========================================
  Serial.println("[1] XANH - Di chuyen");
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH); // Bật xanh
  delay(TIME_GREEN_FULL);        // Chờ sáng đứng

  // >>> NÂNG CẤP: Hiệu ứng đèn xanh nhấp nháy báo hiệu sắp hết giờ
  Serial.println("    XANH - Sap het gio (Nhap nhay)");
  for (int i = 0; i < BLINK_COUNT; i++) {
    digitalWrite(GREEN_LED, LOW);  // Tắt
    delay(250);                    // Chờ
    digitalWrite(GREEN_LED, HIGH); // Bật
    delay(250);                    // Chờ
  }
  digitalWrite(GREEN_LED, LOW);    // Tắt hẳn đèn xanh

  // ==========================================
  // GIAI ĐOẠN 2: ĐÈN VÀNG (Đi chậm)
  // ==========================================
  Serial.println("[2] VANG - Giam toc do");
  digitalWrite(YELLOW_LED, HIGH); // Bật vàng
  delay(TIME_YELLOW);
  digitalWrite(YELLOW_LED, LOW);  // Tắt vàng

  // ==========================================
  // GIAI ĐOẠN 3: ĐÈN ĐỎ (Dừng lại)
  // ==========================================
  Serial.println("[3] DO - Dung lai");
  digitalWrite(RED_LED, HIGH);    // Bật đỏ
  delay(TIME_RED);
  // (Đèn đỏ sẽ tự tắt khi vòng lặp quay lại Giai đoạn 1 bật đèn xanh)
}