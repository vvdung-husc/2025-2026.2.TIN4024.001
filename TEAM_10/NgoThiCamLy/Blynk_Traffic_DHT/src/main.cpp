#include <Arduino.h>
#include <TM1637Display.h>

/* Blynk Info */
#define BLYNK_TEMPLATE_ID "TMPL6JoJWpKDu"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT"
#define BLYNK_AUTH_TOKEN "EUXbZSNGKNPBFoPYxOf6SxRoahUb82PB"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi (Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define btnBLED  23
#define pinBLED  21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

// ===== GLOBAL =====
unsigned long currentMiliseconds = 0;
bool blueButtonON = true;
unsigned long activeSeconds = 0; // BIẾN MỚI: Chỉ dùng để đếm giây khi đèn bật

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// ===== FUNCTION DECLARE =====
bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void sendDHT();

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  display.setBrightness(0x0f);
  dht.begin();
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Blynk.virtualWrite(V0, activeSeconds); // Gửi giá trị khởi tạo lên Blynk
}

// ================== LOOP ==================
void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  sendDHT();
}

// ================= FUNCTIONS =================

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// ----- Button Control -----
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
  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) display.clear();
  else display.showNumberDec(activeSeconds); // Hiện lại số giây hiện tại khi bật đèn

  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
}

// ----- Uptime (Đã sửa logic) -----
void uptimeBlynk() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  // CHỈ THỰC HIỆN KHI ĐÈN ĐANG BẬT
  if (blueButtonON) {
    activeSeconds++; // Tăng biến đếm thủ công mỗi 1 giây
    
    Blynk.virtualWrite(V0, activeSeconds); // Cập nhật lên Blynk V0
    display.showNumberDec(activeSeconds);  // Cập nhật lên màn hình TM1637
  }
}

// ----- DHT22 -----
void sendDHT() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum)) return;
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

// ----- Blynk Receive -----
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) display.clear();
  else display.showNumberDec(activeSeconds);
  Serial.println(blueButtonON ? "Blynk -> ON" : "Blynk -> OFF");
}