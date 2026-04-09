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
 * Chân V0 dùng cho Switch trên Blynk
 */
BLYNK_WRITE(V0)
{
  isDeviceOn = param.asInt(); 
  digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);

  if (!isDeviceOn) {
    display.clear(); 
  }
}

/**
 * 2. ĐỌC CẢM BIẾN VÀ GỬI DỮ LIỆU (V1, V2, V3)
 * Chạy mỗi 1 giây
 */
void sendDataToBlynk()
{
  // Tăng và gửi thời gian hoạt động lên V3
  uptime++;
  Blynk.virtualWrite(V3, uptime);

  // Hiển thị TM1637 nếu đang BẬT
  if (isDeviceOn) {
    display.showNumberDec(uptime);
  }

  // Đọc Nhiệt độ (V1) và Độ ẩm (V2)
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V1, t); 
    Blynk.virtualWrite(V2, h); 
  }
}

/**
 * 3. XỬ LÝ NÚT NHẤN CỨNG
 * Đồng bộ trạng thái với Widget V0 trên App
 */
void checkPhysicalButton()
{
  int btnState = digitalRead(BTN_PIN);

  // Phát hiện cạnh xuống (nhấn nút)
  if (lastBtnState == HIGH && btnState == LOW) {
    delay(50); // Khử rung nút nhấn
    if (digitalRead(BTN_PIN) == LOW) {
      isDeviceOn = !isDeviceOn; 
      digitalWrite(LED_PIN, isDeviceOn ? HIGH : LOW);

      if (!isDeviceOn) display.clear();

      // CẬP NHẬT TRẠNG THÁI LÊN APP (Đồng bộ Switch V0)
      Blynk.virtualWrite(V0, isDeviceOn);
    }
  }
  lastBtnState = btnState;
}

void setup()
{
  Serial.begin(115200);

  // Cấu hình chân
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Khởi động thiết bị
  dht.begin();
  display.setBrightness(0x0f); 
  display.clear();

  // Kết nối Blynk
  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thiết lập Timer
  timer.setInterval(1000L, sendDataToBlynk);    
  timer.setInterval(100L, checkPhysicalButton); 
}

void loop()
{
  Blynk.run();
  timer.run();
}