/*
THÔNG TIN NHÓM 07
1. Nguyễn Văn Phong
2. Bùi Khắc Hiếu
3. Hồ Văn Diễn
4. Nguyễn Văn Tiến Đạt
5. Lương Thanh Ngọc Như
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẤU HÌNH CHÂN (PINOUT) ---
#define PIN_DHT      15  // DHT22 Data

// Lưu ý: Sơ đồ mạch bạn dùng LED Blue ở chân 5, 
// nhưng bảng yêu cầu gọi là Green. Tôi sẽ dùng chân 5 cho mức "Lạnh".
#define PIN_LED_GREEN  5   
#define PIN_LED_YELLOW 18  
#define PIN_LED_RED    19  

// --- CẤU HÌNH OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CẤU HÌNH SENSOR ---
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Cấu hình LED
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // Khởi động Sensor & OLED
  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED Init Failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("HE THONG BAT DAU");
  display.display();
  delay(2000);
}

void loop() {
  // 1. Đọc dữ liệu
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Kiểm tra lỗi đọc
  if (isnan(t) || isnan(h)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  // 2. Xác định Trạng thái, Chữ hiển thị và Đèn cần nháy
  String statusText = "";
  int activeLed = -1; // -1 nghĩa là không đèn nào sáng

  // --- LOGIC THEO BẢNG YÊU CẦU ---
  if (t < 13.0) {
    statusText = "TOO COLD";
    activeLed = PIN_LED_GREEN;
  } 
  else if (t >= 13.0 && t < 20.0) {
    statusText = "COLD";
    activeLed = PIN_LED_GREEN;
  }
  else if (t >= 20.0 && t < 25.0) {
    statusText = "COOL";
    activeLed = PIN_LED_YELLOW;
  }
  else if (t >= 25.0 && t < 30.0) {
    statusText = "WARM";
    activeLed = PIN_LED_YELLOW;
  }
  else if (t >= 30.0 && t <= 35.0) {
    statusText = "HOT";
    activeLed = PIN_LED_RED;
  }
  else { // > 35 độ
    statusText = "TOO HOT";
    activeLed = PIN_LED_RED;
  }

  // 3. Hiển thị thông tin lên OLED
  display.clearDisplay();

  // Dòng 1: Nhiệt độ & Độ ẩm
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(t, 1); display.print(" C");
  display.setCursor(0, 10);
  display.print("Hum:  "); display.print(h, 1); display.print(" %");

  // Dòng 2: Đường kẻ ngang
  display.drawLine(0, 25, 128, 25, WHITE);

  // Dòng 3: Trạng thái (In Chữ To)
  display.setTextSize(2);
  
  // Căn giữa chữ (tương đối)
  int16_t x1, y1;
  uint16_t w, h_text;
  display.getTextBounds(statusText, 0, 0, &x1, &y1, &w, &h_text);
  display.setCursor((128 - w) / 2, 35); // Căn giữa màn hình
  
  display.print(statusText);
  display.display();

  // 4. Xử lý NHẤP NHÁY đèn (Blinking)
  // Tắt tất cả đèn trước
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  // Bật đèn cần sáng
  if (activeLed != -1) {
    digitalWrite(activeLed, HIGH);
  }
  
  delay(500); // Sáng trong 0.5s

  // Tắt đèn (tạo hiệu ứng nháy)
  if (activeLed != -1) {
    digitalWrite(activeLed, LOW);
  }
  
  delay(500); // Tắt trong 0.5s -> Tổng chu kỳ 1 giây
}



