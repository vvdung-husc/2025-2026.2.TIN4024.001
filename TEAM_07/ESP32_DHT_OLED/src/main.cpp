/*
THÔNG TIN NHÓM 07
1. Nguyễn Văn Phong
2. Bùi Khắc Hiếu
3. Hồ Văn Diễn
4.Nguyễn Văn Tiến Đạt
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 13
#define OLED_SCL 12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();
}

void blinkLED(int pin) {
  digitalWrite(pin, LOW);
  delay(300);
  digitalWrite(pin, HIGH);
  delay(300);
}

void loop() {
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(temp) || isnan(humi)) {
    return;
  }

  String statusText = "";
  int ledPin = -1;

  // ===== TEMPERATURE LOGIC =====
  if (temp < 13) {
    statusText = "TOO COLD";
    ledPin = LED_GREEN;
  }
  else if (temp < 20) {
    statusText = "COLD";
    ledPin = LED_GREEN;
  }
  else if (temp < 25) {
    statusText = "COOL";
    ledPin = LED_YELLOW;
  }
  else if (temp < 30) {
    statusText = "WARM";
    ledPin = LED_YELLOW;
  }
  else if (temp < 35) {
    statusText = "HOT";
    ledPin = LED_RED;
  }
  else {
    statusText = "TOO HOT";
    ledPin = LED_RED;
  }

  // ===== OLED DISPLAY =====
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 5);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.print("Humi: ");
  display.print(humi);
  display.println(" %");

  display.setTextSize(2);
  display.setCursor(0, 35);
  display.print(statusText);

  display.display();

  // ===== LED BLINK =====
  blinkLED(ledPin);
}



