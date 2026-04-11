#define BLYNK_TEMPLATE_ID "TMPL6T-HveCwO"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic DHT"
#define BLYNK_AUTH_TOKEN "y0us9bxKqZSi1uuJzLvfDRpXX5XRZicc"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; 

#define DHTPIN 15      
#define DHTTYPE DHT22  
#define CLK 18         
#define DIO 19         
#define LED_PIN 2      

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

int countdown = 0;
bool isRunning = false;

void sendData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  }
}

void timerEvent() {
  if (isRunning && countdown > 0) {
    countdown--;
    display.showNumberDec(countdown, false, 4, 0);
    Blynk.virtualWrite(V3, countdown);
    if (countdown == 0) {
      isRunning = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V0, 0);
    }
  }
}

BLYNK_WRITE(V0) {
  int val = param.asInt();
  if (val == 1) {
    isRunning = true;
    countdown = 11;
    digitalWrite(LED_PIN, HIGH);
  } else {
    isRunning = false;
    countdown = 0;
    digitalWrite(LED_PIN, LOW);
    display.clear();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  display.setBrightness(0x0f);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendData);
  timer.setInterval(1000L, timerEvent);
}

void loop() {
  Blynk.run();
  timer.run();
}