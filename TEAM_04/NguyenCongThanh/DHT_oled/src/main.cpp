#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Cấu hình chân dựa trên thiết kế của Thành
#define DHTPIN 16      // DHT22 trên chân 16
#define DHTTYPE DHT22
#define OLED_SDA 13    // OLED SDA trên chân 13
#define OLED_SCL 12    // OLED SCL trên chân 12

#define LED_RED 4      // LED Đỏ trên chân 4
#define LED_YELLOW 2   // LED Vàng trên chân 2
#define LED_CYAN 15    // LED Xanh (Cyan/Green) trên chân 15

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  // Khởi tạo Serial ở tốc độ 115200 baud
  Serial.begin(115200);
  Serial.println("--- HE THONG GIAM SAT MOI TRUONG ---");
  
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Loi: Khong tim thay OLED!"));
    for(;;);
  }
  
  dht.begin();
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_CYAN, OUTPUT);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Loi: Khong doc duoc du lieu tu DHT22!");
    return;
  }

  String label = "";
  int activeLED = -1;

  // Logic phân loại ngưỡng nhiệt độ theo yêu cầu của Thành
  if (temp < 13.0) { label = "❄️ TOO COLD"; activeLED = LED_CYAN; }
  else if (temp < 20.0) { label = "☔ COLD"; activeLED = LED_CYAN; }
  else if (temp < 25.0) { label = "☁️ COOL"; activeLED = LED_YELLOW; }
  else if (temp < 30.0) { label = "☀️ WARM"; activeLED = LED_YELLOW; }
  else if (temp < 35.0) { label = "🌞 HOT"; activeLED = LED_RED; }
  else { label = "🔥 TOO HOT"; activeLED = LED_RED; }

  // --- XUẤT DỮ LIỆU RA TERMINAL ---
  Serial.println("------------------------------------");
  Serial.print("Nhiet do: "); Serial.print(temp, 1); Serial.println(" *C");
  Serial.print("Do am:    "); Serial.print(hum, 1); Serial.println(" %");
  Serial.print("Trang thai: "); Serial.println(label);

  // --- HIỂN THỊ OLED ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Status: "); display.println(label);
  
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print(temp, 1); display.print(" C");
  
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Humidity: "); display.print(hum, 1); display.print("%");
  display.display();

  // --- HIEU UNG NHAP NHAY LED ---
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_CYAN, LOW);

  digitalWrite(activeLED, HIGH);
  delay(500);
  digitalWrite(activeLED, LOW);
  delay(500);
}

