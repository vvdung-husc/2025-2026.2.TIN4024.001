#include <Arduino.h>

//Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

#define PIN_LED_RED 23

void setup() {
  // put your setup code here, to run once:
  printf("WELCOME IOT\n");
  pinMode(PIN_LED_RED, OUTPUT); 
}


void loop() {
  static int i = 0;
  static unsigned long ulTimer = 0;
  static bool status = false;
  // put your main code here, to run repeatedly:
  if (IsReady(ulTimer, 500)) {
    //printf("Loop running ... %d\n", ++i);
    status = !status;
    digitalWrite(PIN_LED_RED , status ? HIGH : LOW); // Turn LED ON/OFF
  }
  
}

