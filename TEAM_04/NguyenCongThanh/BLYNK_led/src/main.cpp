// #define BLYNK_TEMPLATE_ID   "TMPL6Ml8uNZwj"
// #define BLYNK_TEMPLATE_NAME "BLYNK DHT"
// #define BLYNK_AUTH_TOKEN    "D16by_jFGEmHsp3bkaDye3kFUJw-88vq"
// #define BLYNK_PRINT         Serial

// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>
// #include <DHT.h>
// #include <TM1637Display.h>

// // ── WiFi ────────────────────────────────────────────────
// char ssid[] = "Wokwi-GUEST";
// char pass[] = "";

// // ── GPIO ────────────────────────────────────────────────
// #define DHT_PIN   16
// #define DHT_TYPE  DHT22
// #define LED_PIN   21
// #define BTN_PIN   23
// #define CLK_PIN   18
// #define DIO_PIN   19

// // ── Virtual Pin ─────────────────────────────────────────
// #define VP_TEMP    V0
// #define VP_HUMID   V1
// #define VP_HEAT    V2
// #define VP_LED     V3
// #define VP_BTN     V4
// #define VP_TIMER   V5
// #define VP_SYSTEM  V6

// // ── Đối tượng ───────────────────────────────────────────
// DHT            dht(DHT_PIN, DHT_TYPE);
// TM1637Display  display(CLK_PIN, DIO_PIN);
// BlynkTimer     timer;

// // ── Biến trạng thái ─────────────────────────────────────
// bool          systemON    = false;
// bool          lastBtnState = HIGH;
// unsigned long startTime   = 0;   // millis() lúc bắt đầu chạy
// unsigned long pausedTime  = 0;   // số giây đã tích lũy trước khi pause

// // ════════════════════════════════════════════════════════
// //  Forward declarations
// // ════════════════════════════════════════════════════════
// void applySystemState();
// void updateTimer();
// void readButton();
// void readSensor();

// // ════════════════════════════════════════════════════════
// //  applySystemState — xử lý bật/tắt hệ thống
// // ════════════════════════════════════════════════════════
// void applySystemState() {
//   if (systemON) {
//     // Bật: tính lại mốc millis có tính phần đã chạy trước
//     startTime = millis() - pausedTime * 1000UL;
//   } else {
//     // Tắt: lưu số giây đã tích lũy
//     pausedTime = (millis() - startTime) / 1000UL;
//     digitalWrite(LED_PIN, LOW);
//     display.clear();
//     Blynk.virtualWrite(VP_TIMER, pausedTime);
//   }
//   Blynk.virtualWrite(VP_SYSTEM, systemON ? 1 : 0);
//   Blynk.virtualWrite(VP_BTN,   systemON ? 1 : 0);
// }

// // ════════════════════════════════════════════════════════
// //  Blynk — nhận lệnh ON/OFF từ app (V6)
// // ════════════════════════════════════════════════════════
// BLYNK_WRITE(VP_SYSTEM) {
//   systemON = param.asInt();
//   applySystemState();
// }

// // ════════════════════════════════════════════════════════
// //  Blynk — điều khiển LED từ app (V3)
// // ════════════════════════════════════════════════════════
// BLYNK_WRITE(VP_LED) {
//   if (systemON) {
//     digitalWrite(LED_PIN, param.asInt());
//   }
// }

// // ════════════════════════════════════════════════════════
// //  Blynk — khi kết nối lại server, đồng bộ trạng thái
// // ════════════════════════════════════════════════════════
// BLYNK_CONNECTED() {
//   Blynk.syncVirtual(VP_SYSTEM);
//   Blynk.syncVirtual(VP_LED);
// }

// // ════════════════════════════════════════════════════════
// //  Đọc nút nhấn vật lý — toggle hệ thống
// // ════════════════════════════════════════════════════════
// void readButton() {
//   bool currentState = digitalRead(BTN_PIN);

//   // Phát hiện cạnh xuống (nhấn)
//   if (lastBtnState == HIGH && currentState == LOW) {
//     systemON = !systemON;
//     applySystemState();
//   }
//   lastBtnState = currentState;
// }

// // ════════════════════════════════════════════════════════
// //  Cập nhật bộ đếm thời gian + TM1637 (mỗi 1 giây)
// // ════════════════════════════════════════════════════════
// void updateTimer() {
//   if (!systemON) return;

//   // Thời gian thực từ millis() — không bị ảnh hưởng bởi lag WiFi
//   unsigned long elapsedSec = (millis() - startTime) / 1000UL;

