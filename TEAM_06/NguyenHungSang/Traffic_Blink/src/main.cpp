#include <Arduino.h>

#define LED_RED    25
#define LED_YELLOW 33
#define LED_GREEN  32

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RED, HIGH);
    delay(500); 
    digitalWrite(LED_RED, LOW);
    delay(500); 
  }


  for (int i = 0; i < 7; i++) {
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    delay(500);
  }


  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
}