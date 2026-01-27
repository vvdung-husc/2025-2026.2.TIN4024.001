#include <Arduino.h>
#include <TM1637Display.h>

// LED pins
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// Button
#define BTN_PIN 23

// LDR
#define LDR_PIN 13
#define DARK_THRESHOLD 2000

// TM1637
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

bool isRunning = false;
bool lastButtonState = HIGH;

// ===== HÀM LDR =====
bool isDark() {
  return analogRead(LDR_PIN) < DARK_THRESHOLD;
}

// ===== LED =====
void turnOffAllLed() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// ===== BUTTON =====
void checkButton() {
  bool current = digitalRead(BTN_PIN);
  if (lastButtonState == HIGH && current == LOW) {
    isRunning = !isRunning;
    if (!isRunning) {
      turnOffAllLed();
      display.clear();
    }
    delay(200);
  }
  lastButtonState = current;
}

// ===== BLINK =====
void blinkLedWithCountdown(int pin, int seconds) {
  for (int i = seconds; i > 0; i--) {
    if (!isRunning) return;

    display.showNumberDec(i, true);

    unsigned long t = millis();
    while (millis() - t < 500) {
      checkButton();
      if (isDark() && pin != LED_YELLOW) return;
    }

    digitalWrite(pin, HIGH);

    t = millis();
    while (millis() - t < 500) {
      checkButton();
      if (isDark() && pin != LED_YELLOW) return;
    }

    digitalWrite(pin, LOW);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();
  turnOffAllLed();
}

void loop() {
  checkButton();

  // 🌙 BAN ĐÊM → CHỈ NHÁY VÀNG
  if (isDark()) {
    isRunning = true;
    blinkLedWithCountdown(LED_YELLOW, 1);
    return;
  }

  // 🌞 BAN NGÀY → CHỜ START
  if (!isRunning) return;

  blinkLedWithCountdown(LED_RED, 5);
  blinkLedWithCountdown(LED_YELLOW, 3);
  blinkLedWithCountdown(LED_GREEN, 7);
}
