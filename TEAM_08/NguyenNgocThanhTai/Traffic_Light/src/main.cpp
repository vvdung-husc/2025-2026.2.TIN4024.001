#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN (PINS) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_STREET  21  // Đèn đường (Blue)

#define CLK 18
#define DIO 19

#define BTN_PIN 4
#define LDR_PIN 13      // Chân Digital Output của Module LDR

// --- CẤU HÌNH THỜI GIAN (Mili giây) ---
const long RED_TIME = 5000;    // Đèn đỏ 5 giây
const long GREEN_TIME = 5000;  // Đèn xanh 5 giây
const long YELLOW_TIME = 2000; // Đèn vàng 2 giây

// --- KHỞI TẠO ĐỐI TƯỢNG ---
TM1637Display display(CLK, DIO);

// --- BIẾN TOÀN CỤC ---
int trafficState = 0; // 0: RED, 1: GREEN, 2: YELLOW
unsigned long previousMillis = 0;
long currentTimer = RED_TIME; // Bộ đếm thời gian hiện tại
bool isDisplayVisible = true; // Trạng thái hiển thị màn hình

void setup() {
  Serial.begin(115200);

  // Cài đặt chế độ chân
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); // Nút nhấn có trở kéo lên nội bộ
  pinMode(LDR_PIN, INPUT);

  // Cài đặt độ sáng màn hình (0-7)
  display.setBrightness(0x0f);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. XỬ LÝ ĐÈN GIAO THÔNG (State Machine) ---
  long duration = 0;
  if (trafficState == 0) duration = RED_TIME;
  else if (trafficState == 1) duration = GREEN_TIME;
  else duration = YELLOW_TIME;

  // Tính thời gian còn lại (Countdown)
  long remainingTime = (duration - (currentMillis - previousMillis)) / 1000;

  // Chuyển trạng thái đèn khi hết giờ
  if (currentMillis - previousMillis >= duration) {
    previousMillis = currentMillis; // Reset mốc thời gian
    trafficState++;
    if (trafficState > 2) trafficState = 0; // Quay vòng về Đỏ
    
    // Cập nhật trạng thái đèn ngay lập tức
    digitalWrite(LED_RED, trafficState == 0 ? HIGH : LOW);
    digitalWrite(LED_GREEN, trafficState == 1 ? HIGH : LOW);
    digitalWrite(LED_YELLOW, trafficState == 2 ? HIGH : LOW);
  } else {
    // Giữ trạng thái đèn hiện tại (tránh nhấp nháy)
    if(digitalRead(LED_RED) != (trafficState == 0)) digitalWrite(LED_RED, trafficState == 0);
    if(digitalRead(LED_GREEN) != (trafficState == 1)) digitalWrite(LED_GREEN, trafficState == 1);
    if(digitalRead(LED_YELLOW) != (trafficState == 2)) digitalWrite(LED_YELLOW, trafficState == 2);
  }

  // --- 2. XỬ LÝ HIỂN THỊ (TM1637) ---
  if (isDisplayVisible) {
    display.showNumberDec(remainingTime + 1); // +1 để không hiện số 0 quá lâu
  } else {
    display.clear();
  }

  // --- 3. XỬ LÝ NÚT NHẤN (Toggle Display) ---
  // Phát hiện nhấn nút (Logic đơn giản, có thể thêm debounce nếu cần)
  static int lastBtnState = HIGH;
  int btnState = digitalRead(BTN_PIN);
  if (lastBtnState == HIGH && btnState == LOW) {
    isDisplayVisible = !isDisplayVisible; // Đảo trạng thái hiển thị
    delay(200); // Chống dội phím đơn giản
  }
  lastBtnState = btnState;

  // --- 4. XỬ LÝ CẢM BIẾN ÁNH SÁNG (LDR) ---
  // Module LDR trả về LOW khi trời sáng, HIGH khi trời tối (tùy chỉnh biến trở trên module)
  // Logic: Trời tối -> Bật đèn đường
  int ldrValue = digitalRead(LDR_PIN);
  if (ldrValue == HIGH) { 
    digitalWrite(LED_STREET, HIGH); // Bật đèn đường
  } else {
    digitalWrite(LED_STREET, LOW);  // Tắt đèn đường
  }
}