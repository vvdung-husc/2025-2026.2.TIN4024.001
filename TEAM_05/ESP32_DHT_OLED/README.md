## Mô tả dự án

Dự án **ESP32_DHT_OLED** là hệ thống giám sát nhiệt độ và độ ẩm môi trường sử dụng:
- **Cảm biến DHT22**: Đo nhiệt độ và độ ẩm
- **Màn hình OLED SSD1306**: Hiển thị thông tin trực quan
- **3 LED RGB**: Cảnh báo trạng thái nhiệt độ bằng màu sắc

### Tính năng chính

1. **Đo nhiệt độ và độ ẩm** - Sử dụng cảm biến DHT22 độ chính xác cao
2. **Hiển thị trực quan** - Màn hình OLED 128x64 pixels
3. **Cảnh báo LED** - 3 LED nhấp nháy theo ngưỡng nhiệt độ
4. **Serial Monitor** - In thông tin ra Serial để theo dõi

## Ngưỡng nhiệt độ và trạng thái

| Nhiệt độ (°C) | Trạng thái | Màu LED | Mô tả |
|--------------|------------|---------|-------|
| **< 13°C** | TOO COLD | GREEN | Quá lạnh - cảnh báo nhiệt độ thấp |
| **13 - 20°C** | COLD | GREEN | Lạnh - môi trường mát |
| **20 - 25°C** | COOL |  YELLOW | Mát mẻ - nhiệt độ thoải mái |
| **25 - 30°C** | WARM | YELLOW | Ấm - nhiệt độ hơi nóng |
| **30 - 35°C** | HOT | RED | Nóng - cảnh báo nhiệt độ cao |
| **> 35°C** | TOO HOT | RED | Quá nóng - cảnh báo khẩn cấp |

**Lưu ý:** Các LED sẽ nhấp nháy với tần số 500ms (0.5 giây) để dễ nhận biết

## Phần cứng

### Linh kiện cần thiết

| STT | Linh kiện | Số lượng | Mô tả |
|-----|-----------|----------|-------|
| 1 | ESP32 DevKit C V4 | 1 | Vi điều khiển chính |
| 2 | DHT22 | 1 | Cảm biến nhiệt độ và độ ẩm |
| 3 | OLED SSD1306 (128x64) | 1 | Màn hình hiển thị I2C |
| 4 | LED Đỏ | 1 | Cảnh báo nóng |
| 5 | LED Vàng | 1 | Cảnh báo ấm/mát |
| 6 | LED Xanh lá | 1 | Cảnh báo lạnh |
| 7 | Điện trở 1kΩ | 2 | Hạn chế dòng cho LED |
| 8 | Breadboard | 1 | Lắp mạch |
| 9 | Dây jumper | ~ | Kết nối linh kiện |

---

## Sơ đồ kết nối

### Kết nối DHT22

| DHT22 | ESP32 |
|-------|-------|
| VCC | 3V3 |
| DATA | GPIO 16 |
| GND | GND |

### Kết nối OLED SSD1306 (I2C)

| OLED | ESP32 | Chú thích |
|------|-------|-----------|
| VCC | 3V3 | Nguồn 3.3V |
| GND | GND | Ground |
| SCL | GPIO 12 | Serial Clock |
| SDA | GPIO 13 | Serial Data |

**I2C Address:** `0x3C`

### Kết nối LED

| LED | Màu | ESP32 GPIO | Điện trở |
|-----|-----|------------|----------|
| LED1 | Đỏ | GPIO 4 | 1kΩ |
| LED2 | Xanh | GPIO 15 | Không cần (nối trực tiếp) |
| LED3 | Vàng | GPIO 2 | 1kΩ |

**Lưu ý:** 
- Tất cả LED nối cathode (-) về GND
- LED đỏ và vàng có điện trở 1kΩ nối nối tiếp
- LED xanh nối trực tiếp (GPIO 15 chịu được dòng)

### Sơ đồ tổng quan

```
                    ┌─────────────────┐
                    │   ESP32 DevKit  │
                    │      C V4       │
                    └─────────────────┘
                           │ │ │
         ┌─────────────────┼─┼─┼─────────────────┐
         │                 │ │ │                 │
         │                 │ │ │                 │
    GPIO 16            GPIO 12,13           GPIO 4,15,2
         │                 │ │                   │
    ┌────▼────┐       ┌────▼─▼────┐         ┌───▼────┐
    │  DHT22  │       │   OLED    │         │ 3 LEDs │
    │         │       │  SSD1306  │         │ R,G,Y  │
    │ 🌡️ 💧   │       │  128x64   │         │ 💡 💡 💡│
    └─────────┘       └───────────┘         └────────┘
```

---

## Cài đặt và chạy dự án

### 1.Yêu cầu hệ thống

- [PlatformIO IDE](https://platformio.org/platformio-ide) (khuyến nghị) hoặc Arduino IDE
- ESP32 board hoặc [Wokwi Simulator](https://wokwi.com/)
- Cáp USB Type-C (để nạp code vào ESP32)

### 2️.Thư viện cần thiết

Dự án sử dụng các thư viện sau (đã được khai báo trong `platformio.ini`):

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.3
    adafruit/DHT sensor library@^1.4.4
```

**Thư viện:**
1. **Adafruit SSD1306** - Điều khiển màn hình OLED
2. **Adafruit GFX** - Thư viện đồ họa cơ bản
3. **DHT sensor library** - Đọc cảm biến DHT22

### 3️.Hướng dẫn cài đặt

#### Option A: Sử dụng PlatformIO (Khuyến nghị)

```bash
# 1. Di chuyển vào thư mục dự án
cd TEAM_05/ESP32_DHT_OLED

# 2. Build dự án (PlatformIO sẽ tự động tải thư viện)
pio run

# 3. Upload code lên ESP32
pio run --target upload

# 4. Mở Serial Monitor để xem output
pio device monitor
```

#### Option B: Sử dụng Wokwi Simulator

1. Truy cập [Wokwi.com](https://wokwi.com/)
2. Tạo project mới hoặc import file `diagram.json`
3. Copy code từ `src/main.cpp` vào editor
4. Click **"Start Simulation"**
5. Xem kết quả trên màn hình OLED và Serial Monitor

### 4.Kiểm tra kết nối

Sau khi upload code, hệ thống sẽ:

1. Hiển thị thông báo khởi động trên OLED
2. Đọc nhiệt độ và độ ẩm từ DHT22
3. Hiển thị thông tin lên OLED
4. Nhấp nháy LED tương ứng
5. In thông tin ra Serial Monitor
---

## Giao diện OLED

```
┌────────────────────────┐
│ ESP32 DHT OLED         │
├────────────────────────┤
│ Nhiet do: 28.5 C       │
│ Do am   : 65.2 %       │
├────────────────────────┤
│                        │
│   WARM                 │
│                        │
└────────────────────────┘
```

**Thông tin hiển thị:**
- Dòng 1: Tiêu đề dự án
- Dòng 2-3: Nhiệt độ và độ ẩm (1 chữ số thập phân)
- Dòng 4-5: Trạng thái nhiệt độ (font lớn)

---

## Output Serial Monitor

```
He thong san sang!
Nhiet do: 28.5 C | Do am: 65.2 % | Trang thai: WARM
Nhiet do: 28.4 C | Do am: 65.3 % | Trang thai: WARM
Nhiet do: 32.1 C | Do am: 60.5 % | Trang thai: HOT
Nhiet do: 18.3 C | Do am: 70.2 % | Trang thai: COLD
```