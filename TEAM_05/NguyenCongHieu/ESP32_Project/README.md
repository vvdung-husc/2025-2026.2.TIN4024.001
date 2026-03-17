# ESP32_Project — ESP8266 Environmental Sensor Controller

## Mô tả dự án

Dự án lập trình bo mạch **NodeMCU v2 (ESP8266)** sử dụng framework Arduino thông qua **PlatformIO**. Chương trình thu thập dữ liệu môi trường từ hai cảm biến và hiển thị kết quả lên màn hình OLED SH1106 128×64, đồng thời xuất thông tin ra Serial Monitor.

| Chức năng | Mô tả |
|---|---|
| Đo nhiệt độ & độ ẩm | Cảm biến DHT11 kết nối chân D3 |
| Đo nồng độ khí/khói | Cảm biến MQ-2 kết nối chân A0 |
| Hiển thị OLED | Màn hình SH1106 128×64 qua I2C |
| LED nhấp nháy | LED_BUILTIN nhấp nháy mỗi 500 ms (heartbeat) |
| Serial Monitor | In dữ liệu cảm biến mỗi 2 giây (baud 115200) |

---

## Cấu trúc thư mục

```
ESP32_Project/
│
├── src/
│   └── main.cpp            # Mã nguồn chính
│
├── include/
│   └── README              # Hướng dẫn sử dụng thư mục include
│
├── lib/
│   └── README              # Hướng dẫn thêm thư viện riêng
│
├── test/                   # Thư mục kiểm thử (PlatformIO)
│
├── platformio.ini          # Cấu hình môi trường PlatformIO
└── README.md               # Tài liệu dự án
```
---

## Phần cứng

| Thành phần | Mô tả | Chân kết nối |
|---|---|---|
| NodeMCU v2 (ESP8266) | Vi điều khiển chính | — |
| DHT11 | Cảm biến nhiệt độ & độ ẩm | D3 |
| MQ-2 | Cảm biến khí gas / khói | A0 (analog) |
| OLED SH1106 128×64 | Màn hình hiển thị | I2C (SDA/SCL) |
| LED tích hợp | Đèn báo trạng thái hoạt động | LED_BUILTIN |

---

## Môi trường & Thư viện

**Nền tảng:** [PlatformIO](https://platformio.org/)  
**Framework:** Arduino  
**Board:** `nodemcuv2` (espressif8266)

Các thư viện sử dụng (khai báo trong `platformio.ini`):

| Thư viện | Phiên bản | Mục đích |
|---|---|---|
| `adafruit/DHT sensor library` | ^1.4.6 | Đọc dữ liệu cảm biến DHT11/DHT22 |
| `adafruit/Adafruit Unified Sensor` | ^1.1.15 | Thư viện cảm biến thống nhất của Adafruit |
| `olikraus/U8g2` | ^2.36.6 | Điều khiển màn hình OLED (SH1106) |

---

## Hướng dẫn cài đặt & chạy

### 1. Yêu cầu

- [Visual Studio Code](https://code.visualstudio.com/) với extension [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
- Cáp USB micro để kết nối NodeMCU với máy tính

### 2. Clone / tải dự án

```bash
git clone <repository-url>
cd TEAM_05/NguyenCongHieu/ESP32_Project
```

### 3. Build dự án

```bash
pio run
```

### 4. Nạp firmware lên board

```bash
pio run --target upload
```

### 5. Xem Serial Monitor

```bash
pio device monitor --baud 115200
```

---

## Mô tả mã nguồn (`src/main.cpp`)

### Hằng số cấu hình

| Hằng số | Giá trị | Ý nghĩa |
|---|---|---|
| `LED_PIN` | `LED_BUILTIN` | Chân LED tích hợp |
| `DHT_PIN` | `D3` | Chân data của DHT11 |
| `MQ2_PIN` | `A0` | Chân ADC đọc MQ-2 |
| `DHT_TYPE` | `DHT11` | Loại cảm biến DHT |
| `LED_BLINK_MS` | `500 ms` | Chu kỳ nhấp nháy LED |
| `SENSOR_READ_MS` | `2000 ms` | Chu kỳ đọc cảm biến |

### Các hàm chính

| Hàm | Mô tả |
|---|---|
| `setup()` | Khởi tạo UART (115200 baud), DHT, I2C, OLED; đọc cảm biến lần đầu |
| `loop()` | Vòng lặp chính: nhấp nháy LED, đọc cảm biến & cập nhật OLED mỗi 2 s |
| `readSensors()` | Đọc nhiệt độ, độ ẩm (DHT11) và giá trị ADC (MQ-2); in ra Serial |
| `drawOled()` | Vẽ dữ liệu cảm biến lên màn hình OLED (font 6×12) |
| `updateBlink()` | Quản lý nhấp nháy LED không chặn (non-blocking) bằng `millis()` |

### Luồng xử lý

```
setup()
  ├─ Khởi tạo LED, Serial, Wire, DHT, OLED
  └─ Đọc & hiển thị dữ liệu ban đầu

loop() [lặp liên tục]
  ├─ updateBlink()   → Toggle LED mỗi 500 ms
  └─ mỗi 2000 ms:
       ├─ readSensors()  → Đọc DHT11 + MQ-2, in Serial
       └─ drawOled()     → Cập nhật màn hình OLED
```

---

## Ví dụ đầu ra Serial Monitor

```
Khoi dong ESP8266 Controller...
Nhiet do hien tai: 28.5 C
Do am hien tai: 65.0 %
MQ2 raw: 312

Nhiet do hien tai: 28.6 C
Do am hien tai: 64.8 %
MQ2 raw: 315
```

---

## Bố cục màn hình OLED

```
┌─────────────────────┐
│  ESP8266 Controller │  ← Tiêu đề
│─────────────────────│
│  Nhiet do: 28.5 C   │  ← Nhiệt độ
│  Do am: 65.0 %      │  ← Độ ẩm
│  MQ2:  312 (30%)    │  ← Nồng độ khí (raw & %)
└─────────────────────┘
```

---

