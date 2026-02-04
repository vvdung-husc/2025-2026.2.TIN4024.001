/*
THÔNG TIN NHÓM 5
1. Nguyễn Công Hiếu
2.Trần Văn Tiến
3.Phạm Đức Thành Đạt
4.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Cấu hình DHT22 (dựa trên diagram.json)
#define DHTPIN 16
#define DHTTYPE DHT22

// Cấu hình OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Cấu hình chân LED (dựa trên diagram.json)
#define LED_RED 4      // LED1 - đỏ
#define LED_GREEN 15   // LED2 - xanh lá (cyan)
#define LED_YELLOW 2   // LED3 - vàng

// Khởi tạo cảm biến DHT
DHT dht(DHTPIN, DHTTYPE);

// Khởi tạo màn hình OLED (SDA: GPIO 13, SCL: GPIO 12)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Biến cho LED nhấp nháy
unsigned long previousMillis = 0;
bool ledState = false;
const long blinkInterval = 500; // Nhấp nháy mỗi 500ms

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo I2C với các chân tùy chỉnh (SDA: 13, SCL: 12)
  Wire.begin(13, 12);
  
  // Khởi tạo cảm biến DHT
  dht.begin();
  
  // Khởi tạo màn hình OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Lỗi khởi tạo SSD1306!"));
    for(;;); // Dừng chương trình nếu OLED không hoạt động
  }
  
  // Hiển thị thông báo khởi động
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP32 DHT OLED"));
  display.println(F("TEAM 5"));
  display.println();
  display.println(F("Dang khoi tao..."));
  display.display();
  delay(2000);
  
  // Khởi tạo các chân LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  
  // Tắt tất cả LED
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  
  Serial.println(F("He thong san sang!"));
}

void loop() {
  // Đọc nhiệt độ và độ ẩm
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Kiểm tra dữ liệu có hợp lệ không
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Loi doc cam bien DHT!"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("LOI CAM BIEN DHT!"));
    display.display();
    delay(2000);
    return;
  }
  
  // Xác định trạng thái và LED theo ngưỡng nhiệt độ
  String status = "";
  int activeLED = -1;
  
  if (temperature < 13) {
    status = "TOO COLD";
    activeLED = LED_GREEN;
  } else if (temperature >= 13 && temperature < 20) {
    status = "COLD";
    activeLED = LED_GREEN;
  } else if (temperature >= 20 && temperature < 25) {
    status = "COOL";
    activeLED = LED_YELLOW;
  } else if (temperature >= 25 && temperature < 30) {
    status = "WARM";
    activeLED = LED_YELLOW;
  } else if (temperature >= 30 && temperature < 35) {
    status = "HOT";
    activeLED = LED_RED;
  } else { // temperature >= 35
    status = "TOO HOT";
    activeLED = LED_RED;
  }
  
  // Hiển thị thông tin lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("ESP32 DHT OLED"));
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  
  // Hiển thị nhiệt độ
  display.setCursor(0, 15);
  display.print(F("Nhiet do: "));
  display.print(temperature, 1);
  display.println(F(" C"));
  
  // Hiển thị độ ẩm
  display.print(F("Do am   : "));
  display.print(humidity, 1);
  display.println(F(" %"));
  
  display.drawLine(0, 38, SCREEN_WIDTH, 38, SSD1306_WHITE);
  
  // Hiển thị trạng thái với font lớn hơn
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.println(status);
  
  display.display();
  
  // Điều khiển LED nhấp nháy
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    
    // Tắt tất cả LED
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    
    // Bật LED tương ứng với trạng thái
    if (activeLED != -1) {
      digitalWrite(activeLED, ledState ? HIGH : LOW);
    }
  }
  
  // In thông tin ra Serial Monitor
  Serial.print(F("Nhiet do: "));
  Serial.print(temperature);
  Serial.print(F(" C | Do am: "));
  Serial.print(humidity);
  Serial.print(F(" % | Trang thai: "));
  Serial.println(status);
  
  delay(100); // Delay ngắn để giảm tải CPU
}

