#include <Arduino.h>
#include <TM1637Display.h>

/* Blynk Info */
#define BLYNK_TEMPLATE_ID "TMPL68EwJFXV1"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic DHT"
#define BLYNK_AUTH_TOKEN "Ze8m7pD2pettcUcJBuCHmt6B3h6RhrPI"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>

/* WiFi */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* Pin định nghĩa */
#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19

#define DHTPIN 4
#define DHTTYPE DHT22

/* Khởi tạo */
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

unsigned long currentMiliseconds = 0;
bool blueButtonON = true;

/* Prototype */
bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

/* ======================= SETUP ======================= */
void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  digitalWrite(pinBLED, HIGH);

  // Đồng bộ trạng thái LED với Blynk
  Blynk.virtualWrite(V3, blueButtonON);

  Serial.println("== START ==");
}

/* ======================= LOOP ======================= */
void loop() {
  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHT();
}

/* ======================= FUNCTIONS ======================= */

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

/* ----------- Đọc nút bấm cứng ----------- */
void updateBlueButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  Blynk.virtualWrite(V3, blueButtonON);

  if (!blueButtonON) display.clear();
}

/* ----------- Gửi thời gian hoạt động ----------- */
void uptimeBlynk() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  unsigned long value = lastTime / 1000;

  Blynk.virtualWrite(V0, value);

  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

/* ----------- Đọc cảm biến DHT ----------- */
void readDHT() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc DHT!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("  Hum: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}

/* ----------- Nhận dữ liệu từ Blynk Switch ----------- */
BLYNK_WRITE(V3) {
  blueButtonON = param.asInt();

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON) {
    display.clear();
  }
}