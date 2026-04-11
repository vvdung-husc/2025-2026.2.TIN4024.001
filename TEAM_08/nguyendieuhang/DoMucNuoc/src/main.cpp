#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL6QD3U9QO4"
#define BLYNK_TEMPLATE_NAME "Đo Mức Nước"
#define BLYNK_AUTH_TOKEN "5IcL6wBZsJ0JN0hqUM_mneQhEtNaTGn4"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// --- THÔNG TIN MẠNG WIFI CỦA WOKWI ---
char ssid[] = "Wokwi-GUEST"; // Wi-Fi giả lập của Wokwi
char pass[] = "";            // Mật khẩu để trống

// --- ĐỊNH NGHĨA CÁC CHÂN KẾT NỐI (Theo file JSON) ---
#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_PIN 19
#define BTN_PIN 4

// --- THÔNG SỐ BỂ CHỨA NƯỚC (Đơn vị: cm) ---
const float TANK_HEIGHT = 200.0; // Chiều cao tối đa của bể (từ cảm biến đến đáy)
const float SAFE_MARGIN = 20.0;  // Khoảng cách an toàn (tránh nước chạm cảm biến)

// Khởi tạo màn hình LCD (Địa chỉ 0x27, 16 cột, 2 hàng)
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// Hàm thực hiện đo đạc và gửi dữ liệu
void measureWaterLevel() {
  // 1. Kích hoạt cảm biến phát sóng siêu âm
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 2. Đọc thời gian sóng phản hồi và tính khoảng cách
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  // 3. Quy đổi sang phần trăm (%) mực nước
  int percent = 0;
  if (distance >= TANK_HEIGHT) {
    percent = 0; // Cạn hoàn toàn
  } else if (distance <= SAFE_MARGIN) {
    percent = 100; // Đầy bình
  } else {
    // Áp dụng thuật toán map() để tính %
    percent = map(distance, SAFE_MARGIN, TANK_HEIGHT, 100, 0);
  }

  // 4. Hiển thị thông số lên màn hình LCD
  lcd.setCursor(0, 0);
  lcd.print("Muc nuoc: ");
  lcd.print(percent);
  lcd.print("%   "); // Thêm khoảng trắng để xóa ký tự thừa cũ

  lcd.setCursor(0, 1);
  lcd.print("Khoang cach:");
  lcd.print((int)distance);
  lcd.print("cm  ");

  // 5. Gửi dữ liệu lên máy chủ Blynk
  Blynk.virtualWrite(V0, percent);   // V0: Phần trăm nước
  Blynk.virtualWrite(V1, distance);  // V1: Khoảng cách thực tế

  // 6. Xử lý cảnh báo (Cạn dưới 10% hoặc đầy trên 90%)
  if (percent <= 10 || percent >= 90) {
    digitalWrite(LED_PIN, HIGH); // Bật LED cảnh báo
    Blynk.virtualWrite(V2, 1);   // Gửi tín hiệu cảnh báo lên app
  } else {
    digitalWrite(LED_PIN, LOW);  // Tắt LED
    Blynk.virtualWrite(V2, 0);
  }
}

void setup() {
  Serial.begin(115200);

  // Thiết lập chế độ cho các chân GPIO
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Khởi động màn hình LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Dang ket noi...");

  // Kết nối Wi-Fi và Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thông báo kết nối thành công
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WIFI OK!");
  delay(1500);
  lcd.clear();

  // Thiết lập đồng hồ đếm nhịp: Gọi hàm measureWaterLevel mỗi 1000ms (1 giây)
  timer.setInterval(1000L, measureWaterLevel);
}

void loop() {
  // Duy trì hoạt động của Blynk và Timer (Tuyệt đối không dùng delay trong đây)
  Blynk.run();
  timer.run();
}