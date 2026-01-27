#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (Theo sơ đồ chuẩn "đẹp" mới nhất) ---
#define CLK 18
#define DIO 19

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define LED_BLUE    22 // Đèn cho người đi bộ
#define BTN_WALK    23 // Nút nhấn
#define LDR_PIN     34 // Cảm biến ánh sáng

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  pinMode(BTN_WALK, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7); // Độ sáng tối đa
}

// Hàm kiểm tra nút nhấn (Trả về true nếu nút được nhấn)
bool checkPedestrianButton() {
  if (digitalRead(BTN_WALK) == LOW) {
    digitalWrite(LED_BLUE, HIGH); // Bật đèn xanh dương báo hiệu
    return true;
  }
  digitalWrite(LED_BLUE, LOW);
  return false;
}

/**
 * Hàm điều khiển đèn giao thông
 * @param seconds: Số giây đếm ngược
 * @param ledPin: Chân đèn cần bật (Xanh/Đỏ/Vàng)
 * @param isBlinking: true = Nhấp nháy, false = Sáng đứng
 */
void runTrafficLight(int seconds, int ledPin, bool isBlinking) {
  // Tắt hết các đèn trước khi bắt đầu pha mới
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, false); // Hiện số giây
    
    // --- NỬA GIÂY ĐẦU: BẬT ĐÈN ---
    digitalWrite(ledPin, HIGH);
    // Thay vì delay(500), ta chia nhỏ để nút bấm nhạy hơn
    for(int j=0; j<5; j++) { 
      checkPedestrianButton(); 
      delay(100); 
    }

    // --- NỬA GIÂY SAU: TẮT ĐÈN (Nếu chế độ nhấp nháy) ---
    if (isBlinking) {
      digitalWrite(ledPin, LOW); // Tắt để tạo hiệu ứng nháy
    } 
    // Nếu không phải chế độ nhấp nháy thì đèn vẫn giữ nguyên trạng thái HIGH từ bước trên

    // Chờ tiếp 0.5s (vừa chờ vừa check nút)
    for(int j=0; j<5; j++) { 
      checkPedestrianButton(); 
      delay(100); 
    }
  }
}

void loop() {
  // 1. ĐÈN XANH: 10 giây (Cho phép NHẤP NHÁY: true)
  // Bạn muốn đèn xanh chớp tắt liên tục thì để true
  runTrafficLight(10, LED_GREEN, true);

  // 2. ĐÈN VÀNG: 3 giây (Sáng đứng: false)
  runTrafficLight(3, LED_YELLOW, false);

  // 3. ĐÈN ĐỎ: 10 giây (Sáng đứng: false)
  runTrafficLight(10, LED_RED, false);
}