#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN ---
#define CLK 18
#define DIO 19

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    22
#define BTN_WALK    23
#define LDR_PIN     34 

// Ngưỡng ánh sáng
#define LIGHT_THRESHOLD 2000 

TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  pinMode(BTN_WALK, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
}

// Kiểm tra nút đi bộ
void checkPedestrianButton() {
  if (digitalRead(BTN_WALK) == LOW) {
    digitalWrite(LED_BLUE, HIGH);
  } else {
    digitalWrite(LED_BLUE, LOW);
  }
}

// --- HÀM CHẠY ĐÈN (Hỗ trợ nhấp nháy) ---
// isBlinking = true: Đèn sẽ chớp tắt (dùng cho đèn Xanh)
// isBlinking = false: Đèn sáng đứng (dùng cho đèn Đỏ/Vàng)
void runTrafficLight(int seconds, int ledPin, bool isBlinking) {
  // Tắt hết đèn trước khi chạy pha này
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  for (int i = seconds; i >= 0; i--) {
    // Nếu ánh sáng thay đổi đột ngột (đang chạy mà trời tối) -> Thoát ngay
    // Lưu ý: Logic ở đây phải khớp với logic trong loop()
    if (analogRead(LDR_PIN) > LIGHT_THRESHOLD) return; 

    display.showNumberDec(i, false);
    
    // --- GIAI ĐOẠN 1: ĐÈN SÁNG (0.5 giây đầu) ---
    digitalWrite(ledPin, HIGH);
    for(int j=0; j<5; j++) { // Chờ 0.5s
      checkPedestrianButton();
      delay(100); 
    }

    // --- GIAI ĐOẠN 2: XỬ LÝ NHẤP NHÁY (0.5 giây sau) ---
    if (isBlinking) {
      digitalWrite(ledPin, LOW); // Tắt đèn để tạo hiệu ứng nháy
    } 
    // Nếu không nhấp nháy thì đèn vẫn giữ nguyên là HIGH

    for(int j=0; j<5; j++) { // Chờ tiếp 0.5s
      checkPedestrianButton();
      delay(100); 
    }
  }
}

// --- CHẾ ĐỘ BAN ĐÊM (Nháy Vàng) ---
void runNightMode() {
  Serial.println("--- CHE DO BAN DEM (Nhay Vang) ---");
  
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  
  // Hiện gạch ngang
  uint8_t data[] = { 0x40, 0x40, 0x40, 0x40 }; 
  display.setSegments(data);

  // Nháy Vàng
  digitalWrite(LED_YELLOW, HIGH);
  delay(500);
  digitalWrite(LED_YELLOW, LOW);
  delay(500);
}

void loop() {
  int lightLevel = analogRead(LDR_PIN);
  Serial.print("Anh sang: ");
  Serial.println(lightLevel);

  // --- SỬA LỖI NGƯỢC: Đổi dấu < thành > ---
  // Nếu giá trị lớn (Thanh trượt bên Phải - Sáng) -> Chạy chế độ Đêm (theo logic của bạn đang bị ngược)
  // Nếu logic vẫn chưa đúng ý, bạn chỉ cần đổi dấu > thành < tại dòng dưới đây:
  
  if (lightLevel > LIGHT_THRESHOLD) { 
    // === TRỜI TỐI (Ban đêm) ===
    runNightMode();
  } else {
    // === TRỜI SÁNG (Ban ngày) ===
    Serial.println("--- CHE DO BAN NGAY ---");
    
    // 1. ĐÈN XANH: 10 giây - CÓ NHẤP NHÁY (true)
    runTrafficLight(10, LED_GREEN, true);
    
    // Kiểm tra lại ánh sáng để thoát nhanh nếu cần
    if (analogRead(LDR_PIN) > LIGHT_THRESHOLD) return;

    // 2. ĐÈN VÀNG: 3 giây - KHÔNG NHÁY (false)
    runTrafficLight(3, LED_YELLOW, true);

    if (analogRead(LDR_PIN) > LIGHT_THRESHOLD) return;

    // 3. ĐÈN ĐỎ: 10 giây - KHÔNG NHÁY (false)
    runTrafficLight(10, LED_RED, true);
  }
}