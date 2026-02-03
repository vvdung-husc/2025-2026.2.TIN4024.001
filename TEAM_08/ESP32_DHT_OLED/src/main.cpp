/*
THÔNG TIN NHÓM 08
1. Nguyễn Ngọc Thành Tài
2. Trần Văn Quốc Khánh 
3. ...
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẤU HÌNH OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- CẤU HÌNH DHT22 ---
#define DHTPIN 15     // Chân kết nối cảm biến DHT
#define DHTTYPE DHT22 // Loại cảm biến là DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- CẤU HÌNH LED ---
#define LED_GREEN  5
#define LED_YELLOW 18
#define LED_RED    19

void setup() {
  Serial.begin(115200);

  // 1. Khởi động LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // 2. Khởi động DHT
  dht.begin();

  // 3. Khởi động OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Khong tim thay OLED"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
}

// Hàm nhấp nháy LED
void blinkLED(int pin) {
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
  delay(200);
}

void loop() {
  // Đợi 2 giây giữa các lần đo
  delay(1000);

  // Đọc dữ liệu
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi cảm biến
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi doc DHT!"));
    return;
  }

  // --- XỬ LÝ LOGIC HIỂN THỊ VÀ ĐÈN ---
  String statusMsg = "";
  int activeLed = -1; // -1: Không đèn nào, hoặc định nghĩa logic khác

  // Tắt tất cả đèn trước khi bật đèn mới
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  /* Dựa trên bảng ngưỡng nhiệt độ (Yêu cầu đề bài)
     < 13: TOO COLD (Green)
     13 - 20: COLD (Green)
     20 - 25: COOL (Yellow)
     25 - 30: WARM (Yellow)
     30 - 35: HOT (Red)
     > 35: TOO HOT (Red)
  */
  if (t < 13) {
    statusMsg = "TOO COLD";
    activeLed = LED_GREEN;
  } else if (t >= 13 && t < 20) {
    statusMsg = "COLD";
    activeLed = LED_GREEN;
  } else if (t >= 20 && t < 25) {
    statusMsg = "COOL";
    activeLed = LED_YELLOW;
  } else if (t >= 25 && t < 30) {
    statusMsg = "WARM";
    activeLed = LED_YELLOW;
  } else if (t >= 30 && t <= 35) {
    statusMsg = "HOT";
    activeLed = LED_RED;
  } else {
    statusMsg = "TOO HOT";
    activeLed = LED_RED;
  }

  // --- HIỂN THỊ LÊN OLED ---
  display.clearDisplay();
  
  // Dòng 1: Nhiệt độ
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Temperature:");
  display.setTextSize(2);
  display.print(t);
  display.println(" C");

  // Dòng 2: Độ ẩm
  display.setTextSize(1);
  display.print("Humidity: ");
  display.print(h);
  display.println(" %");

  // Dòng 3: Trạng thái (Ví dụ: HOT, COLD...)
  display.setCursor(0, 45); // Xuống dòng dưới
  display.setTextSize(2);
  display.println(statusMsg);

  display.display();

  // --- NHẤP NHÁY ĐÈN TƯƠNG ỨNG ---
  if (activeLed != -1) {
    blinkLED(activeLed);
  }
}