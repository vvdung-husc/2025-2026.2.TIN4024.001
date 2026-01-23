#include <Arduino.h>

// ================= PIN MAP =================
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// ================= TIME CONFIG (ms) =================
#define GREEN_TIME   7000
#define YELLOW_TIME  3000
#define RED_TIME     5000

#define BLINK_INTERVAL 500   // LED toggle mỗi 500ms



// ===== Function Prototypes =====
void allLedOff();
void handleBlink(unsigned long now);
void handleStateChange(unsigned long now);
void resetStateTimer(const char* stateName);

// ================= STATE =================
enum TrafficState {
  GREEN,
  YELLOW,
  RED
};

TrafficState currentState = GREEN;

// ================= TIMER =================
unsigned long stateStartTime = 0;
unsigned long lastBlinkTime  = 0;
bool ledBlinkState = false;

// ====================================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  allLedOff();
  stateStartTime = millis();
}

// ====================================================
void loop() {
  unsigned long now = millis();

  handleBlink(now);
  handleStateChange(now);
}

// ================= LED CONTROL =================

void allLedOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// ----------------------------------------------------

void handleBlink(unsigned long now) {
  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = now;
    ledBlinkState = !ledBlinkState;

    allLedOff();

    switch (currentState) {
      case GREEN:
        digitalWrite(LED_GREEN, ledBlinkState);
        break;

      case YELLOW:
        digitalWrite(LED_YELLOW, ledBlinkState);
        break;

      case RED:
        digitalWrite(LED_RED, ledBlinkState);
        break;
    }
  }
}

// ----------------------------------------------------

void handleStateChange(unsigned long now) {
  unsigned long elapsed = now - stateStartTime;

  switch (currentState) {

    case GREEN:
      if (elapsed >= GREEN_TIME) {
        currentState = YELLOW;
        resetStateTimer("YELLOW");
      }
      break;

    case YELLOW:
      if (elapsed >= YELLOW_TIME) {
        currentState = RED;
        resetStateTimer("RED");
      }
      break;

    case RED:
      if (elapsed >= RED_TIME) {
        currentState = GREEN;
        resetStateTimer("GREEN");
      }
      break;
  }
}

// ----------------------------------------------------

void resetStateTimer(const char* stateName) {
  stateStartTime = millis();
  lastBlinkTime  = millis();
  ledBlinkState  = false;
  allLedOff();

  Serial.print("Change State -> ");
  Serial.println(stateName);
}
