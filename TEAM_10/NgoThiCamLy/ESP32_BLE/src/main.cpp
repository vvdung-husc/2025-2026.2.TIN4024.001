#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

// ================= CẤU HÌNH WIFI & THINGSPEAK =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
String apiKey = "A3J7YST63TTI390I"; // API Key ThingSpeak của bạn

// ================= CẤU HÌNH PHẦN CỨNG (LED) =================
#define LED_B1 25
#define LED_B2 33
#define LED_B3 32

// ================= TỌA ĐỘ CÁC BEACON (Cố định) =================
// Giả định đơn vị là mét (m) trong phòng 3x3m
float bx1 = 0, by1 = 0;
float bx2 = 3, by2 = 0;
float bx3 = 0, by3 = 3;

// ================= BIẾN HỖ TRỢ MÔ PHỎNG =================
float deviceX = 1.0;
float deviceY = 1.0;

// ================= BỘ LỌC NHIỄU (KALMAN/SMOOTH) =================
float last_d1 = -1, last_d2 = -1, last_d3 = -1; 
float smooth_factor = 0.7; // Càng cao càng mượt nhưng phản ứng chậm hơn

// ================= CÁC HÀM THUẬT TOÁN =================

// 1. Tính khoảng cách thực tế (Dùng cho mô phỏng)
float getDistance(float x, float y, float bx, float by) {
  return sqrt(pow(x - bx, 2) + pow(y - by, 2));
}

// 2. Chuyển khoảng cách sang RSSI (Công thức Log-Distance Path Loss)
int distanceToRSSI(float d) {
  if (d < 0.1) d = 0.1;
  int txPower = -69; // RSSI ở khoảng cách 1m
  float n = 2.0;     // Hệ số môi trường
  return (int)(txPower - (10 * n * log10(d)));
}

// 3. Chuyển RSSI ngược lại thành Khoảng cách
float calculateDistance(int rssi) {
  int txPower = -69;
  float n = 2.0;
  return pow(10, (float)(txPower - rssi) / (10 * n));
}

// 4. Bộ lọc làm mượt tín hiệu
float applyFilter(float lastVal, float newVal) {
  if (lastVal < 0) return newVal; // Lần đầu lấy luôn giá trị mới
  return (smooth_factor * lastVal) + ((1.0 - smooth_factor) * newVal);
}

