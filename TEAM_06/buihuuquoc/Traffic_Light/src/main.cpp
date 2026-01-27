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
int countdown = 5;
int state = 1; // 1: Xanh | 2: Vàng | 0: Đỏ
bool isSystemOn = false;
bool ledToggle = true;

const int NIGHT_THRESHOLD = 1000;

// ===== HÀM ĐIỀU KHIỂN ĐÈN =====
void setTrafficLights(bool r, bool y, bool g)
{
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

// ===== TẮT TOÀN BỘ =====
void turnOffAll()
{
  setTrafficLights(LOW, LOW, LOW);
  digitalWrite(LED_BLUE, LOW); // LED xanh dương TẮT (đấu thuận)
  display.clear();
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

void loop()
{
  // ===== 1. NÚT BẬT / TẮT =====
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(50); // chống dội
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      isSystemOn = !isSystemOn;

      if (!isSystemOn)
      {
        turnOffAll();
      }
      else
      {
        digitalWrite(LED_BLUE, HIGH); // LED xanh dương SÁNG
        prevMillis = millis();
        state = 1;
        countdown = 5;
        ledToggle = true;
      }

      while (digitalRead(BUTTON_PIN) == LOW)
        ; // chờ thả nút
    }
  }

  if (!isSystemOn)
    return;

  // ===== 2. ĐỌC CẢM BIẾN ÁNH SÁNG =====
  int lightValue = analogRead(LDR_PIN);

  // ===== CHẾ ĐỘ BAN ĐÊM =====
  if (lightValue < NIGHT_THRESHOLD)
  {
    if (millis() - prevMillis >= 500)
    {
      prevMillis = millis();
      ledToggle = !ledToggle;
      setTrafficLights(LOW, ledToggle, LOW); // Đèn vàng nhấp nháy
      display.clear();
    }
    return;
  }

  // ===== CHẾ ĐỘ BAN NGÀY =====
  if (millis() - prevMillis >= 1000)
  {
    prevMillis = millis();
    display.showNumberDec(countdown, true);

    if (state == 1)
      setTrafficLights(LOW, LOW, HIGH); // Xanh
    else if (state == 2)
      setTrafficLights(LOW, HIGH, LOW); // Vàng
    else
      setTrafficLights(HIGH, LOW, LOW); // Đỏ

    countdown--;

    if (countdown < 0)
    {
      if (state == 1)
      {
        state = 2;
        countdown = 2;
      }
      else if (state == 2)
      {
        state = 0;
        countdown = 5;
      }
      else
      {
        state = 1;
        countdown = 5;
      }
    }
  }
}
