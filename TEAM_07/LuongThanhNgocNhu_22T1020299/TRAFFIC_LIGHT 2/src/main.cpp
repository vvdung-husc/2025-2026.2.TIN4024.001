#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (PINOUT) ---
#define PIN_TL_RED    27  
#define PIN_TL_YELLOW 26  
#define PIN_TL_GREEN  25  

#define PIN_BTN       23  // Nút nhấn
#define PIN_LED_BLUE  21  // Đèn báo trạng thái (đồng bộ với màn hình)
#define PIN_LDR       34  // Cảm biến quang

#define CLK           18  
#define DIO           19  

// --- CẤU HÌNH NGƯỠNG & THỜI GIAN ---
// Ngưỡng ánh sáng: Dưới 1000 là ĐÊM, Trên 1000 là NGÀY
#define LDR_THRESHOLD 1000 

// Thời gian đếm cho mỗi đèn (8 giây theo yêu cầu)
#define TIME_DELAY    8   

TM1637Display display(CLK, DIO);

// Biến trạng thái
bool isDisplayVisible = true; // Trạng thái hiển thị (Mặc định BẬT)
bool isNightMode = false;     // Trạng thái ngày/đêm
int lastBtnState = HIGH;      

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_TL_RED, OUTPUT);
  pinMode(PIN_TL_YELLOW, OUTPUT);
  pinMode(PIN_TL_GREEN, OUTPUT);
  
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);

  display.setBrightness(0x0f);
  
  // Cập nhật đèn Xanh Dương ngay khi khởi động
  digitalWrite(PIN_LED_BLUE, isDisplayVisible ? HIGH : LOW);
  
  Serial.println("--- SYSTEM STARTED: 8s Blinking Mode ---");
}

// Hàm tắt tất cả đèn giao thông
void clearTrafficLights() {
  digitalWrite(PIN_TL_RED, LOW);
  digitalWrite(PIN_TL_YELLOW, LOW);
  digitalWrite(PIN_TL_GREEN, LOW);
}

// Hàm hiển thị số (hoặc xóa nếu đang tắt)
void showTime(int seconds) {
  if (isDisplayVisible) {
    display.showNumberDec(seconds, true);
  } else {
    display.clear();
  }
}

// Hàm hiển thị gạch ngang (cho chế độ đêm)
void showDashes() {
  if (isDisplayVisible) {
    const uint8_t DASHES[] = {0x40, 0x40, 0x40, 0x40}; 
    display.setSegments(DASHES);
  } else {
    display.clear();
  }
}

// --- HÀM KIỂM TRA INPUT (Nút nhấn & LDR) ---
// Trả về TRUE nếu cần thoát chế độ hiện tại ngay lập tức (do đổi ngày/đêm)
bool checkInputs() {
  // 1. XỬ LÝ NÚT NHẤN (Đồng bộ Màn hình & Đèn Blue)
  int currentBtnState = digitalRead(PIN_BTN);
  
  // Phát hiện cạnh xuống (vừa nhấn nút)
  if (lastBtnState == HIGH && currentBtnState == LOW) {
    isDisplayVisible = !isDisplayVisible; // Đảo trạng thái
    
    // Cập nhật Đèn Xanh Dương & Màn hình NGAY LẬP TỨC
    if (isDisplayVisible) {
      digitalWrite(PIN_LED_BLUE, HIGH); // Màn hình ON -> Đèn Blue ON
      Serial.println("Display: ON -> Blue LED: ON");
      if (isNightMode) showDashes(); // Nếu đang đêm thì vẽ lại gạch ngang
    } else {
      digitalWrite(PIN_LED_BLUE, LOW);  // Màn hình OFF -> Đèn Blue OFF
      display.clear();
      Serial.println("Display: OFF -> Blue LED: OFF");
    }
    delay(50); // Chống rung nút
  }
  lastBtnState = currentBtnState;

  // 2. XỬ LÝ CẢM BIẾN QUANG (LDR)
  int ldrValue = analogRead(PIN_LDR);
  bool isDark = (ldrValue < LDR_THRESHOLD); // Kiểm tra xem có tối không

  if (isDark != isNightMode) { // Nếu trạng thái thay đổi
    isNightMode = isDark;
    Serial.print("Che do thay doi. LDR: ");
    Serial.println(ldrValue);
    return true; // Báo hiệu để thoát vòng lặp chờ
  }

  return false;
}

// --- HÀM CHỜ THÔNG MINH ---
// Thay thế delay(). Vừa đếm thời gian, vừa nháy đèn, vừa check nút
// pin: Đèn cần nháy
// durationMs: Thời gian chờ (thường là 1000ms cho 1 giây đếm)
bool blinkWait(int pin, int durationMs) {
  // Chia 1 giây thành 20 phần nhỏ (50ms mỗi lần check)
  for (int ms = 0; ms < durationMs; ms += 50) {
    
    // Logic nhấp nháy: 50% đầu sáng, 50% sau tắt
    if (ms < (durationMs / 2)) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }

    if (checkInputs()) return false; // Nếu đổi chế độ Ngày/Đêm -> Thoát ngay
    delay(50);
  }
  return true;
}

// --- CHẾ ĐỘ BAN ĐÊM ---
void runNightMode() {
  clearTrafficLights();
  showDashes(); // Hiện ----

  while (isNightMode) {
    if (isDisplayVisible) showDashes(); // Đảm bảo màn hình luôn cập nhật
    
    // Nháy đèn vàng (Dùng blinkWait để vừa nháy vừa check nút)
    // Gọi blinkWait cho chân Vàng, chu kỳ 1000ms
    if (!blinkWait(PIN_TL_YELLOW, 1000)) return; 
  }
}

// --- CHẾ ĐỘ BAN NGÀY ---
void runActiveMode(int pin, int seconds) {
  if (isNightMode) return;
  clearTrafficLights(); // Tắt các đèn khác

  for (int i = seconds; i > 0; i--) {
    showTime(i);
    // Nháy đèn pin hiện tại trong 1 giây
    if (!blinkWait(pin, 1000)) return; // Nếu LDR đổi -> thoát ngay
  }
  digitalWrite(pin, LOW); // Đảm bảo tắt hẳn khi hết giờ
}

void loop() {
  checkInputs(); // Check ngay đầu vòng lặp

  if (isNightMode) {
    runNightMode();
  } else {
    // Chạy tuần tự 3 đèn, mỗi đèn 8 giây
    runActiveMode(PIN_TL_RED, TIME_DELAY);
    runActiveMode(PIN_TL_GREEN, TIME_DELAY);
    runActiveMode(PIN_TL_YELLOW, TIME_DELAY);
  }
}