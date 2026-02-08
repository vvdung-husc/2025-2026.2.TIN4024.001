#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_GREEN  15
#define LED_YELLOW 2
#define LED_RED    4

unsigned long lastBlink = 0;
bool ledState = false;

// ===== BLINK =====
void blinkLED(int pin) {
  if (millis() - lastBlink >= 500) {
    ledState = !ledState;
    digitalWrite(pin, ledState);
    lastBlink = millis();
  }
}

// ===== OLED UI =====
void drawOLED(float t, float h, const char* status) {
  display.clearDisplay();

  // Line 1
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.println(status);

  // Temperature big
  display.setTextSize(2);
  display.setCursor(20, 14);
  display.print(t, 2);
  display.println(" C");

  // Humidity label
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println("Humidity:");

  // Humidity big
  display.setTextSize(2);
  display.setCursor(28, 48);
  display.print(h, 2);
  display.println(" %");

  display.display();
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(13, 12);   // SDA, SCL

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextColor(SSD1306_WHITE);

  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) return;

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  const char* status;

  if (temp < 13) {
    status = "TOO COLD";
    blinkLED(LED_GREEN);
  } else if (temp < 20) {
    status = "COLD";
    blinkLED(LED_GREEN);
  } else if (temp < 25) {
    status = "COOL";
    blinkLED(LED_YELLOW);
  } else if (temp < 30) {
    status = "WARM";
    blinkLED(LED_YELLOW);
  } else if (temp < 35) {
    status = "HOT";
    blinkLED(LED_RED);
  } else {
    status = "TOO HOT";
    blinkLED(LED_RED);
  }

  drawOLED(temp, hum, status);
  delay(200);
}
