/*
 * ================================================================================
 * Dự án: LED_Blink_LDR - Đèn Giao Thông Tự Động với Cảm Biến Ánh Sáng
 * Tác giả: Nguyễn Công Hiếu
 * TEAM: 05
 * Ngày: 09/02/2026
 * ================================================================================
 * 
 * MÔ TẢ DỰ ÁN:
 * Hệ thống đèn giao thông thông minh tự động điều chỉnh hoạt động dựa trên
 * điều kiện ánh sáng môi trường được đo bởi cảm biến LDR (Light Dependent Resistor)
 * 
 * CHỨC NĂNG:
 * - Ban ngày (đủ ánh sáng): Hoạt động như đèn giao thông bình thường
 *   + Đèn XANH → 7 giây
 *   + Đèn VÀNG → 3 giây  
 *   + Đèn ĐỎ   → 5 giây
 * 
 * - Ban đêm (thiếu ánh sáng): Chỉ đèn VÀNG nhấp nháy để cảnh báo
 * 
 * PHẦN CỨNG:
 * - ESP32 DevKit
 * - 3x LED (Đỏ, Vàng, Xanh) + 3x điện trở 220Ω
 * - 1x LDR (Quang trở) + 1x điện trở 10kΩ
 * 
 * ================================================================================
 */

#include "main.h"

// ===== ĐỊNH NGHĨA CÁC CHÂN GPIO =====

// Chân điều khiển đèn LED giao thông
#define PIN_LED_RED     25    // GPIO 25 - Đèn ĐỎ
#define PIN_LED_YELLOW  33    // GPIO 33 - Đèn VÀNG
#define PIN_LED_GREEN   32    // GPIO 32 - Đèn XANH

// Chân đọc cảm biến LDR (Light Dependent Resistor - Quang trở)
#define PIN_LDR 34            // GPIO 34 - ADC1 (chân Analog để đọc giá trị LDR)

// ===== NGƯỠNG PHÁT HIỆN ÁNH SÁNG =====

/*
 * Ngưỡng ADC để phân biệt ban ngày/ban đêm
 * - ADC < 2000: Môi trường sáng (ban ngày) → Đèn hoạt động bình thường
 * - ADC > 2000: Môi trường tối (ban đêm) → Chỉ đèn vàng nhấp nháy
 * 
 * Giá trị ADC của ESP32: 0-4095 (12-bit resolution)
 * Có thể điều chỉnh giá trị này tùy theo điều kiện ánh sáng thực tế
 */
int DAY_ADC_THRESHOLD = 2000;

// ===== KHỞI TẠO CÁC ĐỐI TƯỢNG =====

// Đối tượng điều khiển đèn giao thông đơn giản (chưa sử dụng trong version này)
//LED_Blink ledYellow;

// Đối tượng điều khiển hệ thống đèn giao thông 3 màu với logic tự động
Trafic_Blink traficLight;

// Đối tượng đọc cảm biến quang trở LDR và chuyển đổi sang Lux
LDR ldrSensor;

/*
 * ================================================================================
 * HÀM SETUP - Khởi tạo hệ thống (chạy 1 lần khi khởi động)
 * ================================================================================
 */
void setup() {
  // In thông báo chào mừng ra Serial Monitor (baudrate mặc định: 115200)
  printf("Welcome IoT\n");

  // ===== KHỞI TẠO CẢM BIẾN LDR =====
  /*
   * Thiết lập cảm biến LDR để đọc cường độ ánh sáng
   * Tham số 1: PIN_LDR (GPIO 34) - Chân analog đọc giá trị LDR
   * Tham số 2: false - Sử dụng nguồn VCC 3.3V (không phải 5V)
   * 
   * LDR sẽ đọc cường độ ánh sáng và chuyển đổi thành:
   * - Giá trị ADC: 0-4095 (12-bit)
   * - Giá trị Lux: Cường độ ánh sáng thực tế
   */
  ldrSensor.setup(PIN_LDR, false); // VCC = 3.3V

  // ===== KHỞI TẠO HỆ THỐNG ĐÈN GIAO THÔNG =====
  /*
   * Thiết lập các chân GPIO cho 3 đèn LED giao thông
   * - PIN_LED_RED (GPIO 25):    Đèn ĐỎ
   * - PIN_LED_YELLOW (GPIO 33): Đèn VÀNG
   * - PIN_LED_GREEN (GPIO 32):  Đèn XANH
   * 
   * Hàm này sẽ tự động:
   * - Cấu hình các chân là OUTPUT
   * - Tắt tất cả đèn ban đầu
   * - Bật đèn XANH để bắt đầu chu kỳ
   */
  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  
  /*
   * Thiết lập thời gian hiển thị cho mỗi đèn
   * Tham số 1: 5 giây  - Thời gian đèn ĐỎ
   * Tham số 2: 3 giây  - Thời gian đèn VÀNG
   * Tham số 3: 7 giây  - Thời gian đèn XANH
   * 
   * Chu kỳ hoạt động: XANH (7s) → VÀNG (3s) → ĐỎ (5s) → lặp lại
   * Tổng thời gian 1 chu kỳ: 15 giây
   */
  traficLight.setupWaitTime(5, 3, 7); // seconds
}

