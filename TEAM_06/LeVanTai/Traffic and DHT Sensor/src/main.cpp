#define BLYNK_TEMPLATE_ID "TMPL6CzfTBJhx"
#define BLYNK_TEMPLATE_NAME "LeVanTai"
#define BLYNK_AUTH_TOKEN "MxwqPkHFZczSGIvHMw209kiuA0lnnrR3"

#define BLYNK_PRINT Serial
#define BLYNK_HEARTBEAT 30

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* PIN theo sơ đồ của bạn */
#define LED_PIN 21
#define BUTTON_PIN 23

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

bool systemState = false;
int secondsRun = 0;

/* đọc DHT22 */
void sendSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) {
    Blynk.virtualWrite(V1, t);
  }

  if (!isnan(h)) {
    Blynk.virtualWrite(V2, h);
  }
}

/* đếm thời gian */
void countTime() {

  if (systemState) {

    secondsRun++;

    display.showNumberDec(secondsRun);

    Blynk.virtualWrite(V3, secondsRun);
  }
}

/* điều khiển từ Blynk */
BLYNK_WRITE(V0) {

  systemState = param.asInt();

  digitalWrite(LED_PIN, systemState);

  if (!systemState) {

    secondsRun = 0;

    display.showNumberDec(0);

    Blynk.virtualWrite(V3, 0);
  }
}

/* nút trên Wokwi */
void checkButton() {

  static bool lastState = HIGH;

  bool state = digitalRead(BUTTON_PIN);

  if (state == LOW && lastState == HIGH) {

    systemState = !systemState;

    digitalWrite(LED_PIN, systemState);

    Blynk.virtualWrite(V0, systemState);

    if (!systemState) {

      secondsRun = 0;

      display.showNumberDec(0);

      Blynk.virtualWrite(V3, 0);
    }

    delay(200);
  }

  lastState = state;
}

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  timer.setInterval(1000L, countTime);
  timer.setInterval(2000L, sendSensor);
}

void loop() {

  Blynk.run();
  timer.run();

  checkButton();
}