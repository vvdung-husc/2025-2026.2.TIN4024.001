/*
THÔNG TIN NHÓM 14
1. Phan Đại Tấn
2. Nguyễn Trí Nhân
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_GREEN 5
#define LED_YELLOW 18
#define LED_RED 19

void setup()
{
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("Khong tim thay OLED"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void blinkLED(int pin)
{
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
  delay(200);
}

void loop()
{

  delay(1000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Loi doc DHT!"));
    return;
  }

  String statusMsg = "";
  int activeLed = -1;

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (t < 13)
  {
    statusMsg = "TOO COLD";
    activeLed = LED_GREEN;
  }
  else if (t >= 13 && t < 20)
  {
    statusMsg = "COLD";
    activeLed = LED_GREEN;
  }
  else if (t >= 20 && t < 25)
  {
    statusMsg = "COOL";
    activeLed = LED_YELLOW;
  }
  else if (t >= 25 && t < 30)
  {
    statusMsg = "WARM";
    activeLed = LED_YELLOW;
  }
  else if (t >= 30 && t <= 35)
  {
    statusMsg = "HOT";
    activeLed = LED_RED;
  }
  else
  {
    statusMsg = "TOO HOT";
    activeLed = LED_RED;
  }

  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Temperature:");
  display.setTextSize(2);
  display.print(t);
  display.println(" C");

  display.setTextSize(1);
  display.print("Humidity: ");
  display.print(h);
  display.println(" %");

  display.setCursor(0, 45);
  display.setTextSize(2);
  display.println(statusMsg);

  display.display();

  if (activeLed != -1)
  {
    blinkLED(activeLed);
  }
}
