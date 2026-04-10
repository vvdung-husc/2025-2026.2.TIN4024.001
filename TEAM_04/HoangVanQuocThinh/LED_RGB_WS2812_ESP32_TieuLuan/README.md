# 🎨 Smart RGB LED Control System - Wokwi Simulation

**Chạy trên Wokwi - Không cần hardware thật!** ✨

---

## 🚀 Khởi Động Nhanh (10 phút)

### 1️⃣ Vào Wokwi
```
https://wokwi.com/projects/new/esp32
```

### 2️⃣ Paste Code
- Mở `src/main.cpp`
- Copy toàn bộ → Paste vào Wokwi

### 3️⃣ Bấm Play (▶️)
- Chờ compile (~5 giây)
- Bấm "Web Preview"

### 4️⃣ Điều Khiển LED!
- Chọn chế độ: Solid, Rainbow, Pulse, Fade, Strobe
- Chọn màu, điều chỉnh độ sáng

🎉 **Done! LED ảo đang chạy!**

---

## 📁 Cấu Trúc File

```
LED_RGB_ESP32/
├── src/main.cpp          ← CODE CHÍNH (Paste vào Wokwi)
├── diagram.json          ← Cấu hình mô phỏng (tự động load)
├── platformio.ini        ← Cấu hình build
├── wokwi.toml           ← Settings Wokwi
└── QUICKSTART.md        ← Hướng dẫn chi tiết
```

---

## 🔧 Wokwi Tự Động Setup

File `diagram.json` chứa:
- ✅ ESP32 Dev Kit
- ✅ NeoPixel Strip 60 LED
- ✅ Serial Monitor
- ✅ Kết nối GPIO5 → LED

**Không cần setup thêm!**

---

## 🎮 Các Chế Độ

| Chế Độ | Mô Tả |
|--------|-------|
| **Solid Color** | Màu tĩnh, có thể chọn bất kỳ |
| **Rainbow** | Cầu vồng chạy trôi |
| **Pulse** | Nhấp nháy nhẹ |
| **Fade** | Mờ dần từ sáng → tối |
| **Strobe** | Flashing nhanh |

---

## 🌐 Web Interface

**URL**: http://localhost/ (Wokwi tự cấp)

**Tính năng**:
- 🎨 Chọn màu RGB
- 💡 Điều chỉnh độ sáng (0-255)
- ⚙️ Chọn chế độ
- 🎯 Điều chỉnh tốc độ

---

## 📡 API REST

```bash
# Đặt màu đỏ
POST /api/color {"r":255,"g":0,"b":0}

# Độ sáng
POST /api/brightness {"brightness":200}

# Chế độ Rainbow
POST /api/mode {"mode":1}

# Tắt
POST /api/off

# Lấy trạng thái
GET /api/status
```

---

## 🐛 Debug

**Serial Monitor** (Wokwi):
```
Wokwi → Serial Monitor (tab dưới)
```

Bạn sẽ thấy:
```
WiFi connected!
IP address: 192.168.1.1
HTTP server started
```

---

## ✨ Mẹo

✅ **Wokwi compile nhanh** - 5-10 giây  
✅ **Có Web UI** - Điều khiển giống hardware  
✅ **Có Serial Monitor** - Debug dễ  
✅ **Share public** - Gửi link cho bạn  
✅ **Save & reload** - Code được lưu  

---

## 🎯 Tiếp Theo?

### Khi LED đã chạy:

1. **Sửa code** → Reload Wokwi
2. **Test API** → curl http://localhost/api/...
3. **Share project** → Copy link Wokwi
4. **Chuyển sang hardware thật** → Sửa WiFi credentials → Upload lên ESP32

---

## 📚 Tài Liệu

- **QUICKSTART.md** - Hướng dẫn chi tiết
- **src/main.cpp** - Code annotated
- **diagram.json** - Cấu hình simulation

---

## 💡 FAQ

**Q: Tôi cần hardware không?**  
A: Không! Wokwi mô phỏng 100%

**Q: LED sáng được không?**  
A: Có! Wokwi hiển thị LED dạng 3D graphics

**Q: Code có khác khi chuyển sang hardware không?**  
A: Không! Code giống 100%, chỉ sửa WiFi

**Q: Làm sao share project?**  
A: Wokwi → Share → Copy link

---

## 🚀 Bắt Đầu Ngay!

1. Vào https://wokwi.com/projects/new/esp32
2. Paste code từ `src/main.cpp`
3. Bấm Play
4. Enjoy! 🎨

---

**Tác Giả**: IoT Development Team  
**Phiên Bản**: 1.0 (Wokwi Edition)  
**Cập Nhật**: Tháng 4 năm 2026
