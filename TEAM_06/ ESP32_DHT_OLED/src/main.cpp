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

// --- CẤU HÌNH PHẦN CỨNG (Theo diagram.json) ---
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
// Lưu ý: Trong sơ đồ Wokwi, LED Cyan nối chân 15, tôi gán nó là GREEN theo yêu cầu bài
#define LED_GREEN  15 
#define LED_YELLOW 2
#define LED_RED    4

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
  delay(2000); // Đợi cảm biến ổn định
}

void loop() {
  // 1. Đọc dữ liệu
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi đọc cảm biến
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi doc DHT!"));
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Sensor Error!");
    display.display();
    return;
  }

}