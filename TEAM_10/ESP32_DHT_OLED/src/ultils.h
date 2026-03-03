#pragma once
#include <Arduino.h>

void setupLED();
void setLED(bool g, bool y, bool r);
void blinkLED(int pin, int delayMs);
void offAllLED();
