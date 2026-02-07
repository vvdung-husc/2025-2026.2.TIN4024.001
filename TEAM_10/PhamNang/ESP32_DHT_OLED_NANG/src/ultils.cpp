#include "ultils.h"
#include "main.h"

void setupLED() {
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
}

void setLED(bool g, bool y, bool r) {
    digitalWrite(LED_GREEN, g);
    digitalWrite(LED_YELLOW, y);
    digitalWrite(LED_RED, r);
}

void offAllLED() {
    setLED(false, false, false);
}

void blinkLED(int pin, int delayMs) {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
}