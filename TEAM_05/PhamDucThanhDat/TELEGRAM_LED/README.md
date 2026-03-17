# Telegram LED Control with PIR Sensor – ESP32

Điều khiển đèn LED và nhận cảnh báo chuyển động qua Telegram Bot sử dụng ESP32, mô phỏng trên Wokwi.

## Mục lục
- [Giới thiệu](#giới-thiệu)
- [Tính năng](#tính-năng)
- [Phần cứng & Sơ đồ mạch](#phần-cứng--sơ-đồ-mạch)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Thư viện sử dụng](#thư-viện-sử-dụng)
- [Cài đặt & Cấu hình](#cài-đặt--cấu-hình)
- [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
- [Mô phỏng trên Wokwi](#mô-phỏng-trên-wokwi)
- [Kết quả thực tế](#kết-qua-thực-tế)

## Giới thiệu
Dự án sử dụng vi điều khiển ESP32 kết nối WiFi để giao tiếp với Telegram Bot API, cho phép người dùng:

- Điều khiển từ xa trạng thái LED (bật/tắt) thông qua tin nhắn Telegram.
- Nhận cảnh báo tự động khi cảm biến PIR phát hiện chuyển động.

Dự án được mô phỏng hoàn toàn trên Wokwi và xây dựng bằng PlatformIO + Arduino Framework.

## Tính năng
| Lệnh Telegram | Mô tả |
|---------------|-------|
| `/start`      | Hiển thị hướng dẫn sử dụng bot |
| `/led_on`     | Bật đèn LED |
| `/led_off`    | Tắt đèn LED |
| `/get_state`  | Kiểm tra trạng thái LED hiện tại |
| _(tự động)_   | Cảnh báo khi PIR phát hiện chuyển động |

- **Bảo mật**: Chỉ CHAT_ID được cấu hình mới có quyền điều khiển bot.
- **WiFi tự động**: Kết nối WiFi và tự thiết lập DNS (Google DNS 8.8.8.8).
- **Polling vòng lặp**: Bot kiểm tra tin nhắn mới mỗi 2 giây.

## Phần cứng & Sơ đồ mạch
### Linh kiện sử dụng
| Linh kiện | Số lượng | Ghi chú |
|-----------|----------|---------|
| ESP32 DevKit C v4 | 1 | Vi điều khiển chính |
| LED đỏ    | 1 | Đèn báo trạng thái |
| Điện trở 220Ω | 1 | Hạn dòng cho LED |
| Cảm biến PIR | 1 | Phát hiện chuyển động |

### Kết nối chân (Pin Mapping)
| ESP32 Pin | Kết nối với |
|-----------|-------------|
| GPIO23    | Anode LED (qua R 220Ω) |
| GPIO27    | OUTPUT của cảm biến PIR |
| 3.3V      | VCC cảm biến PIR |
| GND       | GND của LED và PIR |

### Sơ đồ mạch (Wokwi)
```
ESP32
 ├── GPIO23 ──► [R 220Ω] ──► LED(+) ──► GND
 ├── GPIO27 ◄── PIR(OUT)
 ├── 3V3    ──► PIR(VCC)
 └── GND    ──► PIR(GND)
```

## Cấu trúc dự án
```
Telegram_Led/
├── src/
│   └── main.cpp          # Mã nguồn chính (logic bot + PIR + LED)
├── images result/
│   ├── wokwi.png         # Ảnh mô phỏng mạch trên Wokwi
│   └── telebot.png       # Ảnh kết quả trên Telegram
├── diagram.json          # Sơ đồ mạch Wokwi
├── platformio.ini        # Cấu hình PlatformIO (board, thư viện)
├── wokwi.toml            # Cấu hình Wokwi Simulator
├── .gitignore
└── README.md
```
> **Lưu ý:** File `secrets.h` chứa `BOT_TOKEN` và `CHAT_ID` không được đưa lên repository (đã có trong `.gitignore`). Xem hướng dẫn tạo file này ở phần Cài đặt.

## Thư viện sử dụng
| Thư viện | Phiên bản | Mục đích |
|----------|-----------|----------|
| WiFi.h (built-in) | – | Kết nối mạng WiFi |
| WiFiClientSecure.h (built-in) | – | Kết nối HTTPS/TLS |
| UniversalTelegramBot | ^1.3.0 | Giao tiếp với Telegram Bot API |
| ArduinoJson | ^7.2.1 | Phân tích JSON từ API Telegram |

**Cấu hình thư viện trong `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board    = esp32dev
framework = arduino
lib_deps =
    bblanchon/ArduinoJson@^7.2.1
    witnessmenow/UniversalTelegramBot@^1.3.0
```

## Cài đặt & Cấu hình
**Bước 1: Tạo Telegram Bot**
1. Mở Telegram → tìm `@BotFather`.
2. Gửi lệnh `/newbot` và làm theo hướng dẫn.
3. Lưu lại Bot Token (dạng `123456789:AAFxxxx...`).
4. Lấy Chat ID của bạn bằng cách nhắn tin cho bot `@userinfobot`.

**Bước 2: Tạo file `secrets.h`**
Tạo file `src/secrets.h` với nội dung:
```c
#define BOT_TOKEN  "YOUR_BOT_TOKEN_HERE"
#define CHAT_ID    "YOUR_CHAT_ID_HERE"
```
> **Không** commit file này lên GitHub. File đã được thêm vào `.gitignore`.

**Bước 3: Cài đặt PlatformIO**
1. Cài Visual Studio Code.
2. Cài extension PlatformIO IDE từ VS Code Marketplace.
3. Mở thư mục dự án bằng VS Code.

**Bước 4: Build và nạp code**
```bash
# Build firmware
pio run

# Nạp lên ESP32 thật
pio run --target upload

# Hoặc chạy mô phỏng Wokwi (xem phần bên dưới)
```

## Hướng dẫn sử dụng
- ESP32 sẽ kết nối WiFi (Serial Monitor hiển thị "WiFi connected").
- Mở Telegram, tìm tên bot bạn đã tạo.
- Gửi các lệnh:
  - `/start` → Hiển thị menu hướng dẫn
  - `/led_on` → Bật đèn LED
  - `/led_off` → Tắt đèn LED
  - `/get_state` → Xem trạng thái đèn hiện tại
- Khi cảm biến PIR phát hiện chuyển động, bot sẽ tự động gửi cảnh báo đến Telegram.

## Mô phỏng trên Wokwi
Dự án hỗ trợ mô phỏng trực tiếp trên Wokwi bằng extension Wokwi for VS Code.

**Cách chạy mô phỏng**
1. Cài extension Wokwi for VS Code.
2. Build firmware trước: `pio run`.
3. Nhấn F1 → chọn "Wokwi: Start Simulator".
4. Mạch sẽ tự tải theo `diagram.json`.

**Lưu ý khi mô phỏng**
- WiFi SSID sử dụng: `Wokwi-GUEST` (không cần mật khẩu).
- DNS được ghi đè sang `8.8.8.8` (Google DNS) để bypass giới hạn DNS của Wokwi.
- SSL certificate verification bị tắt (`setInsecure()`) – chỉ dùng cho mô phỏng/học tập.

## Kết quả thực tế
