#include <Arduino.h>
#include <TM1637Display.h>

// ===== PIN =====
#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27
#define LED_BLUE    21
#define BTN_PIN     23

#define CLK 18
#define DIO 19

// ===== TIME (seconds) =====
#define TIME_RED     5
#define TIME_YELLOW  3
#define TIME_GREEN   7

TM1637Display display(CLK, DIO);

// ===== STATE =====
enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN
};

TrafficState currentState = STATE_RED;

// ===== TIMER =====
unsigned long timerSecond = 0;
unsigned long timerBlink  = 0;

int countdown = TIME_RED;
bool blinkState = false;

// ===== BUTTON =====
bool showCountdown = true;
bool lastButtonState = HIGH;

// ===== FUNCTIONS =====
void setLights(bool r, bool y, bool g) {
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

void changeState(TrafficState state, int timeSec) {
  currentState = state;
  countdown = timeSec;

  switch (state) {
    case STATE_RED:
      Serial.println("[RED] 5 seconds");
      break;
    case STATE_YELLOW:
      Serial.println("[YELLOW] 3 seconds");
      break;
    case STATE_GREEN:
      Serial.println("[GREEN] 7 seconds");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_BLUE, HIGH);

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
      "DISPLAY ON" :
      "DISPLAY OFF"
    );

    delay(200);
  }
  lastButtonState = btnState;

  // ===== BLINK LED (300ms) =====
  if (now - timerBlink >= 300) {
    timerBlink = now;
    blinkState = !blinkState;

    setLights(
      currentState == STATE_RED    && blinkState,
      currentState == STATE_YELLOW && blinkState,
      currentState == STATE_GREEN  && blinkState
    );
  }

  // ===== COUNTDOWN EVERY 1 SECOND =====
  if (now - timerSecond >= 1000) {
    timerSecond = now;
    countdown--;

    if (showCountdown) {
      display.showNumberDec(countdown, true);
    } else {
      display.clear();
    }

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
