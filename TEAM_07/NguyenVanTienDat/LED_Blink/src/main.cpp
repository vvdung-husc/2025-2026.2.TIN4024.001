#include <TM1637Display.h>

// ===== PIN SETUP =====
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define LED_STREET 21
#define BUTTON_PIN 23
#define LDR_PIN 13

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== TRAFFIC LIGHT TIMES =====
const int GREEN_TIME = 7;
const int YELLOW_TIME = 3;
const int RED_TIME = 5;

bool showCountdown = true;
bool lastButtonState = HIGH;

unsigned long previousMillis = 0;
int countdown = GREEN_TIME;
int state = 0; 
// 0 = GREEN, 1 = YELLOW, 2 = RED

// ===== SET TRAFFIC LIGHT =====
void setLights(bool g, bool y, bool r) {
  digitalWrite(LED_GREEN, g);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_RED, r);
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  setLights(HIGH, LOW, LOW); // bắt đầu đèn xanh
}

void loop() {

  // ===== PUSH BUTTON: TOGGLE DISPLAY =====
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    showCountdown = !showCountdown;
    delay(200); // chống dội nút
  }
  lastButtonState = buttonState;

  // ===== LDR: STREET LIGHT CONTROL =====
  int lightValue = analogRead(LDR_PIN);

  if (lightValue < 2000) { // trời tối
    digitalWrite(LED_STREET, LOW);   // LED mắc ngược → LOW = sáng
  } else {
    digitalWrite(LED_STREET, HIGH);  // tắt
  }

  // ===== TRAFFIC LIGHT TIMER (NON-BLOCKING) =====
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    countdown--;

    if (countdown <= 0) {
      state++;
      if (state > 2) state = 0;

      switch (state) {
        case 0:
          setLights(HIGH, LOW, LOW);
          countdown = GREEN_TIME;
          break;
        case 1:
          setLights(LOW, HIGH, LOW);
          countdown = YELLOW_TIME;
          break;
        case 2:
          setLights(LOW, LOW, HIGH);
          countdown = RED_TIME;
          break;
      }
    }
  }

  // ===== DISPLAY COUNTDOWN =====
  if (showCountdown) {
    display.showNumberDec(countdown, true);
  } else {
    display.clear();
  }
}
