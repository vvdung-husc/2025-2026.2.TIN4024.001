/*
THÔNG TIN NHÓM 5
1. Nguyễn Công Hiếu
2.
3.
4.
5.
6.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// DHT22 Configuration
#define DHTPIN 15
#define DHTTYPE DHT22

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// LED Pins
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 DHT OLED"));
  display.println(F("Initializing..."));
  display.display();
  delay(2000);
  
  // Initialize LED pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // Read temperature and humidity
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("ESP32 DHT OLED"));
  display.println();
  
  display.print(F("Temp: "));
  display.print(temperature);
  display.println(F(" C"));
  
  display.print(F("Humi: "));
  display.print(humidity);
  display.println(F(" %"));
  display.println();
  
  // Determine status and LED based on temperature
  String status = "";
  int activeLED = -1;
  
  if (temperature < 13) {
    status = "TOO COLD";
    activeLED = LED_GREEN;
  } else if (temperature < 20) {
    status = "COLD";
    activeLED = LED_GREEN;
  } else if (temperature < 25) {
    status = "COOL";
    activeLED = LED_YELLOW;
  } else if (temperature < 30) {
    status = "WARM";
    activeLED = LED_YELLOW;
  } else if (temperature < 35) {
    status = "HOT";
    activeLED = LED_RED;
  } else {
    status = "TOO HOT";
    activeLED = LED_RED;
  }
  
  display.setTextSize(2);
  display.println(status);
  display.display();
  
  // Control LEDs (blinking)
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    ledState = !ledState;
    
    // Turn off all LEDs
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    
    // Turn on active LED
    if (activeLED != -1) {
      digitalWrite(activeLED, ledState ? HIGH : LOW);
    }
  }
  
  // Print to Serial Monitor
  Serial.print(F("Temperature: "));
  Serial.print(temperature);
  Serial.print(F(" °C, Humidity: "));
  Serial.print(humidity);
  Serial.print(F(" %, Status: "));
  Serial.println(status);
  
  delay(100);
}
