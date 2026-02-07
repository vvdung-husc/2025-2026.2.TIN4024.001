/*
 * 
 * Các class chính:
 * - LED_Blink: Điều khiển LED nhấp nháy đơn giản
 * - Trafic_Blink: Điều khiển đèn giao thông 3 màu (Đỏ, Vàng, Xanh)
 * - LDR: Đọc cảm biến quang trở, chuyển đổi sang Lux
 */

#pragma once

#include "ultils.h"

// ===== ĐỊNH NGHĨA CÁC HẰNG SỐ =====

// Ngưỡng ADC để xác định ban ngày/ban đêm
// ADC < DAY_ADC_THRESHOLD => Ban ngày (sáng)
// ADC > DAY_ADC_THRESHOLD => Ban đêm (tối)
#define DAY_ADC_THRESHOLD_DEFAULT 2000

// Điện áp tham chiếu cho ESP32
#define VCC_3V3 3.3f
#define VCC_5V  5.0f

// Giá trị ADC của ESP32 (12-bit)
#define ADC_MAX_VALUE 4095
#define ADC_MIN_VALUE 0

// ===== CÁC MACRO TIỆN ÍCH =====

// Chuyển đổi giây sang milliseconds
#define SECONDS_TO_MS(sec) ((sec) * 1000)

// Kiểm tra giá trị ADC hợp lệ
#define IS_VALID_ADC(val) ((val) >= ADC_MIN_VALUE && (val) <= ADC_MAX_VALUE)

// ===== ENUM VÀ TYPEDEF =====

// Trạng thái ánh sáng môi trường
typedef enum {
    LIGHT_BRIGHT = 0,   // Sáng (ban ngày)
    LIGHT_DARK = 1      // Tối (ban đêm)
} LightCondition;

// Trạng thái đèn LED
typedef enum {
    LED_OFF = LOW,
    LED_ON = HIGH
} LedState;

// ===== CÁC HÀM INLINE TIỆN ÍCH =====

/*
 * Kiểm tra môi trường có tối không dựa trên giá trị ADC
 * @param adcValue: Giá trị ADC đọc từ LDR (0-4095)
 * @param threshold: Ngưỡng ADC (mặc định: 2000)
 * @return: true = tối, false = sáng
 */
inline bool isDarkEnvironment(int adcValue, int threshold = DAY_ADC_THRESHOLD_DEFAULT) {
    return (adcValue > threshold);
}

/*
 * Chuyển đổi ADC sang điện áp
 * @param adcValue: Giá trị ADC (0-4095)
 * @param vcc: Điện áp nguồn (3.3V hoặc 5V)
 * @return: Điện áp (V)
 */
inline float adcToVoltage(int adcValue, float vcc = VCC_3V3) {
    return (adcValue / (float)ADC_MAX_VALUE) * vcc;
}

// ===== THÔNG TIN DỰ ÁN =====
/*
 * CẤU TRÚC DỰ ÁN:
 * 
 * 1. LED_Blink_LDR/
 *    ├── src/
 *    │   ├── main.cpp       - File chương trình chính
 *    │   ├── main.h         - File header chính (file này)
 *    │   ├── ultils.h       - Khai báo các class
 *    │   └── ultils.cpp     - Implementation các class
 *    ├── include/
 *    ├── lib/
 *    └── diagram.json        - Sơ đồ mạch Wokwi
 * 
 * 2. Chức năng chính:
 *    - Đèn giao thông tự động chạy theo chu kỳ: XANH -> VÀNG -> ĐỎ
 *    - Sử dụng cảm biến LDR để phát hiện ánh sáng môi trường
 *    - Khi trời tối (ADC > 2000): Chỉ nhấp nháy đèn VÀNG
 *    - Khi trời sáng (ADC < 2000): Hoạt động bình thường
 * 
 * 3. Phần cứng:
 *    - ESP32 DevKit
 *    - 3 x LED (Đỏ, Vàng, Xanh)
 *    - 3 x Điện trở 220Ω
 *    - 1 x LDR (Quang trở)
 *    - 1 x Điện trở 10kΩ (cho LDR)
 * 
 * 4. Kết nối:
 *    - LED_RED    -> GPIO 25
 *    - LED_YELLOW -> GPIO 33
 *    - LED_GREEN  -> GPIO 32
 *    - LDR        -> GPIO 34 (ADC1)
 */
