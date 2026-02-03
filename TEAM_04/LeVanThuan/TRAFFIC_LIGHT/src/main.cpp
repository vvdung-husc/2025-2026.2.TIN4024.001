#include <Arduino.h>

// ===== PIN =====
#define LED_RED     18
#define LED_YELLOW  5
#define LED_GREEN   17
#define LDR_PIN     34
#define BUTTON_PIN  23

// ===== NIGHT THRESHOLD =====
#define NIGHT_LUX_THRESHOLD 80   // chỉnh theo môi trường

// ===== SENSOR =====
int adcValue = 0;
float voltage = 0;
float lux = 0;

// ===== MODE =====
bool isNight = false;
bool manualMode = false;

// ===== TRAFFIC =====
int currentLight = 0; // 0=GREEN, 1=YELLOW, 2=RED
int countdown = 0;

// ===== TIMER =====
unsigned long trafficTimer = 0;
unsigned long printTimer = 0;
unsigned long blinkTimer = 0;

// ===== TIME CONFIG =====
int greenTime = 7;
int yellowTime = 3;
int redTime = 5;

// ===== LUX CONVERT (AUTO FIX) =====
float estimateLux(int adc) {
  float normalized = adc / 4095.0;
  return normalized * 500;  // ADC cao = sáng
}

// ===== LED CONTROL =====
void setLights(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

// ===== NIGHT MODE =====
void nightModeBlink() {
  if (millis() - blinkTimer >= 500) {
    blinkTimer = millis();
    digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
  }
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
}

// ===== TRAFFIC MODE =====
void trafficMode() {
  if (millis() - trafficTimer >= 1000) {
    trafficTimer = millis();
    countdown--;

    if (countdown <= 0) {
      currentLight = (currentLight + 1) % 3;

      if (currentLight == 0) countdown = greenTime;
      if (currentLight == 1) countdown = yellowTime;
      if (currentLight == 2) countdown = redTime;
    }
  }

  if (currentLight == 0) setLights(false, false, true);
  if (currentLight == 1) setLights(false, true, false);
  if (currentLight == 2) setLights(true, false, false);
}

// ===== BUTTON (FAST + DEBOUNCE) =====
void handleButton() {
  static bool lastState = HIGH;
  static unsigned long lastTime = 0;

  bool state = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && state == LOW && millis() - lastTime > 150) {
    lastTime = millis();

    if (!manualMode) {
      manualMode = true;
      Serial.println("MODE = MANUAL ENABLED");
    } else {
      currentLight = (currentLight + 1) % 3;

      if (currentLight == 0) countdown = greenTime;
      if (currentLight == 1) countdown = yellowTime;
      if (currentLight == 2) countdown = redTime;

      Serial.println("MANUAL = SWITCH LIGHT");
    }
  }

  lastState = state;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  countdown = greenTime;

  Serial.println("=======================================");
  Serial.println("🚦 ESP32 SMART TRAFFIC LIGHT READY!");
  Serial.println("🌞 AUTO DAY / 🌙 NIGHT + BUTTON");
  Serial.println("=======================================");
}

// ===== LOOP =====
void loop() {
  handleButton();

  // ===== READ SENSOR =====
  adcValue = 2013;//analogRead(LDR_PIN);
  voltage = adcValue * (3.3 / 4095.0);
  lux = estimateLux(adcValue);

  // ===== AUTO DAY/NIGHT SWITCH INSTANT =====
  if (!manualMode) {
    bool newNight = lux < NIGHT_LUX_THRESHOLD;

    if (newNight != isNight) {
      isNight = newNight;

      if (!isNight) {
        countdown = greenTime;
        currentLight = 0;
      }

      Serial.println("⚡ MODE CHANGED!");
    }
  }

  // ===== SERIAL PRINT (EVERY 1S) =====
  if (millis() - printTimer >= 1000) {
    printTimer = millis();

    Serial.print("ADC=");
    Serial.print(adcValue);
    Serial.print(" | Volt=");
    Serial.print(voltage, 2);
    Serial.print("V | Lux=");
    Serial.print(lux, 1);
    Serial.print(" | Countdown=");
    Serial.print(countdown);
    Serial.print(" | Mode=");

    if (manualMode) Serial.println("MANUAL");
    else Serial.println(isNight ? "AUTO NIGHT" : "AUTO DAY");
  }

  // ===== LOGIC =====
  if (!manualMode) {
    if (isNight) nightModeBlink();
    else trafficMode();
  } else {
    trafficMode();
  }
}
