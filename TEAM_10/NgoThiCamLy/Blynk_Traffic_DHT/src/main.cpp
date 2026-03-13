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
bool blueButtonON = false;
unsigned long activeSeconds = 0; 

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

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
  //Blynk.virtualWrite(V1, blueButtonON);
  //Blynk.virtualWrite(V0, activeSeconds); 
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}
// ================== LOOP ==================
void loop() {
  Blynk.run();
  currentMiliseconds = millis();

  updateBlueButton();   // xử lý nút trước
  uptimeBlynk();        // rồi mới đếm
  sendDHT();
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// ----- Button Control (Đã sửa logic Reset) -----
void updateBlueButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  
  // RESET KHI BẬT LÊN
  if (blueButtonON) {
    //activeSeconds = 0; // Xóa dữ liệu cũ, bắt đầu lại từ đầu
    Blynk.virtualWrite(V0, activeSeconds);
    display.showNumberDec(activeSeconds);
  } else {
    display.clear();
  }

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println(blueButtonON ? "Blue Light ON - Restart from 0" : "Blue Light OFF");
}

// ----- Uptime -----
void uptimeBlynk() {
  static unsigned long lastTime = 0;

  if (!blueButtonON) {
    return;   // TẮT LÀ THOÁT NGAY
  }

  if (millis() - lastTime >= 1000) {
    lastTime = millis();
    activeSeconds++;

    Blynk.virtualWrite(V0, activeSeconds);
    display.showNumberDec(activeSeconds);

    Serial.print("Seconds: ");
    Serial.println(activeSeconds);
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

// ----- Blynk Receive (Đã sửa logic Reset) -----
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  
  // RESET KHI BẬT TỪ APP BLYNK
  if (blueButtonON) {
    //activeSeconds = 0; // Xóa dữ liệu cũ, bắt đầu lại từ đầu
    Blynk.virtualWrite(V0, activeSeconds);
    display.showNumberDec(activeSeconds);
  } else {
    display.clear();
  }

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Serial.println(blueButtonON ? "Blynk -> ON - Restart from 0" : "Blynk -> OFF");
}