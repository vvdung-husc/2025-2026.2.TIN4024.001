#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"
#include "arduino.h"
// --- CẤU HÌNH CHÂN ---
#define DHTPIN 4       // Chân SDA của DHT22 nối D4
#define DHTTYPE DHT22  // Loại cảm biến
#define SD_CS_PIN 5    // Chân CS của MicroSD nối D5

DHT dht(DHTPIN, DHTTYPE);

// --- CẤU HÌNH WI-FI ẢO CỦA WOKWI ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- CẤU HÌNH THINGSPEAK ---
// (Đây là API Key mẫu, bạn có thể chạy thử luôn)
String serverName = "http://api.thingspeak.com/update?api_key=SBLYD2JAYF21X9HW";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== TRAM GHI NHAT KY MOI TRUONG ===");

  // 1. Khởi tạo cảm biến DHT22
  Serial.println("[1] Dang khoi tao DHT22...");
  dht.begin();

  // 2. Khởi tạo thẻ nhớ MicroSD
  Serial.print("[2] Dang khoi tao the SD... ");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("LOI: Khong tim thay the SD!");
  } else {
    Serial.println("OK!");
    // Tạo file CSV nếu chưa có
    File dataFile = SD.open("/nhatky.csv", FILE_APPEND);
    if (dataFile) {
      if(dataFile.size() == 0) dataFile.println("Nhiet do (*C), Do am (%)");
      dataFile.close();
    }
  }

  // 3. Kết nối Wi-Fi
  Serial.print("[3] Dang ket noi Wi-Fi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n -> Ket noi Wi-Fi thanh cong!");
  Serial.println("-------------------------------------------");
}

void loop() {
  // Đọc dữ liệu
  float do_am = dht.readHumidity();
  float nhiet_do = dht.readTemperature();

  if (isnan(do_am) || isnan(nhiet_do)) {
    Serial.println("LOI: Khong doc duoc DHT22!");
    delay(2000);
    return;
  }

  Serial.printf("Nhiet do: %.1f *C  |  Do am: %.1f %%\n", nhiet_do, do_am);

  // --- LƯU VÀO THẺ NHỚ ---
  File dataFile = SD.open("/nhatky.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%.2f,%.2f\n", nhiet_do, do_am);
    dataFile.close();
    Serial.println(" -> Luu the SD: Thanh cong");
  } else {
    Serial.println(" -> Luu the SD: Loi!");
  }

  // --- ĐẨY LÊN THINGSPEAK ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverName + "&field1=" + String(nhiet_do) + "&field2=" + String(do_am);
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.printf(" -> ThingSpeak: Thanh cong (Ma: %d)\n", httpResponseCode);
    } else {
      Serial.printf(" -> ThingSpeak: Loi (Ma: %d)\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println(" -> ThingSpeak: Mat mang!");
  }

  Serial.println("-------------------------------------------");
  delay(15000); // Đợi 15 giây (Quy định của ThingSpeak)
}