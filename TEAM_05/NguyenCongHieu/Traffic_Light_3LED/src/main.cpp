#include <Arduino.h>
#include "TrafficLight.h"  // Import class TrafficLight

// ===== ĐỊNH NGHĨA CHÂN GPIO =====
#define PIN_LED_RED 25      // GPIO 25 - Đèn ĐỎ
#define PIN_LED_YELLOW 33   // GPIO 33 - Đèn VÀNG
#define PIN_LED_GREEN 32    // GPIO 32 - Đèn XANH
#define PIN_BUTTON 23       // GPIO 23 - Nút nhấn Emergency

// ===== THỜI GIAN MỖI ĐÈN (milliseconds) =====
#define RED_DURATION 5000    // 5 giây - Đèn đỏ
#define GREEN_DURATION 7000  // 7 giây - Đèn xanh
#define YELLOW_DURATION 3000 // 3 giây - Đèn vàng

// ===== KHỞI TẠO ĐỐI TƯỢNG TrafficLight =====
TrafficLight trafficLight(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);

// ===== BIẾN XỬ LÝ NÚT NHẤN (Debouncing) =====
unsigned long lastButtonPress = 0;    // Lưu thời gian nhấn nút lần cuối
const unsigned long DEBOUNCE_DELAY = 300; // Thời gian chống dội nút (300ms)

void setup() {
  // Khởi tạo Serial Monitor
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("Traffic Light System - Refactored");
  Serial.println("========================================");
  
  // Khởi tạo đèn giao thông
  trafficLight.begin();
  trafficLight.setDurations(RED_DURATION, YELLOW_DURATION, GREEN_DURATION);
  
  // Cấu hình nút nhấn Emergency
  pinMode(PIN_BUTTON, INPUT_PULLUP); // Sử dụng điện trở kéo lên nội bộ
  
  Serial.println("Traffic Light System Started");
  Serial.println("Press BUTTON (GPIO 23) for Emergency Mode");
  Serial.println("========================================\n");
}

/*
 * ================================================================================
 * HÀM LOOP - Vòng lặp chính (chạy liên tục)
 * ================================================================================
 */
void loop() {
  // ===== XỬ LÝ NÚT NHẤN EMERGENCY =====
  /*
   * Debouncing: Chống dội nút nhấn
   * - Chỉ xử lý nút nhấn nếu đã qua thời gian DEBOUNCE_DELAY (300ms)
   * - Ngăn chặn nhiều lần nhấn do dội cơ học của nút
   */
  if (digitalRead(PIN_BUTTON) == LOW) { // Nút nhấn (active LOW)
    unsigned long currentTime = millis();
    
    // Kiểm tra debounce
    if (currentTime - lastButtonPress > DEBOUNCE_DELAY) {
      lastButtonPress = currentTime;
      
      // Toggle giữa Normal và Emergency mode
      if (trafficLight.isEmergencyMode()) {
        trafficLight.setNormalMode();  // Emergency → Normal
      } else {
        trafficLight.setEmergencyMode(); // Normal → Emergency
      }
    }
  }
  
  // ===== CẬP NHẬT TRẠNG THÁI ĐÈN GIAO THÔNG =====
  /*
   * Hàm update() sẽ tự động:
   * - Chế độ NORMAL: Chuyển đèn theo chu kỳ
   * - Chế độ EMERGENCY: Nhấp nháy tất cả đèn
   */
  trafficLight.update();
}

// #include <Arduino.h>
//
// #define PIN_LED_RED 25
// #define PIN_LED_YELLOW 33
// #define PIN_LED_GREEN 32
//
// enum TrafficState {
//   RED_ON,
//   GREEN_ON,
//   YELLOW_ON
// };
//
// #define RED_DURATION 5000    // 5 seconds
// #define GREEN_DURATION 7000  // 7 seconds
// #define YELLOW_DURATION 3000 // 3 seconds
//
// void setup() {
//   
//   Serial.begin(115200);
//   
//   pinMode(PIN_LED_RED, OUTPUT);
//   pinMode(PIN_LED_YELLOW, OUTPUT);
//   pinMode(PIN_LED_GREEN, OUTPUT);
//   
//   digitalWrite(PIN_LED_RED, LOW);
//   digitalWrite(PIN_LED_YELLOW, LOW);
//   digitalWrite(PIN_LED_GREEN, LOW);
//   
//   Serial.println("Traffic Light System Started");
// }
//
// void loop() {
//   static TrafficState currentState = RED_ON;
//   static unsigned long previousMillis = 0;
//   static bool stateChanged = true;
//   
//   unsigned long currentMillis = millis();
//   unsigned long duration = 0;
//   
//   switch (currentState) {
//     case RED_ON:
//       duration = RED_DURATION;
//       break;
//     case GREEN_ON:
//       duration = GREEN_DURATION;
//       break;
//     case YELLOW_ON:
//       duration = YELLOW_DURATION;
//       break;
//   }
//   
//   
//   if (stateChanged) {
//     digitalWrite(PIN_LED_RED, LOW);
//     digitalWrite(PIN_LED_YELLOW, LOW);
//     digitalWrite(PIN_LED_GREEN, LOW);
//     
//     switch (currentState) {
//       case RED_ON:
//         digitalWrite(PIN_LED_RED, HIGH);
//         Serial.println("LED RED ON => 5 Seconds");
//         break;
//       case GREEN_ON:
//         digitalWrite(PIN_LED_GREEN, HIGH);
//         Serial.println("LED GREEN ON => 7 Seconds");
//         break;
//       case YELLOW_ON:
//         digitalWrite(PIN_LED_YELLOW, HIGH);
//         Serial.println("LED YELLOW ON => 3 Seconds");
//         break;
//     }
//     stateChanged = false;
//     previousMillis = currentMillis;
//   }
//   
//   if (currentMillis - previousMillis >= duration) {
//     switch (currentState) {
//       case RED_ON:
//         currentState = GREEN_ON;
//         break;
//       case GREEN_ON:
//         currentState = YELLOW_ON;
//         break;
//       case YELLOW_ON:
//         currentState = RED_ON;
//         break;
//     }
//     stateChanged = true;
//   }
// }

