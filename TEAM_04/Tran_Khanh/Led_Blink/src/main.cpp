#include <Arduino.h>

// GPIO theo diagram.json
#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

enum TrafficState {
  YELLOW,
  RED,
  GREEN
};

TrafficState currentState = YELLOW;

unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastPrintTime = 0;

bool ledOn = true;

// Thời gian từng đèn (ms)
const unsigned long TIME_YELLOW = 3000;
const unsigned long TIME_RED    = 5000;
const unsigned long TIME_GREEN  = 7000;

// Nháy mỗi 500ms
const unsigned long BLINK_INTERVAL = 500;

void turnOffAll() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setCurrentLed(bool state) {
  turnOffAll();
  if (currentState == RED)    digitalWrite(LED_RED, state);
  if (currentState == YELLOW) digitalWrite(LED_YELLOW, state);
  if (currentState == GREEN)  digitalWrite(LED_GREEN, state);
}

unsigned long getStateDuration() {
  if (currentState == YELLOW) return TIME_YELLOW;
  if (currentState == RED)    return TIME_RED;
  return TIME_GREEN;
}

const char* getStateName() {
  if (currentState == YELLOW) return "YELLOW";
  if (currentState == RED)    return "RED";
  return "GREEN";
}

void nextState() {
  if (currentState == YELLOW) currentState = RED;
  else if (currentState == RED) currentState = GREEN;
  else currentState = YELLOW;

  stateStartTime = millis();
  lastBlinkTime = millis();
  lastPrintTime = millis();
  ledOn = true;
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);

  currentState = YELLOW;
  stateStartTime = millis();
  lastBlinkTime = millis();
  lastPrintTime = millis();

  setCurrentLed(true);
}

void loop() {
  unsigned long now = millis();
  unsigned long duration = getStateDuration();

  // LED nháy
  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    ledOn = !ledOn;
    setCurrentLed(ledOn);
    lastBlinkTime = now;
  }

  // In số giây còn lại mỗi 1s
  if (now - lastPrintTime >= 1000) {
    unsigned long elapsed = now - stateStartTime;
    int remain = (duration - elapsed + 999) / 1000;
    if (remain < 0) remain = 0;

    Serial.print(getStateName());
    Serial.print(": ");
    Serial.print(remain);
    Serial.println(" s");

    lastPrintTime = now;
  }

  // Chuyển trạng thái
  if (now - stateStartTime >= duration) {
    nextState();
  }
}
