# ⚡ Quick Start - Chạy Trên Wokwi (10 Phút)

## 🎯 Mục Tiêu
Chạy hệ thống LED RGB trên **mô phỏng Wokwi** trong **10 phút** - **KHÔNG cần hardware thật!**

---

## 📦 Bạn Cần

- ✅ **Máy tính** (Windows/Mac/Linux)
- ✅ **Trình duyệt** (Chrome, Firefox, Edge)
- ✅ **VS Code** (optional - hoặc dùng web Wokwi)
- ✅ **Code đã sẵn sàng** ✓

**KHÔNG cần**: ESP32, LED, dây cấp, USB, etc.

---

## 🚀 Bước 1: Vào Wokwi (2 phút)

### Cách 1: Dùng Web (Đơn Giản Nhất)
```
https://wokwi.com/projects/new/esp32
```

Hoặc:
```
1. Vào https://wokwi.com
2. Click "New Project"
3. Chọn "ESP32"
```

### Cách 2: Dùng VS Code (Nếu Cài Extension)
```
F1 → Wokwi: Start Simulator
```

---

## 🔧 Bước 2: Paste Code (2 phút)

### 2.1 Mở File Code
```
src/main.cpp → Copy toàn bộ code
```

### 2.2 Paste Vào Wokwi
```
Wokwi → File Editor
Paste code từ main.cpp
```

---

## 📡 Bước 3: Wokwi Tự Động Setup (1 phút)

Wokwi sẽ **tự động**:
```
✅ Nhận diện diagram.json
✅ Tạo ESP32 ảo
✅ Thêm NeoPixel Strip (60 LED)
✅ Kết nối GPIO5 → LED Data
```

Bạn sẽ thấy:
```
[Wire up] 
ESP32 + NeoPixel Strip mô phỏng
```

---

## ▶️ Bước 4: Bấm Play (1 phút)

```
Wokwi → Nút ▶️ (Play)
Hoặc: F5
```

**Chờ 5-10 giây** để compile + run.

Bạn sẽ thấy:
```
[Compilation...]
[Upload...]
Starting LED RGB Control System...
.....
WiFi connected!
IP address: 192.168.1.1
HTTP server started
```

✅ **LED System đang chạy!**

---

## 💻 Bước 5: Truy Cập Web UI (1 phút)

### 5.1 Tìm Web Preview
```
Wokwi → KIconn "Web Preview" hoặc "localhost"
(hoặc gõ: http://localhost/api/status)
```

### 5.2 Bạn Sẽ Thấy
```
🎨 LED RGB Control
├─ Chọn chế độ: Solid Color, Rainbow, Pulse, Fade, Strobe
├─ Bộ chọn màu
├─ Slider độ sáng
└─ Slider tốc độ
```

### 5.3 Điều Khiển LED!
```
1. Chọn "Rainbow"
2. Xem LED ảo chạy cầu vồng bên trái
3. Chọn "Solid Color" → Pick màu đỏ
4. Kéo slider độ sáng
```

🎉 **HOÀN THÀNH! LED của bạn đang chạy trên mô phỏng!**

---

## 📊 So Sánh: Web vs Wokwi

| Tính Năng | Hardware Thật | Wokwi |
|-----------|--------|--------|
| LED sáng | Ánh sáng thực | 3D mô phỏng |
| WiFi | Thực | Giả lập (auto-connect) |
| Kết nối | USB | Chỉ cần browser |
| Setup | 30 phút | 10 phút |
| Code | Giống nhau 100% | Giống nhau 100% |

---

## 🎮 Các Chế Độ LED

```
1. Solid Color     → Màu tĩnh, có thể chọn bất kỳ màu nào
2. Rainbow        → Cầu vồng chạy trôi
3. Pulse          → Nhấp nháy nhẹ từ sáng → tối
4. Fade           → Mờ dần từ sáng → tối (chậm hơn)
5. Strobe         → Flashing nhanh
```

---

## 🎨 Test Nhanh

### Test 1: Solid Color
```
1. Bấm "Solid Color"
2. Click vào ô chọn màu (color picker)
3. Chọn màu đỏ
→ LED ảo sẽ chuyển sang màu đỏ
```

### Test 2: Rainbow
```
1. Bấm "Rainbow"
2. Xem LED chạy cầu vồng liên tục
3. Kéo slider "Tốc độ" để tăng/giảm tốc
```

### Test 3: Brightness Control
```
1. Bất kỳ chế độ nào
2. Kéo slider "Độ sáng"
→ LED sáng hoặc tối theo
```

---

## 📱 Serial Monitor

Xem output thực thời:
```
Wokwi → Serial Monitor (tab dưới)
Hoặc: Ctrl+Shift+J
```

Bạn sẽ thấy:
```
WiFi connected!
IP address: 192.168.1.1
HTTP server started
[GET] /api/status
[POST] /api/color
```

---

## 🆘 Gặp Vấn Đề?

### ❌ Wokwi không compile
```
→ Lỗi C++? Check code syntax
→ Include missing? Wokwi sẽ cảnh báo
```

### ❌ Web UI không mở
```
→ Bấm Play lại
→ Chờ 10 giây
→ Refresh browser
```

### ❌ LED không hiển thị
```
→ Trong diagram.json, check GPIO5 kết nối chưa?
→ Reload Wokwi
```

---

## 🚀 Tiếp Theo?

### Khi LED đã chạy, bạn có thể:

✅ **Sửa code để thêm hiệu ứng**
```cpp
// Thêm trong void loop()
// Bạn sẽ thấy ngay trên Wokwi
```

✅ **Test API với curl/Python**
```bash
curl http://localhost/api/color -X POST -H "Content-Type: application/json" -d '{"r":255,"g":0,"b":0}'
```

✅ **Lưu project trên Wokwi**
```
Share → Copy link → Gửi bạn bè
```

✅ **Chuyển sang Hardware Thật**
```
Sửa SSID/Password → Upload vào ESP32 thật
Code hoàn toàn giống nhau!
```

---

## 💾 File Cấu Hình

```
diagram.json  ← Mô phỏng Wokwi (ESP32 + LED)
wokwi.toml    ← Build settings
main.cpp      ← Code chính
```

---

## ✨ Mẹo Nhanh

✅ **Wokwi chạy nhanh 1000x hơn hardware**
```
Delay 1 giây trong code → Chỉ 1ms trên Wokwi
```

✅ **Có thể pause & debug**
```
⏸️ Pause
Nhìn Serial Monitor
Debug từng dòng
```

✅ **Có thể share public**
```
Wokwi → Share Project
Link: wokwi.com/projects/...
Bạn bè click vào chạy ngay (không cần code)
```

---

## 📖 Tài Liệu Thêm

**File: QUICKSTART.md** ← Bạn đang đọc  

Các file khác đã xóa để giữ project gọn gàng.

---

## 🎓 Đó là Tất Cả!

**Total: 10 phút**

Bây giờ bạn có:
```
✅ Hệ thống LED RGB chạy
✅ Web UI điều khiển
✅ 5 hiệu ứng LED
✅ Độ sáng & chọn màu
✅ Chạy trên mô phỏng 100%
```

---

**Bạn có thể:**
- 🔧 Sửa code
- 🎨 Thêm hiệu ứng
- 📤 Upload lên hardware thật (sau)
- 🌐 Share project

---

**Happy Coding!** 🚀

*P.S. Khi sẵn sàng chuyển sang hardware thật, chỉ cần sửa WiFi credentials + upload - code không đổi!*
