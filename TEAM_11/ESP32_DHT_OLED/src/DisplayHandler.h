#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Định nghĩa chân I2C cho ESP32 (theo Diagram của khoa)
#define OLED_SDA 13
#define OLED_SCL 12
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// Khởi tạo đối tượng hiển thị
// Sử dụng &Wire thay vì để mặc định để ép dùng chân 13, 12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setupDisplay() {
  // Bắt đầu I2C với chân SDA 13 và SCL 12
  Wire.begin(OLED_SDA, OLED_SCL);

  // Địa chỉ màn hình thường là 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Dừng lại nếu không tìm thấy màn hình
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("System Starting...");
  display.display();
}

void updateOLED(float t, float h, String status) {
  display.clearDisplay();

  // Vẽ phần nhiệt độ
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.setTextSize(2); // Làm số to lên cho dễ nhìn
  display.setCursor(0, 10);
  display.print(t, 1);
  display.print(" C");

  // Vẽ phần độ ẩm
  display.setTextSize(1);
  display.setCursor(70, 0);
  display.print("Hum: ");
  display.setCursor(70, 10);
  display.print(h, 0);
  display.print("%");

  // Vẽ đường kẻ ngang phân cách
  display.drawLine(0, 30, 128, 30, SSD1306_WHITE);

  // Hiển thị trạng thái (Status) ở dưới cùng
  display.setTextSize(2);
  display.setCursor(0, 40);
  display.print(status);

  display.display();
}

#endif
