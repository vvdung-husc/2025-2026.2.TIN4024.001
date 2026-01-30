#include <Arduino.h>
#include <TM1637Display.h>

// ===== CHÂN THEO MẠCH =====
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21
#define BUTTON_PIN  23

#define CLK_PIN 18
#define DIO_PIN 19

// ===== THỜI GIAN (giây) =====
#define TIME_RED     5
#define TIME_GREEN  7
#define TIME_YELLOW 3

#define DEBOUNCE_TIME 50

TM1637Display display(CLK_PIN, DIO_PIN);

// ===== BIẾN =====
bool showCountdown = true;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ===== KHAI BÁO HÀM =====
void checkButton();
void redLight();
void greenLight();
void yellowLight();

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_BLUE, HIGH);

  Serial.println("🚦 TRAFFIC LIGHT - NORMAL MODE");
}

// ===== LOOP =====
void loop() {
  checkButton();
  redLight();
  greenLight();
  yellowLight();
}

// ===== NÚT NHẤN =====
void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading == LOW && lastButtonState == HIGH) {
      showCountdown = !showCountdown;
      digitalWrite(LED_BLUE, showCountdown);
    }
  }

  lastButtonState = reading;
}

// ===== ĐÈN ĐỎ =====
void redLight() {
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);

  for (int i = TIME_RED; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(LED_RED, LOW);
}

// ===== ĐÈN XANH =====
void greenLight() {
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);

  for (int i = TIME_GREEN; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(LED_GREEN, LOW);
}

// ===== ĐÈN VÀNG =====
void yellowLight() {
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  for (int i = TIME_YELLOW; i > 0; i--) {
    checkButton();
    if (showCountdown) display.showNumberDec(i, false);
    else display.clear();
    delay(1000);
  }

  digitalWrite(LED_YELLOW, LOW);
}
