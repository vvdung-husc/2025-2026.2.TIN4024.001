#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "DHT.h"

// --- CHỈNH LẠI CHÂN CẢM BIẾN THEO BO MẠCH CỦA TRƯỜNG ---
// Theo sơ đồ, DHT nối vào D3 (tương đương GPIO 0 trong code)
#define DHTPIN 0      

// Cảm biến màu trắng thường là DHT22. 
// (Lưu ý: Nếu nạp xong vẫn báo lỗi 'nan', Công chúa hãy đổi chữ DHT22 thành DHT11 nhé)
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);

// --- CẤU HÌNH LẠI MÀN HÌNH ---
// Đổi U8G2_R0 thành U8G2_R2 để lật ngược màn hình lại 180 độ cho đúng chiều
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);

  // Khởi động cảm biến và màn hình
  dht.begin();
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); 
}

void loop() {
  delay(2000); // Đợi 2 giây để cảm biến lấy hơi

  // Đọc thông tin nhiệt độ và độ ẩm
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // In ra bảng đen Serial Monitor dưới máy tính
  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.print(" *C   |   Do am: ");
  Serial.println(h);

  // --- VẼ LÊN MÀN HÌNH OLED TRÊN MẠCH ---
  u8g2.clearBuffer();          
  
  u8g2.setCursor(0, 20);       
  u8g2.print("Nhiet do: ");
  
  if (isnan(t)) {
    u8g2.print("Loi Doc!"); // Nếu in ra chữ này nghĩa là sai loại DHT
  } else {
    u8g2.print(t);
    u8g2.print(" C");
  }

  u8g2.setCursor(0, 40);
  u8g2.print("Do am: ");
  if (!isnan(h)) {
    u8g2.print(h);
    u8g2.print(" %");
  }

  u8g2.sendBuffer(); // Bắn hình ra màn hình 
}