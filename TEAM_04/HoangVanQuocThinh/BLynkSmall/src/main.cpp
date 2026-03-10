#define BLYNK_TEMPLATE_ID "TMPL6n34QV4mT"
#define BLYNK_TEMPLATE_NAME "SmallKON"
#define BLYNK_AUTH_TOKEN "jWDAZ5S1TxThm6fUK9rjN6rDQTTRoNLG"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define BUTTON_PIN 23
#define LED_PIN 21
#define DHT_PIN 16
#define CLK 18
#define DIO 19

DHT dht(DHT_PIN, DHT22);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// ===== CLOCK VARIABLES =====
unsigned long previousMillis = 0;
int seconds = 0;
int minutes = 0;

// ===== BUTTON VARIABLES =====
bool showDisplay = true;
bool buttonState = HIGH;          // trạng thái đã ổn định
bool lastReading = HIGH;          // đọc gần nhất
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// =======================
// ===== BLYNK WRITE =====
// =======================
BLYNK_WRITE(V3) {
  showDisplay = param.asInt();
}

// =======================
// ===== SEND DHT ========
// =======================
void sendDHT() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
  }
}

// =======================
// ===== SEND TIME =======
// =======================
void sendTime() {
  int totalSeconds = minutes * 60 + seconds;
  Blynk.virtualWrite(V2, totalSeconds);
}

// =======================
// ===== SETUP ===========
// =======================
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  display.setBrightness(7);
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendDHT);
  timer.setInterval(1000L, sendTime);

  // Khởi động bật sẵn
  digitalWrite(LED_PIN, HIGH);
  Blynk.virtualWrite(V3, 1);
}

// =======================
// ===== LOOP ============
// =======================
void loop() {
  Blynk.run();
  timer.run();

  // ===== CLOCK RUNNING CHUẨN 1 GIÂY =====
  if (millis() - previousMillis >= 1000) {
    previousMillis += 1000;
    seconds++;

    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) minutes = 0;
    }
  }

  // ===== BUTTON DEBOUNCE CHUẨN =====
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Nhấn xuống (LOW vì INPUT_PULLUP)
      if (buttonState == LOW) {
        showDisplay = !showDisplay;
        Blynk.virtualWrite(V3, showDisplay);
      }
    }
  }

  lastReading = reading;

  // ===== DISPLAY CONTROL =====
  if (showDisplay) {
    digitalWrite(LED_PIN, HIGH);
    int displayTime = minutes * 100 + seconds;
    display.showNumberDecEx(displayTime, 0b01000000, true);
  } else {
    digitalWrite(LED_PIN, LOW);
    display.clear();
  }
}