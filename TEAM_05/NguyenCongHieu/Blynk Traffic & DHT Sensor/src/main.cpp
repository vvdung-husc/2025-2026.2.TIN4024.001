#define BLYNK_TEMPLATE_ID "TMPL6NguyenCongHieu"
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
bool displayON = false;       // Trạng thái hiển thị
bool blueLedON = false;       // Trạng thái LED xanh

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

  // Toggle trạng thái display và LED
  displayON = !displayON;
  blueLedON = !blueLedON;
  
  if (displayON) {
    Serial.println("Display & LED ON");
    digitalWrite(PIN_LED_BLUE, HIGH);
  } else {
    Serial.println("Display & LED OFF");
    digitalWrite(PIN_LED_BLUE, LOW);
    display.clear();
  }
  
  // Gửi trạng thái lên Blynk
  Blynk.virtualWrite(V0, blueLedON);
}

// === Đọc và hiển thị cảm biến DHT ===
void readDHTSensor() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;  // Đọc mỗi 2 giây
  
  float temperature = dht.readTemperature();  // Đọc nhiệt độ (°C)
  float humidity = dht.readHumidity();        // Đọc độ ẩm (%)
  
  // Kiểm tra lỗi đọc
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
  
  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, temperature);  // Virtual Pin V1: Nhiệt độ
  Blynk.virtualWrite(V2, humidity);     // Virtual Pin V2: Độ ẩm
  
  // Hiển thị nhiệt độ trên TM1637 nếu display đang bật
  if (displayON) {
    int temp = (int)temperature;
    display.showNumberDec(temp);
  }
}

// === Gửi uptime lên Blynk ===
void sendUptimeToBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;  // Gửi mỗi 1 giây
  
  ulong uptimeSeconds = lastTime / 1000;
  Blynk.virtualWrite(V3, uptimeSeconds);  // Virtual Pin V3: Uptime
  
  // Hiển thị uptime trên TM1637 nếu display bật và không hiển thị nhiệt độ
  // (có thể thêm logic switch giữa các chế độ hiển thị)
}

// === Nhận lệnh từ Blynk để điều khiển LED ===
BLYNK_WRITE(V0) {
  blueLedON = param.asInt();
  displayON = blueLedON;
  
  if (blueLedON) {
    Serial.println("Blynk -> LED & Display ON");
    digitalWrite(PIN_LED_BLUE, HIGH);
  } else {
    Serial.println("Blynk -> LED & Display OFF");
    digitalWrite(PIN_LED_BLUE, LOW);
    display.clear();
  }
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo các chân
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  
  // Khởi tạo DHT
  dht.begin();
  
  // Khởi tạo TM1637
  display.setBrightness(0x0f);
  display.clear();
  
  // Kết nối WiFi và Blynk
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");
  
  // Đồng bộ trạng thái ban đầu
  digitalWrite(PIN_LED_BLUE, blueLedON ? HIGH : LOW);
  Blynk.virtualWrite(V0, blueLedON);
  
  Serial.println("=== ESP32 Traffic & DHT Sensor Started ===");
}

void loop() {
  Blynk.run();  // Chạy Blynk
  
  currentMilliseconds = millis();
  
  updateButton();        // Xử lý nút bấm
  readDHTSensor();       // Đọc cảm biến DHT
  sendUptimeToBlynk();   // Gửi uptime
}
