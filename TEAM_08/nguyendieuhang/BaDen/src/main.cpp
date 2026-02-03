#include <Arduino.h>

// --- 1. KHAI BÁO CÁC CHÂN (Theo sơ đồ JSON) ---
#define PIN_YELLOW 33
#define PIN_RED    25
#define PIN_GREEN  32

// --- 2. HÀM ISREADY (Giữ nguyên của bạn) ---
// Hàm này giúp kiểm tra xem đã trôi qua bao nhiêu mili-giây rồi
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200); // Khởi động Serial để in log
  printf("HE THONG DEN GIAO THONG - NON BLOCKING\n");

  // Cấu hình 3 chân là Output
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Trạng thái ban đầu: Bật đèn Vàng lên trước
  digitalWrite(PIN_YELLOW, HIGH);
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
}

void loop() {
  // Biến lưu thời gian
  static unsigned long ulTimer = 0;
  
  // Biến trạng thái: 0=Vàng, 1=Đỏ, 2=Xanh
  static int state = 0; 
  
  // Biến lưu thời gian chờ (khởi tạo 3000ms cho đèn Vàng đang sáng ở setup)
  static uint32_t timeLimit = 3000; 

  // --- LOGIC CHÍNH ---
  // Kiểm tra: Đã hết thời gian của đèn hiện tại chưa?
  if (IsReady(ulTimer, timeLimit)) {
    
    // Nếu hết giờ, chuyển sang trạng thái tiếp theo
    state++;
    if (state > 2) state = 0; // Nếu quá 2 (Xanh) thì quay về 0 (Vàng)

    // Xử lý bật/tắt đèn và cài đặt thời gian cho trạng thái mới
    switch (state) {
      case 0: // Chuyển sang VÀNG (Yellow)
        digitalWrite(PIN_YELLOW, HIGH);
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_GREEN, LOW);
        timeLimit = 3000; // Vàng sáng 3 giây
        printf("Trang thai: VANG (3s)\n");
        break;

      case 1: // Chuyển sang ĐỎ (Red)
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_GREEN, LOW);
        timeLimit = 5000; // Đỏ sáng 5 giây
        printf("Trang thai: DO (5s)\n");
        break;

      case 2: // Chuyển sang XANH (Green)
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_RED, LOW);
        digitalWrite(PIN_GREEN, HIGH);
        timeLimit = 7000; // Xanh sáng 7 giây
        printf("Trang thai: XANH (7s)\n");
        break;
    }
  }
  
  // Bạn có thể viết code khác ở đây, hệ thống vẫn chạy mà không bị dừng (delay)
}