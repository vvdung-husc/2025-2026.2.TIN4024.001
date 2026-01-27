#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (Giữ nguyên theo code bạn gửi) ---
#define CLK 18
#define DIO 19

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define LED_BLUE    22 // Đèn báo trạng thái nút nhấn
#define BTN_WALK    23 // Nút nhấn (Dùng trở kéo lên - INPUT_PULLUP)
#define LDR_PIN     34 // Cảm biến ánh sáng

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

// Biến trạng thái hệ thống (true = đang chạy, false = tắt)
bool systemState = true; 

// Hàm xử lý nút bấm (Trả về true nếu trạng thái hệ thống vừa thay đổi)
bool checkButton() {
  // Nút nhấn nối đất (GND) nên khi nhấn sẽ là LOW
  if (digitalRead(BTN_WALK) == LOW) {
    delay(20); // Chống dội phím (Debounce)
    if (digitalRead(BTN_WALK) == LOW) {
      // 1. Đảo trạng thái hệ thống
      systemState = !systemState;
      
      // 2. Nháy đèn xanh dương 1 lần báo hiệu
      digitalWrite(LED_BLUE, HIGH);
      delay(200);
      digitalWrite(LED_BLUE, LOW);

      // 3. Chờ thả nút để tránh nhảy trạng thái liên tục
      while(digitalRead(BTN_WALK) == LOW); 
      
      return true; // Có thay đổi trạng thái
    }
  }
  return false; // Không nhấn nút
}

// Hàm delay thông minh: Vừa chờ vừa kiểm tra nút bấm
// Trả về true nếu hệ thống bị TẮT giữa chừng
bool smartWait(int ms) {
  int steps = ms / 10; // Chia nhỏ thời gian check mỗi 10ms
  for (int j = 0; j < steps; j++) {
    if (checkButton()) return true; // Nếu có nhấn nút -> Thoát ngay
    // Nếu hệ thống đang tắt, trả về true để thoát vòng lặp chính
    if (!systemState) return true; 
    delay(10);
  }
  return false;
}

// Hàm đếm ngược và nhấp nháy đèn
// Trả về false nếu bị ngắt bởi nút bấm
bool runPhase(int seconds, int ledPin) {
  // Nếu hệ thống đang tắt thì không chạy
  if (!systemState) return false;

  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, false); 
    
    // --- NỬA GIÂY ĐẦU: BẬT ĐÈN ---
    digitalWrite(ledPin, HIGH);
    if (smartWait(500)) { // Chờ 500ms, nếu bấm nút thì thoát
        digitalWrite(ledPin, LOW); // Tắt đèn ngay
        return false; 
    }

    // --- NỬA GIÂY SAU: TẮT ĐÈN (Tạo hiệu ứng nhấp nháy) ---
    digitalWrite(ledPin, LOW);
    if (smartWait(500)) { // Chờ 500ms tiếp
        return false;
    }
  }
  return true; // Chạy xong pha này trọn vẹn
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  pinMode(BTN_WALK, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
  Serial.println("HE THONG BAT DAU...");
}

void loop() {
  // Kiểm tra nút bấm liên tục nếu hệ thống đang tắt
  if (!systemState) {
    display.clear(); // Tắt màn hình
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    checkButton(); // Chỉ ngồi chờ bấm nút
    return;
  }

  // Nếu hệ thống đang BẬT, chạy tuần tự các đèn
  // Các hàm runPhase sẽ tự thoát nếu nút được nhấn giữa chừng
  if (systemState) {
    if (!runPhase(7, LED_GREEN)) return;  // Đèn Xanh 10s
  }
  
  if (systemState) {
    if (!runPhase(3, LED_YELLOW)) return;  // Đèn Vàng 3s
  }
  
  if (systemState) {
    if (!runPhase(5, LED_RED)) return;    // Đèn Đỏ 10s
  }
}