#define BLYNK_TEMPLATE_ID "TMPL6yrROMUTd"
#define BLYNK_TEMPLATE_NAME "He Thong Giam Sat"
#define BLYNK_AUTH_TOKEN "SMLQAiTuOQK50uNEiYDNXlvlSqOa89hi"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN PIN ---
#define PIN_DHT 16      
#define PIN_LED 21      
#define PIN_BTN 23      
#define CLK_TM1637 18   
#define DIO_TM1637 19   

DHT dht(PIN_DHT, DHT22);
TM1637Display display(CLK_TM1637, DIO_TM1637);
BlynkTimer timer;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Hàm đọc cảm biến
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi: Không đọc được DHT22!");
    return;
  }

  // Luôn luôn hiển thị lên màn hình LED 7 đoạn dù có mạng hay không
  display.showNumberDec(round(t), false, 2, 0);
  
  // Nếu có kết nối Blynk thì mới đẩy lên App
  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
    Serial.printf("Online - Gửi Blynk -> Nhiệt độ: %.1f°C | Độ ẩm: %.1f%%\n", t, h);
  } else {
    Serial.printf("Offline - Nhiệt độ: %.1f°C | Độ ẩm: %.1f%%\n", t, h);
  }
}

// Nhận lệnh từ App
BLYNK_WRITE(V3) {
  int value = param.asInt();
  digitalWrite(PIN_LED, value);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);
  
  Serial.println("\n--- ĐANG KẾT NỐI WIFI ---");
  WiFi.begin(ssid, pass);
  
  // Chỉ đợi WiFi 5 giây thôi, không được thì bỏ qua chạy tiếp
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK! Bắt đầu cấu hình Blynk...");
    // Dùng Blynk.config thay cho Blynk.begin để không bị treo máy
    Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80); 
  } else {
    Serial.println("\nWiFi Lỗi! Chuyển sang chạy Offline.");
  }

  // Hẹn giờ đọc cảm biến 2 giây/lần
  timer.setInterval(2000L, sendSensorData);
}

bool lastBtnState = HIGH;

void loop() {
  // Chỉ chạy Blynk nếu kết nối thành công
  if (Blynk.connected()) {
    Blynk.run();
  } else {
    // Tự động kết nối ngầm Blynk ở background
    if (WiFi.status() == WL_CONNECTED) {
       Blynk.connect(); 
    }
  }
  
  // LUÔN LUÔN chạy Timer để đọc cảm biến và hiện màn hình
  timer.run();

  // Xử lý nút nhấn vật lý
  bool btnState = digitalRead(PIN_BTN);
  if (btnState == LOW && lastBtnState == HIGH) {
    int currentLedState = digitalRead(PIN_LED);
    digitalWrite(PIN_LED, !currentLedState);
    
    if (Blynk.connected()) {
      Blynk.virtualWrite(V3, !currentLedState); 
    }
    Serial.println("Đã nhấn nút thay đổi đèn!");
    delay(200);
  }
  lastBtnState = btnState;
}