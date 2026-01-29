#include <Arduino.h>
#include <TM1637Display.h>

// ===== CẤU HÌNH CHÂN =====
#define PIN_LED_RED     14
#define PIN_LED_YELLOW  27
#define PIN_LED_GREEN   26
#define PIN_LED_BLUE    21
#define PIN_BUTTON      23
#define PIN_LDR         13
#define PIN_CLK         18
#define PIN_DIO         19

// ===== THỜI GIAN (giây) =====
#define TIME_RED     5
#define TIME_GREEN  7
#define TIME_YELLOW 3

// ESP32 ADC: 0–4095
#define LDR_THRESHOLD 2000
#define DEBOUNCE_TIME 50

TM1637Display display(PIN_CLK, PIN_DIO);

// ===== BIẾN TRẠNG THÁI =====
bool showCountdown = true;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ===== KHAI BÁO HÀM =====
void checkButton();
void runRedLight();
void runGreenLight();
void runYellowLight();
void runNightMode();
int  readLDR();

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(PIN_LED_BLUE, HIGH);

  Serial.println("\n===== ĐÈN GIAO THÔNG THÔNG MINH =====");
}

// ===== LOOP =====
void loop() {
  checkButton();

  int lightLevel = readLDR();

  if (lightLevel >= LDR_THRESHOLD) {
    runRedLight();
    runGreenLight();
    runYellowLight();
  } else {
    runNightMode();
  }
}

// ===== ĐỌC LDR CHỐNG NHIỄU =====
int readLDR() {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(PIN_LDR);
    delay(5);
  }
  return sum / 5;
}

// ===== XỬ LÝ NÚT NHẤN =====
void checkButton() {
  bool reading = digitalRead(PIN_BUTTON);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading == LOW && lastButtonState == HIGH) {
      showCountdown = !showCountdown;
      digitalWrite(PIN_LED_BLUE, showCountdown);
      Serial.println(showCountdown ? "🔵 Đếm ngược: BẬT" : "🔵 Đếm ngược: TẮT");
    }
  }

  lastButtonState = reading;
}

// ===== ĐÈN ĐỎ =====
void runRedLight() {
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);

  for (int i = TIME_RED; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(PIN_LED_RED, LOW);
}

// ===== ĐÈN XANH =====
void runGreenLight() {
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);

  for (int i = TIME_GREEN; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(PIN_LED_GREEN, LOW);
}

// ===== ĐÈN VÀNG =====
void runYellowLight() {
  digitalWrite(PIN_LED_YELLOW, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  for (int i = TIME_YELLOW; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(PIN_LED_YELLOW, LOW);
}

// ===== CHẾ ĐỘ ĐÊM =====
void runNightMode() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  while (readLDR() < LDR_THRESHOLD) {
    checkButton();

    if (showCountdown) display.showNumberDec(0, false);
    else display.clear();

    digitalWrite(PIN_LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(PIN_LED_YELLOW, LOW);
    delay(500);
  }

  display.clear();
}
