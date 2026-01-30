#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  delay(1000); 
  Serial.println("--- LED_BLINK SYSTEM READY -");
}

void loop() {
  Serial.println("LED [RED   ] ON => 5 Seconds");
  digitalWrite(25, HIGH);
  delay(5000);
  digitalWrite(25, LOW);

  Serial.println("LED [YELLOW] ON => 3 Seconds");
  digitalWrite(26, HIGH);
  delay(3000);
  digitalWrite(26, LOW);

  Serial.println("LED [GREEN ] ON => 7 Seconds");
  digitalWrite(27, HIGH);
  delay(7000);
  digitalWrite(27, LOW);
}