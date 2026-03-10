## Mô tả dự án

Dự án **Traffic_Light_3LED** là hệ thống đèn giao thông 3 màu được **refactor** với kiến trúc OOP (Object-Oriented Programming) sử dụng class `TrafficLight`.

### Tính năng

#### Phiên bản cũ (Original)
- Điều khiển 3 LED giao thông (Đỏ, Vàng, Xanh)
-  Tự động chuyển đổi theo chu kỳ: ĐỎ (5s) → XANH (7s) → VÀNG (3s)
-  Non-blocking với `millis()`

####  Phiên bản mới (Refactored)
-  **Sử dụng class TrafficLight** - Code có cấu trúc, dễ bảo trì
-  **Tách logic vào header file** - Tái sử dụng code dễ dàng
-  **Emergency Mode** - Chế độ khẩn cấp (nhấn nút → tất cả đèn nhấp nháy)
- **Debouncing** - Xử lý chống dội nút nhấn
- **Mở rộng dễ dàng** - Thêm tính năng mới đơn giản

##  Chế độ hoạt động

### 1️.Normal Mode (Chế độ bình thường)

Đèn giao thông hoạt động theo chu kỳ tự động:

| Đèn | Thời gian | Ý nghĩa |
|-----|-----------|---------|
| 🔴 ĐỎ | 5 giây | Dừng lại |
| 🟢 XANH | 7 giây | Đi qua |
| 🟡 VÀNG | 3 giây | Chuẩn bị dừng |

### 2️.Emergency Mode (Chế độ khẩn cấp)

Khi nhấn nút (GPIO 23):
- **Tất cả 3 đèn nhấp nháy đồng thời** với tần số 300ms
- Báo hiệu tình huống khẩn cấp (xe ưu tiên, tai nạn, bảo trì...)
- Nhấn nút lần nữa để quay về Normal Mode

```
🔴🟡🟢 BẬT → 🔴🟡🟢 TẮT → 🔴🟡🟢 BẬT → ... (nhấp nháy liên tục)
```
---
## Phần cứng

### Linh kiện

| STT | Linh kiện | Số lượng | Mô tả |
|-----|-----------|----------|-------|
| 1 | ESP32 DevKit C V4 | 1 | Vi điều khiển chính |
| 2 | LED Đỏ | 1 | Đèn tín hiệu đỏ |
| 3 | LED Vàng | 1 | Đèn tín hiệu vàng |
| 4 | LED Xanh lá | 1 | Đèn tín hiệu xanh |
| 5 | Điện trở 1kΩ | 3 | Hạn chế dòng điện cho LED |
| 6 | Nút nhấn (Button) | 1 | Kích hoạt Emergency Mode |
| 7 | Breadboard | 1 | Lắp mạch |
| 8 | Dây jumper | ~ | Kết nối |

### Sơ đồ kết nối
#### LED Giao thông

| LED | Màu | GPIO | Điện trở |
|-----|-----|------|----------|
| LED1 | 🔴 Đỏ | GPIO 25 | 1kΩ |
| LED2 | 🟡 Vàng | GPIO 33 | 1kΩ |
| LED3 | 🟢 Xanh | GPIO 32 | 1kΩ |

**Kết nối:**
- Anode (+) LED → Điện trở 1kΩ → GPIO ESP32
- Cathode (-) LED → GND

#### Nút nhấn Emergency

| Nút nhấn | GPIO | Chế độ |
|----------|------|--------|
| Button | GPIO 23 | INPUT_PULLUP |

**Kết nối:**
- 1 chân nút → GPIO 23
- 1 chân nút → GND
- ESP32 sử dụng điện trở kéo lên nội bộ (INPUT_PULLUP)

### Sơ đồ tổng quan

