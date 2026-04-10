#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Arduino.h>
// --- CẤU HÌNH CẢM BIẾN DHT22 ---
#define DHTPIN 15      // Chân DATA của DHT nối GPIO 15
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

// --- CẤU HÌNH MÀN HÌNH OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CẤU HÌNH WI-FI ẢO CỦA WOKWI ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- CẤU HÌNH THINGSPEAK ---
// Thay YOUR_API_KEY bằng API Key thực của bạn nếu có
String serverName = "http://api.thingspeak.com/update?api_key=D777ITHHAXNEVQ25";

void setup() {
  Serial.begin(115200); 
  delay(1000);
  Serial.println("\n=== TRAM THOI TIET MINI ESP32 ===");

  // 1. Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("LOI: Khong tim thay OLED!"));
    for(;;); 
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Khoi tao he thong...");
  display.display();
  delay(1000);

  // 2. Khởi tạo DHT22
  Serial.println("Dang khoi tao DHT22...");
  dht.begin();

  // 3. Kết nối Wi-Fi
  Serial.print("Dang ket noi Wi-Fi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n-> Ket noi Wi-Fi thanh cong!");
}

void loop() {
  // Đọc dữ liệu từ cảm biến
  float do_am = dht.readHumidity();
  float nhiet_do = dht.readTemperature();

  if (isnan(do_am) || isnan(nhiet_do)) {
    Serial.println("LOI: Khong doc duoc DHT22!");
    delay(2000);
    return;
  }

  // In ra Serial Monitor
  Serial.printf("Nhiet do: %.1f *C  |  Do am: %.1f %%\n", nhiet_do, do_am);

  // Hiển thị lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("TRAM THOI TIET MINI");
  
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("T: "); display.print(nhiet_do, 1); display.println(" C");
  display.setCursor(0, 40);
  display.print("H: "); display.print(do_am, 1); display.println(" %");
  display.display();

  // Đẩy dữ liệu lên ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverName + "&field1=" + String(nhiet_do) + "&field2=" + String(do_am);
    
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.printf("-> ThingSpeak: Thanh cong (Ma: %d)\n", httpResponseCode);
    } else {
      Serial.printf("-> ThingSpeak: Loi (Ma: %d)\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("-> Mat mang Wi-Fi!");
  }

  Serial.println("-------------------------------------------");
  delay(15000); // Chờ 15s trước lần đọc tiếp theo
}