#include <Arduino.h>

// Hàm kiểm tra thời gian không gây nghẽn (Non-blocking)
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

#define PIN_LED_RED 23

void setup() {
  // Khởi tạo Serial để sử dụng được printf hoặc Serial.print
  Serial.begin(115200); 
  
  Serial.println("WELCOME IOT");
  pinMode(PIN_LED_RED, OUTPUT); 
}

void loop() {
  static int i = 0;
  static unsigned long ulTimer = 0;
  static bool status = false;

  if (IsReady(ulTimer, 500)) {
    status = !status;
    digitalWrite(PIN_LED_RED, status ? HIGH : LOW);
    
    // In ra console để kiểm tra
    if (status) {
        Serial.printf("Loop running ... %d | LED ON\n", ++i);
    } else {
        Serial.printf("Loop running ... %d | LED OFF\n", i);
    }
  }
}