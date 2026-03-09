#define BLYNK_TEMPLATE_ID "TMPL64uSRDbGx"
#define BLYNK_TEMPLATE_NAME "Traffic DHT"
#define BLYNK_AUTH_TOKEN "jUB3r-q1gcgicGMynE8qeJDo4uPJgsn2"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN PIN (Khớp chính xác với diagram.json của bạn) ---
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

int countdown = 11;
bool isAuto = false;

// Hàm gửi dữ liệu cảm biến
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    if (Blynk.connected()) {
      Blynk.virtualWrite(V1, t); // Gauge Nhiệt độ
      Blynk.virtualWrite(V2, h); // Gauge Độ ẩm
    }
  }
}

// Hàm xử lý đếm ngược (Traffic Light Logic)
void trafficTimer() {
  if (isAuto) {
    display.showNumberDec(countdown);
    if (Blynk.connected()) Blynk.virtualWrite(V0, countdown);

    if (countdown > 0) {
      countdown--;
      digitalWrite(PIN_LED, HIGH);
    } else {
      countdown = 11;
    }
  } else {
    display.clear();
    digitalWrite(PIN_LED, LOW);
  }
}

// Nhận lệnh từ Switch trên Blynk
BLYNK_WRITE(V3) {
  isAuto = param.asInt();
}

void setup() {
  Serial.begin(115200);
  
  // --- HIỂN THỊ TÊN ĐỊNH DANH (Bắt buộc để nộp bài) ---
  Serial.println("================================");
  Serial.println("ĐÀO VĂN LỢI"); // Tên của bạn từ file json
  Serial.println("PROJECT: ESP32 BLYNK & WOKWI");
  Serial.println("================================");

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);
  
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN); 

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, trafficTimer);
}

void loop() {
  if (Blynk.connected()) Blynk.run();
  timer.run();

  // Nút nhấn vật lý thay đổi trạng thái Auto
  static bool lastBtnState = HIGH;
  bool btnState = digitalRead(PIN_BTN);
  if (btnState == LOW && lastBtnState == HIGH) {
    isAuto = !isAuto;
    if (Blynk.connected()) Blynk.virtualWrite(V3, isAuto);
    delay(200); 
  }
  lastBtnState = btnState;
}