#include "main.h"

//Định nghĩa cho LDR 
#define PIN_LDR 34 

#define PIN_CLK  22
#define PIN_DIO  23

#define PIN_LED_BLUE      12
#define PIN_BUTTON_BLUE   13

//Định nghĩa chân cho đèn LED
#define PIN_LED_RED     18
#define PIN_LED_YELLOW  5
#define PIN_LED_GREEN    17

Trafic_Blink traficLight;
LDR ldrSensor;
TM1637Display display(PIN_CLK, PIN_DIO);

void setup() {
  printf("Welcome IoT\n");

  ldrSensor.DAY_THRESHOLD = 2000; // Ngưỡng ánh sáng ban ngày

  ldrSensor.setup(PIN_LDR, false); // VCC = 3.3V

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN, PIN_LED_BLUE, PIN_BUTTON_BLUE);
  traficLight.setupWaitTime(5, 3, 7); // seconds

  display.setBrightness(0x0A);
  display.clear();

}

void loop() {
  traficLight.run(ldrSensor, display);
}