//   // Gửi lên Blynk
//   Blynk.virtualWrite(VP_TIMER, elapsedSec);

//   // Hiển thị MM:SS trên TM1637 (reset sau 99 phút 59 giây)
//   unsigned long totalSec = elapsedSec % 6000UL;
//   int minutes = totalSec / 60;
//   int seconds = totalSec % 60;

//   display.showNumberDecEx(
//     minutes * 100 + seconds,
//     0b01000000,   // bật dấu ':' giữa
//     true          // leading zero (00:05 thay vì 0:5)
//   );

//   // Log serial
//   Serial.printf("[TIMER] %02d:%02d (%lu s)\n", minutes, seconds, elapsedSec);
// }

// // ════════════════════════════════════════════════════════
// //  Đọc DHT22 và gửi lên Blynk (mỗi 5 giây)
// // ════════════════════════════════════════════════════════
// void readSensor() {
//   if (!systemON) return;

//   float temp  = dht.readTemperature();
//   float humid = dht.readHumidity();

//   if (isnan(temp) || isnan(humid)) {
//     Serial.println("[DHT] Đọc cảm biến thất bại!");
//     return;
//   }

//   float heatIndex = dht.computeHeatIndex(temp, humid, false);

//   Blynk.virtualWrite(VP_TEMP,  temp);
//   Blynk.virtualWrite(VP_HUMID, humid);
//   Blynk.virtualWrite(VP_HEAT,  heatIndex);

//   Serial.printf("[DHT] Temp: %.1f°C | Humid: %.1f%% | HeatIdx: %.1f°C\n",
//                 temp, humid, heatIndex);
// }

// // ════════════════════════════════════════════════════════
// //  setup
// // ════════════════════════════════════════════════════════
// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n[BOOT] Khởi động hệ thống...");

//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BTN_PIN, INPUT_PULLUP);
//   digitalWrite(LED_PIN, LOW);

//   dht.begin();

//   display.setBrightness(7);
//   display.clear();

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

//   // Đăng ký các timer
//   timer.setInterval(100L,  readButton);   // quét nút mỗi 100ms
//   timer.setInterval(1000L, updateTimer);  // đồng hồ mỗi 1 giây
//   timer.setInterval(5000L, readSensor);   // cảm biến mỗi 5 giây

//   Serial.println("[BOOT] Sẵn sàng!");
// }

// // ════════════════════════════════════════════════════════
// //  loop
// // ════════════════════════════════════════════════════════
// void loop() {
//   Blynk.run();
//   timer.run();
// }
// // ```

// // ---

// // ## 📋 Cập nhật Virtual Pin Datastream

// // | Virtual Pin | Tên | Type | Min | Max | Mô tả |
// // |-------------|-----|------|-----|-----|-------|
// // | **V0** | Temperature | Double | -40 | 80 | Nhiệt độ (°C) |
// // | **V1** | Humidity | Double | 0 | 100 | Độ ẩm (%) |
// // | **V2** | Heat Index | Double | -40 | 80 | Chỉ số nhiệt |
// // | **V3** | LED Control | Integer | 0 | 1 | Bật/tắt LED từ app |
// // | **V4** | Button State | Integer | 0 | 1 | Phản hồi trạng thái nút |
// // | **V5** | Timer | Integer | 0 | 999999 | Bộ đếm giây |
// // | **V6** | System ON/OFF | Integer | 0 | 1 | **Bật/tắt toàn bộ hệ thống** |

// // ---

// // ## 🖥️ Widget Blynk Dashboard

// // | Widget | Pin | Ghi chú |
// // |--------|-----|---------|
// // | Gauge | V0 | Nhiệt độ |
// // | Gauge | V1 | Độ ẩm |
// // | Label | V2 | Heat Index |
// // | Button (Switch) | **V6** | **ON/OFF hệ thống** — widget chính |
// // | Button (Switch) | V3 | Bật/tắt LED |
// // | Label / Value Display | V5 | Hiển thị giây đếm |

// // ---

// // ## ⚙️ Logic hoạt động
// // ```
// // Nút vật lý GPIO23 ──┐
// //                      ├──► toggle systemON ──► TM1637 hiển thị MM:SS
// // Blynk V6 (app) ─────┘                    ──► DHT22 đọc & gửi Blynk
// //                                           ──► LED có thể điều khiển

// // Khi systemON = false:
// //   ✗ TM1637 tắt (display.clear)
// //   ✗ DHT22 dừng đọc
// //   ✗ LED tắt
// //   ✗ Counter reset về 0





















