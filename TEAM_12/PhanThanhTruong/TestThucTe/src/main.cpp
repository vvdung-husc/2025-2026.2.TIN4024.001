#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// Cấu hình màn hình OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Cấu hình cảm biến DHT22
// Lưu ý: Đảm bảo chân Data của con DHT màu trắng đang cắm vào D5 nhé!
#define DHTPIN D3 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo cảm biến
  dht.begin();

  // Khởi tạo giao tiếp I2C và màn hình
  Wire.begin(D2, D1);   // SDA, SCL
  u8g2.begin();
}

void loop() {
  // 1. Đọc dữ liệu từ cảm biến
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 2. Bắt đầu vẽ lên màn hình
  u8g2.clearBuffer(); // Dọn dẹp bộ nhớ đệm cũ

  // Kiểm tra xem cảm biến có bị lỗi/lỏng dây không
  if (isnan(h) || isnan(t)) {
    u8g2.setFont(u8g2_font_ncenB10_tr); // Font chữ nhỏ hơn chút
    u8g2.drawStr(10, 30, "Loi doc DHT!");
  } else {
    // Cài đặt font chữ to, in đậm (giống code của bạn)
    u8g2.setFont(u8g2_font_ncenB14_tr);

    // In Nhiệt độ ở nửa trên màn hình
    u8g2.setCursor(0, 25); // Tọa độ (x=0, y=25)
    u8g2.print("T: ");
    u8g2.print(t, 1);      // Lấy 1 chữ số thập phân
    u8g2.print(" C");

    // In Độ ẩm ở nửa dưới màn hình
    u8g2.setCursor(0, 55); // Tọa độ (x=0, y=55)
    u8g2.print("H: ");
    u8g2.print(h, 1);
    u8g2.print(" %");
  }

  // 3. Đẩy tất cả dữ liệu vừa vẽ lên màn hình hiển thị
  u8g2.sendBuffer(); 

  // Đợi 2 giây trước khi cập nhật nhiệt độ mới
  delay(2000); 
}