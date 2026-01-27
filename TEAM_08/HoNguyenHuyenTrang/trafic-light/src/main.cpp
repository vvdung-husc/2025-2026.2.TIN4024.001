#include <Arduino.h>
#include <TM1637Display.h>

/* ================== PIN DEFINE ================== */
// LED giao thông
#define PIN_LED_RED     27
#define PIN_LED_YELLOW  26
#define PIN_LED_GREEN   25
#define PIN_LED_BLUE    21

// Push button
#define PIN_BUTTON_DISPLAY 23

// TM1637
#define CLK 18
#define DIO 19

/* ================== OBJECT ================== */
TM1637Display display(CLK, DIO);

/* ================== GLOBAL ================== */
bool displayEnable = false;     // 🔥 TOGGLE ON / OFF
bool lastButtonState = HIGH;

/* ================== TIMER HELPER ================== */
bool IsReady(unsigned long &timer, uint32_t interval)
{
  if (millis() - timer < interval) return false;
  timer = millis();
  return true;
}

/* ================== INIT ================== */
void Init_LED_Traffic()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_BLUE, LOW);
}

/* ================== BUTTON (CLICK / TOGGLE) ================== */
void ProcessButtonPressed()
{
  static unsigned long timer = 0;
  if (!IsReady(timer, 20)) return;

  bool currentState = digitalRead(PIN_BUTTON_DISPLAY);

  // 🔥 phát hiện nhấn (HIGH → LOW)
  if (lastButtonState == HIGH && currentState == LOW) {
    displayEnable = !displayEnable;   // TOGGLE

    if (displayEnable) {
      digitalWrite(PIN_LED_BLUE, HIGH);
    } else {
      digitalWrite(PIN_LED_BLUE, LOW);
      display.clear();
    }
  }

  lastButtonState = currentState;
}

/* ================== LED TRAFFIC + COUNTDOWN ================== */
void ProcessLEDTraffic()
{
  static unsigned long timerBlink = 0;
  static unsigned long timerSecond = 0;

  static uint8_t stage = 0; // 0=RED,1=YELLOW,2=GREEN
  static int secondLeft = 5;
  static bool ledState = false;

  const uint8_t leds[3] = {PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN};
  const int durations[3] = {5, 3, 5};

  // Blink LED mỗi 500ms
  if (IsReady(timerBlink, 500)) {
    ledState = !ledState;
    digitalWrite(leds[stage], ledState);
  }

  // Đếm giây
  if (IsReady(timerSecond, 1000)) {
    secondLeft--;

    if (displayEnable) {
      display.showNumberDec(secondLeft, true);
    }

    if (secondLeft <= 0) {
      digitalWrite(leds[stage], LOW);
      stage = (stage + 1) % 3;
      secondLeft = durations[stage];
    }
  }
}

/* ================== SETUP ================== */
void setup()
{
  Serial.begin(115200);
  Init_LED_Traffic();

  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLUP);

  display.setBrightness(0x0a);
  display.clear();
}

/* ================== LOOP ================== */
void loop()
{
  ProcessButtonPressed();
  ProcessLEDTraffic();
}