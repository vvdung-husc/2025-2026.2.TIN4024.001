/*
THÔNG TIN NHÓM 11
1. Phạm Mạnh Lân
2. Phan Minh Nhật Khoa
3. Nguyễn Đức Tín
4. Đào Văn Lợi
5. Võ Thanh Toàn
*/

#include <Arduino.h>
#include "SensorHandler.h"
#include "DisplayHandler.h"
#include "AlertHandler.h"

float temp = 0;
float hum = 0;

void setup() {
  Serial.begin(115200);
  setupSensors();  
  setupDisplay();  
  setupAlerts();   
  
  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  delay(2000);
}

void loop() {
  // Đọc cảm biến mỗi 2 giây
  if (handleSensorReading(hum, temp)) {
    // Cập nhật OLED với nhiệt độ và trạng thái
    updateOLED(temp, hum, getStatus(temp));
  }
  
  // Điều khiển LED nhấp nháy theo ngưỡng nhiệt độ
  handleLEDBloking(getActiveLed(temp)); 
}