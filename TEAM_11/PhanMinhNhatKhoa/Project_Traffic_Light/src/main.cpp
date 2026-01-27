#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (Theo sơ đồ hình ảnh) ---
#define CLK 18
#define DIO 19

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define LED_BLUE    22 // Đèn người đi bộ
#define BTN_WALK    23 // Nút nhấn
#define LDR_PIN     34 // Cảm biến ánh sáng

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(115200);
  
  // Cấu hình LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Cấu hình Nút nhấn và LDR
  pinMode(BTN_WALK, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  // Cấu hình độ sáng màn hình (0-7)
  display.setBrightness(7);
}

// Hàm đếm ngược hiển thị lên màn hình LED 4 số
void countdown(int seconds, int ledPin) {
  // Bật đèn tương ứng
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(ledPin, HIGH);

  for (int i = seconds; i >= 0; i--) {
    // Hiển thị số lên màn hình
    display.showNumberDec(i, false); // false để không hiện số 0 vô nghĩa
    
    // Kiểm tra nút bấm người đi bộ
    if (digitalRead(BTN_WALK) == LOW) {
        Serial.println("Nguoi di bo bam nut!");
        digitalWrite(LED_BLUE, HIGH); // Bật đèn xanh dương
    } else {
        digitalWrite(LED_BLUE, LOW);
    }

    // Đọc cảm biến ánh sáng (Chỉ để in ra cho biết)
    int lightLevel = analogRead(LDR_PIN);
    // Serial.printf("Light: %d\n", lightLevel);

    delay(1000);
  }
}

void loop() {
  // 1. ĐÈN XANH (10 giây)
  countdown(10, LED_GREEN);

  // 2. ĐÈN VÀNG (3 giây)
  countdown(3, LED_YELLOW);

  // 3. ĐÈN ĐỎ (10 giây)
  countdown(10, LED_RED);
}