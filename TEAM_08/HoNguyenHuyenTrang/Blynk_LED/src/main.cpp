#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK"
#define BLYNK_TEMPLATE_ID "TMPL6zjVI8UOk"
#define BLYNK_AUTH_TOKEN "YWRVTmyvYZ5PZwAMgOthN3GvBmAI5yJ2"

// Khai báo in thông tin BLYNK ra Serial Monitor
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TMPL6zjVI8UOk.h>

// --- THÔNG TIN WIFI CỦA WOKWI (BẮT BUỘC) ---
// Mạng Wokwi-GUEST là mạng giả lập internet riêng của Wokwi, không cần pass
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

// 1. Hàm nhận lệnh từ Nút nhấn trên Blynk (Virtual Pin V1)
BLYNK_WRITE(V1) {
  isDeviceOn = param.asInt(); // Đọc giá trị từ app (0 hoặc 1)
  digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
  
  if (!isDeviceOn) {
    display.clear(); // Tắt màn hình nếu nút đang OFF
  }
}

// 2. Hàm đọc cảm biến và gửi lên Blynk (Chạy mỗi 1 giây)
void sendDataToBlynk() {
  // Gửi thời gian hoạt động
  uptime++;
  Blynk.virtualWrite(V0, uptime);

  // Hiển thị thời gian lên TM1637 nếu trạng thái đang BẬT
  if (isDeviceOn) {
    display.showNumberDec(uptime);
  }

  // Đọc và gửi Nhiệt độ, Độ ẩm
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t); // V2 cho Nhiệt độ
    Blynk.virtualWrite(V3, h); // V3 cho Độ ẩm
  }
}

// 3. Hàm xử lý nút nhấn cứng (Trên sơ đồ mạch)
void checkPhysicalButton() {
  int btnState = digitalRead(BTN_PIN);
  
  // Phát hiện cạnh lên (nhấn vào)
  if (lastBtnState == HIGH && btnState == LOW) {
    isDeviceOn = !isDeviceOn; // Đảo trạng thái
    digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
    
    if (!isDeviceOn) display.clear();
    
    // Cập nhật trạng thái mới này lên Nút nhấn trên App Blynk
    Blynk.virtualWrite(V1, isDeviceOn);
  }
  lastBtnState = btnState;
}

void setup() {
  Serial.begin(115200);

  // Cài đặt chân
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Khởi động thiết bị
  dht.begin();
  display.setBrightness(0x0f); // Độ sáng tối đa
  display.clear();

  // Kết nối Blynk
  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thiết lập Timer
  timer.setInterval(1000L, sendDataToBlynk);      // Gửi dữ liệu mỗi 1s
  timer.setInterval(100L, checkPhysicalButton);   // Kiểm tra nút nhấn mỗi 0.1s
}

void loop() {
  Blynk.run();
  timer.run();
}