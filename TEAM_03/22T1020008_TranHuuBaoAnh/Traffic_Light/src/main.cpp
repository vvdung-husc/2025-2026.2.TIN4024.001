#include <Arduino.h>
#include <TM1637Display.h>

// ===== PIN =====
#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27
#define BTN_PIN     23
#define LED_BLUE 21


#define CLK 18
#define DIO 19

// ===== TIME (seconds) =====
#define TIME_RED     5
#define TIME_YELLOW  3
#define TIME_GREEN   7

// ===== DISPLAY =====
TM1637Display display(CLK, DIO);

// ===== STATE =====
enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN
};

TrafficState currentState = STATE_RED;

// ===== TIMER =====
unsigned long lastMillis = 0;
int countdown = TIME_RED;

// ===== BUTTON =====
bool showCountdown = true;
bool lastButtonState = HIGH;

// ===== FUNCTIONS =====
void setLights(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

void changeState(TrafficState newState, int timeSec) {
  currentState = newState;
  countdown = timeSec;

  switch (newState) {
    case STATE_RED:
      setLights(HIGH, LOW, LOW);
      Serial.println("[RED] - 5s");
      break;

    case STATE_YELLOW:
      setLights(LOW, HIGH, LOW);
      Serial.println("[YELLOW] - 3s");
      break;

    case STATE_GREEN:
      setLights(LOW, LOW, HIGH);
      Serial.println("[GREEN] - 7s");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  display.setBrightness(7);
  display.clear();

  changeState(STATE_RED, TIME_RED);
}

void loop() {
  unsigned long now = millis();

  // ===== BUTTON TOGGLE DISPLAY =====
  bool btnState = digitalRead(BTN_PIN);
  if (lastButtonState == HIGH && btnState == LOW) {
  showCountdown = !showCountdown;

  digitalWrite(LED_BLUE, showCountdown ? HIGH : LOW);

  Serial.println(showCountdown ? 
    "DISPLAY ON - BLUE LED ON" : 
    "DISPLAY OFF - BLUE LED OFF"
  );

  delay(200); // debounce
  }

  lastButtonState = btnState;

  // ===== COUNTDOWN EVERY 1s =====
  if (now - lastMillis >= 1000) {
    lastMillis = now;
    countdown--;

    if (showCountdown) {
      display.showNumberDec(countdown, true);
    } else {
      display.clear();
    }

    // ===== CHANGE STATE =====
    if (countdown <= 0) {
      if (currentState == STATE_RED)
        changeState(STATE_GREEN, TIME_GREEN);
      else if (currentState == STATE_GREEN)
        changeState(STATE_YELLOW, TIME_YELLOW);
      else
        changeState(STATE_RED, TIME_RED);
    }
  }
}