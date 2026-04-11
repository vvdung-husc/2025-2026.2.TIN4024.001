#define BLYNK_TEMPLATE_ID "TMPL68k02dsRr"
#define BLYNK_TEMPLATE_NAME "nguyenductin"
#define BLYNK_AUTH_TOKEN "dwjW7GFIlJsaConmIM9oGd6jDBeJmwz_"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

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
bool isDeviceOn = false; 
int lastBtnState = HIGH; 

/**
 * 1. ĐỒNG BỘ NÚT NHẤN (V0)
 */
BLYNK_WRITE(V0) {
  isDeviceOn = param.asInt();
  digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
  if (!isDeviceOn) display.clear();
}

/**
 * 2. HÀM GỬI DỮ LIỆU (Đã sửa để luôn gửi Nhiệt độ/Độ ẩm)
 */
void sendDataToBlynk() {
  // Gửi thời gian hoạt động lên V3
  uptime++;
  Blynk.virtualWrite(V3, uptime);

  // Hiển thị TM1637 nếu trạng thái là ON
  if (isDeviceOn) {
    display.showNumberDec(uptime);
  }

  // Đọc Nhiệt độ (V1) và Độ ẩm (V2)
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Kiểm tra lỗi cảm biến và GỬI LUÔN (Không cần check isDeviceOn)
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V1, t); 
    Blynk.virtualWrite(V2, h); 
    Serial.print("Nhiet do: "); Serial.print(t);
    Serial.print(" - Do am: "); Serial.println(h);
  } else {
    Serial.println("Loi doc cam bien DHT!");
  }
}

/**
 * 3. XỬ LÝ NÚT NHẤN CỨNG
 */
void checkPhysicalButton() {
  int btnState = digitalRead(BTN_PIN);
  if (lastBtnState == HIGH && btnState == LOW) {
    delay(50);
    if (digitalRead(BTN_PIN) == LOW) {
      isDeviceOn = !isDeviceOn; 
      digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);
      if (!isDeviceOn) display.clear();
      Blynk.virtualWrite(V0, isDeviceOn ? 1 : 0);
    }
  }
  lastBtnState = btnState;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  dht.begin();
  display.setBrightness(0x0f);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Chạy hàm sendDataToBlynk mỗi 1 giây (1000L)
  timer.setInterval(1000L, sendDataToBlynk);
  timer.setInterval(100L, checkPhysicalButton);
}

void loop() {
  Blynk.run();
  timer.run();
}