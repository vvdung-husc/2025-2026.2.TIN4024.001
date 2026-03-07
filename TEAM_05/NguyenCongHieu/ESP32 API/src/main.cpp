#define BLYNK_TEMPLATE_ID "TMPL62nFcEqow"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic  DHT"
#define BLYNK_AUTH_TOKEN "KjKLyDntOpkCrKU1M9ewn-Wvt5s177ZI"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  // Tên mạng WiFi
char pass[] = "";             // Mật khẩu mạng WiFi

// Định nghĩa các chân kết nối theo diagram.json
#define PIN_BUTTON        23  // Nút bấm
#define PIN_LED_BLUE      21  // LED xanh dương
#define CLK               18  // TM1637 CLK
#define DIO               19  // TM1637 DIO
#define PIN_DHT           16  // DHT22 data pin
#define DHTTYPE           DHT22

// Biến toàn cục
DHT dht(PIN_DHT, DHTTYPE);
TM1637Display display(CLK, DIO);
ulong currentMilliseconds = 0;
bool blueLedON = false;       // Trạng thái LED xanh và hiển thị

// === Hàm tiện ích ===
bool IsReady(ulong &ulTimer, uint32_t millisecond)
{
  if (currentMilliseconds - ulTimer < millisecond) return false;
  ulTimer = currentMilliseconds;
  return true;
}

// === Xử lý nút bấm ===
void updateButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  
  if (!IsReady(lastTime, 50)) return;
  
  int v = digitalRead(PIN_BUTTON);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;  // Chỉ xử lý khi nhả nút
  
  // Toggle trạng thái
  if (!blueLedON) {
    Serial.println("Man hinh & LED BAT");
    digitalWrite(PIN_LED_BLUE, HIGH);
    blueLedON = true;
    Blynk.virtualWrite(V0, blueLedON);
  } else {
    Serial.println("Man hinh & LED TAT");
    digitalWrite(PIN_LED_BLUE, LOW);
    blueLedON = false;
    Blynk.virtualWrite(V0, blueLedON);
    display.clear();
  }
}

// === Đọc và hiển thị cảm biến DHT ===
void readDHTSensor() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;  // Đọc mỗi 2 giây
  
  float temperature = dht.readTemperature();  // Đọc nhiệt độ (°C)
  float humidity = dht.readHumidity();        // Đọc độ ẩm (%)
  
  // Kiểm tra lỗi đọc
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Khong doc duoc cam bien DHT!");
    return;
  }
  
  Serial.print("Nhiet do: ");
  Serial.print(temperature, 1);
  Serial.print("°C    Do am: ");
  Serial.print((int)humidity);
  Serial.println("%");
  
  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, temperature);  // Virtual Pin V1: Nhiệt độ
  Blynk.virtualWrite(V2, humidity);     // Virtual Pin V2: Độ ẩm
}

// === Gửi uptime lên Blynk ===
void sendUptimeToBlynk() {
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return;  // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  
  ulong uptimeSeconds = lastTime / 1000;  // Sử dụng lastTime đã được cập nhật
  Blynk.virtualWrite(V3, uptimeSeconds);  // Gửi lên Blynk
  
  // Hiển thị trên TM1637 nếu LED đang bật
  if (blueLedON) {
    display.showNumberDec(uptimeSeconds % 10000);
  }
}

// === Nhận lệnh từ Blynk để điều khiển LED ===
BLYNK_WRITE(V0) {
  blueLedON = param.asInt();
  
  if (blueLedON) {
    Serial.println("Blynk -> LED & Man hinh BAT");
    digitalWrite(PIN_LED_BLUE, HIGH);
  } else {
    Serial.println("Blynk -> LED & Man hinh TAT");
    digitalWrite(PIN_LED_BLUE, LOW);
    display.clear();
  }
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo các chân
  pinMode(PIN_BUTTON, INPUT_PULLUP);  // Sử dụng điện trở kéo lên nội
  pinMode(PIN_LED_BLUE, OUTPUT);
  
  // Khởi tạo DHT
  dht.begin();
  
  // Khởi tạo TM1637
  display.setBrightness(0x0f);
  display.clear();
  
  // Kết nối WiFi và Blynk
  Serial.print("Dang ket noi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);
  Serial.println("Da ket noi WiFi");
  
  // Đồng bộ trạng thái ban đầu
  digitalWrite(PIN_LED_BLUE, blueLedON ? HIGH : LOW);
  Blynk.virtualWrite(V0, blueLedON);
  
  Serial.println("=== ESP32 Traffic & DHT Sensor Started ===");
  Blynk.virtualWrite(V4, "Nguyen Cong Hieu");
}

void loop() {
  Blynk.run();  // Chạy Blynk
  
  currentMilliseconds = millis();
  
  updateButton();        // Xử lý nút bấm
  sendUptimeToBlynk();   // Gửi uptime và cập nhật hiển thị
  readDHTSensor();       // Đọc cảm biến DHT
}
