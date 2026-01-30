#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (PINS) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_STREET  21  

#define CLK 18
#define DIO 19

#define BTN_PIN 4
#define LDR_PIN 34      // Chân Analog Input (AO)

// --- CẤU HÌNH THỜI GIAN ---
const long RED_TIME = 5000;
const long GREEN_TIME = 5000;
const long YELLOW_TIME = 2000;

TM1637Display display(CLK, DIO);

// --- BIẾN TRẠNG THÁI ---
int trafficState = 0; // 0: RED, 1: GREEN, 2: YELLOW
unsigned long previousMillis = 0;
bool isDisplayVisible = true; 

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Chân 34 là Input-only nên không cần pinMode OUTPUT, chỉ cần analogRead

  display.setBrightness(0x0f);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. ĐỌC CẢM BIẾN LDR ---
  // ESP32 Analog Read: 0 (0V) đến 4095 (3.3V)
  // Theo nguyên lý: Lux càng cao -> Điện trở LDR càng nhỏ -> Analog đọc được càng nhỏ (nếu nối Pull-up)
  // Ta giả lập: Giá trị Analog < 500 tương ứng với Lux > 2000 (Trời rất sáng)
  int analogValue = analogRead(LDR_PIN);
  
  // Biến đổi ngược để dễ hình dung (Giả lập Lux)
  // Giá trị này chỉ mang tính tương đối để so sánh với ngưỡng 2000
  int simulatedLux = map(analogValue, 0, 4095, 3000, 0); 

  // --- LOGIC CHÍNH ---
  if (simulatedLux > 2000) {
    // === CHẾ ĐỘ 1: TRỜI SÁNG (LUX > 2000) ===
    // Yêu cầu: Đèn vàng nhấp nháy, Màn hình tắt.
    
    display.clear(); // Tắt màn hình hiển thị số
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_STREET, LOW); // Trời sáng tắt đèn đường

    // Nhấp nháy đèn vàng (Chu kỳ 500ms)
    if ((currentMillis / 500) % 2 == 0) {
      digitalWrite(LED_YELLOW, HIGH);
    } else {
      digitalWrite(LED_YELLOW, LOW);
    }
    
    // Reset bộ đếm giao thông để khi quay lại chế độ thường sẽ bắt đầu từ đầu
    previousMillis = currentMillis; 

  } else {
    // === CHẾ ĐỘ 2: BÌNH THƯỜNG (LUX <= 2000) ===
    
    // Bật đèn đường (vì trời tối)
    digitalWrite(LED_STREET, HIGH);

    // Xử lý đèn giao thông
    long duration = 0;
    if (trafficState == 0) duration = RED_TIME;
    else if (trafficState == 1) duration = GREEN_TIME;
    else duration = YELLOW_TIME;

    long elapsed = currentMillis - previousMillis;
    long remainingTime = (duration - elapsed) / 1000;

    if (elapsed >= duration) {
      previousMillis = currentMillis;
      trafficState++;
      if (trafficState > 2) trafficState = 0;
      elapsed = 0;
    }

    // Hiển thị đèn
    digitalWrite(LED_RED, trafficState == 0 ? HIGH : LOW);
    digitalWrite(LED_GREEN, trafficState == 1 ? HIGH : LOW);
    digitalWrite(LED_YELLOW, trafficState == 2 ? HIGH : LOW);

    // Xử lý nút nhấn (Bật/tắt màn hình)
    if (digitalRead(BTN_PIN) == LOW) {
        isDisplayVisible = !isDisplayVisible;
        delay(200); // Debounce
    }

    if (isDisplayVisible) {
      display.showNumberDec(remainingTime + 1);
    } else {
      display.clear();
    }
  }
}