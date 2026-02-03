/*
THÔNG TIN NHÓM 5
1. Nguyễn Công Hiếu
2.Trần Văn Tiến
3.Phạm Đức Thành Đạt
4.Đậu Thiện Nhật
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Cấu hình OLED (SDA: 13, SCL: 12 theo diagram.json)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Cấu hình DHT22 (Chân 17)
#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Định nghĩa chân LED
const int LED_GREEN = 15;
const int LED_YELLOW = 2;
const int LED_RED = 4;
const int DHT_GND = 21; // Chân GND giả lập cho DHT trong JSON

unsigned long lastUpdate = 0;
unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  
  // Thiết lập chân GND cho DHT22
  pinMode(DHT_GND, OUTPUT);
  digitalWrite(DHT_GND, LOW);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Khởi tạo I2C cho OLED
  Wire.begin(13, 12);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  dht.begin();
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  unsigned long currentMillis = millis();

  // Đọc cảm biến mỗi 2 giây (Thời gian lấy mẫu chuẩn của DHT22)
  if (currentMillis - lastUpdate >= 2000) {
    lastUpdate = currentMillis;
    float temp = dht.readTemperature();
    float humi = dht.readHumidity();

    if (isnan(temp) || isnan(humi)) return;

    String status = "";
    int activeLed = -1;

    // Phân loại ngưỡng nhiệt độ theo yêu cầu
    if (temp < 13) { status = "TOO COLD"; activeLed = LED_GREEN; }
    else if (temp <= 20) { status = "COLD"; activeLed = LED_GREEN; }
    else if (temp <= 25) { status = "COOL"; activeLed = LED_YELLOW; }
    else if (temp <= 30) { status = "WARM"; activeLed = LED_YELLOW; }
    else if (temp <= 35) { status = "HOT"; activeLed = LED_RED; }
    else { status = "TOO HOT"; activeLed = LED_RED; }

    // Hiển thị lên OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Humi: "); display.print(humi); display.println("%");
    
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print("T: "); display.print(temp); display.println(" C");
    
    display.setCursor(0, 45);
    display.setTextSize(1);
    display.print("STATUS: "); display.println(status);
    display.display();

    // Tắt tất cả LED trước khi nhấp nháy LED mục tiêu
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    
    // Lưu LED cần nhấp nháy vào một biến static để loop xử lý nhấp nháy
    static int currentActiveLed;
    currentActiveLed = activeLed;
  }

  // Xử lý nhấp nháy LED (không dùng delay để tránh đứng màn hình)
  static int currentActiveLed; // Cần logic nhấp nháy ở đây
  if (currentMillis - lastBlink >= 500) {
    lastBlink = currentMillis;
    ledState = !ledState;
    
    // Xác định lại LED cần nháy dựa trên nhiệt độ hiện tại
    float t = dht.readTemperature();
    int targetLed = -1;
    if (t <= 20) targetLed = LED_GREEN;
    else if (t <= 30) targetLed = LED_YELLOW;
    else targetLed = LED_RED;

    digitalWrite(targetLed, ledState);
  }
}