// // ===== BLYNK CREDENTIALS (BẮT BUỘC ĐỂ TRÊN CÙNG) =====
// #define BLYNK_TEMPLATE_ID "TMPL6Ml8uNZwj"
// #define BLYNK_TEMPLATE_NAME "BLYNK DHT"
// #define BLYNK_AUTH_TOKEN "D16by_jFGEmHsp3bkaDye3kFUJw-88vq"

// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>
// #include <DHT.h>
// #include <TM1637Display.h>

// char ssid[] = "Wokwi-GUEST";
// char pass[] = "";

// // ===== PIN CONFIG =====
// #define BUTTON_PIN 23
// #define LED_PIN 21
// #define DHT_PIN 16
// #define CLK 18
// #define DIO 19

// DHT dht(DHT_PIN, DHT22);
// TM1637Display display(CLK, DIO);
// BlynkTimer timer;

// // ===== CLOCK VARIABLES =====
// int seconds = 0;
// int minutes = 0;

// // ===== SYSTEM STATE =====
// bool isSystemOn = true;           // Đổi tên biến cho rõ nghĩa hơn
// bool buttonState = HIGH;          
// bool lastReading = HIGH;          
// unsigned long lastDebounceTime = 0;
// const unsigned long debounceDelay = 50;

// // ==========================================
// // ===== ĐỒNG BỘ TỪ APP BLYNK XUỐNG MẠCH ====
// // ==========================================
// BLYNK_WRITE(V3) {
//   isSystemOn = param.asInt();
//   digitalWrite(LED_PIN, isSystemOn ? HIGH : LOW);
//   if (!isSystemOn) {
//     display.clear(); // Xóa màn hình ngay khi bấm tắt trên điện thoại
//   }
// }

// // ==========================================
// // ===== ĐỌC VÀ GỬI DỮ LIỆU DHT22 ===========
// // ==========================================
// void sendDHT() {
//   // Chỉ đọc và gửi khi hệ thống đang BẬT
//   if (!isSystemOn) return; 

//   float temp = dht.readTemperature();
//   float hum = dht.readHumidity();

//   if (!isnan(temp) && !isnan(hum)) {
//     Blynk.virtualWrite(V0, temp);
//     Blynk.virtualWrite(V1, hum);
//   } else {
//     Serial.println("Lỗi đọc DHT22!");
//   }
// }

// // ==========================================
// // ===== XỬ LÝ BỘ ĐẾM THỜI GIAN =============
// // ==========================================
// void updateClock() {
//   // Bộ đếm thời gian vẫn tiếp tục chạy
//   seconds++;
//   if (seconds >= 60) {
//     seconds = 0;
//     minutes++;
//     if (minutes >= 60) minutes = 0;
//   }

//   // Cập nhật tổng số giây lên dashboard Blynk
//   int totalSeconds = minutes * 60 + seconds;
//   Blynk.virtualWrite(V2, totalSeconds);

//   // Hiển thị ra TM1637 nếu hệ thống đang BẬT
//   if (isSystemOn) {
//     int displayTime = minutes * 100 + seconds;
    
//     // Tạo hiệu ứng nhấp nháy dấu ':' mỗi giây
//     uint8_t colonConfig = (seconds % 2 == 0) ? 0b01000000 : 0x00;
//     display.showNumberDecEx(displayTime, colonConfig, true);
//   }
// }

// // ==========================================
// // ===== SETUP ==============================
// // ==========================================
// void setup() {
//   Serial.begin(115200);

//   pinMode(BUTTON_PIN, INPUT_PULLUP);
//   pinMode(LED_PIN, OUTPUT);

//   display.setBrightness(7);
//   dht.begin();

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

//   // Thay vì dùng millis() ở loop, ta dùng timer gọi hàm theo chu kỳ
//   timer.setInterval(2000L, sendDHT);     // 2 giây đọc DHT 1 lần
//   timer.setInterval(1000L, updateClock); // 1 giây cập nhật đồng hồ 1 lần

//   // Khởi động mạch ở trạng thái BẬT
//   digitalWrite(LED_PIN, HIGH);
//   Blynk.virtualWrite(V3, 1);
// }

// // ==========================================
// // ===== LOOP ===============================
// // ==========================================
// void loop() {
//   Blynk.run();
//   timer.run();

//   // ===== XỬ LÝ NÚT NHẤN VẬT LÝ (DEBOUNCE) =====
//   bool reading = digitalRead(BUTTON_PIN);

