/*
THÔNG TIN NHÓM 6
1. Nguyễn Hưng Sang
2. Hồ Tiến Bảo
3. Bùi Hữu Quốc
4. Phạm Thanh Hiếu
5. Lê Văn Tài
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// 1. Cấu hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define I2C_SDA       13  // Chân SDA theo sơ đồ
#define I2C_SCL       12  // Chân SCL theo sơ đồ
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 2. Cấu hình DHT
#define DHTPIN  16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// 3. Cấu hình LED
#define LED_GREEN  15 
#define LED_YELLOW 2
#define LED_RED    4

// SYSTEM TIMER
unsigned long startMillis;
bool systemReady = false;

// LOOP TIMER
unsigned long lastReadMillis = 0;
unsigned long lastDisplayMillis = 0;
unsigned long lastBlinkMillis = 0;

// INTERVAL
const unsigned long READ_INTERVAL = 2000;
const unsigned long DISPLAY_INTERVAL = 2000;
const unsigned long BLINK_INTERVAL = 500;

// DATA
float t = 0, h = 0;
String statusMsg = "";
int activeLed = -1;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  // Khởi tạo LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Tắt hết đèn ban đầu
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // Khởi tạo I2C cho OLED
  Wire.begin(I2C_SDA, I2C_SCL);

  // Khởi tạo màn hình
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Khởi tạo DHT
  dht.begin();
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("Dang khoi tao...");
  display.display();
}

void loop()
{

    // NON-BLOCKING: Đợi hệ thống ổn định và lấy giá trị trong 2 giây 
    if (!systemReady)
    {
        if (millis() - startMillis >= 2000)
        {
            systemReady = true;
            display.clearDisplay();
            display.display();
        }
        return;
    }

    unsigned long now = millis();

    // PHẦN 1: Đọc dữ liệu
    if (now - lastReadMillis >= READ_INTERVAL)
    {
        lastReadMillis = now;

        h = dht.readHumidity();
        t = dht.readTemperature();

        if (isnan(h) || isnan(t))
        {
            statusMsg = "SENSOR ERR";
            activeLed = -1;
            return;
        }
    }

    // PHẦN 2: Xử lý Logic + Chọn LED
    if (t < 13)
    {
        statusMsg = "TOO COLD";
        activeLed = LED_GREEN;
    }
    else if (t <= 20)
    {
        statusMsg = "COLD";
        activeLed = LED_GREEN;
    }
    else if (t <= 25)
    {
        statusMsg = "COOL";
        activeLed = LED_YELLOW;
    }
    else if (t <= 30)
    {
        statusMsg = "WARM";
        activeLed = LED_YELLOW;
    }
    else if (t <= 35)
    {
        statusMsg = "HOT";
        activeLed = LED_RED;
    }
    else
    {
        statusMsg = "TOO HOT";
        activeLed = LED_RED;
    }

    // PHẦN 3: Hiển thị OLED
    if (now - lastDisplayMillis >= DISPLAY_INTERVAL)
    {
        lastDisplayMillis = now;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Temp: ");
        display.print(t, 1);
        display.print(" C");

        display.setCursor(0, 12);
        display.print("Hum : ");
        display.print(h, 1);
        display.print(" %");

        display.drawLine(0, 25, 128, 25, WHITE);

        display.setTextSize(2);
        int16_t x1, y1;
        uint16_t w, h_text;
        display.getTextBounds(statusMsg, 0, 0, &x1, &y1, &w, &h_text);
        display.setCursor((SCREEN_WIDTH - w) / 2, 35);
        display.println(statusMsg);

        display.display();
    }

    // PHẦN 4: LED nhấp nháy 
    if (now - lastBlinkMillis >= BLINK_INTERVAL)
    {
        lastBlinkMillis = now;
        ledState = !ledState;

        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);

        if (activeLed != -1 && ledState)
        {
            digitalWrite(activeLed, HIGH);
        }
    }
}
