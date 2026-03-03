#define BLYNK_TEMPLATE_ID "TMPL6Ml8uNZwj"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "D16by_jFGEmHsp3bkaDye3kFUJw-88vq"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// ── Cấu hình WiFi ──────────────────────────────────────
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ── Cấu hình chân ──────────────────────────────────────
#define DHT_PIN     16
#define DHT_TYPE    DHT22
#define LED_PIN     21
#define BTN_PIN     23
#define CLK_PIN     18
#define DIO_PIN     19

// ── Virtual Pin ─────────────────────────────────────────
#define VP_TEMP     V0
#define VP_HUMID    V1
#define VP_HEAT     V2
#define VP_LED      V3
#define VP_BTN      V4
#define VP_TIMER    V5   // bộ đếm thời gian (giây)
#define VP_SYSTEM   V6   // trạng thái hệ thống ON/OFF

// ── Đối tượng ───────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

// ── Biến trạng thái ─────────────────────────────────────
bool systemON       = false;   // hệ thống đang tắt khi khởi động
bool lastBtnState   = HIGH;
unsigned long counter = 0;     // bộ đếm giây

// ── Forward declaration ──────────────────────────────────
void applySystemState();  // ← thêm dòng này
// ════════════════════════════════════════════════════════
// Blynk: nhận lệnh ON/OFF từ app (V6)
// ════════════════════════════════════════════════════════
BLYNK_WRITE(VP_SYSTEM) {
  systemON = param.asInt();
  applySystemState();
}

// ════════════════════════════════════════════════════════
// Blynk: điều khiển LED từ app (V3)
// ════════════════════════════════════════════════════════
BLYNK_WRITE(VP_LED) {
  if (systemON) {
    digitalWrite(LED_PIN, param.asInt());
  }
}

// ════════════════════════════════════════════════════════
void applySystemState() {
  if (!systemON) {
    // Tắt toàn bộ
    digitalWrite(LED_PIN, LOW);
    display.clear();
    counter = 0;
    Blynk.virtualWrite(VP_TIMER, 0);
  }
  // Gửi trạng thái lên app
  Blynk.virtualWrite(VP_SYSTEM, systemON ? 1 : 0);
  Blynk.virtualWrite(VP_BTN,   systemON ? 1 : 0);
}

// ════════════════════════════════════════════════════════
// Đọc nút nhấn vật lý (GPIO 23) — toggle hệ thống
// ════════════════════════════════════════════════════════
void readButton() {
  bool currentState = digitalRead(BTN_PIN);

  if (lastBtnState == HIGH && currentState == LOW) {  // cạnh xuống
    systemON = !systemON;
    applySystemState();
  }
  lastBtnState = currentState;
}

// ════════════════════════════════════════════════════════
// Đọc DHT22 và gửi lên Blynk (mỗi 5 giây)
// ════════════════════════════════════════════════════════
void readSensor() {
  if (!systemON) return;

  float temp  = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) return;

  float heatIndex = dht.computeHeatIndex(temp, humid, false);

  Blynk.virtualWrite(VP_TEMP,  temp);
  Blynk.virtualWrite(VP_HUMID, humid);
  Blynk.virtualWrite(VP_HEAT,  heatIndex);

  Serial.printf("Temp: %.1f°C | Humid: %.1f%% | HeatIdx: %.1f°C\n",
                temp, humid, heatIndex);
}

// ════════════════════════════════════════════════════════
// Cập nhật bộ đếm thời gian trên TM1637 (mỗi 1 giây)
// ════════════════════════════════════════════════════════
void updateTimer() {
  if (!systemON) return;

  counter++;                             // đếm giây
  Blynk.virtualWrite(VP_TIMER, counter);

  // Hiển thị MM:SS (tối đa 99:59)
  unsigned long totalSec = counter % 6000; // reset sau 99p59s
  int minutes = totalSec / 60;
  int seconds = totalSec % 60;

  display.showNumberDecEx(
    minutes * 100 + seconds,   // giá trị MMSS
    0b01000000,                 // bật dấu ':'
    true                        // leading zero
  );
}

// ════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  display.setBrightness(7);
  display.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer Blynk
  timer.setInterval(1000L,  updateTimer);   // đếm thời gian mỗi 1s
  timer.setInterval(100L,   readButton);    // quét nút mỗi 100ms
  timer.setInterval(5000L,  readSensor);    // đọc cảm biến mỗi 5s
}

// ════════════════════════════════════════════════════════
void loop() {
  Blynk.run();
  timer.run();
}
// ```

// ---

// ## 📋 Cập nhật Virtual Pin Datastream

// | Virtual Pin | Tên | Type | Min | Max | Mô tả |
// |-------------|-----|------|-----|-----|-------|
// | **V0** | Temperature | Double | -40 | 80 | Nhiệt độ (°C) |
// | **V1** | Humidity | Double | 0 | 100 | Độ ẩm (%) |
// | **V2** | Heat Index | Double | -40 | 80 | Chỉ số nhiệt |
// | **V3** | LED Control | Integer | 0 | 1 | Bật/tắt LED từ app |
// | **V4** | Button State | Integer | 0 | 1 | Phản hồi trạng thái nút |
// | **V5** | Timer | Integer | 0 | 999999 | Bộ đếm giây |
// | **V6** | System ON/OFF | Integer | 0 | 1 | **Bật/tắt toàn bộ hệ thống** |

// ---

// ## 🖥️ Widget Blynk Dashboard

// | Widget | Pin | Ghi chú |
// |--------|-----|---------|
// | Gauge | V0 | Nhiệt độ |
// | Gauge | V1 | Độ ẩm |
// | Label | V2 | Heat Index |
// | Button (Switch) | **V6** | **ON/OFF hệ thống** — widget chính |
// | Button (Switch) | V3 | Bật/tắt LED |
// | Label / Value Display | V5 | Hiển thị giây đếm |

// ---

// ## ⚙️ Logic hoạt động
// ```
// Nút vật lý GPIO23 ──┐
//                      ├──► toggle systemON ──► TM1637 hiển thị MM:SS
// Blynk V6 (app) ─────┘                    ──► DHT22 đọc & gửi Blynk
//                                           ──► LED có thể điều khiển

// Khi systemON = false:
//   ✗ TM1637 tắt (display.clear)
//   ✗ DHT22 dừng đọc
//   ✗ LED tắt
//   ✗ Counter reset về 0