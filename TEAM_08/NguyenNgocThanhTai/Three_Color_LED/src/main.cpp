#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CẤU HÌNH MÀN HÌNH OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 13
#define OLED_SCL 12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CẤU HÌNH CHÂN ĐÈN LED (Theo sơ đồ) ---
#define LED_CYAN    15  // Đèn màu xanh lơ
#define LED_YELLOW  2   // Đèn màu vàng
#define LED_MAGENTA 4   // Đèn màu tím

void setup() {
  Serial.begin(115200);

  // 1. Cấu hình các chân LED là Output
  pinMode(LED_CYAN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_MAGENTA, OUTPUT);

  // 2. Khởi động màn hình OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED khong tim thay!"));
    for(;;);
  }

  // 3. Hiển thị chữ "IOT Welcome!"
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2); 
  
  display.setCursor(45, 10); 
  display.println("IOT");
  
  display.setCursor(15, 35);
  display.println("Welcome!");
  
  display.display();
}

void loop() {
  // --- HIỆU ỨNG ĐÈN GIAO THÔNG (Chớp tắt từng đèn) ---

  // Bật đèn Cyan, tắt 2 đèn kia
  digitalWrite(LED_CYAN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_MAGENTA, LOW);
  delay(1000); // Đợi 1 giây

  // Bật đèn Vàng, tắt 2 đèn kia
  digitalWrite(LED_CYAN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_MAGENTA, LOW);
  delay(1000); // Đợi 1 giây

  // Bật đèn Tím, tắt 2 đèn kia
  digitalWrite(LED_CYAN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_MAGENTA, HIGH);
  delay(1000); // Đợi 1 giây
}