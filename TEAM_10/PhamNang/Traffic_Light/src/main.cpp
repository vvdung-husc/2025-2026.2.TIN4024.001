#include <Arduino.h>
#include <TM1637TinyDisplay.h>

#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32
#define BUTTON_PIN 23
#define LDR_PIN 13
#define CLK 18
#define DIO 19

TM1637TinyDisplay display(CLK, DIO);

bool displayEnabled = true;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
const int NIGHT_THRESHOLD = 2500; 

void checkButton() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      displayEnabled = !displayEnabled;
      if (!displayEnabled) display.clear();
    }
  }
  lastButtonState = reading;
}

void handleTrafficLight(int ledPin, int seconds) {
  digitalWrite(ledPin, HIGH);
  for (int i = seconds; i > 0; i--) {
    checkButton();
    if (analogRead(LDR_PIN) > NIGHT_THRESHOLD) {
       digitalWrite(ledPin, LOW);
       return; 
    }
    if (displayEnabled) {
      display.showNumberDec(i);
    } else {
      display.clear();
    }
    delay(1000);
  }
  digitalWrite(ledPin, LOW);
}

void runNormalMode() {
  handleTrafficLight(RED_LED, 5);
  handleTrafficLight(GREEN_LED, 6);
  handleTrafficLight(YELLOW_LED, 3);
}

void runNightMode() {
  display.clear();
  while (analogRead(LDR_PIN) > NIGHT_THRESHOLD) {
    digitalWrite(YELLOW_LED, HIGH);
    unsigned long startFlash = millis();
    while(millis() - startFlash < 500) checkButton();
    digitalWrite(YELLOW_LED, LOW);
    startFlash = millis();
    while(millis() - startFlash < 500) checkButton();
  }
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.setBrightness(7);
}

void loop() {
  int ldrValue = analogRead(LDR_PIN);
  checkButton();
  if (ldrValue > NIGHT_THRESHOLD) {
    runNightMode();
  } else {
    runNormalMode();
  }
}