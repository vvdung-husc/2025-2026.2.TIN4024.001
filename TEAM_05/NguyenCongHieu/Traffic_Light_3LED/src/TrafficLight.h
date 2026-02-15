#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <Arduino.h>

// ===== ENUM - Định nghĩa các trạng thái đèn =====
enum TrafficState {
  RED_ON,       // Đèn đỏ sáng
  GREEN_ON,     // Đèn xanh sáng
  YELLOW_ON     // Đèn vàng sáng
};

// ===== ENUM - Định nghĩa chế độ hoạt động =====
enum OperationMode {
  NORMAL,       // Chế độ bình thường
  EMERGENCY     // Chế độ khẩn cấp (tất cả đèn nhấp nháy)
};

// ===== CLASS TrafficLight =====
/*
 * Class quản lý đèn giao thông 3 màu
 * 
 * Tính năng:
 * - Điều khiển 3 LED (Đỏ, Vàng, Xanh)
 * - Tự động chuyển đổi trạng thái theo thời gian
 * - Hỗ trợ chế độ khẩn cấp (tất cả đèn nhấp nháy)
 * - Non-blocking operation (không dùng delay)
 */
class TrafficLight {
private:
  // ===== Chân GPIO =====
  int _pinRed;      // Chân điều khiển đèn đỏ
  int _pinYellow;   // Chân điều khiển đèn vàng
  int _pinGreen;    // Chân điều khiển đèn xanh
  
  // ===== Thời gian cho mỗi đèn (milliseconds) =====
  unsigned long _redDuration;     // Thời gian đèn đỏ (mặc định: 5000ms)
  unsigned long _yellowDuration;  // Thời gian đèn vàng (mặc định: 3000ms)
  unsigned long _greenDuration;   // Thời gian đèn xanh (mặc định: 7000ms)
  
  // ===== Biến trạng thái =====
  TrafficState _currentState;     // Trạng thái hiện tại (RED/GREEN/YELLOW)
  OperationMode _mode;            // Chế độ hoạt động (NORMAL/EMERGENCY)
  
  // ===== Biến thời gian (non-blocking) =====
  unsigned long _previousMillis;  // Lưu thời gian lần cuối chuyển trạng thái
  unsigned long _emergencyBlinkPrevious; // Lưu thời gian nhấp nháy emergency
  
  // ===== Biến cờ (flags) =====
  bool _stateChanged;             // Cờ đánh dấu đèn vừa chuyển trạng thái
  bool _emergencyBlinkState;      // Trạng thái nhấp nháy trong emergency mode
  
  // ===== Hằng số =====
  static const unsigned long EMERGENCY_BLINK_INTERVAL = 300; // Tần số nhấp nháy emergency (300ms)

public:
  /*
   * Constructor - Khởi tạo TrafficLight
   * 
   * @param pinRed: Chân GPIO cho đèn đỏ
   * @param pinYellow: Chân GPIO cho đèn vàng
   * @param pinGreen: Chân GPIO cho đèn xanh
   */
  TrafficLight(int pinRed, int pinYellow, int pinGreen) {
    _pinRed = pinRed;
    _pinYellow = pinYellow;
    _pinGreen = pinGreen;
    
    // Khởi tạo giá trị mặc định
    _redDuration = 5000;      // 5 giây
    _yellowDuration = 3000;   // 3 giây
    _greenDuration = 7000;    // 7 giây
    
    _currentState = RED_ON;   // Bắt đầu với đèn đỏ
    _mode = NORMAL;           // Bắt đầu ở chế độ bình thường
    
    _previousMillis = 0;
    _emergencyBlinkPrevious = 0;
    _stateChanged = true;
    _emergencyBlinkState = false;
  }
  
  /*
   * Khởi tạo các chân GPIO và trạng thái ban đầu
   * Gọi hàm này trong setup()
   */
  void begin() {
    // Cấu hình các chân là OUTPUT
    pinMode(_pinRed, OUTPUT);
    pinMode(_pinYellow, OUTPUT);
    pinMode(_pinGreen, OUTPUT);
    
    // Tắt tất cả đèn ban đầu
    turnOffAll();
    
    Serial.println("Traffic Light System Initialized");
  }
  
  /*
   * Thiết lập thời gian cho mỗi đèn
   * 
   * @param redMs: Thời gian đèn đỏ (milliseconds)
   * @param yellowMs: Thời gian đèn vàng (milliseconds)
   * @param greenMs: Thời gian đèn xanh (milliseconds)
   */
  void setDurations(unsigned long redMs, unsigned long yellowMs, unsigned long greenMs) {
    _redDuration = redMs;
    _yellowDuration = yellowMs;
    _greenDuration = greenMs;
  }
  
  /*
   * Tắt tất cả các đèn LED
   */
  void turnOffAll() {
    digitalWrite(_pinRed, LOW);
    digitalWrite(_pinYellow, LOW);
    digitalWrite(_pinGreen, LOW);
  }
  
