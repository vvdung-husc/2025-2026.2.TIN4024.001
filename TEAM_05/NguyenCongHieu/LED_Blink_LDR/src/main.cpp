#include "main.h"

//Định nghĩa chân cho đèn LED
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

//Định nghĩa cho LDR (Light Dependent Resistor)
#define PIN_LDR 34 // Analog ADC1 GPIO34 connected to LDR

int DAY_ADC_THRESHOLD = 2000; // Ngưỡng ánh sáng ban ngày

//LED_Blink ledYellow;
Trafic_Blink traficLight;
LDR ldrSensor;

void setup() {
  // put your setup code here, to run once:
  printf("Welcome IoT\n");

  ldrSensor.setup(PIN_LDR, false); // VCC = 3.3V

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traficLight.setupWaitTime(5, 3, 7); // seconds
}

void loop() {
  //ledYellow.blink(500);
  static int analogValue = 0;
  static float lux = 0;
  static bool isDark = false;
  
  bool switched = traficLight.blink(500, isDark);
  
  if (switched) {
    // Read LDR when switching LEDs
    lux = ldrSensor.readLux(&analogValue);
    isDark = (analogValue > DAY_ADC_THRESHOLD);
  }

}