// #include <Arduino.h>

// #define LED_RED    25
// #define LED_YELLOW 33
// #define LED_GREEN  32

// void setup() {
//   pinMode(LED_RED, OUTPUT);
//   pinMode(LED_YELLOW, OUTPUT);
//   pinMode(LED_GREEN, OUTPUT);
// }

// void loop() {

//   digitalWrite(LED_RED, HIGH);
//   digitalWrite(LED_YELLOW, LOW);
//   digitalWrite(LED_GREEN, LOW);
//   delay(3000);

//   digitalWrite(LED_RED, LOW);
//   digitalWrite(LED_YELLOW, LOW);
//   digitalWrite(LED_GREEN, HIGH);
//   delay(4000);

//   digitalWrite(LED_RED, LOW);
//   digitalWrite(LED_YELLOW, HIGH);
//   digitalWrite(LED_GREEN, LOW);
//   delay(2000);
// }
#include <Arduino.h>

#define LED_RED 25
#define LED_YELLOW 33
#define LED_GREEN 32

void blinkBySecond(int ledPin, int seconds)
{
  for (int i = 0; i < seconds; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop()
{

  // 🔴 ĐỎ nhấp nháy 3 giây
  blinkBySecond(LED_RED, 3);

  // 🟢 XANH nhấp nháy 4 giây
  blinkBySecond(LED_GREEN, 4);

  // 🟡 VÀNG nhấp nháy 2 giây
  blinkBySecond(LED_YELLOW, 2);
}
