#include <Arduino.h>
#include <TM1637Display.h>

// ===== 1. CẤU HÌNH CHÂN (THEO DIAGRAM) =====
#define CLK 18
#define DIO 19

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21  // Đèn báo trạng thái nút/người đi bộ
#define BUTTON_PIN  23
#define LDR_PIN     13  // Cảm biến ánh sáng

// ===== 2. CẤU HÌNH THỜI GIAN (GIÂY) =====
#define TIME_GREEN  5
#define TIME_YELLOW 2
#define TIME_RED    3

// ===== 3. CẤU HÌNH NGƯỠNG ÁNH SÁNG =====
// Bạn hãy bật Serial Monitor để xem giá trị thực tế
// Giả định mới: Giá trị CAO = TỐI, Giá trị THẤP = SÁNG
const int NIGHT_THRESHOLD = 2000; 

TM1637Display display(CLK, DIO);

// ===== 4. BIẾN HỆ THỐNG =====
unsigned long lastBlinkTime = 0; // Timer cho nhấp nháy đèn
const int BLINK_INTERVAL = 500;  // Nhấp nháy mỗi 0.5 giây

int countdown = 0;
int trafficState = 0; // 0: Xanh | 1: Vàng | 2: Đỏ

bool isSystemOn = false;   // Bật/Tắt hệ thống
bool isLedOn = false;      // Trạng thái nhấp nháy (Sáng/Tối)
bool isNightMode = false;  // Trạng thái Ngày/Đêm

// Biến nút nhấn
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ===== 5. CÁC HÀM HỖ TRỢ =====

void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  display.clear();
}

// Hàm điều khiển đèn (Dùng chung cho cả logic đếm giờ và nhấp nháy)
void updateTrafficLeds(bool turnOn) {
  // Luôn tắt hết trước khi bật lại
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  if (!turnOn) return; // Nếu đang ở pha tắt của nhịp nháy thì thoát

  // Bật đèn theo pha hiện tại
  if (trafficState == 0) digitalWrite(LED_GREEN, HIGH);
  else if (trafficState == 1) digitalWrite(LED_YELLOW, HIGH);
  else if (trafficState == 2) digitalWrite(LED_RED, HIGH);
}

// Reset hệ thống về trạng thái ban đầu
void resetSystemState() {
  trafficState = 0; // Bắt đầu Xanh
  countdown = TIME_GREEN;
  isLedOn = true;
  lastBlinkTime = millis();
  
  digitalWrite(LED_BLUE, HIGH); // Báo hiệu bật
  display.showNumberDec(countdown, true);
  updateTrafficLeds(true);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
  turnOffAll();
  Serial.println("HE THONG BAT DAU");
}

// ===== LOGIC BAN ĐÊM =====
void runNightModeLogic() {
  display.clear(); // Tắt màn hình
  
  // Nhấp nháy đèn Vàng mỗi 0.5s
  if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = millis();
    isLedOn = !isLedOn;

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, isLedOn ? HIGH : LOW);
  }
}

// ===== LOGIC BAN NGÀY =====
void runDayModeLogic() {
  // Timer 0.5 giây để xử lý nhấp nháy
  if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = millis();
    isLedOn = !isLedOn; // Đảo trạng thái đèn (Sáng -> Tắt -> Sáng...)

    // CHỈ ĐẾM NGƯỢC KHI BẮT ĐẦU 1 GIÂY MỚI (Khi đèn bật sáng lại)
    if (isLedOn) {
      countdown--;
      
      // Hết giờ -> Chuyển đèn
      if (countdown < 0) {
        trafficState++;
        if (trafficState > 2) trafficState = 0; // Quay lại Xanh

        // Cài đặt thời gian mới
        if (trafficState == 0) countdown = TIME_GREEN;       // 5s
        else if (trafficState == 1) countdown = TIME_YELLOW; // 2s
        else if (trafficState == 2) countdown = TIME_RED;    // 3s
      }
      
      // Cập nhật màn hình (chỉ khi số thay đổi)
      display.showNumberDec(countdown, true);
    }

    // Cập nhật trạng thái đèn (Nhấp nháy theo biến isLedOn)
    updateTrafficLeds(isLedOn);
  }
}

void loop() {
  // 1. XỬ LÝ NÚT BẤM (BẬT/TẮT)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    static int stableState = HIGH;
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) { // Nhấn xuống
        isSystemOn = !isSystemOn;
        if (isSystemOn) resetSystemState();
        else turnOffAll();
      }
    }
  }
  lastButtonState = reading;

  if (!isSystemOn) return;

  // 2. ĐỌC CẢM BIẾN & XỬ LÝ NGÀY/ĐÊM
  int lightVal = analogRead(LDR_PIN);
  
  // Debug: In giá trị ra Serial để kiểm tra
  // Serial.print("LDR Value: "); Serial.println(lightVal);

  // --- SỬA LỖI NGƯỢC SÁNG/TỐI TẠI ĐÂY ---
  // Nếu Lux Cao bị nhận là Đêm -> Lux Cao trả về giá trị thấp.
  // Vậy: Giá trị CAO (> 2000) mới là ĐÊM (Trời tối điện trở tăng -> áp tăng/giảm tùy mạch)
  // Logic sửa đổi: Nếu giá trị đọc được > 2000 là ĐÊM. Ngược lại là NGÀY.
  
  bool currentIsNight = (lightVal > NIGHT_THRESHOLD); 

  // Reset nhịp nháy nếu chế độ thay đổi đột ngột
  if (currentIsNight != isNightMode) {
    isNightMode = currentIsNight;
    lastBlinkTime = millis();
    isLedOn = false;
  }

  if (isNightMode) {
    runNightModeLogic();
  } else {
    runDayModeLogic();
  }
}