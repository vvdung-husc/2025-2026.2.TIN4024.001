// Thư viện cơ bản của Arduino (dùng cho ESP8266/ESP32)
#include <Arduino.h>

// Thư viện giao tiếp I2C (dùng để kết nối với màn hình OLED qua giao thức I2C)
#include <Wire.h>

// Thư viện điều khiển màn hình OLED dùng chip SH1106 (thông qua U8g2)
#include <U8g2lib.h>

// Thư viện đọc dữ liệu từ cảm biến nhiệt độ và độ ẩm DHT
#include <DHT.h>

// ============================================================
// Khai báo chân (Pin) kết nối cho NodeMCU 1.0 (ESP8266)
// ============================================================

// Chân LED tích hợp trên board (LED_BUILTIN)
constexpr uint8_t LED_PIN = LED_BUILTIN;

// Chân DATA của cảm biến DHT11/DHT22 kết nối vào chân D3
constexpr uint8_t DHT_PIN = D3;

// Chân đọc tín hiệu analog từ cảm biến khí gas MQ2 (chân A0 - ADC)
constexpr uint8_t MQ2_PIN = A0;

// ============================================================
// Loại cảm biến DHT đang sử dụng
// Đổi thành DHT22 nếu module của bạn là DHT22 hoặc AM2302
// ============================================================
constexpr uint8_t DHT_TYPE = DHT11;

// Khoảng thời gian nhấp nháy LED: 500ms (0.5 giây)
constexpr unsigned long LED_BLINK_MS = 500;

// Khoảng thời gian đọc cảm biến: 2000ms (2 giây)
constexpr unsigned long SENSOR_READ_MS = 2000;

// ============================================================
// Khởi tạo đối tượng cảm biến và màn hình
// ============================================================

// Tạo đối tượng DHT với chân và loại cảm biến đã khai báo
DHT dht(DHT_PIN, DHT_TYPE);

// Tạo đối tượng màn hình OLED SH1106 128x64, giao tiếp I2C phần cứng
// U8G2_R0: không xoay màn hình; U8X8_PIN_NONE: không dùng chân RESET riêng
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ============================================================
// Biến toàn cục lưu trạng thái và dữ liệu cảm biến
// ============================================================

// Thời điểm (ms) lần cuối LED nhấp nháy
unsigned long lastBlinkMs = 0;

// Thời điểm (ms) lần cuối đọc cảm biến
unsigned long lastReadMs = 0;

// Trạng thái hiện tại của LED (true = bật, false = tắt)
bool ledState = false;

// Giá trị nhiệt độ (°C) đọc từ DHT, khởi tạo là NAN (chưa có dữ liệu)
float temperatureC = NAN;

// Giá trị độ ẩm (%) đọc từ DHT, khởi tạo là NAN (chưa có dữ liệu)
float humidity = NAN;

// Giá trị analog thô đọc từ cảm biến MQ2 (0 - 1023)
int mq2Raw = 0;

