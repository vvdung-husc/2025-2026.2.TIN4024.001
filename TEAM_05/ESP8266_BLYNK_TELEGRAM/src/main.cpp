/*
THÔNG TIN NHÓM 5
1. Nguyễn Công Hiếu
2. Phạm Đức Thành Đạt
3. Trần Văn Tiến
4.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Khởi tạo màn hình OLED kết nối I2C (SDA = GPIO 21, SCL = GPIO 22 trên ESP32)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Khởi tạo màn hình OLED với địa chỉ 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khởi tạo OLED thất bại!"));
    for(;;); // Dừng chương trình nếu lỗi
  }

  // Xóa bộ đệm và thiết lập chữ
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // In thông tin nhóm lên màn hình (Nhớ sửa lại tên thành viên thật không dấu để tránh lỗi font)
  display.println("TEAM 5 - Lap trinh IOT");
  display.println("1. Nguyen Cong Hieu");
  display.println("2. Pham Duc Thanh Dat");
  display.println("3. Tran Van Tien");
  
  // Lệnh đẩy dữ liệu từ bộ đệm ra màn hình
  display.display(); 
  
  Serial.println("OLED khoi tao thanh cong!");
}

void loop() {
  // Chưa cần làm gì trong loop lúc này
}