//   if (reading != lastReading) {
//     lastDebounceTime = millis();
//   }

//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     if (reading != buttonState) {
//       buttonState = reading;

//       // Khi phát hiện nút được bấm XUỐNG (LOW)
//       if (buttonState == LOW) {
//         isSystemOn = !isSystemOn; // Đảo trạng thái ON/OFF
        
//         // Thay đổi phần cứng ngay lập tức
//         digitalWrite(LED_PIN, isSystemOn ? HIGH : LOW);
//         if (!isSystemOn) {
//           display.clear();
//         }
        
//         // Đồng bộ trạng thái ngược lên server Blynk
//         Blynk.virtualWrite(V3, isSystemOn);
//       }
//     }
//   }
//   lastReading = reading;
// }















#define BLYNK_TEMPLATE_ID "TMPL6Ml8uNZwj"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "D16by_jFGEmHsp3bkaDye3kFUJw-88vq"

#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";   // Empty password

// Chân kết nối
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BUTTON_PIN 23
#define TM1637_CLK 18
#define TM1637_DIO 19

// Khởi tạo đối tượng
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(TM1637_CLK, TM1637_DIO);

// Biến toàn cục
bool systemOn = true;
unsigned long lastRead = 0;
const unsigned long readInterval = 2000; // 2 giây
unsigned long lastSecond = 0;
unsigned long seconds = 0;

// Biến cho nút nhấn (chống dội)
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Virtual Pin Blynk
#define VPIN_TEMP   V0
#define VPIN_HUMID  V1
#define VPIN_SYSTEM V2
#define VPIN_REMOTE V3   // Nút bật/tắt từ xa

void setup() {
  Serial.begin(115200);

  // Cấu hình LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED sáng khi khởi động

  // Cấu hình nút nhấn (pull-up nội)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);
  display.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// Hàm xử lý khi có lệnh từ Blynk (nút nhấn ảo)
BLYNK_WRITE(VPIN_REMOTE) {
  int newState = param.asInt();
  if (newState != systemOn) {
    systemOn = newState;
    if (systemOn) {
      // Bật hệ thống
      seconds = 0;
      lastSecond = millis();
      digitalWrite(LED_PIN, HIGH);
      display.setBrightness(7);
    } else {
      // Tắt hệ thống
      digitalWrite(LED_PIN, LOW);
      display.clear();
      display.setBrightness(0);
    }
    // Cập nhật trạng thái lên V2 (LED báo)
    Blynk.virtualWrite(VPIN_SYSTEM, systemOn ? 1 : 0);
  }
}

void loop() {
  Blynk.run();

  // Đọc trạng thái nút nhấn (chống dội)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) { // Nút được nhấn
      systemOn = !systemOn;
      if (systemOn) {
        seconds = 0;
        lastSecond = millis();
        digitalWrite(LED_PIN, HIGH);
        display.setBrightness(7);
      } else {
        digitalWrite(LED_PIN, LOW);
        display.clear();
        display.setBrightness(0);
      }
      // Gửi trạng thái lên Blynk (V2 và V3)
      Blynk.virtualWrite(VPIN_SYSTEM, systemOn ? 1 : 0);
      Blynk.virtualWrite(VPIN_REMOTE, systemOn ? 1 : 0); // Đồng bộ nút ảo
      delay(200); // Tránh đọc lại nhiều lần
    }
  }
  lastButtonState = reading;

  // Nếu hệ thống đang bật
  if (systemOn) {
    // Cập nhật bộ đếm thời gian mỗi giây
    if (millis() - lastSecond >= 1000) {
      lastSecond = millis();
      seconds++;
      if (seconds > 5999) seconds = 0; // Giới hạn 99:59

      // Hiển thị MM:SS lên TM1637 (dấu hai chấm ở giữa)
      int minutes = seconds / 60;
      int secs = seconds % 60;
      int displayValue = minutes * 100 + secs; // Dạng MMSS
      display.showNumberDecEx(displayValue, 0b01000000, true, 4, 0);
    }

    // Đọc DHT và gửi lên Blynk định kỳ
    if (millis() - lastRead >= readInterval) {
      lastRead = millis();
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (!isnan(h) && !isnan(t)) {
        Blynk.virtualWrite(VPIN_TEMP, t);
        Blynk.virtualWrite(VPIN_HUMID, h);
        Serial.print("Nhiệt độ: "); Serial.print(t);
        Serial.print(" °C, Độ ẩm: "); Serial.println(h);
      } else {
        Serial.println("Lỗi đọc DHT!");
      }
    }
  }
}