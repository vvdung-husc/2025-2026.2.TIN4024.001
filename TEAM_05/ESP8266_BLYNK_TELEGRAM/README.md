# ESP32 IoT Project - Tích hợp nền tảng Blynk & Telegram Bot

Dự án sử dụng vi điều khiển ESP32, cho phép theo dõi, cảnh báo thay đổi môi trường và điều khiển thiết bị từ xa (LED) được tích hợp song song thông qua ứng dụng **Blynk** cùng với **Telegram Bot**. Dự án cũng hỗ trợ chạy mô phỏng trực tiếp trên môi trường **Wokwi**.

## Nhóm Thực Hiện (Team 05)
1. Nguyễn Công Hiếu
2. Phạm Đức Thành Đạt
3. Trần Văn Tiến
4. Đậu Thiên Nhật

---

## Các Chức Năng Của Hệ Thống

1. **Hiển thị thông tin tại chỗ (OLED):**
   - Màn hình sử dụng OLED SSD1306 hiển thị thông tin các thành viên của Nhóm 05 lúc khởi động.
   - Thường xuyên cập nhật trạng thái BẬT / TẮT của một bóng đèn LED.
2. **Theo dõi và quản lý qua nền tảng Blynk:**
   - Đọc dữ liệu từ cảm biến `DHT22` (nhiệt độ, độ ẩm) và thông số Gas giả lập định kỳ sau mỗi 2.5s rồi gửi thông số lên Blynk Dashboard `(Datastreams V2, V3, V4)`.
   - Lưu trữ Uptime (thời gian hoạt động của mạch).
   - Có thể điều khiển đèn LED từ trên app và đồng bộ trạng thái về bo mạch `(Datastream V1)`.
3. **Điều khiển và cảnh báo qua Telegram Bot:**
   - Tương tác để bật, tắt, hoặc xem trạng thái LED thông qua các lệnh với Bot.
   - Gửi lại thống kê thông số khí hậu khi được yêu cầu.
   - **Tự động gửi cảnh báo (Notification)** về máy khi môi trường thay đổi đáng kể (nhiệt độ lệch 1°C hoặc độ ẩm lệch > 2%).

---

## Linh Kiện & Dịch Vụ

- **Bo mạch:** ESP32 Development Board (esp32doit-devkit-v1).
- **Màn hình:** OLED SSD1306.
- **Cảm biến:** Nhiệt độ và Độ ẩm DHT22.
- **LED thông báo:** Bất kỳ loại LED nào có trong linh kiện.
- **Nền tảng Firmware:** PlatformIO thông qua Arduino Framework.
- **Mô phỏng:** [Wokwi](https://wokwi.com).

---

## Tổng Quan Cấu Trúc Dự Án

```
ESP8266_BLYNK_TELEGRAM/
│
├── diagram.json          # File chứa dữ liệu các thành phần kết nối cho mô phỏng mạch trên Wokwi
├── platformio.ini        # Tệp tin cấu hình chung của PlatformIO chứa thông tin Board mạch và thư viện
├── wokwi.toml            # Tệp cấu hình của hệ thống Wokwi
│
├── src/                  
│   ├── main.cpp          # Tệp chính xử lý toàn bộ logic và cấu hình hệ thống
│   └── secrets.h         # (Được tự định nghĩa thêm) Chứa token Blynk, Wifi SSID/Pass, Telegram Bot Token,... 
│
├── images result/        # Chứa những hình ảnh demo, kết nối sau khi chạy dự án
│   ├── team05.png
│   ├── telebot.png
│   └── web.png
│
├── lib/                  # Đường dẫn cấu hình cho thư viện thêm vào riêng 
└── include/              # (Cấu trúc có sẵn của PlatformIO)
```

---

## Cấu Hình Đấu Nối Các Chân (Wiring)

| Linh Kiện    | ESP32 Pin      | Ghi Chú                                          |
|--------------|----------------|--------------------------------------------------|
| **DHT22**    | GPIO 15        | Lấy dữ liệu thời tiết (Data Pin)                 |
| **LED**      | GPIO 2         | Đại diện cho thiết bị nhận lệnh Bật/Tắt          |
| **OLED SDA** | GPIO 21        | I2C Data                                         |
| **OLED SCL** | GPIO 22        | I2C Clock                                        |

---

## Hướng Dẫn Sử Dụng Telegram Bot

Bot Telegram hỗ trợ các câu lệnh (Commands) điều khiển sau đây, bạn có thể gửi trực tiếp từ chat log:

- `/led_on` : Kích hoạt bật bóng đèn LED (đồng bộ ngay lập tức với OLED và Blynk).
- `/led_off`: Vô hiệu hóa, tắt bóng đèn LED.
- `/led_status` : Kiểm tra trạng thái hiện tại của đèn (Bật hay Tắt).
- `/get_weather`: Yêu cầu gửi lại thông số nhiệt độ và Độ ẩm ở thời điểm thực tại từ cảm biến.

---

## Cách Chạy Dự Án

### 1. Cấu hình thông số (Secrets)
Tạo file `src/secrets.h` hoặc kiểm tra code gốc để thiết lập Token của bạn, bao gồm:
```cpp
#define BLYNK_AUTH_TOKEN "Your_Blynk_Token_Here"
#define BOT_TOKEN "Your_Telegram_Bot_Token_Here"
#define CHAT_ID "Your_Telegram_Chat_ID"
```

### 2. Mô phỏng (Simulation Tool)
- Nếu dùng trực tiếp Extension **Wokwi** trong VSCode: Chọn file `diagram.json` -> Bấm Start Simulation.
- Hoặc sao chép Code gốc dán vào trình mô phỏng trên web (Wokwi.com). Hệ thống Wokwi đi kèm mạng WiFi nội bộ `Wokwi-GUEST`. 

### 3. Build & Nạp Board Thực (Hardware)
- Gắn mạch ESP32 vào cổng USB.
- Mở Terminal của PlatformIO trong VSCode, sử dụng lệnh `pio run -t upload` để biên dịch trực tiếp và nạp vào mạch, hoặc dùng công cụ GUI của PlatformIO.
- Bật Serial Monitor với baudrate: `115200` để tham chiếu kết nối WiFi hay lấy log trạng thái của Bo mạch.