// ============================================================
// Hàm đọc dữ liệu từ các cảm biến và in ra Serial Monitor
// ============================================================
void readSensors() {
  // Đọc giá trị độ ẩm từ cảm biến DHT
  humidity = dht.readHumidity();

  // Đọc giá trị nhiệt độ (°C) từ cảm biến DHT
  temperatureC = dht.readTemperature();

  // Đọc giá trị analog từ cảm biến khí gas MQ2
  mq2Raw = analogRead(MQ2_PIN);

  // --- In nhiệt độ ra Serial Monitor ---
  Serial.print("Nhiet do hien tai: ");
  if (isnan(temperatureC)) {
    // Nếu đọc thất bại (giá trị không hợp lệ), in "N/A"
    Serial.println("N/A C");
  } else {
    // In nhiệt độ với 1 chữ số thập phân
    Serial.print(temperatureC, 1);
    Serial.println(" C");
  }

  // --- In độ ẩm ra Serial Monitor ---
  Serial.print("Do am hien tai: ");
  if (isnan(humidity)) {
    // Nếu đọc thất bại, in "N/A"
    Serial.println("N/A %");
  } else {
    // In độ ẩm với 1 chữ số thập phân
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  // In giá trị thô của cảm biến MQ2
  Serial.print("MQ2 raw: ");
  Serial.println(mq2Raw);

  // In dòng trống để dễ đọc
  Serial.println();
}

// ============================================================
// Hàm hiển thị thông tin lên màn hình OLED
// ============================================================
void drawOled() {
  // Khai báo các mảng ký tự để chứa nội dung 3 dòng hiển thị
  char line1[24];
  char line2[24];
  char line3[24];

  // Kiểm tra nếu dữ liệu cảm biến DHT không hợp lệ
  if (isnan(temperatureC) || isnan(humidity)) {
    // Hiển thị thông báo lỗi đọc DHT
    snprintf(line1, sizeof(line1), "DHT: Read error");
    // Dòng 2 để trống
    line2[0] = '\0';
  } else {
    // Định dạng chuỗi nhiệt độ với 1 chữ số thập phân
    snprintf(line1, sizeof(line1), "Nhiet do: %.1f C", temperatureC);
    // Định dạng chuỗi độ ẩm với 1 chữ số thập phân (dùng %% để hiện ký tự %)
    snprintf(line2, sizeof(line2), "Do am: %.1f %%", humidity);
  }

  // Quy đổi giá trị ADC thô (0-1023) sang phần trăm (0-100%)
  const float mq2Percent = (mq2Raw / 1023.0f) * 100.0f;

  // Định dạng chuỗi MQ2: giá trị thô và phần trăm
  snprintf(line3, sizeof(line3), "MQ2: %4d (%.0f%%)", mq2Raw, mq2Percent);

  // Xóa toàn bộ buffer màn hình trước khi vẽ mới
  u8g2.clearBuffer();

  // Chọn font chữ nhỏ 6x12 pixel
  u8g2.setFont(u8g2_font_6x12_tf);

  // Vẽ tiêu đề ở dòng đầu (tọa độ y=12)
  u8g2.drawStr(0, 12, "ESP8266 Controller");

  // Vẽ đường kẻ ngang phân cách tiêu đề và nội dung (từ x=0 đến x=127, y=15)
  u8g2.drawLine(0, 15, 127, 15);

  // Hiển thị dòng nhiệt độ (hoặc thông báo lỗi) tại y=32
  u8g2.drawStr(0, 32, line1);

  // Hiển thị dòng độ ẩm tại y=46
  u8g2.drawStr(0, 46, line2);

  // Hiển thị dòng MQ2 tại y=60 (dòng cuối màn hình)
  u8g2.drawStr(0, 60, line3);

  // Gửi toàn bộ buffer lên màn hình OLED để hiển thị
  u8g2.sendBuffer();
}

// ============================================================
// Hàm xử lý nhấp nháy LED theo chu kỳ không chặn (non-blocking)
// ============================================================
void updateBlink() {
  // Lấy thời gian hiện tại (ms) kể từ khi khởi động
  const unsigned long now = millis();

  // Kiểm tra xem đã đủ thời gian LED_BLINK_MS kể từ lần nhấp nháy trước chưa
  if (now - lastBlinkMs >= LED_BLINK_MS) {
    // Cập nhật thời điểm nhấp nháy gần nhất
    lastBlinkMs = now;

    // Đảo trạng thái LED (bật -> tắt hoặc tắt -> bật)
    ledState = !ledState;

    // LED_BUILTIN trên ESP8266 hoạt động theo logic ĐẢO (active LOW):
    // ledState = true  -> ghi LOW  -> LED SÁNG
    // ledState = false -> ghi HIGH -> LED TẮT
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  }
}

// ============================================================
// Hàm setup(): chạy một lần khi board khởi động
// ============================================================
void setup() {
  // Đặt chân LED là OUTPUT (ngõ ra)
  pinMode(LED_PIN, OUTPUT);

  // Tắt LED ban đầu (HIGH = tắt vì active LOW)
  digitalWrite(LED_PIN, HIGH);

  // Khởi động cổng Serial với tốc độ baud 115200 bps
  Serial.begin(115200);

  // Chờ 300ms cho Serial ổn định
  delay(300);

  // In dòng trống và thông báo khởi động
  Serial.println();
  Serial.println("Khoi dong ESP8266 Controller...");

  // Khởi động giao tiếp I2C (Wire) để màn hình OLED hoạt động
  Wire.begin();

  // Khởi động cảm biến DHT
  dht.begin();

  // Khởi động màn hình OLED
  u8g2.begin();

  // Đọc cảm biến và hiển thị lên OLED ngay khi khởi động
  readSensors();
  drawOled();
}

// ============================================================
// Hàm loop(): lặp lại liên tục sau khi setup() hoàn thành
// ============================================================
void loop() {
  // Gọi hàm cập nhật nhấp nháy LED mỗi vòng lặp
  updateBlink();

  // Lấy thời gian hiện tại
  const unsigned long now = millis();

  // Kiểm tra nếu đã đủ SENSOR_READ_MS (2 giây) kể từ lần đọc cảm biến trước
  if (now - lastReadMs >= SENSOR_READ_MS) {
    // Cập nhật thời điểm đọc gần nhất
    lastReadMs = now;

    // Đọc lại dữ liệu từ các cảm biến
    readSensors();

    // Cập nhật hiển thị trên màn hình OLED
    drawOled();
  }
}