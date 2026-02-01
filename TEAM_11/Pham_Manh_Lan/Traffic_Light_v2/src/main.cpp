#include <Arduino.h>
#include <TM1637Display.h>

// ================= TIỆN ÍCH =================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// ================= PIN THEO DIAGRAM.JSON =================
// LED giao thông
#define PIN_LED_RED     14
#define PIN_LED_YELLOW  27
#define PIN_LED_GREEN   26

// TM1637
#define CLK 18
#define DIO 19

// Button + LED xanh
#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE      21

#define PIN_LDR 13
#define LIGHT_THRESHOLD 2000

// ================= THIẾT BỊ =================
TM1637Display display(CLK, DIO);
int valueButtonDisplay = LOW;

// ================= HỖ TRỢ =================
const char* LEDString(uint8_t pin)
{
  switch (pin)
  {
    case PIN_LED_RED:     return "RED";
    case PIN_LED_YELLOW:  return "YELLOW";
    case PIN_LED_GREEN:   return "GREEN";
    default:              return "UNKNOWN";
  }
}

void Init_LED_Traffic()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}
bool ProcessYellowBlinkAtNight()
{
  static unsigned long ulTimer = 0;
  static bool ledState = false;
  static int secondCount = 1;

  if (!IsReady(ulTimer, 250)) return false;

  // Tắt RED & GREEN
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  // Nhấp nháy YELLOW
  ledState = !ledState;
  digitalWrite(PIN_LED_YELLOW, ledState);

  // Đếm giây (chỉ khi bật)
  if (ledState)
  {
    if (valueButtonDisplay == HIGH)
    {
      display.showNumberDec(secondCount);
      printf("[NIGHT] YELLOW => %d s\n", secondCount);
    }
    secondCount++;
    if (secondCount > 99) secondCount = 1;
  }

  return true;
}

// ================= LOGIC ĐÈN GIAO THÔNG =================
bool ProcessLEDTrafficWaitTime()
{
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;
  static uint8_t LEDs[3] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED};
  static uint32_t waitTime[3] = {7000, 3000, 5000};
  static uint32_t count = waitTime[idxLED];
  static bool ledStatus = false;
  static int secondCount = 0;

  if (!IsReady(ulTimer, 250)) return false;

  if (count == waitTime[idxLED])
  {
    secondCount = (count / 1000) - 1;
    ledStatus = true;

    for (int i = 0; i < 3; i++)
    {
      digitalWrite(LEDs[i], i == idxLED ? HIGH : LOW);
    }

    printf("LED [%s] ON (%d s)\n", LEDString(LEDs[idxLED]), count / 1000);
  }
  else
  {
    ledStatus = !ledStatus;
    digitalWrite(LEDs[idxLED], ledStatus);
  }

  if (ledStatus && valueButtonDisplay == HIGH)
  {
    display.showNumberDec(secondCount);
    --secondCount;
  }

  count -= 500;
  if (count > 0) return true;

  idxLED = (idxLED + 1) % 3;
  count = waitTime[idxLED];
  return true;
}

// ================= BUTTON =================
void ProcessButtonPressed()
{
  static unsigned long ulTimer = 0;
  if (!IsReady(ulTimer, 250)) return;

  int newValue = digitalRead(PIN_BUTTON_DISPLAY);
  if (newValue == valueButtonDisplay) return;

  if (newValue == HIGH)
  {
    digitalWrite(PIN_LED_BLUE, HIGH);
    printf("*** DISPLAY ON ***\n");
  }
  else
  {
    digitalWrite(PIN_LED_BLUE, LOW);
    display.clear();
    printf("*** DISPLAY OFF ***\n");
  }

  valueButtonDisplay = newValue;
}

// ================= SETUP & LOOP =================
void setup()
{
  Serial.begin(115200);
  printf("*** PROJECT LED TRAFFIC ***\n");

  Init_LED_Traffic();

  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BUTTON_DISPLAY, INPUT_PULLUP); // ← BẮT BUỘC
  pinMode(PIN_LED_BLUE, OUTPUT);

  analogSetAttenuation(ADC_11db);

  display.setBrightness(0x0a);
  display.clear();
}

void loop()
{
  static bool isNight = false;

  ProcessButtonPressed();

  int lightValue = analogRead(PIN_LDR);

  if (lightValue > LIGHT_THRESHOLD && !isNight)
  {
    isNight = true;
  }
  else if (lightValue <= LIGHT_THRESHOLD && isNight)
  {
    isNight = false;
  }

  if (isNight)
  {
    ProcessYellowBlinkAtNight();
  }
  else
  {
    ProcessLEDTrafficWaitTime();
  }
}
