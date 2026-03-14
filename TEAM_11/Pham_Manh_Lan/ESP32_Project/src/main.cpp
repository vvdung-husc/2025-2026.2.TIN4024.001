#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Cấu hình màn hình SH1106 (Sử dụng I2C phần cứng mặc định: SDA=D2, SCL=D1)
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);

// Cấu hình đèn LED nhấp nháy trên board NodeMCU
const int ledPin = LED_BUILTIN; 
int ledState = LOW;             
unsigned long previousMillisLed = 0; 
const long ledInterval = 500;   // Nháy mỗi 0.5 giây

// Cấu hình biến nhiệt độ
float currentTemp = 25.0; 
unsigned long previousMillisTemp = 0;
const long tempInterval = 2000; // Cập nhật mỗi 2 giây

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  u8g2.begin();
  
  Serial.println("He thong da khoi dong!");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Nhấp nháy LED
  if (currentMillis - previousMillisLed >= ledInterval) {
    previousMillisLed = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(ledPin, ledState);
  }

  // 2. Cập nhật nhiệt độ giả lập
  if (currentMillis - previousMillisTemp >= tempInterval) {
    previousMillisTemp = currentMillis;
    currentTemp += 0.5; 
    if(currentTemp > 35.0) currentTemp = 25.0; 
  }

  // 3. Xử lý hiển thị màn hình OLED
  u8g2.clearBuffer();          
  
  u8g2.setFont(u8g2_font_ncenB10_tr); 
  
  // Đã đổi thành text bạn yêu cầu và căn chỉnh tọa độ
  u8g2.drawStr(10, 20, "Manh Lan dep trai"); 
  
  u8g2.setCursor(10, 50);
  u8g2.print("Nhiet do: ");
  u8g2.print(currentTemp, 1); 
  u8g2.print(" C");

  u8g2.sendBuffer();          
}