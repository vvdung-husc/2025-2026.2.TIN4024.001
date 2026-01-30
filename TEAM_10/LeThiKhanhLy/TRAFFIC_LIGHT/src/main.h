#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <TM1637Display.h>

// ====== LED GIAO THÔNG ======
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

// ====== LED & BUTTON BLUE ======
#define LED_BLUE    21
#define BUTTON_BLUE 23

// ====== TM1637 ======
#define CLK 18
#define DIO 19

extern TM1637Display display;
extern bool blueEnabled;
extern bool lastButton;

#endif
