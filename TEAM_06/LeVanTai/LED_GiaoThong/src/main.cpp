#include <Arduino.h>
#include <TM1637Display.h>

// ===== LED =====
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// ===== BUTTON =====
#define BTN_PIN 23

// ===== LDR =====
#define LDR_PIN 13
#define DARK_THRESHOLD 2000

// ===== TM1637 =====
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

bool isRunning = false;
bool lastButtonState = HIGH;

// ===== LDR CHECK =====
bool isDark() {
  return analogRead(LDR_PIN) < DARK_THRESHOLD;
}

// ===== BUTTON CHECK =====
void checkButton() {
  bool current = digitalRead(BTN_PIN);
  if (lastButtonState == HIGH && current == LOW) {
    isRunning = !isRunning;

    if (!isRunning) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      display.clear();
    }
    delay(200);
  }
  lastButtonState = current;
}

// ===== BLINK WITH TIME =====
void blinkLed(int pin, int seconds) {
  for (int i = seconds; i > 0 && isRunning && !isDark(); i--) {
    display.showNumberDec(i, true);

    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);

    checkButton();
  }
}

// ===== NIGHT MODE =====
void blinkYellowForever() {
  display.clear();

  while (isDark()) {
    checkButton();
    if (!isRunning) return;

    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();
}

void loop() {
  checkButton();

  // 🌙 BAN ĐÊM → VÀNG NHẤP NHÁY LIÊN TỤC
  if (isDark()) {
    isRunning = true;       // ép chạy chế độ đêm
    blinkYellowForever();
    return;
  }

  // 🌞 BAN NGÀY → CHỜ START
  if (!isRunning) return;

  blinkLed(LED_RED, 5);
  blinkLed(LED_YELLOW, 3);
  blinkLed(LED_GREEN, 7);
}
