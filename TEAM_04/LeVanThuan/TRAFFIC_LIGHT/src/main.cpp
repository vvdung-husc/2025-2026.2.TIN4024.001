#include <Arduino.h>
#include <TM1637Display.h>

// ===== PIN =====
#define LED_RED     14
#define LED_YELLOW  27
#define LED_GREEN   26
#define LED_BLUE    21

#define BUTTON_PIN  23
#define LDR_PIN     13

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== KHAI BÁO HÀM =====
void checkButton();
void runTrafficCycle();
void dayMode();
void nightMode();
void redPhase(int speed);
void yellowPhase(int speed);
void greenPhase(int speed);
void smartDelay(int ms);

// ===== BIẾN =====
bool displayEnabled = true;

int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

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

  Serial.println("🚦 Traffic Light Ready!");
  Serial.println("🔵 Blue LED ON = Countdown Enabled");
}

// ===== LOOP =====
void loop() {
  checkButton();
  runTrafficCycle();
}

// ===== NÚT =====
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        displayEnabled = !displayEnabled;

        Serial.print("🔵 Countdown: ");
        Serial.println(displayEnabled ? "ON" : "OFF");

        digitalWrite(LED_BLUE, displayEnabled ? HIGH : LOW);
      }
    }
  }

  lastButtonState = reading;
}

// ===== MAIN =====
void runTrafficCycle() {
  int ldrValue = analogRead(LDR_PIN);

  Serial.print("🌞 LDR Value = ");
  Serial.println(ldrValue);

  if (ldrValue < 1500) {
    nightMode();
  } else {
    dayMode();
  }
}

// ===== DAY MODE =====
void dayMode() {
  Serial.println("🌞 DAY MODE");

  redPhase(500);
  yellowPhase(500);
  greenPhase(500);
}

// ===== NIGHT MODE =====
void nightMode() {
  Serial.println("🌙 NIGHT MODE");

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  while (true) {
    checkButton();

    int ldrValue = analogRead(LDR_PIN);
    if (ldrValue >= 1500) return;

    Serial.println("🟡 Yellow Blink");

    if (displayEnabled)
      display.showNumberDec(0, false);
    else
      display.clear();

    digitalWrite(LED_YELLOW, HIGH);
    smartDelay(500);

    digitalWrite(LED_YELLOW, LOW);
    smartDelay(500);
  }
}

// ===== RED =====
void redPhase(int speed) {
  for (int i = 5; i >= 1; i--) {
    checkButton();

    Serial.print("🔴 RED: ");
    Serial.println(i);

    if (displayEnabled) display.showNumberDec(i, false);
    else display.clear();

    digitalWrite(LED_RED, HIGH);
    smartDelay(speed);
    digitalWrite(LED_RED, LOW);
    smartDelay(speed);
  }
}

// ===== YELLOW =====
void yellowPhase(int speed) {
  for (int i = 3; i >= 1; i--) {
    checkButton();

    Serial.print("🟡 YELLOW: ");
    Serial.println(i);

    if (displayEnabled) display.showNumberDec(i, false);
    else display.clear();

    digitalWrite(LED_YELLOW, HIGH);
    smartDelay(speed);
    digitalWrite(LED_YELLOW, LOW);
    smartDelay(speed);
  }
}

// ===== GREEN =====
void greenPhase(int speed) {
  digitalWrite(LED_GREEN, HIGH);

  for (int i = 7; i >= 1; i--) {
    checkButton();

    Serial.print("🟢 GREEN: ");
    Serial.println(i);

    if (displayEnabled) display.showNumberDec(i, false);
    else display.clear();

    smartDelay(speed);

    digitalWrite(LED_GREEN, LOW);
    smartDelay(speed);
    digitalWrite(LED_GREEN, HIGH);
  }

  digitalWrite(LED_GREEN, LOW);
}

// ===== SMART DELAY =====
void smartDelay(int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkButton();
  }
}
