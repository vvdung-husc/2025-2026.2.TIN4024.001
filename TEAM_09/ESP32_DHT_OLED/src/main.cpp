/*
THÔNG TIN NHÓM 9
1. Nguyễn Hoàng Huyền Tran
2. 
3. 
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- CẬP NHẬT CẤU HÌNH CHÂN MỚI ---
#define DHTPIN 16      // Cảm biến DHT22 giờ nối chân 16
#define DHTTYPE DHT22  

// Cấu hình chân đèn LED mới
#define LED_GREEN  15  // Đèn Xanh nối chân 15
#define LED_YELLOW 2   // Đèn Vàng nối chân 2
#define LED_RED    4   // Đèn Đỏ nối chân 4

// Cấu hình màn hình OLED (Chân I2C Tùy chỉnh)
#define OLED_SDA 13    // Chân SDA mới
#define OLED_SCL 12    // Chân SCL mới
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  
  // 1. Khởi động các chân đèn
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // 2. Cài đặt chân I2C riêng cho OLED (Quan trọng!)
  // Vì Công chúa đổi chân nên phải dùng lệnh này OLED mới lên hình
  Wire.begin(OLED_SDA, OLED_SCL);

  // 3. Khởi động màn hình và cảm biến
  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khong tim thay OLED!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
}

// Hàm tắt hết đèn
void resetLeds() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

void loop() {
  // 1. Đọc dữ liệu
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Loi doc DHT22!");
    return;
  }

  // 2. Xử lý logic bật đèn theo nhiệt độ
  String status = "";
  int blinkLed = -1;

  if (t < 13) {
    status = "TOO COLD";
    blinkLed = LED_GREEN;
  } else if (t >= 13 && t < 20) {
    status = "COLD";
    blinkLed = LED_GREEN;
  } else if (t >= 20 && t < 25) {
    status = "COOL";
    blinkLed = LED_YELLOW;
  } else if (t >= 25 && t < 30) {
    status = "WARM";
    blinkLed = LED_YELLOW;
  } else if (t >= 30 && t < 35) {
    status = "HOT";
    blinkLed = LED_RED;
  } else { 
    status = "TOO HOT";
    blinkLed = LED_RED;
  }

  // 3. Hiển thị thông tin lên OLED
  // 3. Hiển thị thông tin lên OLED (GIỐNG HỆT ẢNH 2)
  display.clearDisplay(); 

  // --- DÒNG 1: Tiêu đề + Trạng thái (COOL/HOT) ---
  display.setTextSize(1);      // Chữ nhỏ
  display.setCursor(0, 0);     // Góc trên cùng
  display.print("Temperature: "); 
  display.print(status);       // In chữ COOL/HOT ngay bên cạnh

  // --- DÒNG 2: Số Nhiệt độ (Xuống hàng) ---
  display.setTextSize(2);      // Chữ to
  display.setCursor(0, 12);    // Xuống dòng một chút
  display.print(t);
  display.cp437(true);         // Kích hoạt bảng ký tự đặc biệt
  display.write(167);          // In dấu độ (°) cho xịn
  display.print("C");

  // --- DÒNG 3: Tiêu đề Độ ẩm ---
  display.setTextSize(1);      // Chữ nhỏ
  display.setCursor(0, 35);    // Cách ra một đoạn
  display.print("Humidity:");

  // --- DÒNG 4: Số Độ ẩm + % ---
  display.setTextSize(2);      // Chữ to
  display.setCursor(0, 47);    // Dòng cuối cùng
  display.print(h);
  display.print(" %");         // Thêm dấu % theo ý Công chúa

  display.display();

  // 4. Hiệu ứng nhấp nháy đèn
  resetLeds(); 
  if (blinkLed != -1) {
    digitalWrite(blinkLed, HIGH); 
    delay(500);
    digitalWrite(blinkLed, LOW);  
    delay(500);
  }
}