  /*
   * Bật tất cả các đèn LED
   */
  void turnOnAll() {
    digitalWrite(_pinRed, HIGH);
    digitalWrite(_pinYellow, HIGH);
    digitalWrite(_pinGreen, HIGH);
  }
  
  /*
   * Chuyển sang chế độ khẩn cấp (Emergency Mode)
   * Tất cả đèn sẽ nhấp nháy đồng thời
   */
  void setEmergencyMode() {
    if (_mode != EMERGENCY) {
      _mode = EMERGENCY;
      _emergencyBlinkPrevious = millis();
      _emergencyBlinkState = false;
      turnOffAll();
      Serial.println(">>> EMERGENCY MODE ACTIVATED <<<");
      Serial.println(">>> All lights blinking <<<");
    }
  }
  
  /*
   * Chuyển về chế độ bình thường (Normal Mode)
   */
  void setNormalMode() {
    if (_mode != NORMAL) {
      _mode = NORMAL;
      _stateChanged = true;
      _previousMillis = millis();
      Serial.println(">>> NORMAL MODE ACTIVATED <<<");
    }
  }
  
  /*
   * Kiểm tra chế độ hiện tại
   * 
   * @return: true nếu đang ở emergency mode, false nếu normal mode
   */
  bool isEmergencyMode() {
    return (_mode == EMERGENCY);
  }
  
  /*
   * Hàm cập nhật trạng thái đèn giao thông
   * Gọi hàm này trong loop() liên tục
   * 
   * Hoạt động:
   * - NORMAL MODE: Tự động chuyển đèn theo chu kỳ
   * - EMERGENCY MODE: Tất cả đèn nhấp nháy
   */
  void update() {
    unsigned long currentMillis = millis();
    
    // ===== XỬ LÝ CHẾ ĐỘ KHẨN CẤP =====
    if (_mode == EMERGENCY) {
      // Nhấp nháy tất cả đèn với tần số 300ms
      if (currentMillis - _emergencyBlinkPrevious >= EMERGENCY_BLINK_INTERVAL) {
        _emergencyBlinkPrevious = currentMillis;
        _emergencyBlinkState = !_emergencyBlinkState;
        
        if (_emergencyBlinkState) {
          turnOnAll();   // Bật tất cả đèn
        } else {
          turnOffAll();  // Tắt tất cả đèn
        }
      }
      return; // Không xử lý logic chế độ bình thường
    }
    
    // ===== XỬ LÝ CHẾ ĐỘ BÌNH THƯỜNG =====
    
    // Lấy thời gian chờ cho trạng thái hiện tại
    unsigned long duration = getDuration(_currentState);
    
    // Khi vừa chuyển trạng thái mới
    if (_stateChanged) {
      turnOffAll(); // Tắt tất cả đèn trước
      
      // Bật đèn tương ứng với trạng thái hiện tại
      switch (_currentState) {
        case RED_ON:
          digitalWrite(_pinRed, HIGH);
          Serial.println("LED RED ON => " + String(_redDuration/1000) + " Seconds");
          break;
        case GREEN_ON:
          digitalWrite(_pinGreen, HIGH);
          Serial.println("LED GREEN ON => " + String(_greenDuration/1000) + " Seconds");
          break;
        case YELLOW_ON:
          digitalWrite(_pinYellow, HIGH);
          Serial.println("LED YELLOW ON => " + String(_yellowDuration/1000) + " Seconds");
          break;
      }
      
      _stateChanged = false;
      _previousMillis = currentMillis;
    }
    
    // Kiểm tra nếu đã hết thời gian, chuyển sang trạng thái tiếp theo
    if (currentMillis - _previousMillis >= duration) {
      _currentState = getNextState(_currentState);
      _stateChanged = true;
    }
  }
  
private:
  /*
   * Lấy thời gian chờ cho từng trạng thái
   * 
   * @param state: Trạng thái đèn
   * @return: Thời gian chờ (milliseconds)
   */
  unsigned long getDuration(TrafficState state) {
    switch (state) {
      case RED_ON:    return _redDuration;
      case GREEN_ON:  return _greenDuration;
      case YELLOW_ON: return _yellowDuration;
      default:        return 0;
    }
  }
  
  /*
   * Lấy trạng thái tiếp theo trong chu kỳ
   * Chu kỳ: ĐỎ → XANH → VÀNG → ĐỎ → ...
   * 
   * @param state: Trạng thái hiện tại
   * @return: Trạng thái tiếp theo
   */
  TrafficState getNextState(TrafficState state) {
    switch (state) {
      case RED_ON:    return GREEN_ON;   // ĐỎ → XANH
      case GREEN_ON:  return YELLOW_ON;  // XANH → VÀNG
      case YELLOW_ON: return RED_ON;     // VÀNG → ĐỎ
      default:        return RED_ON;
    }
  }
};

#endif // TRAFFIC_LIGHT_H
