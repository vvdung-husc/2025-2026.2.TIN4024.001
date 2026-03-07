

#define BLYNK_TEMPLATE_ID "TMPL6jE1O48Nf"
#define BLYNK_TEMPLATE_NAME "BLYNKDHT"
#define BLYNK_AUTH_TOKEN "uvFWONG6B-0LywdKmRGFwRhwYRVNVSCc"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHT_PIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BUTTON_PIN 23
#define CLK 18
#define DIO 19

DHT dht(DHT_PIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool ledState = false;
unsigned long uptime = 0;

void updateLED(bool state)
{
  ledState = state;
  digitalWrite(LED_PIN, ledState);
  Blynk.virtualWrite(V2, ledState);
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V2);
  Blynk.virtualWrite(V3, uptime);
}

BLYNK_WRITE(V2)
{
  updateLED(param.asInt());
}

void sendSensor()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum))
  {
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
  }
}

void sendUptime()
{
  uptime++;

  display.showNumberDec(uptime, false);
  Blynk.virtualWrite(V3, uptime);

  Serial.print("Uptime: ");
  Serial.println(uptime);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, sendUptime);
}

void loop()
{
  Blynk.run();
  timer.run();

  static bool lastButton = HIGH;
  bool currentButton = digitalRead(BUTTON_PIN);

  if (lastButton == HIGH && currentButton == LOW)
  {
    updateLED(!ledState);
    delay(150);
  }

  lastButton = currentButton;
}
