#define BLYNK_TEMPLATE_ID "TMPL6jkWUBBLb"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT Sensor"
#define BLYNK_AUTH_TOKEN "bMvnciUcK_lwcxq76u8bwFhNvH9L0zfP"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN DEFINE =====
#define LED_PIN 21
#define DHT_PIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19

DHT dht(DHT_PIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// ===== BIẾN =====
unsigned long startTime = 0;
unsigned long runningTime = 0;
bool ledState = false;
bool hasStarted = false;   // kiểm tra đã từng bật LED chưa

// ==========================
// Đồng bộ khi reconnect
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

// ==========================
// Điều khiển LED từ Blynk
BLYNK_WRITE(V1) {

  ledState = param.asInt();

  if (ledState) {

    digitalWrite(LED_PIN, HIGH);

    // Nếu bật lần đầu thì bắt đầu tính giờ
    if (!hasStarted) {
      startTime = millis();
      hasStarted = true;
    }

  } else {

    digitalWrite(LED_PIN, LOW);
    display.clear();   // Tắt hiển thị nhưng vẫn đếm ngầm
  }
}

// ==========================
// Đồng hồ
void updateRuntime() {

  if (hasStarted) {
    runningTime = (millis() - startTime) / 1000;
  }

  // Gửi lên Blynk luôn
  Blynk.virtualWrite(V0, runningTime);

  // Chỉ hiển thị khi LED bật
  if (ledState) {
    display.showNumberDec(runningTime, true);
  }
}

// ==========================
// DHT22
void sendDHT() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) return;

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

// ==========================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  display.setBrightness(7);
  display.clear();

  dht.begin();
  delay(1000);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, updateRuntime);
  timer.setInterval(2000L, sendDHT);
}

// ==========================
void loop() {
  Blynk.run();
  timer.run();
}