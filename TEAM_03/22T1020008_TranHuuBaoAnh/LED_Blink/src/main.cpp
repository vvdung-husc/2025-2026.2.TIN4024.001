#include <Arduino.h>

// Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond)
    return false;
  ulTimer = millis();
  return true;
}

#define PIN_LED_RED 23

void setup()
{
  pinMode(PIN_LED_RED, OUTPUT);
}

void loop()
{
  static unsigned long ulTimer = 0;
  static bool status = false;

  if (IsReady(ulTimer, 500))
  {
    status = !status;
    digitalWrite(PIN_LED_RED, status ? HIGH : LOW); // HIGH sáng, LOW tắt
  }
}
