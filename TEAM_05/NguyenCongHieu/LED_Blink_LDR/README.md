# LED_Blink_LDR

## Mô tả dự án

Dự án **LED_Blink_LDR** là hệ thống đèn giao thông thông minh sử dụng cảm biến quang trở (LDR - Light Dependent Resistor) để tự động điều chỉnh hoạt động dựa trên điều kiện ánh sáng môi trường.

## Chức năng chính

- **Chế độ ban ngày** (đủ ánh sáng): Đèn giao thông hoạt động bình thường theo chu kỳ:
  - 🟢 **Đèn XANH** → 7 giây
  - 🟡 **Đèn VÀNG** → 3 giây
  - 🔴 **Đèn ĐỎ** → 5 giây

- **Chế độ ban đêm** (thiếu ánh sáng): Chỉ đèn VÀNG nhấp nháy liên tục để cảnh báo

- **Tự động phát hiện**: Sử dụng cảm biến LDR để phát hiện điều kiện ánh sáng và chuyển đổi chế độ tự động

## Phần cứng

### Linh kiện cần thiết

| Linh kiện | Số lượng | Mô tả |
|-----------|----------|-------|
| ESP32 DevKit | 1 | Vi điều khiển chính |
| LED Đỏ | 1 | Đèn tín hiệu ĐỎ |
| LED Vàng | 1 | Đèn tín hiệu VÀNG |
| LED Xanh | 1 | Đèn tín hiệu XANH |
| Điện trở 220Ω | 3 | Hạn chế dòng điện cho LED |
| LDR (Quang trở) | 1 | Cảm biến ánh sáng |
| Điện trở 10kΩ | 1 | Điện trở phân áp cho LDR |

### Sơ đồ kết nối

```
ESP32                          Phần cứng
─────────────────────────────────────────────
GPIO 25 ──[220Ω]──┤►├── LED ĐỎ ── GND
GPIO 33 ──[220Ω]──┤►├── LED VÀNG ── GND
GPIO 32 ──[220Ω]──┤►├── LED XANH ── GND

GPIO 34 (ADC1) ── LDR ── 3.3V
                  │
               [10kΩ]
                  │
                 GND
```

### Cấu trúc thư mục

```
LED_Blink_LDR/
├── src/
│   ├── main.cpp          # Chương trình chính
│   ├── main.h            # Header chính với documentation
│   ├── ultils.h          # Khai báo các class
│   └── ultils.cpp        # Implementation các class
├── include/              # Thư viện header tùy chỉnh
├── lib/                  # Thư viện bên ngoài
├── test/                 # Unit tests
├── diagram.json          # Sơ đồ mạch Wokwi
├── platformio.ini        # Cấu hình PlatformIO
├── wokwi.toml           # Cấu hình Wokwi simulator
└── README.md            # File này
```

### Các class chính

#### 1. **Trafic_Blink**
Class điều khiển đèn giao thông 3 màu với các chức năng:
- Quản lý 3 LED (Đỏ, Vàng, Xanh)
- Chuyển đổi đèn theo thời gian cài đặt
- Nhấp nháy từng đèn với tần số tùy chỉnh
- Tự động chuyển sang chế độ đêm (chỉ nhấp nháy đèn vàng)

**Các phương thức:**
```cpp
setupPin(pinRed, pinYellow, pinGreen)  // Thiết lập chân GPIO
setupWaitTime(red, yellow, green)      // Thiết lập thời gian (giây)
blink(interval, isDark)                // Điều khiển nhấp nháy
```

#### 2. **LDR**
Class đọc cảm biến quang trở và chuyển đổi sang đơn vị Lux:
- Đọc giá trị ADC (0-4095)
- Tính toán điện áp và điện trở
- Chuyển đổi sang cường độ ánh sáng (Lux)

**Các phương thức:**
```cpp
setup(pin, vcc5Volt)      // Khởi tạo cảm biến
getValue()                 // Đọc giá trị ADC thô
readLux(*analogValue)      // Đọc và chuyển đổi sang Lux
```

## 📊 Ngưỡng ánh sáng

```cpp
#define DAY_ADC_THRESHOLD 2000  // Ngưỡng phân biệt ngày/đêm

// Giá trị ADC < 2000 → Ban ngày (sáng) → Chế độ hoạt động bình thường
// Giá trị ADC > 2000 → Ban đêm (tối) → Chỉ nhấp nháy đèn vàng
```

## Cài đặt và chạy

### Yêu cầu

- [PlatformIO IDE](https://platformio.org/platformio-ide) hoặc [PlatformIO Core](https://platformio.org/install/cli)
- ESP32 board hoặc [Wokwi Simulator](https://wokwi.com/)

### Hướng dẫn

1. **Clone hoặc download dự án**
   ```bash
   cd TEAM_05/NguyenCongHieu/LED_Blink_LDR
   ```

2. **Build dự án**
   ```bash
   pio run
   ```

3. **Upload lên ESP32**
   ```bash
   pio run --target upload
   ```

4. **Mở Serial Monitor**
   ```bash
   pio device monitor
   ```

5. **Hoặc chạy trên Wokwi Simulator**
   - Mở file `diagram.json` trong Wokwi
   - Click nút "Start Simulation"

## Serial Output

Khi chạy, chương trình sẽ in ra thông tin qua Serial Monitor:

```
Welcome IoT
LED [GREEN ] ON => 7 Seconds
 [GREEN] => Second: 6
 [GREEN] => Second: 5
...
LDR Analog: 1850, Voltage: 1.49 V, Resistance: 1620.37 Ohm, Light Intensity: 305.24[305] lux
LED [YELLOW] ON => 3 Seconds
...
IT IS DARK!!!!
(Chỉ đèn vàng nhấp nháy)
```

## Kiến thức áp dụng

### 1. Non-blocking Programming
Sử dụng `millis()` thay vì `delay()` để tránh blocking:
```cpp
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}
```

### 2. Đọc cảm biến Analog (ADC)
ESP32 có ADC 12-bit (0-4095):
```cpp
int analogValue = analogRead(PIN_LDR);  // GPIO 34
```

### 3. State Machine
Quản lý trạng thái đèn giao thông:
- GREEN (0) → YELLOW (1) → RED (2) → GREEN (0) → ...

### 4. Tính toán điện trở và Lux
Từ giá trị ADC → Voltage → Resistance → Lux

## Tùy chỉnh

### Thay đổi thời gian đèn

```cpp
traficLight.setupWaitTime(5, 3, 7);  // Đỏ, Vàng, Xanh (giây)
```

### Thay đổi ngưỡng ánh sáng

```cpp
int DAY_ADC_THRESHOLD = 2000;  // Tăng/giảm để điều chỉnh độ nhạy
```

### Thay đổi tần số nhấp nháy

```cpp
bool switched = traficLight.blink(500, isDark);  // 500ms = 0.5 giây
```
