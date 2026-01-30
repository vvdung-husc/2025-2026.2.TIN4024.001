#include "main.h"
#include "utils.h"

TM1637Display display(CLK, DIO);

// ====== BIẾN TOÀN CỤC ======
bool blueEnabled = true;
bool lastButton = HIGH;

void setup() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    pinMode(BUTTON_BLUE, INPUT_PULLUP);

    display.setBrightness(7);
    display.clear();

    digitalWrite(LED_BLUE, HIGH); // mặc định bật
}

void loop() {
    blinkWithCountdown(LED_RED, 5);
    blinkWithCountdown(LED_YELLOW, 3);
    blinkWithCountdown(LED_GREEN, 7);
}
