#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/* Thông tin Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6oufq1SFq"
#define BLYNK_TEMPLATE_NAME "TRAFFIC SENSORS"
#define BLYNK_AUTH_TOKEN "rvmNeD8a7mUGsX4Qkd61T8c3clF3aUUq"

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST"; // Tên WiFi mặc định của Wokwi 
char pass[] = "";

// Định nghĩa chân cắm
#define btnBLED  23
#define pinBLED  21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

/* ===== KHỞI TẠO ĐỐI TƯỢNG ===== */
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

/* ===== BIẾN TOÀN CỤC ===== */
unsigned long activeSeconds = 0;
bool blueButtonON = false; // Mặc định tắt khi mới cấp nguồn

/* ===================== FUNCTIONS ===================== */

// Đọc cảm biến và gửi lên Blynk
void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, temperature); 
    Blynk.virtualWrite(V3, humidity);    
  }
}

// Cập nhật thời gian hoạt động
void uptimeTask() {
  if (blueButtonON) {
    activeSeconds++;
    display.showNumberDec(activeSeconds);
    
    if (Blynk.connected()) {
      Blynk.virtualWrite(V0, String(activeSeconds) + "s");
    }
  }
}

// Kiểm tra nút nhấn vật lý (Sử dụng Timer gọi hàm này mỗi 100ms)
void checkPhysicalButton() {
  static int lastState = HIGH;
  int state = digitalRead(btnBLED);

  if (state == LOW && lastState == HIGH) { // Có sự kiện nhấn nút
    blueButtonON = !blueButtonON;
    
    // Cập nhật LED vật lý
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    
    // Đồng bộ trạng thái lên App Blynk (Switch V1)
    if (Blynk.connected()) {
      Blynk.virtualWrite(V1, blueButtonON ? 1 : 0);
    }

    if (!blueButtonON) {
      display.clear();
    }
  }
  lastState = state;
}

/* ----------- Nhận lệnh từ App Blynk (V1) ----------- */
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) {
    display.clear();
  }
  Serial.print("Blynk App Triggered: ");
  Serial.println(blueButtonON ? "ON" : "OFF");
}

// Hàm này chạy khi ESP32 kết nối thành công với Blynk Server
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1); // Yêu cầu Server gửi lại trạng thái mới nhất của V1
}

/* ======================= SETUP ======================= */

void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  // Khởi tạo trạng thái LED dựa trên biến blueButtonON
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  // Kết nối WiFi và Cấu hình Blynk (Cơ chế non-blocking)
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  // Thiết lập các tác vụ định kỳ
  timer.setInterval(1000L, uptimeTask);           // Đếm giây mỗi 1s
  timer.setInterval(2000L, sendSensorData);       // Đọc DHT mỗi 2s
  timer.setInterval(100L,  checkPhysicalButton);  // Kiểm tra nút nhấn mỗi 100ms

  Serial.println("=== SYSTEM READY ===");
}

/* ======================== LOOP ======================= */

void loop() {
  // Chạy Blynk.run() mà không cần check WiFi thủ công
  // Blynk sẽ tự động kết nối lại khi có mạng
  Blynk.run();
  timer.run();
}