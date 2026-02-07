/*
THÔNG TIN NHÓM 4
1. Lê Văn Thuần
2. Hoàng Văn Quốc Thịnh
3. Nguyễn Công Thành
4. Trần Khánh
*/

// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <DHT.h>

// // ================= OLED =================
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// // ================= DHT =================
// #define DHTPIN 16
// #define DHTTYPE DHT22
// DHT dht(DHTPIN, DHTTYPE);

// // ================= LED =================
// #define LED_GREEN 15
// #define LED_YELLOW 2
// #define LED_RED 4

// unsigned long timerDHT = 0;
// unsigned long timerBlink = 0;
// bool ledState = false;
// int currentLed = -1;

// // ================= SETUP =================
// void setup() {
//   Serial.begin(115200);

//   pinMode(LED_GREEN, OUTPUT);
//   pinMode(LED_YELLOW, OUTPUT);
//   pinMode(LED_RED, OUTPUT);

//   dht.begin();
//   Wire.begin(13, 12);

//   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//     Serial.println("OLED FAILED");
//     while (1);
//   }

//   display.clearDisplay();
//   display.setTextSize(2);
//   display.setTextColor(SSD1306_WHITE); // ⭐ BẮT BUỘC
//   display.setCursor(10, 20);
//   display.print("IOT START");
//   display.display();
//   delay(1500);
// }

// // ================= LED BLINK =================
// void blinkLED() {
//   if (currentLed == -1) return;

//   if (millis() - timerBlink >= 500) {
//     timerBlink = millis();
//     ledState = !ledState;

//     digitalWrite(LED_GREEN, LOW);
//     digitalWrite(LED_YELLOW, LOW);
//     digitalWrite(LED_RED, LOW);

//     digitalWrite(currentLed, ledState);
//   }
// }

// // ================= LOOP =================
// void loop() {

//   // ===== LED nhấp nháy liên tục =====
//   blinkLED();

//   // ===== Đọc DHT mỗi 2 giây =====
//   if (millis() - timerDHT >= 2000) {
//     timerDHT = millis();

//     float temp = dht.readTemperature();
//     float hum  = dht.readHumidity();

//     if (isnan(temp) || isnan(hum)) {
//       Serial.println("DHT ERROR");
//       return;
//     }

//     String status = "";

//     if (temp < 13) {
//       status = "TOO COLD";
//       currentLed = LED_GREEN;
//     } else if (temp < 20) {
//       status = "COLD";
//       currentLed = LED_GREEN;
//     } else if (temp < 25) {
//       status = "COOL";
//       currentLed = LED_YELLOW;
//     } else if (temp < 30) {
//       status = "WARM";
//       currentLed = LED_YELLOW;
//     } else if (temp < 35) {
//       status = "HOT";
//       currentLed = LED_RED;
//     } else {
//       status = "TOO HOT";
//       currentLed = LED_RED;
//     }

//     // ===== OLED =====
//     display.clearDisplay();
//     display.setTextColor(SSD1306_WHITE);

//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.print("Temp: ");
//     display.print(temp);
//     display.print(" C");

//     display.setCursor(0, 14);
//     display.print("Humi: ");
//     display.print(hum);
//     display.print(" %");

//     display.setCursor(0, 28);
//     display.print("Status:");

//     display.setTextSize(2);
//     display.setCursor(0, 44);
//     display.print(status);

//     display.display();
//   }
// }


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= DHT =================
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ================= LED =================
#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4

unsigned long timerDHT = 0;
unsigned long timerBlink = 0;
bool ledState = false;
int currentLed = -1;

// ================= SETUP =================
void setup() {
  Serial.begin(115200); // Khởi tạo Terminal

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  dht.begin();
  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print("IOT START");
  display.display();
  delay(1500);
}

// ================= LED BLINK =================
void blinkLED() {
  if (currentLed == -1) return;

  if (millis() - timerBlink >= 500) {
    timerBlink = millis();
    ledState = !ledState;

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    digitalWrite(currentLed, ledState);
  }
}

// ================= LOOP =================
void loop() {
  blinkLED();

  if (millis() - timerDHT >= 2000) {
    timerDHT = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("DHT ERROR: Khong doc duoc cam bien!");
      return;
    }

    String status = "";
    String emoji = "";

    // Phân loại logic theo yêu cầu
    if (temp < 13) {
      status = "TOO COLD";
      emoji = "❄️";
      currentLed = LED_GREEN;
    } else if (temp < 20) {
      status = "COLD";
      emoji = "☔";
      currentLed = LED_GREEN;
    } else if (temp < 25) {
      status = "COOL";
      emoji = "☁️";
      currentLed = LED_YELLOW;
    } else if (temp < 30) {
      status = "WARM";
      emoji = "☀️";
      currentLed = LED_YELLOW;
    } else if (temp < 35) {
      status = "HOT";
      emoji = "🌞";
      currentLed = LED_RED;
    } else {
      status = "TOO HOT";
      emoji = "🔥";
      currentLed = LED_RED;
    }

    // ===== HIỂN THỊ TERMINAL =====
    Serial.println("-------------------------------");
    Serial.print("Nhiet do: "); Serial.print(temp); Serial.println(" *C");
    Serial.print("Do am:    "); Serial.print(hum);  Serial.println(" %");
    Serial.print("Trang thai: "); Serial.print(status); Serial.print(" "); Serial.println(emoji);
    Serial.println("-------------------------------");

    // ===== HIỂN THỊ OLED =====
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Temp: "); display.print(temp); display.print(" C");

    display.setCursor(0, 14);
    display.print("Humi: "); display.print(hum); display.print(" %");

    display.setCursor(0, 28);
    display.print("Status:");

    display.setTextSize(2);
    display.setCursor(0, 44);
    display.print(status);

    display.display();
  }
}