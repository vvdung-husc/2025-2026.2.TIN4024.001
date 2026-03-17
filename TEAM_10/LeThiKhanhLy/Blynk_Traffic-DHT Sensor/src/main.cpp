#include <Arduino.h>
#include <TM1637Display.h>

/* ===== BLYNK INFO ===== */
#define BLYNK_TEMPLATE_ID "TMPL68EwJFXV1"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic DHT"
#define BLYNK_AUTH_TOKEN "Ze8m7pD2pettcUcJBuCHmt6B3h6RhrPI"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>

/* ===== WIFI ===== */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* ===== PIN DEFINE ===== */
#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

/* ===== OBJECT ===== */
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

/* ===== GLOBAL VARIABLE ===== */
unsigned long currentMillis = 0;
unsigned long activeSeconds = 0;

bool blueButtonON = true;

/* ===================================================== */
/* ===================== FUNCTIONS ===================== */
/* ===================================================== */

bool IsReady(unsigned long &timer, uint32_t interval) {
  if (currentMillis - timer < interval) return false;
  timer = currentMillis;
  return true;
}

/* ----------- UPTIME COUNTER ----------- */
void uptimeTask() {
  static unsigned long previousMillis = 0;

  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();

    if (blueButtonON) {
      activeSeconds++;

      Serial.print("Send to Blynk: ");
      Serial.println(activeSeconds);

      display.showNumberDec(activeSeconds);
      Blynk.virtualWrite(V0, activeSeconds);
    }
  }
}
/* ----------- READ DHT ----------- */
void readDHT() {
  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("  Hum: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}

/* ----------- BUTTON HARDWARE ----------- */
void updateButton() {
  static unsigned long lastTime = 0;
  static int lastState = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int state = digitalRead(btnBLED);

  if (state == lastState) return;
  lastState = state;

  if (state == LOW) return;   // nhấn nhả mới tính

  blueButtonON = !blueButtonON;

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V3, blueButtonON);

  if (!blueButtonON) {
  display.clear();   // chỉ tắt màn hình thôi
}
}

/* ----------- RECEIVE FROM BLYNK SWITCH ----------- */
BLYNK_WRITE(V3) {
  blueButtonON = param.asInt();

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

if (!blueButtonON) {
  display.clear();
}
}

/* ======================= SETUP ======================= */

void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(pinBLED, HIGH);
  Blynk.virtualWrite(V3, blueButtonON);

  Serial.println("=== SYSTEM START ===");
}

/* ======================== LOOP ======================= */

void loop() {
  Blynk.run();
  uptimeTask();
  updateButton();
  readDHT();
}