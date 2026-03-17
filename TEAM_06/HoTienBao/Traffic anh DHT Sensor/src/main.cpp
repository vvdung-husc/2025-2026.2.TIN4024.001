#define BLYNK_TEMPLATE_ID "TMPL6Rye4D7K_"
#define BLYNK_TEMPLATE_NAME "ESP32 DHT Traffic"
#define BLYNK_AUTH_TOKEN "oLQgjHxJFDy1T4DFOYuVw3s7rnsDeuVq"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi mô phỏng của Wokwi (Không được đổi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình chân (Phù hợp với diagram.json)
#define DHTPIN 15
#define DHTTYPE DHT22
#define CLK_PIN 22
#define DIO_PIN 23
#define LED_PIN 2
#define BUTTON_PIN 4

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

int uptime = 0;
bool isCountingDown = false;
int countdownValue = 10; 

// Gửi dữ liệu cảm biến và uptime lên Blynk
void sendSensorData() {
  uptime++;
  Blynk.virtualWrite(V2, uptime); // Gửi thời gian hoạt động

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V0, t); // Gửi nhiệt độ
    Blynk.virtualWrite(V1, h); // Gửi độ ẩm
  }
}

// Xử lý đếm ngược hiển thị trên LED 7 đoạn
void handleCountdown() {
  if (isCountingDown) {
    if (countdownValue >= 0) {
      display.showNumberDec(countdownValue, true);
      countdownValue--;
    } else {
      isCountingDown = false;
      digitalWrite(LED_PIN, LOW); 
      Blynk.virtualWrite(V3, 0);  // Đồng bộ: Tắt công tắc trên Blynk
      display.clear();
    }
  }
}

// Lắng nghe lệnh từ nút Switch trên Blynk (Chân V3)
BLYNK_WRITE(V3) {
  int switchState = param.asInt();
  if (switchState == 1) {
    digitalWrite(LED_PIN, HIGH); // Bật đèn xanh
    isCountingDown = true;
    countdownValue = 10;         // Bắt đầu đếm từ 10
  } else {
    digitalWrite(LED_PIN, LOW);  // Tắt đèn xanh
    isCountingDown = false;
    display.clear();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  dht.begin();
  display.setBrightness(0x0f); 
  display.clear();

  // Kết nối Blynk
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Đặt lịch chạy hàm (1000ms = 1 giây)
  timer.setInterval(1000L, sendSensorData);
  timer.setInterval(1000L, handleCountdown);
}

void loop() {
  Blynk.run();
  timer.run();
  
  // Tính năng thêm: Nhấn nút vật lý để kích hoạt giống như bật app
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(LED_PIN, HIGH);
    isCountingDown = true;
    countdownValue = 10;
    Blynk.virtualWrite(V3, 1); // Cập nhật trạng thái lên app
    delay(300); // Chống dội phím
  }
}