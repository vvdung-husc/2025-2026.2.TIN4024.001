# ESP32 API — HTTP GET & Blynk IoT

Dự án lập trình ESP32 sử dụng **HTTP GET API** để lấy thông tin địa lý từ địa chỉ IP (IPv4, Latitude, Longitude), tạo link Google Maps và hiển thị nhiệt độ thời tiết thực tế qua **Blynk IoT** (Web + Mobile).
## Chức năng

- Kết nối WiFi và gọi API `ip-api.com` để lấy **IPv4, Latitude, Longitude**
- Tạo **Link Google Maps** từ tọa độ và hiển thị tại Terminal (Serial Monitor)
- Gọi API `openweathermap.org` để lấy **nhiệt độ thực tế** theo vị trí
- Gửi dữ liệu lên **Blynk IoT** (web & mobile) qua Virtual Pins

---

## Blynk Virtual Pins

| Pin | Loại | Nội dung |
|-----|------|----------|
| `V0` | Integer | Thời gian hoạt động (giây) |
| `V1` | String  | Địa chỉ IPv4 |
| `V2` | String  | Link Google Maps |
| `V3` | Double  | Nhiệt độ (°C) — range: -40 → 80 |
| `V4` | String  | Tên sinh viên |

---
## Thư viện sử dụng

| Thư viện | Mục đích |
|----------|----------|
| `WiFi.h` / `WiFiClient.h` | Kết nối WiFi |
| `WiFiClientSecure.h` | Gọi HTTPS (OpenWeatherMap) |
| `BlynkSimpleEsp32.h` | Giao tiếp Blynk IoT |
| `HTTPClient.h` | Gọi HTTP/HTTPS API |
| `ArduinoJson.h` | Phân tích JSON từ API |
---
##  API sử dụng

| API | Endpoint | Mô tả |
|-----|----------|-------|
| ip-api.com | `http://208.95.112.1/json/` | Lấy IP, tọa độ, địa danh |
| OpenWeatherMap | `https://api.openweathermap.org/data/2.5/weather` | Nhiệt độ theo lat/lon |

---

## Hướng dẫn chạy trên Wokwi

1. Mở dự án tại [wokwi.com](https://wokwi.com) (import từ `diagram.json`)
2. Đảm bảo đã điền đúng:
   - `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, `BLYNK_AUTH_TOKEN` trong `main.cpp`
   - `OPENWEATHERMAP_KEY` — đăng ký miễn phí tại [openweathermap.org](https://openweathermap.org)
3. Nhấn **▶ Play** để chạy
4. Xem kết quả tại **Terminal** (Serial Monitor) và **Blynk Dashboard**

>  **Lưu ý Wokwi:** Simulator không hỗ trợ DNS cho một số domain. Dự án đã xử lý bằng cách dùng IP trực tiếp. Nhiệt độ trên Wokwi hiển thị giá trị demo 28.5°C; trên phần cứng thật sẽ lấy nhiệt độ thực từ OpenWeatherMap.

---

##  Cấu trúc dự án

```
ESP32 API/
├── src/
│   └── main.cpp          # Code chính
├── images result/               # Ảnh kết quả chạy
├── diagram.json          # Sơ đồ mạch Wokwi
├── platformio.ini        # Cấu hình PlatformIO
└── README.md
```
---

##  Kết quả Terminal

```
Connecting to WiFi Wokwi-GUEST. Connected!
IP Address: 116.98.248.189
Country: Vietnam
Region: Thừa Thiên Huế Province
City: Huế
Longitude: 107.6009
Latitude: 16.4704
IPv4 => 116.98.248.189
https://www.google.com/maps/place/16.4704,107.6009
Blynk connected!
```
