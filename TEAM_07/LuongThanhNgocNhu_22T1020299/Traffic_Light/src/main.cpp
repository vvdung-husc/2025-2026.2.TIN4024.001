#include <Arduino.h>
#include <TM1637Display.h>

#define LED_GREEN   25
#define LED_YELLOW  26
#define LED_RED     27
#define LED_PURPLE  21

#define BUTTON_PIN  23
#define LDR_PIN     13

#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

bool displayOn = true;
bool lastButtonState = HIGH;

// ===== HÀM =====
void setTrafficLight(bool r, bool y, bool g) {
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

bool isDark() {
  int lightValue = analogRead(LDR_PIN);
   Serial.print("LDR = ");
  Serial.println(lightValue);
  return (lightValue > 800);

}

void showNumber(int num) {
  if (displayOn)
    display.showNumberDec(num, true);
  else
    display.clear();
}

void checkButton() {
  bool state = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && state == LOW) {
    displayOn = !displayOn;

    if (displayOn){
      digitalWrite(LED_PURPLE, LOW);
      Serial.println("Display: ON");
    }
    else    {
      digitalWrite(LED_PURPLE, HIGH);
      Serial.println("Display: OFF");
    }
    delay(0);
  }

  lastButtonState = state;
}

// ===== NHẤP NHÁY VÀNG =====
void nightBlinkMode() {
  for (int i = 0; i < 5; i++) {
    setTrafficLight(LOW, HIGH, LOW);
    delay(500);
    setTrafficLight(LOW, LOW, LOW);
    delay(500);
    checkButton();
  }
}

// ===== ĐÈN GIAO THÔNG =====
void trafficMode() {

  setTrafficLight(HIGH, LOW, LOW);
  for (int i = 5; i >= 1; i--) {
    showNumber(i);
    delay(1000);
    checkButton();
    if (isDark()) return;
  }

  setTrafficLight(LOW, HIGH, LOW);
  for (int i = 2; i >= 1; i--) {
    showNumber(i);
    delay(1000);
    checkButton();
    if (isDark()) return;
  }

  setTrafficLight(LOW, LOW, HIGH);
  for (int i = 5; i >= 1; i--) {
    showNumber(i);
    delay(1000);
    checkButton();
    if (isDark()) return;
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_PURPLE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  digitalWrite(LED_PURPLE, LOW);
}

// ===== LOOP =====
void loop() {
  checkButton();

  if (isDark()) {
    nightBlinkMode();
  } else {
    trafficMode();
  }
}
