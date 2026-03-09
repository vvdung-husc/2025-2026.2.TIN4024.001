#include <Arduino.h>
#include <TM1637Display.h>
#include "DHTesp.h"

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6bVJjfPth"
#define BLYNK_TEMPLATE_NAME "Blynk Temperature Humidity"
#define BLYNK_AUTH_TOKEN "O00jCBLaQUZFROT2qpRszh2Uh3eN8sEk"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ====== PIN ======
#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19

#define DHT_PIN 16   // đúng với diagram.json

// ====== BIẾN ======
ulong currentMiliseconds = 0;
bool blueButtonON = true;

// ====== OBJECT ======
TM1637Display display(CLK, DIO);
DHTesp dht;

// ====== PROTOTYPE ======
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);

  // DHT22 init
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==");
}

// ====== LOOP ======
void loop() {
  Blynk.run();

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT22();
}

// ====== FUNCTION ======
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON) {
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);

  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

// ====== DHT22 ======
void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // đọc mỗi 2s

  TempAndHumidity data = dht.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("DHT22 read failed!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(data.temperature);
  Serial.print(" °C  | Humidity: ");
  Serial.print(data.humidity);
  Serial.println(" %");

  Blynk.virtualWrite(V2, data.temperature); // nhiệt độ
  Blynk.virtualWrite(V3, data.humidity);    // độ ẩm
}

// ====== BLYNK ======
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}