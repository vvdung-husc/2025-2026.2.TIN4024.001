// Các define của Blynk BẮT BUỘC phải nằm ở trên cùng
#define BLYNK_TEMPLATE_ID "TMPL6EsjbKzV_"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "56C2O7d95NMjyEaUr17WiNLo3T-tCoQ1"

#include <Arduino.h>
#include <TM1637Display.h>
#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ====== PIN ======
#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19

#define DHT_PIN 16   // đúng với diagram.json

// ====== BIẾN ======
ulong currentMiliseconds = 0;
bool blueButtonON = false; // Mặc định tắt
ulong uptime = 0;          // Biến lưu thời gian hoạt động (giây)
int countdownValue = 0;    // Biến lưu giá trị đếm ngược

// ====== OBJECT ======
TM1637Display display(CLK, DIO);
DHTesp dht;

// ====== PROTOTYPE ======
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);

  // DHT22 init
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Khởi tạo trạng thái ban đầu
  digitalWrite(pinBLED, LOW);
  Blynk.virtualWrite(V3, 0); // V3 là Công tắc

  Serial.println("== START ==");
}

// ====== LOOP ======
void loop() {
  Blynk.run();

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT22();
}

// ====== FUNCTION ======
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm xử lý nút nhấn cứng trên board
void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return; // Debounce 50ms

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return; // Chỉ xử lý khi nhả nút (hoặc tùy logic pullup)

  if (!blueButtonON) {
    Serial.println("Physical Button -> Blue Light ON (Start Countdown)");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    countdownValue = uptime;   // Bắt đầu đếm ngược từ thời gian uptime hiện tại
    Blynk.virtualWrite(V3, 1); // Đồng bộ trạng thái BẬT lên V3
  } else {
    Serial.println("Physical Button -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    blueButtonON = false;
    Blynk.virtualWrite(V3, 0); // Đồng bộ trạng thái TẮT lên V3
  }
}

// Hàm xử lý thời gian, đếm ngược và hiển thị LED
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; // Chạy mỗi 1 giây

  uptime++; // Tăng thời gian hoạt động
  Blynk.virtualWrite(V2, uptime); // Gửi Uptime lên V2

  if (blueButtonON) {
    // Đang ở chế độ đếm ngược
    if (countdownValue > 0) {
      display.showNumberDec(countdownValue, false);
      countdownValue--;
    } else {
      // Đếm ngược về 0
      display.showNumberDec(0, false);
      blueButtonON = false;
      digitalWrite(pinBLED, LOW);
      Blynk.virtualWrite(V3, 0); // Tự động tắt công tắc V3 trên Blynk
      Serial.println("Countdown finished -> Blue Light OFF");
    }
  } else {
    // Chế độ bình thường: Hiển thị thời gian hoạt động
    display.showNumberDec(uptime, false);
  }
}

// ====== DHT22 ======
void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // đọc mỗi 2s

  TempAndHumidity data = dht.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("DHT22 read failed!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(data.temperature);
  Serial.print(" °C  | Humidity: ");
  Serial.print(data.humidity);
  Serial.println(" %");

  Blynk.virtualWrite(V0, data.temperature); // Gửi nhiệt độ lên V0
  Blynk.virtualWrite(V1, data.humidity);    // Gửi độ ẩm lên V1
}

// ====== BLYNK ======
// Nhận lệnh từ công tắc trên App/Web (V3)
BLYNK_WRITE(V3) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON (Start Countdown)");
    digitalWrite(pinBLED, HIGH);
    countdownValue = uptime; // Bắt đầu đếm ngược
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
  }
}