```
                    ┌─────────────────┐
                    │   ESP32 DevKit  │
                    │      C V4       │
                    └─────────────────┘
                     │  │   │      │
         ┌───────────┼──┼───┼──────┼────────┐
         │           │  │   │      │        │
    GPIO 25      GPIO 33 │ GPIO 32│    GPIO 23
         │           │    │        │        │
    ┌────▼────┐ ┌───▼─┐ ┌▼────┐  ┌▼──────┐
    │ LED ĐỎ  │ │ LED │ │ LED │  │ BUTTON │
    │  [1kΩ]  │ │VÀNG │ │XANH │  │Emergency│
    └─────────┘ │[1kΩ]│ │[1kΩ]│  └────────┘
                └─────┘ └─────┘
                   │       │
                  GND     GND
```
---
## Cấu trúc code (Refactored)

### Cấu trúc thư mục

```
Traffic_Light_3LED/
├── src/
│   ├── main.cpp          # Code chương trình chính
│   └── TrafficLight.h    # Class TrafficLight (header only)
├── include/              # Header files khác (nếu có)
├── lib/                  # Thư viện bên ngoài
├── test/                 # Unit tests
├── diagram.json          # Sơ đồ mạch Wokwi
├── platformio.ini        # Cấu hình PlatformIO
├── wokwi.toml           # Cấu hình Wokwi
└── README.md            # File này
```
### Kiến trúc code
#### 1. **TrafficLight.h** - Class quản lý đèn giao thông

```cpp
class TrafficLight {
private:
    int _pinRed, _pinYellow, _pinGreen;
    TrafficState _currentState;
    OperationMode _mode;
    
public:
    TrafficLight(int red, int yellow, int green);
    void begin();
    void update();
    void setEmergencyMode();
    void setNormalMode();
    bool isEmergencyMode();
};
```

**Chức năng:**
- Quản lý trạng thái đèn (RED/GREEN/YELLOW)
- Quản lý chế độ (NORMAL/EMERGENCY)
- Tự động chuyển đổi đèn
- Xử lý nhấp nháy emergency

#### 2. **main.cpp** - Chương trình chính

```cpp
#include "TrafficLight.h"

TrafficLight trafficLight(25, 33, 32);

void setup() {
    trafficLight.begin();
    pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void loop() {
    // Xử lý nút nhấn
    if (button pressed) {
        trafficLight.setEmergencyMode();
    }
    
    // Cập nhật đèn
    trafficLight.update();
}
```

---

## Cài đặt và chạy

### 1.Clone hoặc download dự án

```bash
cd TEAM_05/NguyenCongHieu/Traffic_Light_3LED
```
### 2.Build dự án
```bash
pio run
```

### 3.Upload lên ESP32
```bash
pio run --target upload
```

### 4.Mở Serial Monitor
```bash
pio device monitor
```

### 5.Hoặc chạy trên Wokwi
- Mở file `diagram.json` trong Wokwi
- Click "Start Simulation"
- Click vào nút để test Emergency Mode

---

## Output Serial Monitor
### Normal Mode
```
========================================
Traffic Light System - Refactored
Author: Nguyen Cong Hieu
========================================
Traffic Light System Initialized
Traffic Light System Started
Press BUTTON (GPIO 23) for Emergency Mode
========================================

LED RED ON => 5 Seconds
LED GREEN ON => 7 Seconds
LED YELLOW ON => 3 Seconds
LED RED ON => 5 Seconds
...
```

### Emergency Mode (khi nhấn nút)
```
>>> EMERGENCY MODE ACTIVATED <<<
>>> All lights blinking <<<
(Tất cả đèn nhấp nháy)
```

### Quay về Normal Mode (nhấn nút lần 2)
```
>>> NORMAL MODE ACTIVATED <<<
LED RED ON => 5 Seconds
...
```
---

##  Kiến thức áp dụng
### 1.OOP (Object-Oriented Programming)

```cpp
// Tạo class riêng cho TrafficLight
class TrafficLight {
    // Encapsulation: Ẩn implementation details
    private:
        int _pinRed;
        TrafficState _currentState;
    
    // Public interface
    public:
        void begin();
        void update();
};
```

**Lợi ích:**
- Code có cấu trúc, dễ đọc
-  Tái sử dụng code (có thể tạo nhiều đèn giao thông)
-  Dễ bảo trì và mở rộng

