#include <TM1637Display.h>

// 1. Khai báo chân (Đã cập nhật chân 21 cho LED Blue)
#define RED_PIN 27
#define YEL_PIN 26
#define GRN_PIN 25
#define BLUE_PIN 21  // Sửa từ 2 thành 21 theo sơ đồ của bạn
#define BTN_PIN 23
#define LDR_PIN 34 
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(115200);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(YEL_PIN, OUTPUT);
  pinMode(GRN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  display.clear();
}

void loop() {
  // CHU KỲ 1: ĐÈN XANH (Xe đi)
  digitalWrite(GRN_PIN, HIGH);
  digitalWrite(YEL_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  runCountdown(12, true); // true nghĩa là cho phép nút bấm rút ngắn thời gian

  // CHU KỲ 2: ĐÈN VÀNG (Chờ)
  digitalWrite(GRN_PIN, LOW);
  digitalWrite(YEL_PIN, HIGH);
  digitalWrite(RED_PIN, LOW);
  runCountdown(3, false);

  // CHU KỲ 3: ĐÈN ĐỎ (Dừng)
  digitalWrite(GRN_PIN, LOW);
  digitalWrite(YEL_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);
  runCountdown(15, false);
}

// Hàm đếm ngược thông minh
void runCountdown(int seconds, bool canInterrupt) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i);

    // Vòng lặp nhỏ kiểm tra nút bấm liên tục (để nút bấm cực nhạy)
    unsigned long startMillis = millis();
    while (millis() - startMillis < 1000) {
      
      // Kiểm tra nút bấm
      if (digitalRead(BTN_PIN) == LOW) {
        digitalWrite(BLUE_PIN, HIGH); // Đèn Blue PHẢI sáng ở đây
        
        // Nếu là đèn xanh và được phép ngắt, nhảy về 3 giây cuối
        if (canInterrupt && i > 3) {
          i = 3; 
        }
      } else {
        digitalWrite(BLUE_PIN, LOW);
      }
      
      delay(10); 
    }
  }
}