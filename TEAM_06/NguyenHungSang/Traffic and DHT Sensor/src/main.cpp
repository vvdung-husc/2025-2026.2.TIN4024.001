#define BLYNK_TEMPLATE_ID "TMPL6xB0NbhkA"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "41jGiQCKYOVqKZako90AvPvhfpE9dd_Z"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

// WIFI (Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN =====
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BUTTON_PIN 23
#define CLK 18
#define DIO 19

// ===== SENSOR =====
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

// ===== BLYNK TIMER =====
BlynkTimer timer;

bool ledState = false;
unsigned long startTime = 0;
unsigned long elapsedSeconds = 0;

// ============================
// GỬI NHIỆT ĐỘ + ĐỘ ẨM
// ============================
void sendSensor() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (!isnan(humidity) && !isnan(temperature)) {

    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.print(" C | Do am: ");
    Serial.println(humidity);

    // GỬI LÊN BLYNK
    if (Blynk.connected()) {      // <<< BỔ SUNG
      Blynk.virtualWrite(V0, temperature);
      Blynk.virtualWrite(V1, humidity);
    }
  }
}

// ============================
// GỬI THỜI GIAN LED
// ============================
void sendTime() {

  if (ledState) {

    elapsedSeconds = (millis() - startTime) / 1000;

    display.showNumberDec(elapsedSeconds);

    if (Blynk.connected()) {      // <<< BỔ SUNG
      Blynk.virtualWrite(V2, elapsedSeconds);
    }
  }
}

// ============================
// SWITCH TRÊN BLYNK
// ============================
BLYNK_WRITE(V3) {

  int value = param.asInt();

  if (value == 1 && ledState == false) {
    startTime = millis();
  }

  ledState = value;

  digitalWrite(LED_PIN, ledState);

  if (!ledState) {
    display.showNumberDec(0);
    Blynk.virtualWrite(V2, 0);
  }
}

// ============================
// ĐỒNG BỘ KHI CONNECT
// ============================
BLYNK_CONNECTED() {

  Blynk.syncVirtual(V3);

  // <<< BỔ SUNG ĐỒNG BỘ NGAY LẬP TỨC
  sendSensor();
  sendTime();
}

// ============================
// SETUP
// ============================
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);
  display.showNumberDec(0);

  // WIFI
  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" OK");

  // BLYNK
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  Blynk.connect();

  // TIMER
  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, sendTime);
}

// ============================
// LOOP
// ============================
void loop() {

  Blynk.run();
  timer.run();

  static bool lastButton = HIGH;
  bool button = digitalRead(BUTTON_PIN);

  if (button == LOW && lastButton == HIGH) {

    ledState = !ledState;

    digitalWrite(LED_PIN, ledState);

    Blynk.virtualWrite(V3, ledState);

    if (ledState) {
      startTime = millis();
    } else {
      display.showNumberDec(0);
      Blynk.virtualWrite(V2, 0);
    }
  }

  lastButton = button;
}