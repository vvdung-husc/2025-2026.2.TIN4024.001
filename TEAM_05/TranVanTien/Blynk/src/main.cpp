// --- THÔNG TIN DỰ ÁN BLYNK CỦA BẠN ---
#define BLYNK_TEMPLATE_ID "TMPL6QoftEiTy"
#define BLYNK_TEMPLATE_NAME "Blynkled"
#define BLYNK_AUTH_TOKEN "BwhDV3kdHZ1V_PZh-EFkGng7MO4Za8kG"

// Bật tính năng in log của Blynk ra Serial Monitor
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- THÔNG TIN WIFI CỦA WOKWI (BẮT BUỘC) ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- CẤU HÌNH CHÂN THEO DIAGRAM.JSON ---
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BTN_PIN 23
#define CLK 18
#define DIO 19

// --- KHỞI TẠO ĐỐI TƯỢNG ---
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// --- BIẾN TRẠNG THÁI ---
int uptime = 0;
bool isDeviceOn = false; // Trạng thái của Đèn và Màn hình
int lastBtnState = HIGH; // Mặc định nút nhấn nhả (Pull-up)

// 👉 1. Hàm này tự động chạy 1 lần duy nhất khi ESP32 kết nối Blynk thành công
BLYNK_CONNECTED() {
  // Gửi tên của bạn lên V4 (Nhớ cài đặt V4 là String trên Web nhé)
  Blynk.virtualWrite(V4, "Tran Van Tien");
  
  // Đồng bộ trạng thái thực tế của nút V1 từ App về mạch
  Blynk.syncVirtual(V1); 
}

// 👉 2. Hàm nhận lệnh từ Nút nhấn trên App Blynk (Virtual Pin V1)
BLYNK_WRITE(V1) {
  isDeviceOn = param.asInt(); // Đọc giá trị từ app (0 hoặc 1)
  digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
  
  if (!isDeviceOn) {
    display.clear(); // Tắt màn hình nếu nút đang OFF
  }
}

// 👉 3. Hàm đọc cảm biến và gửi lên Blynk (Chạy mỗi 2 giây)
void sendDataToBlynk() {
  // Cập nhật và gửi thời gian hoạt động
  uptime += 2; 
  Blynk.virtualWrite(V0, uptime);

  // Hiển thị thời gian lên TM1637 nếu trạng thái đang BẬT
  if (isDeviceOn) {
    display.showNumberDec(uptime);
  }

  // Đọc cảm biến nhiệt độ & độ ẩm
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Chỉ gửi nếu dữ liệu hợp lệ (không bị lỗi NaN)
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t); 
    Blynk.virtualWrite(V3, h);
  }
}

// 👉 4. Hàm xử lý nút nhấn cứng trên mạch
void checkPhysicalButton() {
  int btnState = digitalRead(BTN_PIN);
  
  // Phát hiện cạnh lên (nhấn vào)
  if (lastBtnState == HIGH && btnState == LOW) {
    isDeviceOn = !isDeviceOn; // Đảo trạng thái
    digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
    
    if (!isDeviceOn) {
      display.clear();
    }
    
    // Cập nhật trạng thái mới này lên Nút nhấn trên App Blynk
    Blynk.virtualWrite(V1, isDeviceOn);
  }
  lastBtnState = btnState;
}

void setup() {
  Serial.begin(115200);

  // Cài đặt chân I/O
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Khởi động các thiết bị ngoại vi
  dht.begin();
  display.setBrightness(0x0f); // Độ sáng tối đa
  display.clear();
  
  // Kết nối Blynk
  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thiết lập Timer chạy tuần hoàn
  timer.setInterval(2000L, sendDataToBlynk);      // Gửi dữ liệu mỗi 2s (Đảm bảo DHT22 không treo)
  timer.setInterval(100L, checkPhysicalButton);   // Kiểm tra nút nhấn mỗi 0.1s
}

void loop() {
  // Chỉ gọi 2 lệnh này trong loop() để đảm bảo Blynk không bị ngắt kết nối
  Blynk.run();
  timer.run();
}