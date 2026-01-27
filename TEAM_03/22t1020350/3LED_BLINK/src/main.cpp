#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

#define BLINK_DELAY 500  

void blinkLed(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
    delay(BLINK_DELAY);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== START LED ===");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // RED
  Serial.println("RED LED (3)");
  blinkLed(LED_RED, 3);

  // YELLOW
  Serial.println("YELLOW LED (5)");
  blinkLed(LED_YELLOW, 5);

  // GREEN
  Serial.println("GREEN LED (7)");
  blinkLed(LED_GREEN, 7);
  delay(1000);
}
