#include <Arduino.h>
#include <TM1637Display.h>

// ===== CẤU HÌNH CHÂN =====
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21
#define BUTTON_PIN 23
#define LDR_PIN 13
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== BIẾN HỆ THỐNG =====
unsigned long prevMillis = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

int buttonState;
int lastButtonState = HIGH;

int countdown = 5;
int state = 1; // 1: Xanh | 2: Vàng | 0: Đỏ
bool isSystemOn = false;
bool ledToggle = true;

const int NIGHT_THRESHOLD = 1000;

// Hàm cập nhật đèn LED đơn giản
void setTrafficLights(bool r, bool y, bool g)
{
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

// Hàm tắt toàn bộ hệ thống
void turnOffAll()
{
  setTrafficLights(LOW, LOW, LOW);
  digitalWrite(LED_BLUE, LOW);
  display.clear();
}

// Hàm khởi tạo trạng thái ban đầu khi vừa bật hệ thống
void resetSystemState()
{
  state = 1;     // Bắt đầu từ đèn Xanh
  countdown = 5; // Đếm từ 5
  ledToggle = true;
  prevMillis = millis(); // Đặt mốc thời gian

  // QUAN TRỌNG: Hiển thị ngay lập tức, không chờ hết 1s đầu tiên
  digitalWrite(LED_BLUE, HIGH);
  setTrafficLights(LOW, LOW, HIGH);
  display.showNumberDec(countdown, true);
}

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
  turnOffAll();
}

// ===== XỬ LÝ CHẾ ĐỘ BAN ĐÊM =====
void runNightMode()
{
  if (millis() - prevMillis >= 500)
  {
    prevMillis = millis();
    ledToggle = !ledToggle;
    setTrafficLights(LOW, ledToggle, LOW);

    // Nhấp nháy dấu hai chấm hoặc tắt màn hình cho đỡ chói
    if (ledToggle)
      display.showNumberDec(0, false, 0, 0); // Ví dụ hiển thị gì đó nhẹ
    else
      display.clear();
  }
}

// ===== XỬ LÝ CHẾ ĐỘ BAN NGÀY =====
void runDayMode()
{
  // Logic đếm ngược
  if (millis() - prevMillis >= 1000)
  {
    prevMillis = millis();
    countdown--; // Giảm trước để chuẩn bị hiển thị số tiếp theo

    // Chuyển trạng thái nều hết giờ
    if (countdown < 0)
    {
      if (state == 1)
      {
        state = 2;
        countdown = 2;
      } // Xanh -> Vàng (3s)
      else if (state == 2)
      {
        state = 0;
        countdown = 5;
      } // Vàng -> Đỏ (5s)
      else
      {
        state = 1;
        countdown = 5;
      } // Đỏ -> Xanh (5s)
    }

    // Cập nhật hiển thị
    display.showNumberDec(countdown, true);

    // Cập nhật đèn
    if (state == 1)
      setTrafficLights(LOW, LOW, HIGH); // Xanh
    else if (state == 2)
      setTrafficLights(LOW, HIGH, LOW); // Vàng
    else
      setTrafficLights(HIGH, LOW, LOW); // Đỏ
  }
}

void loop()
{
  // ===== 1. ĐỌC NÚT NHẤN (NON-BLOCKING) =====
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;

      // Chỉ xử lý khi nhấn xuống (LOW)
      if (buttonState == LOW)
      {
        isSystemOn = !isSystemOn;

        if (isSystemOn)
        {
          resetSystemState(); // Bật lên -> Hiển thị ngay
        }
        else
        {
          turnOffAll(); // Tắt đi
        }
      }
    }
  }
  lastButtonState = reading;

  // Nếu tắt, thoát loop sớm để tiết kiệm tài nguyên
  if (!isSystemOn)
    return;

  // ===== 2. ĐIỀU KHIỂN TRAFFIC LIGHT =====
  // Đọc LDR (Nên đọc liên tục để chuyển chế độ mượt mà)
  if (analogRead(LDR_PIN) < NIGHT_THRESHOLD)
  {
    runNightMode();
  }
  else
  {
    // Nếu đang ở Night mode chuyển sang Day mode, cần setup lại đèn ngay
    // (Phần này có thể mở rộng logic nếu muốn chuyển đổi mượt hơn)
    runDayMode();
  }
}