### 2. State Machine (Máy trạng thái)
```
┌─────────┐
│   RED   │────────7s────────┐
└─────────┘                  │
     ▲                       ▼
     │                  ┌─────────┐
     │                  │  GREEN  │
     │                  └─────────┘
     │                       │
     │                      3s
     │                       ▼
     │                  ┌─────────┐
     └────────5s────────│ YELLOW  │
                        └─────────┘
```

### 3. Debouncing (Chống dội nút nhấn)

```cpp
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 300;

if (button pressed) {
    if (millis() - lastButtonPress > DEBOUNCE_DELAY) {
        // Xử lý nhấn nút
        lastButtonPress = millis();
    }
}
```

### 4. Non-blocking với millis()

```cpp
// Blocking (sai)
digitalWrite(LED, HIGH);
delay(1000);  // Chương trình bị block

// ✅ Non-blocking (đúng)
if (millis() - previousMillis >= 1000) {
    digitalWrite(LED, HIGH);
    previousMillis = millis();
}
```

---

## So sánh code cũ vs mới

| Tiêu chí | Code cũ | Code mới (Refactored) |
|----------|---------|----------------------|
| **Kiến trúc** | Procedural | OOP (Class-based) |
| **Số file** | 1 file (main.cpp) | 2 files (main.cpp + TrafficLight.h) |
| **Tái sử dụng** | Khó | Dễ (chỉ cần include header) |
| **Emergency Mode** | ❌ Không có | ✅ Có |
| **Nút nhấn** | ❌ Không có | ✅ Có (debouncing) |
| **Mở rộng** | Khó | Dễ (thêm methods vào class) |
| **Dòng code** | ~90 dòng | ~70 dòng (main.cpp) |
| **Bảo trì** | Khó | Dễ |

---

## Tùy chỉnh

### Thay đổi thời gian đèn

```cpp
// Trong main.cpp
#define RED_DURATION 8000    // Thay 5000 → 8000 (8 giây)
#define GREEN_DURATION 10000 // Thay 7000 → 10000 (10 giây)
#define YELLOW_DURATION 4000 // Thay 3000 → 4000 (4 giây)
```

### Thay đổi tần số nhấp nháy Emergency

```cpp
// Trong TrafficLight.h
static const unsigned long EMERGENCY_BLINK_INTERVAL = 500; // Thay 300 → 500ms
```
### Thêm chế độ mới
```cpp
// Trong TrafficLight.h - Thêm enum
enum OperationMode {
  NORMAL,
  EMERGENCY,
  MAINTENANCE  // Chế độ bảo trì mới
};

// Thêm method mới
void setMaintenanceMode() {
    _mode = MAINTENANCE;
    // Logic cho chế độ bảo trì
}
```

---
## Troubleshooting

| Vấn đề | Nguyên nhân | Giải pháp |
|--------|-------------|-----------|
| LED không sáng | Sai cực tính hoặc điện trở | Kiểm tra anode/cathode, thử 220Ω |
| Nút không hoạt động | Sai INPUT_PULLUP | Kiểm tra pinMode và kết nối GND |
| Đèn nhấp nháy loạn | Nhiễu nút nhấn | Tăng DEBOUNCE_DELAY lên 500ms |
| Upload lỗi | ESP32 không vào boot mode | Giữ nút BOOT khi upload |

---

## Mở rộng trong tương lai

- [ ] Thêm cảm biến PIR để tự động chuyển Emergency khi phát hiện xe cấp cứu
- [ ] Thêm countdown timer hiển thị trên LCD/OLED
- [ ] Kết nối WiFi để điều khiển từ xa qua web
- [ ] Thêm cảm biến ánh sáng để tự động điều chỉnh độ sáng LED
- [ ] Tích hợp nhiều ngã tư đèn giao thông đồng bộ
- [ ] Thêm buzzer cảnh báo âm thanh trong Emergency Mode

---

- ✅ Chu kỳ tự động RED → GREEN → YELLOW
---