// #include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
#include <Arduino.h>
#include <TM1637Display.h>

// ================= PIN CONFIG =================
#define LED_RED     25
#define LED_GREEN   26
#define LED_YELLOW  27

#define BUTTON_PIN  33

#define CLK  18
#define DIO  19

TM1637Display display(CLK, DIO);

// ================= TIME CONFIG =================
const int RED_TIME    = 5;
const int GREEN_TIME  = 7;
const int YELLOW_TIME = 3;

// ================= STATE =================
enum TrafficState { RED, GREEN, YELLOW };
TrafficState state = RED;

int remainingTime = RED_TIME;
bool paused = false;

// timing
unsigned long lastSecond = 0;

// button
bool lastBtn = HIGH;

// ================= FUNCTIONS =================
void setLights() {
  digitalWrite(LED_RED,    state == RED);
  digitalWrite(LED_GREEN,  state == GREEN);
  digitalWrite(LED_YELLOW, state == YELLOW);
}

void showTime(int t) {
  display.showNumberDec(t, true);
}

void nextState() {
  if (state == RED) {
    state = GREEN;
    remainingTime = GREEN_TIME;
  }
  else if (state == GREEN) {
    state = YELLOW;
    remainingTime = YELLOW_TIME;
  }
  else {
    state = RED;
    remainingTime = RED_TIME;
  }

  setLights();
  showTime(remainingTime);
}

// ================= SETUP =================
void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  setLights();
  showTime(remainingTime);
}

// ================= LOOP =================
void loop() {

  // -------- BUTTON (pause / resume) --------
  bool btn = digitalRead(BUTTON_PIN);

  // bắt cạnh nhấn (HIGH -> LOW)
  if (lastBtn == HIGH && btn == LOW) {
    paused = !paused;
    delay(200);   // debounce đơn giản, đủ ổn cho Wokwi
  }
  lastBtn = btn;

  // -------- TIMER --------
  if (!paused) {
    if (millis() - lastSecond >= 1000) {
      lastSecond = millis();

      remainingTime--;

      if (remainingTime <= 0) {
        nextState();
      } else {
        showTime(remainingTime);
      }
    }
  }
}
