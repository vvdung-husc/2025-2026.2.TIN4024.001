#include <Arduino.h>
#include <math.h>

// ================== DEFINE PIN ==================
#define PIN_LED_RED 25
#define PIN_LED_YELLOW 33
#define PIN_LED_GREEN 32
#define PIN_LDR 34 // ADC1 GPIO34

#define DAY_ADC_THRESHOLD 2000

// ================== UTILS ==================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond)
    return false;
  ulTimer = millis();
  return true;
}

// ================== LED BLINK ==================
class LED_Blink
{
public:
  LED_Blink() : _pin(-1), _state(false), _previousMillis(0) {}
  void setup(int pin)
  {
    _pin = pin;
    pinMode(_pin, OUTPUT);
  }
  void blink(unsigned long interval = 500)
  {
    if (!IsReady(_previousMillis, interval))
      return;
    _state = !_state;
    digitalWrite(_pin, _state);
  }

private:
  int _pin;
  bool _state;
  unsigned long _previousMillis;
};

// ================== TRAFFIC LIGHT ==================
class Trafic_Blink
{
public:
  Trafic_Blink() : _previousMillis(0), _idxLED(0) {}

  void setupPin(int pinRed, int pinYellow, int pinGreen)
  {
    _LEDs[0] = pinGreen;
    _LEDs[1] = pinYellow;
    _LEDs[2] = pinRed;

    for (int i = 0; i < 3; i++)
      pinMode(_LEDs[i], OUTPUT);

    digitalWrite(pinGreen, HIGH);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinRed, LOW);
  }

  void setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait)
  {
    _waitTime[0] = greenWait * 1000;
    _waitTime[1] = yellowWait * 1000;
    _waitTime[2] = redWait * 1000;
    _counter = _waitTime[0];
  }

  bool blink(unsigned long interval, bool isDark)
  {
    if (!IsReady(_previousMillis, interval))
      return false;

    if (isDark)
    {
      digitalWrite(_LEDs[0], LOW);
      digitalWrite(_LEDs[2], LOW);
      digitalWrite(_LEDs[1], !digitalRead(_LEDs[1]));
      return false;
    }

    _counter -= interval;
    if (_counter > 0)
      return false;

    _idxLED = (_idxLED + 1) % 3;
    _counter = _waitTime[_idxLED];

    for (int i = 0; i < 3; i++)
      digitalWrite(_LEDs[i], i == _idxLED);

    return true;
  }

private:
  int _LEDs[3];
  int _idxLED;
  uint32_t _waitTime[3];
  uint32_t _counter;
  unsigned long _previousMillis;
};

// ================== LDR ==================
class LDR
{
public:
  void setup(int pin)
  {
    _pin = pin;
    pinMode(_pin, INPUT);
  }

  int read()
  {
    return analogRead(_pin);
  }

private:
  int _pin;
};

// ================== GLOBAL OBJECT ==================
Trafic_Blink traficLight;
LDR ldrSensor;

// ================== SETUP ==================
void setup()
{
  Serial.begin(115200);
  Serial.println("🚦 Traffic Light + LDR");

  ldrSensor.setup(PIN_LDR);

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traficLight.setupWaitTime(5, 3, 7);
}

// ================== LOOP ==================
void loop()
{
  int ldrValue = ldrSensor.read();
  bool isDark = ldrValue > DAY_ADC_THRESHOLD;

  traficLight.blink(500, isDark);

  Serial.printf(
      "LDR: %-4d | %s\r\n",
      ldrValue,
      isDark ? "NIGHT 🌙" : "DAY ☀️");
  delay(500);
}
