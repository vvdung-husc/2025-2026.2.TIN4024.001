#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 13
#define OLED_SCL 12
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setupDisplay() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void updateOLED(float t, float h, String status) {
  display.clearDisplay();

  // 1. Hiển thị Nhiệt độ (Hàng trên)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature:");
  
  display.setTextSize(2); 
  display.setCursor(0, 12);
  display.print(t, 1);
  display.print(" C");

  // 2. Hiển thị Độ ẩm (Hàng giữa - Thay thế vị trí bị lỗi)
  display.setTextSize(1);
  display.setCursor(0, 32);
  display.print("Humidity:");
  
  display.setTextSize(2);
  display.setCursor(0, 44);
  display.print(h, 1);
  display.print(" %");

  // 3. Hiển thị Trạng thái (Góc trên bên phải)
  display.setTextSize(1);
  display.setCursor(85, 0);
  display.print("[");
  display.print(status);
  display.print("]");

  display.display();
}

#endif