/*
 * ================================================================================
 * HÀM LOOP - Vòng lặp chính (chạy liên tục)
 * ================================================================================
 */
void loop() {
  // ===== KHAI BÁO BIẾN STATIC (giữ giá trị giữa các lần gọi loop) =====
  
  /*
   * analogValue: Lưu giá trị ADC đọc từ cảm biến LDR (0-4095)
   * - Giá trị nhỏ (< 2000): Môi trường sáng - điện trở LDR thấp
   * - Giá trị lớn (> 2000): Môi trường tối - điện trở LDR cao
   * - Dùng static để giữ giá trị giữa các lần gọi loop()
   */
  static int analogValue = 0;
  
  /*
   * lux: Lưu cường độ ánh sáng chuyển đổi từ ADC (đơn vị: Lux)
   * - Lux cao: Môi trường sáng (ví dụ: 500+ lux)
   * - Lux thấp: Môi trường tối (ví dụ: < 100 lux)
   * - Được tính toán từ điện trở LDR theo công thức vật lý
   */
  static float lux = 0;
  
  /*
   * isDark: Cờ (flag) đánh dấu trạng thái môi trường
   * - false: Môi trường sáng (ban ngày)
   *          → Đèn hoạt động bình thường theo chu kỳ XANH-VÀNG-ĐỎ
   * - true:  Môi trường tối (ban đêm)
   *          → Chỉ đèn VÀNG nhấp nháy cảnh báo, tắt đèn ĐỎ và XANH
   */
  static bool isDark = false;
  
  // ===== ĐIỀU KHIỂN ĐÈN GIAO THÔNG =====
  
  /*
   * Gọi hàm blink() của đối tượng traficLight
   * 
   * Tham số 1: 500 (milliseconds)
   *   - Khoảng thời gian nhấp nháy (bật/tắt) của đèn
   *   - 500ms = 0.5 giây → Đèn sáng 0.5s, tắt 0.5s, lặp lại
   * 
   * Tham số 2: isDark (boolean)
   *   - Truyền trạng thái môi trường vào hàm blink()
   *   - Hàm sẽ tự động điều chỉnh chế độ hoạt động dựa vào giá trị này
   * 
   * Giá trị trả về (switched):
   *   - true:  Đèn vừa chuyển trạng thái (VD: XANH → VÀNG hoặc VÀNG → ĐỎ)
   *   - false: Đèn đang nhấp nháy ở màu hiện tại, chưa hết thời gian
   */
  bool switched = traficLight.blink(500, isDark);
  
  // ===== ĐỌC CẢM BIẾN LDR KHI CHUYỂN ĐÈN =====
  
  /*
   * Chỉ đọc cảm biến LDR khi đèn chuyển trạng thái (switched == true)
   * 
   * Lý do:
   * 1. Tối ưu hiệu suất: Không cần đọc cảm biến liên tục mỗi lần loop()
   * 2. Tiết kiệm tài nguyên: ADC đọc analog tốn thời gian (~100 microseconds)
   * 3. Đủ chính xác: Ánh sáng môi trường thay đổi chậm (phút/giờ)
   * 4. Giảm nhiễu: Đọc ít hơn giúp giảm nhiễu từ ADC
   * 
   * Thời gian đọc: Mỗi 3-7 giây (khi đèn chuyển màu)
   */
  if (switched) {
    // Đọc cảm biến LDR và chuyển đổi sang đơn vị Lux
    // Tham số: &analogValue - truyền địa chỉ để nhận giá trị ADC
    lux = ldrSensor.readLux(&analogValue);
    
    /*
     * So sánh giá trị ADC với ngưỡng để xác định môi trường
     * 
     * Logic:
     * - analogValue > 2000 → isDark = true  (môi trường tối)
     * - analogValue ≤ 2000 → isDark = false (môi trường sáng)
     * 
     * Ngưỡng 2000 được chọn dựa trên:
     * - Giá trị giữa của ADC 12-bit (0-4095)
     * - Có thể điều chỉnh tùy theo điều kiện thực tế
     */
    isDark = (analogValue > DAY_ADC_THRESHOLD);
  }

}