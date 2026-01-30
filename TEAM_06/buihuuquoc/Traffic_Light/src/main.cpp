#include <Arduino.h>
#include <TM1637Display.h>

// ===== CHÂN KẾT NỐI =====
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21
#define BUTTON_PIN 23
#define LDR_PIN 13
#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== TIMER =====
unsigned long dayMillis = 0;
unsigned long nightMillis = 0;
unsigned long debounceMillis = 0;

// ===== BUTTON =====
bool isSystemOn = false;
int lastButtonState = HIGH;
int buttonState = HIGH;
const unsigned long debounceDelay = 50;

// ===== TRAFFIC LIGHT =====
int state = 1; // 1:Xanh | 2:Vàng | 0:Đỏ
int countdown = 5;
bool ledToggle = false;

// ===== LDR =====
const int NIGHT_THRESHOLD = 2000;
int lastLdrValue = -1;
const int LDR_DELTA = 20;

// ================== HÀM TIỆN ÍCH ==================
void setTrafficLights(bool r, bool y, bool g)
{
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

void turnOffAll()
{
  setTrafficLights(LOW, LOW, LOW);
  digitalWrite(LED_BLUE, LOW);
  display.clear();
}

void resetSystem()
{
  state = 1;
  countdown = 5;
  ledToggle = false;

  dayMillis = millis();
  nightMillis = millis();

  digitalWrite(LED_BLUE, HIGH);
  setTrafficLights(LOW, LOW, HIGH);
  display.showNumberDec(countdown, true);
}

// ================== NIGHT MODE ==================
void runNightMode()
{
  if (millis() - nightMillis >= 500)
  {
    nightMillis = millis();
    ledToggle = !ledToggle;

    setTrafficLights(LOW, ledToggle, LOW);

    if (ledToggle)
      display.clear();
    else
      display.showNumberDec(0, false);
  }
}

// ================== DAY MODE ==================
void runDayMode()
{
  if (millis() - dayMillis >= 1000)
  {
    dayMillis = millis();
    countdown--;
    ledToggle = !ledToggle; // <<< NHÁY MỖI GIÂY

    if (countdown < 0)
    {
      if (state == 1)
      {
        state = 2;
        countdown = 2;
      }
      else if (state == 2)
      {
        state = 0;
        countdown = 5;
      }
      else
      {
        state = 1;
        countdown = 5;
      }
    }

    display.showNumberDec(countdown, true);

    // ===== LED NHÁY THEO STATE =====
    if (state == 1) // GREEN
    {
      setTrafficLights(LOW, LOW, ledToggle);
      Serial.print("MODE=DAY | STATE=GREEN | COUNTDOWN=");
    }
    else if (state == 2) // YELLOW
    {
      setTrafficLights(LOW, ledToggle, LOW);
      Serial.print("MODE=DAY | STATE=YELLOW | COUNTDOWN=");
    }
    else // RED
    {
      setTrafficLights(ledToggle, LOW, LOW);
      Serial.print("MODE=DAY | STATE=RED | COUNTDOWN=");
    }

    Serial.println(countdown);
  }
}

// ================== SETUP ==================
void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  Serial.begin(115200);
  display.setBrightness(7);
  turnOffAll();
}

// ================== LOOP ==================
void loop()
{
  // ===== BUTTON =====
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState)
    debounceMillis = millis();

  if (millis() - debounceMillis > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;

      if (buttonState == LOW)
      {
        isSystemOn = !isSystemOn;
        if (isSystemOn)
          resetSystem();
        else
          turnOffAll();
      }
    }
  }
  lastButtonState = reading;

  if (!isSystemOn)
    return;

  // ===== LDR READ =====
  int ldrValue = analogRead(LDR_PIN);

  if (lastLdrValue < 0 || abs(ldrValue - lastLdrValue) > LDR_DELTA)
  {
    lastLdrValue = ldrValue;
    Serial.print("LDR = ");
    Serial.print(ldrValue);
    Serial.print(" | MODE = ");
    Serial.println(ldrValue > NIGHT_THRESHOLD ? "NIGHT" : "DAY");
  }

  // ===== MODE SELECT =====
  if (ldrValue > NIGHT_THRESHOLD)
    runNightMode();
  else
    runDayMode();
}