// 5. Thuật toán Tam giác lượng (Trilateration) để tìm tọa độ (X, Y)
void calculatePosition(float d1, float d2, float d3, float &x, float &y) {
  float A = 2*bx2 - 2*bx1;
  float B = 2*by2 - 2*by1;
  float C = pow(d1,2) - pow(d2,2) - pow(bx1,2) + pow(bx2,2) - pow(by1,2) + pow(by2,2);
  float D = 2*bx3 - 2*bx2;
  float E = 2*by3 - 2*by2;
  float F = pow(d2,2) - pow(d3,2) - pow(bx2,2) + pow(bx3,2) - pow(by2,2) + pow(by3,2);

  float denom = (A*E - D*B);
  if (abs(denom) < 0.001) { x = 0; y = 0; return; }

  x = (C*E - F*B) / denom;
  y = (A*F - D*C) / denom;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  pinMode(LED_B1, OUTPUT);
  pinMode(LED_B2, OUTPUT);
  pinMode(LED_B3, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

// ================= LOOP =================
void loop() {
  // --- BƯỚC 1: GIẢ LẬP DI CHUYỂN TRONG PHÒNG ---
  int step = (millis() / 5000) % 4;
  if (step == 0)      { deviceX = 0.5; deviceY = 0.5; }
  else if (step == 1) { deviceX = 2.5; deviceY = 0.5; }
  else if (step == 2) { deviceX = 0.5; deviceY = 2.5; }
  else                { deviceX = 1.5; deviceY = 1.5; }

  // --- BƯỚC 2: GIẢ LẬP NHẬN TÍN HIỆU RSSI TỪ 3 BEACON ---
  int rssi1 = distanceToRSSI(getDistance(deviceX, deviceY, bx1, by1)) + random(-2, 3);
  int rssi2 = distanceToRSSI(getDistance(deviceX, deviceY, bx2, by2)) + random(-2, 3);
  int rssi3 = distanceToRSSI(getDistance(deviceX, deviceY, bx3, by3)) + random(-2, 3);

  // --- BƯỚC 3: XỬ LÝ KHOẢNG CÁCH VÀ LỌC NHIỄU ---
  float raw_d1 = calculateDistance(rssi1);
  float raw_d2 = calculateDistance(rssi2);
  float raw_d3 = calculateDistance(rssi3);

  last_d1 = applyFilter(last_d1, raw_d1);
  last_d2 = applyFilter(last_d2, raw_d2);
  last_d3 = applyFilter(last_d3, raw_d3);

  // --- BƯỚC 4: TÍNH TỌA ĐỘ VỊ TRÍ ---
  float x_calc, y_calc;
  calculatePosition(last_d1, last_d2, last_d3, x_calc, y_calc);

  // --- BƯỚC 5: XÁC ĐỊNH BEACON GẦN NHẤT & BẬT LED ---
  String nearest = "Beacon 1";
  float minDist = last_d1;
  if (last_d2 < minDist) { minDist = last_d2; nearest = "Beacon 2"; }
  if (last_d3 < minDist) { nearest = "Beacon 3"; }

  int nearestValue = 1;
  if (nearest == "Beacon 1") nearestValue = 1;
  else if (nearest == "Beacon 2") nearestValue = 2;
  else nearestValue = 3;

  digitalWrite(LED_B1, nearest == "Beacon 1");
  digitalWrite(LED_B2, nearest == "Beacon 2");
  digitalWrite(LED_B3, nearest == "Beacon 3");
//--- tính giá trị trung bình--
  float avgDist = (last_d1 + last_d2 + last_d3) / 3.0;

  // --- BƯỚC 6: IN KẾT QUẢ ---
  Serial.println("\r\n=========== BLE POSITIONING ===========");
  Serial.printf("REAL POS : (%.2f , %.2f)\r\n", deviceX, deviceY);
  Serial.printf("CALC POS : (%.2f , %.2f)\r\n", x_calc, y_calc);
  Serial.println("---------------------------------------");
  Serial.printf("RSSI     : %d | %d | %d\r\n", rssi1, rssi2, rssi3);
  Serial.printf("DIST     : %.2f | %.2f | %.2f m\r\n", last_d1, last_d2, last_d3);
  Serial.println("---------------------------------------");
  Serial.printf("NEAREST  : %s\r\n", nearest.c_str());
  Serial.printf("AVG DIST : %.2f m\r\n", avgDist);
  
  if (rssi1 < -85 && rssi2 < -85 && rssi3 < -85) {
    Serial.println("WARNING: Weak Signal!");
  }
  Serial.println("=======================================");

  // --- BƯỚC 7: GỬI DỮ LIỆU LÊN THINGSPEAK ---
 // --- BƯỚC 7: GỬI DỮ LIỆU LÊN THINGSPEAK ---

// Kiểm tra lỗi NaN
if (isnan(avgDist)) {
  Serial.println("ERROR: avgDist is NaN -> bỏ qua gửi");
  return;
}

if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
                "&field1=" + String(x_calc, 2) +
                "&field2=" + String(y_calc, 2) +
                "&field3=" + String(nearestValue) +
                "&field4=" + String(avgDist, 2);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    Serial.println("✅ SUCCESS: Data sent to ThingSpeak");
  } else {
    Serial.printf("❌ FAILED: HTTP %d\n", httpCode);
  }

  http.end();
} else {
  Serial.println("❌ WiFi not connected");
}

// thinkspeak
delay(15000);
}