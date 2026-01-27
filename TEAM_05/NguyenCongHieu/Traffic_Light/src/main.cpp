#include <Arduino.h>

#define PIN_LED_RED 25
#define PIN_LED_YELLOW 33
#define PIN_LED_GREEN 32

enum TrafficState {
  RED_ON,
  GREEN_ON,
  YELLOW_ON
};

#define RED_DURATION 5000    // 5 seconds
#define GREEN_DURATION 7000  // 7 seconds
#define YELLOW_DURATION 3000 // 3 seconds

void setup() {
  
  Serial.begin(115200);
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  
  Serial.println("Traffic Light System Started");
}

void loop() {
  static TrafficState currentState = RED_ON;
  static unsigned long previousMillis = 0;
  static bool stateChanged = true;
  
  unsigned long currentMillis = millis();
  unsigned long duration = 0;
  
  switch (currentState) {
    case RED_ON:
      duration = RED_DURATION;
      break;
    case GREEN_ON:
      duration = GREEN_DURATION;
      break;
    case YELLOW_ON:
      duration = YELLOW_DURATION;
      break;
  }
  
  
  if (stateChanged) {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    
    switch (currentState) {
      case RED_ON:
        digitalWrite(PIN_LED_RED, HIGH);
        Serial.println("LED RED ON => 5 Seconds");
        break;
      case GREEN_ON:
        digitalWrite(PIN_LED_GREEN, HIGH);
        Serial.println("LED GREEN ON => 7 Seconds");
        break;
      case YELLOW_ON:
        digitalWrite(PIN_LED_YELLOW, HIGH);
        Serial.println("LED YELLOW ON => 3 Seconds");
        break;
    }
    stateChanged = false;
    previousMillis = currentMillis;
  }
  
  if (currentMillis - previousMillis >= duration) {
    switch (currentState) {
      case RED_ON:
        currentState = GREEN_ON;
        break;
      case GREEN_ON:
        currentState = YELLOW_ON;
        break;
      case YELLOW_ON:
        currentState = RED_ON;
        break;
    }
    stateChanged = true;
  }
}

