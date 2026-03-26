#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DHTPIN D3
#define DHTTYPE DHT22
#define LED_PIN D4 

// Giả định các chân cảm biến thêm vào (Bạn có thể thay đổi tùy thực tế)
#define GAS_PIN A0
#define PIR_PIN D5
#define LIGHT_PIN D6

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);

  dht.begin();
  Wire.begin(D2, D1);

  if (!display.begin(0x3C, true)) {
    for (;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1); // Kích thước chữ nhỏ để vừa 6 hàng
  display.setTextColor(SH110X_WHITE);
}

void loop() {
  // 1. Đọc dữ liệu
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int gasVal = analogRead(GAS_PIN);
  bool motion = digitalRead(PIR_PIN);
  bool lightStatus = digitalRead(LIGHT_PIN);

  // 2. Cập nhật màn hình OLED
  display.clearDisplay();
  
// --- IN RA TERMINAL (SERIAL MONITOR) ---
  Serial.println("=============================");
  Serial.printf("Nhiet do: %.1f C\n", isnan(temp) ? 0 : temp);
  Serial.printf("Do am: %.1f %%\n", isnan(hum) ? 0 : hum);
  Serial.printf("Khi Gas: %d\n", gasVal);
  Serial.printf("Trang thai Den: %s\n", lightStatus ? "BAT" : "TAT");
  Serial.printf("Chuyen dong: %s\n", motion ? "CO!" : "KHONG");

  // Hàng 1: Tên (Cố định)
  display.setCursor(0, 0);
  display.println("Le Van Thuan");
  display.drawLine(0, 9, 120, 9, SH110X_WHITE); // Đường kẻ phân cách nhẹ

  // Hàng 2: Nhiệt độ
  display.setCursor(0, 12);
  display.printf("Nhiet do: %.1f C", isnan(temp) ? 0 : temp);

  // Hàng 3: Độ ẩm
  display.setCursor(0, 23);
  display.printf("Do am:    %.1f %%", isnan(hum) ? 0 : hum);

  // Hàng 4: Khí Gas
  display.setCursor(0, 34);
  display.printf("Khi Gas:  %d", gasVal);

  // Hàng 5: Trạng thái Đèn
  display.setCursor(0, 45);
  display.printf("Trang thai Den: %s", lightStatus ? "BAT" : "TAT");

  // Hàng 6: Chuyển động
  display.setCursor(0, 56);
  display.printf("Chuyen dong: %s", motion ? "CO!" : "KHONG");

  display.display();

  // 3. Blink LED & Delay
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(1500);
}