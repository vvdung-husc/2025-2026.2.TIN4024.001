/* 
THÔNG TIN NHÓM 07
1. Hồ Văn Diễn
2. Bùi Khắc Hiếu


*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>
#include "DHTesp.h"


//=========== BLYNK ===========
#define BLYNK_TEMPLATE_ID "TMPL6PPonUtRv"
#define BLYNK_TEMPLATE_NAME "blynk telegram"
#define BLYNK_AUTH_TOKEN "793CtBqmPKSmHW4CJqNXl_Auc2AnYeDT"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>



char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//=========== PIN ===========
#define btnBLED 23
#define pinBLED 21

#define CLK 18
#define DIO 19

#define DHT_PIN 16
#define MQ2_PIN 34

#define OLED_SDA 13
#define OLED_SCL 12

//=========== OBJECT ===========
TM1637Display display(CLK, DIO);
DHTesp dht;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//=========== VARIABLE ===========
ulong currentMiliseconds = 0;
bool blueButtonON = true;

float temperature;
float humidity;
int gasValue;

//=========== PROTOTYPE ===========
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();
void readGas();
void displayOLED();

//=========== SETUP ===========
void setup() {

  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(7);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==");
}

//=========== LOOP ===========
void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();

  updateBlueButton();

  readDHT22();

  readGas();

  displayOLED();
}

//=========== TIMER ===========
bool IsReady(ulong &ulTimer, uint32_t milisecond) {

  if (currentMiliseconds - ulTimer < milisecond) return false;

  ulTimer = currentMiliseconds;

  return true;
}

//=========== BUTTON ===========
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

//=========== UPTIME ===========
void uptimeBlynk() {

  static ulong lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;

  Blynk.virtualWrite(V0, value);

  if (blueButtonON) {

    display.showNumberDec(value);

  }
}

//=========== DHT22 ===========
void readDHT22() {

  static ulong lastTime = 0;

  if (!IsReady(lastTime, 2000)) return;

  TempAndHumidity data = dht.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {

    Serial.println("DHT22 read failed!");

    return;
  }

  temperature = data.temperature;
  humidity = data.humidity;

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

//=========== GAS ===========
void readGas() {

  static ulong lastTime = 0;

  if (!IsReady(lastTime, 2000)) return;

  gasValue = analogRead(MQ2_PIN);

  Serial.print("Gas: ");
  Serial.println(gasValue);
  Blynk.virtualWrite(V4, gasValue);
}

//=========== OLED ===========
void displayOLED() {

  static ulong lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  oled.clearDisplay();

  oled.setCursor(0,0);
  oled.print("Nhiet Do: ");
  oled.print(temperature);
  oled.println(" C");

  oled.setCursor(0,20);
  oled.print("Do am: ");
  oled.print(humidity);
  oled.println(" %");

  oled.setCursor(0,40);
  oled.print("Gas: ");
  oled.println(gasValue);

  oled.display();
}

//=========== BLYNK ===========
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