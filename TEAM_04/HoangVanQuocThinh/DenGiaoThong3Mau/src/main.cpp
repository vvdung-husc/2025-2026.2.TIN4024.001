#include <Arduino.h>
#include <TM1637Display.h>

// 1. Định nghĩa chân cắm (Khớp với file diagram.json của bạn)
#define PIN_RED    25
#define PIN_YELLOW 33
#define PIN_GREEN  32
#define PIN_BLUE   21
#define PIN_BTN    23
#define PIN_LDR    13
#define CLK_SEG    15
#define DIO_SEG    2

TM1637Display display(CLK_SEG, DIO_SEG);

// 2. Cấu hình thời gian (giây)
const int TIME_GREEN  = 7;
const int TIME_YELLOW = 3;
const int TIME_RED    = 5;

unsigned long prevMillis = 0;
int currentCountdown = TIME_GREEN;
int state = 0; // 0: Green, 1: Yellow, 2: Red

void updateLEDs() {
  digitalWrite(PIN_GREEN,  (state == 0));
  digitalWrite(PIN_YELLOW, (state == 1));
  digitalWrite(PIN_RED,    (state == 2));
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  updateLEDs();
}

void loop() {
  unsigned long currentMillis = millis();

  // A. Đọc cảm biến ánh sáng (LDR)
  int ldrValue = analogRead(PIN_LDR); 

  // B. Điều khiển đèn Blue bằng nút nhấn
  // Nhấn giữ nút (PIN 23) -> Đèn Blue (PIN 21) sáng
  if (digitalRead(PIN_BTN) == LOW) {
    digitalWrite(PIN_BLUE, HIGH);
  } else {
    digitalWrite(PIN_BLUE, LOW);
  }

  // C. Kiểm tra trạng thái đèn Blue để quyết định dừng bộ đếm
  bool isBlueOn = (digitalRead(PIN_BLUE) == HIGH);

  // D. Nếu cảm biến "full sáng" (giá trị nhỏ), ép hệ thống sang đèn Vàng
  if (ldrValue < 500) { 
    state = 1; 
    currentCountdown = TIME_YELLOW;
    updateLEDs();
  }

  // E. Hiển thị số lên bảng đếm (Luôn hiển thị để đèn không bị tắt)
  display.showNumberDec(currentCountdown);

  // F. Logic đếm ngược: Chỉ chạy khi đèn Blue TẮT
  if (!isBlueOn) { 
    if (currentMillis - prevMillis >= 1000) {
      prevMillis = currentMillis;
      
      if (currentCountdown <= 0) {
        state = (state + 1) % 3; // Chuyển trạng thái
        if (state == 0) currentCountdown = TIME_GREEN;
        else if (state == 1) currentCountdown = TIME_YELLOW;
        else if (state == 2) currentCountdown = TIME_RED;
        updateLEDs();
      } else {
        currentCountdown--;
      }
    }
  }
  // Nếu isBlueOn là true, bộ đếm không bị trừ, số trên màn hình sẽ đứng yên.
}