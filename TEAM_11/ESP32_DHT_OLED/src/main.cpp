/* Thành viên hóm 11:
 * 1. Phạm Mạnh Lân - Tích hợp
 * 2. Nguyễn Đức Tín - Xử lý Cảm biến
 * 3. Đào Văn Lợi - Xử lý Hiển thị OLED
 * 4. Võ Thanh Toàn - Logic Cảnh báo & LED
 * 5. Phan Minh Nhật Khoa - Thiết kế Diagram & Tài liệu
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- ĐỊNH NGHĨA CHÂN ---
// định nghĩa chân
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_CYAN   15
#define LED_YELLOW 2
#define LED_RED    4
#define OLED_SDA   13
#define OLED_SCL   12
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// --- KHỞI TẠO ĐỐI TƯỢNG ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

// --- BIẾN QUẢN LÝ THỜI GIAN (MILLIS) ---
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000; // Đọc cảm biến mỗi 2 giây

unsigned long lastLedBlink = 0;
const unsigned long blinkInterval = 300; // Nháy LED mỗi 300ms
bool ledState = false;

// --- BIẾN LƯU TRỮ DỮ LIỆU ---
float h = 0, t = 0;
String status = "STARTING...";
int activeLed = -1;

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_CYAN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Khởi tạo I2C với chân SDA 13 và SCL 12
  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }

  dht.begin();
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  Serial.println("He thong da san sang!");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. NHÁNH ĐỌC CẢM BIẾN (MỖI 2 GIÂY) ---
  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    h = dht.readHumidity();
    t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Loi doc cam bien!");
      status = "SENSOR ERROR";
      activeLed = -1;
    } else {
      // Logic phân loại nhiệt độ
      if (t < 20) { status = "COLD"; activeLed = LED_CYAN; }
      else if (t < 30) { status = "WARM"; activeLed = LED_YELLOW; }
      else { status = "HOT"; activeLed = LED_RED; }

      // Cập nhật màn hình OLED
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Temp: "); display.print(t, 1); display.print(" C");
      display.setCursor(0, 15);
      display.print("Hum:  "); display.print(h, 1); display.print(" %");
      
      display.setTextSize(2);
      display.setCursor(0, 40);
      display.print(status);
      display.display();

      // In ra Serial để theo dõi
      Serial.printf("T: %.1f C | H: %.1f %% | Status: %s\n", t, h, status.c_str());
    }
  }

  // --- 2. NHÁNH NHÁY LED (MỖI 300MS) ---
  if (currentMillis - lastLedBlink >= blinkInterval) {
    lastLedBlink = currentMillis;
    ledState = !ledState; // Đảo trạng thái

    // Tắt tất cả các LED
    digitalWrite(LED_CYAN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    // Chỉ nháy LED đang hoạt động
    if (activeLed != -1) {
      digitalWrite(activeLed, ledState);
    }
  }
}