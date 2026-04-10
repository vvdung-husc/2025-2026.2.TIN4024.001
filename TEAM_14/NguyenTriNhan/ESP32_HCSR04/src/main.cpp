#define BLYNK_TEMPLATE_ID "TMPL6IxSOtyF6"
#define BLYNK_TEMPLATE_NAME "ESP32 HCSR04"
#define BLYNK_AUTH_TOKEN    "I5BYC7l1oHIDT3F4MjFK0-ftqARqgoDl"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Định nghĩa chân theo sơ đồ mạch của bạn
#define TRIG_PIN  5
#define ECHO_PIN  18
#define LED_PIN   2
#define BUZZER_PIN 4 

Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

void updateSystem() {
  // 1. Đo khoảng cách (cm)
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  // 2. Tính phần trăm (Cạn: 100cm, Đầy: 10cm)
  int percent = map(distance, 100, 10, 0, 100);
  percent = constrain(percent, 0, 100);

  // 3. Xác định mức độ ổn định (>= 80% là Nguy Hiểm)
  String status;
  if (percent >= 80) {
    status = "NGUY HIEM!";
    digitalWrite(LED_PIN, HIGH);      // Bật đèn LED đỏ
    tone(BUZZER_PIN, 1000); 
    delay(150);             
    tone(BUZZER_PIN, 1500); 
    delay(150);         // Hú còi báo động
  } else if (percent >= 30) {
    status = "ON DINH";
    digitalWrite(LED_PIN, LOW);       // Tắt LED
    noTone(BUZZER_PIN);       
    delay(300);        // Tắt còi
  } else {
    // TRƯỜNG HỢP: CẠN (< 30)
    status = "NUOC CAN!";
    digitalWrite(LED_PIN, HIGH); 
    tone(BUZZER_PIN, 1000); delay(150); 
    tone(BUZZER_PIN, 1500); delay(150);
  }

  // 4. Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, status);    // Gửi chữ lên Label V0 (String)
  Blynk.virtualWrite(V1, percent);   // Gửi số lên Gauge/Chart V1 (Double)

  // 5. Hiển thị lên OLED
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Hiển thị phần trăm nước
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("MUC NUOC:");
  display.setTextSize(2);
  display.setCursor(45, 12);
  display.print(percent); display.print("%");

  // Hiển thị trạng thái ổn định
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("TRANG THAI:");
  display.setTextSize(2);
  display.setCursor(0, 48);
  display.println(status);

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();
  display.display();

  // Kết nối WiFi (Dùng Wokwi-GUEST cho giả lập)
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  
  // Cập nhật hệ thống mỗi 1 giây
  timer.setInterval(1000L, updateSystem);
}

void loop() {
  Blynk.run();
  timer.run();
}