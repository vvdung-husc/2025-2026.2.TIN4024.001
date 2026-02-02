#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  26
#define LED_GREEN   27

void blinkLED(int pin, int seconds, const char* name) {
  Serial.printf("LED %s ON => %d Seconds\n", name, seconds);

  for (int i = 1; i <= seconds; i++) {
    digitalWrite(pin, HIGH);
    delay(500);          // ON 0.5s
    digitalWrite(pin, LOW);
    delay(500);          // OFF 0.5s  → tổng đúng 1 giây

    Serial.printf("%s: %d second\n", name, i);
  }

  Serial.println("------------------------");
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Traffic Light Simulation Started");
}

void loop() {
  blinkLED(LED_RED, 5, "RED");
  blinkLED(LED_GREEN, 7, "GREEN");
  blinkLED(LED_YELLOW, 3, "YELLOW");
}
