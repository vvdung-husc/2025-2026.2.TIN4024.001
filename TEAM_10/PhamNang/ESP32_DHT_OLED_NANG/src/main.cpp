#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_RED    5
#define LED_YELLOW 18
#define LED_GREEN  19

void setup() {
  Serial.begin(115200);

  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("Sensor Error!");
    display.display();
    return;
  }

  String statusText = "";
  int activeLed = -1;

  if (t < 13) {
    statusText = "TOO COLD";
    activeLed = LED_GREEN;
  } 
  else if (t >= 13 && t < 20) {
    statusText = "COLD";
    activeLed = LED_GREEN;
  }
  else if (t >= 20 && t < 25) {
    statusText = "COOL";
    activeLed = LED_YELLOW;
  }
  else if (t >= 25 && t < 30) {
    statusText = "WARM";
    activeLed = LED_YELLOW;
  }
  else if (t >= 30 && t <= 35) {
    statusText = "HOT";
    activeLed = LED_RED;
  }
  else if (t > 35) {
    statusText = "TOO HOT";
    activeLed = LED_RED;
  }

  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(t, 1); display.println(" C");
  display.print("Hum:  "); display.print(h, 1); display.println(" %");

  display.setCursor(0, 30);
  display.setTextSize(2);
  display.println(statusText);
  
  display.display();

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  if (activeLed != -1) {
    for(int i = 0; i < 2; i++) {
      digitalWrite(activeLed, HIGH);
      delay(500);
      digitalWrite(activeLed, LOW);
      delay(500);
    }
  } else {
    delay(2000); 
  }
}