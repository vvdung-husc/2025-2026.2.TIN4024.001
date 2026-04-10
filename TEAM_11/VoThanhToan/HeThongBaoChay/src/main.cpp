#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>
#include <math.h>

// =========================
// PIN CONFIG
// =========================
#define DHT_PIN        15
#define SMOKE_AO_PIN   34
#define SMOKE_DO_PIN   4
#define LED_PIN        2
#define BUZZER_PIN     18

// =========================
// OPTIONAL THINGSPEAK
// =========================
const bool ENABLE_THINGSPEAK = false; // doi thanh true neu muon gui du lieu
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* THINGSPEAK_API_KEY = "YOUR_WRITE_API_KEY";
const unsigned long THINGSPEAK_INTERVAL_MS = 20000;

// =========================
// HEAT DETECTOR CONFIG
// =========================
// Chon mot trong 3 nhom nhiet mo phong
enum HeatGroup {
  GROUP_A = 0,
  GROUP_B = 1,
  GROUP_C = 2
};

const HeatGroup SELECTED_HEAT_GROUP = GROUP_A;

// Moc chon trong mo phong (nam trong cac dai thong dung EN54-5)
const float FIXED_TEMP_A = 60.0;  // gan nhom A
const float FIXED_TEMP_B = 80.0;  // gan nhom B
const float FIXED_TEMP_C = 90.0;  // gan nhom C

// Nhiet gia tang: thuong dung moc khoang 8.3°C/phut
const float RATE_OF_RISE_THRESHOLD = 8.3;

// =========================
// OPTICAL SMOKE / LIGHT CONFIG
// =========================
// Lux thap => bao dong
const float SMOKE_LUX_LOW_THRESHOLD = 100.0;

// Lux cao => bao dong theo yeu cau moi
const float SMOKE_LUX_HIGH_THRESHOLD = 2000.0;

// Nhiet do > 45°C => bao dong theo yeu cau moi
const float TEMP_ALARM_THRESHOLD = 45.0;

// Cong thuc lux theo Wokwi docs cua photoresistor
const float LDR_GAMMA = 0.7;
const float LDR_RL10 = 50.0;

// =========================
// GLOBALS
// =========================
DHTesp dht;

unsigned long lastLoopMs = 0;
unsigned long lastThingSpeakMs = 0;

// Luu lich su nhiet do de tinh toc do tang trong ~1 phut
const int HISTORY_SIZE = 40; // du cho chu ky 2 giay
float tempHistory[HISTORY_SIZE];
unsigned long timeHistory[HISTORY_SIZE];
int historyCount = 0;
int historyHead = 0; // vi tri ghi tiep theo

struct DetectorState {
  float temperature;
  float humidity;
  int smokeAdc;
  float smokeLux;
  bool smokeDoDark;

  float fixedThreshold;
  float ratePerMinute;

  bool fixedHeatAlarm;
  bool rateRiseAlarm;
  bool smokeAlarm;
  bool highLuxAlarm;
  bool temp45Alarm;
  bool fireAlarm;

  int alarmCode;
  String causeText;
};

float getFixedThreshold(HeatGroup group) {
  switch (group) {
    case GROUP_A: return FIXED_TEMP_A;
    case GROUP_B: return FIXED_TEMP_B;
    case GROUP_C: return FIXED_TEMP_C;
    default:      return FIXED_TEMP_A;
  }
}

void connectWiFi() {
  if (!ENABLE_THINGSPEAK) return;

  Serial.print("Dang ket noi WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi da ket noi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void buzzerOn(int freq = 1000) {
  tone(BUZZER_PIN, freq);
}

void buzzerOff() {
  noTone(BUZZER_PIN);
}

void pushTemperatureSample(float temp, unsigned long nowMs) {
  tempHistory[historyHead] = temp;
  timeHistory[historyHead] = nowMs;
  historyHead = (historyHead + 1) % HISTORY_SIZE;
  if (historyCount < HISTORY_SIZE) historyCount++;
}

float computeRateOfRise(float currentTemp, unsigned long nowMs) {
  if (historyCount < 2) return 0.0;

  int oldestIndex = (historyCount == HISTORY_SIZE) ? historyHead : 0;
  float oldestTemp = tempHistory[oldestIndex];
  unsigned long oldestTime = timeHistory[oldestIndex];

  unsigned long elapsedMs = nowMs - oldestTime;
  if (elapsedMs < 10000) return 0.0; // chua du du lieu

  float elapsedMin = elapsedMs / 60000.0f;
  if (elapsedMin <= 0.0f) return 0.0;

  float rate = (currentTemp - oldestTemp) / elapsedMin;
  if (rate < 0) rate = 0;
  return rate;
}

float adcToLux(int adcValue) {
  // ESP32 analogRead mac dinh 12-bit: 0..4095
  float voltage = (adcValue / 4095.0f) * 5.0f;

  if (voltage <= 0.01f) {
    return 100000.0f; // rat sang
  }
  if (voltage >= 4.99f) {
    return 0.01f; // rat toi
  }

  float resistance = 2000.0f * voltage / (1.0f - voltage / 5.0f);
  float lux = pow((LDR_RL10 * 1000.0f * pow(10.0f, LDR_GAMMA)) / resistance, (1.0f / LDR_GAMMA));

  if (!isfinite(lux) || lux < 0) return 0.01f;
  return lux;
}

DetectorState readDetectors() {
  DetectorState s;

  TempAndHumidity th = dht.getTempAndHumidity();
  s.temperature = th.temperature;
  s.humidity = th.humidity;

  if (isnan(s.temperature) || isnan(s.humidity)) {
    s.temperature = 0;
    s.humidity = 0;
  }

  s.smokeAdc = analogRead(SMOKE_AO_PIN);
  s.smokeLux = adcToLux(s.smokeAdc);
  s.smokeDoDark = (digitalRead(SMOKE_DO_PIN) == HIGH);

  s.fixedThreshold = getFixedThreshold(SELECTED_HEAT_GROUP);
  s.ratePerMinute = computeRateOfRise(s.temperature, millis());

  s.fixedHeatAlarm = (s.temperature >= s.fixedThreshold);
  s.rateRiseAlarm = (s.ratePerMinute >= RATE_OF_RISE_THRESHOLD);
  s.smokeAlarm = (s.smokeLux <= SMOKE_LUX_LOW_THRESHOLD) || s.smokeDoDark;

  // Dieu kien moi
  s.highLuxAlarm = (s.smokeLux > SMOKE_LUX_HIGH_THRESHOLD);
  s.temp45Alarm = (s.temperature > TEMP_ALARM_THRESHOLD);

  // Tong hop bao dong
  s.fireAlarm = s.fixedHeatAlarm || s.rateRiseAlarm || s.smokeAlarm ||
                s.highLuxAlarm || s.temp45Alarm;

  int causeCount = 0;
  s.causeText = "";

  if (s.fixedHeatAlarm) {
    s.causeText += "FIXED_HEAT ";
    causeCount++;
  }
  if (s.rateRiseAlarm) {
    s.causeText += "RATE_OF_RISE ";
    causeCount++;
  }
  if (s.smokeAlarm) {
    s.causeText += "SMOKE_LOW_LUX ";
    causeCount++;
  }
  if (s.highLuxAlarm) {
    s.causeText += "HIGH_LUX ";
    causeCount++;
  }
  if (s.temp45Alarm) {
    s.causeText += "TEMP_OVER_45 ";
    causeCount++;
  }

  if (!s.fireAlarm) {
    s.alarmCode = 0;           // NORMAL
    s.causeText = "NORMAL";
  } else if (causeCount == 1 && s.smokeAlarm) {
    s.alarmCode = 1;           // chi khói/lux thap
  } else if (causeCount == 1 && s.rateRiseAlarm) {
    s.alarmCode = 2;           // chi tang nhiet nhanh
  } else if (causeCount == 1 && s.fixedHeatAlarm) {
    s.alarmCode = 3;           // chi vuot nhiet co dinh
  } else if (causeCount == 1 && s.highLuxAlarm) {
    s.alarmCode = 5;           // chi lux cao
  } else if (causeCount == 1 && s.temp45Alarm) {
    s.alarmCode = 6;           // chi nhiet >45
  } else {
    s.alarmCode = 4;           // nhieu nguyen nhan
  }

  return s;
}

void handleAlarm(const DetectorState& s) {
  if (s.fireAlarm) {
    digitalWrite(LED_PIN, HIGH);
    buzzerOn(1000);
  } else {
    digitalWrite(LED_PIN, LOW);
    buzzerOff();
  }
}

void printState(const DetectorState& s) {
  Serial.println("\n========== FIRE DETECTION ==========");
  Serial.print("Nhiet do hien tai: ");
  Serial.print(s.temperature);
  Serial.println(" C");

  Serial.print("Do am: ");
  Serial.print(s.humidity);
  Serial.println(" %");

  Serial.print("Nguong nhiet co dinh: ");
  Serial.print(s.fixedThreshold);
  Serial.println(" C");

  Serial.print("Toc do tang nhiet: ");
  Serial.print(s.ratePerMinute);
  Serial.println(" C/phut");

  Serial.print("Smoke ADC: ");
  Serial.println(s.smokeAdc);

  Serial.print("Smoke Lux: ");
  Serial.println(s.smokeLux);

  Serial.print("DO Dark: ");
  Serial.println(s.smokeDoDark ? "YES" : "NO");

  Serial.print("Fixed heat alarm: ");
  Serial.println(s.fixedHeatAlarm ? "YES" : "NO");

  Serial.print("Rate-of-rise alarm: ");
  Serial.println(s.rateRiseAlarm ? "YES" : "NO");

  Serial.print("Smoke low lux alarm: ");
  Serial.println(s.smokeAlarm ? "YES" : "NO");

  Serial.print("High lux alarm: ");
  Serial.println(s.highLuxAlarm ? "YES" : "NO");

  Serial.print("Temp > 45C alarm: ");
  Serial.println(s.temp45Alarm ? "YES" : "NO");

  Serial.print("Tong trang thai: ");
  Serial.println(s.fireAlarm ? "FIRE ALARM" : "NORMAL");

  Serial.print("Cause: ");
  Serial.println(s.causeText);

  Serial.print("Alarm code: ");
  Serial.println(s.alarmCode);
  Serial.println("====================================");
}

void sendToThingSpeak(const DetectorState& s) {
  if (!ENABLE_THINGSPEAK) return;

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + String(THINGSPEAK_API_KEY) +
               "&field1=" + String(s.temperature, 2) +
               "&field2=" + String(s.ratePerMinute, 2) +
               "&field3=" + String(s.smokeLux, 2) +
               "&field4=" + String(s.alarmCode);

  http.begin(url);
  int httpCode = http.GET();

  Serial.print("ThingSpeak HTTP code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.print("ThingSpeak response: ");
    Serial.println(payload);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SMOKE_DO_PIN, INPUT);
  analogReadResolution(12);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  digitalWrite(LED_PIN, LOW);
  buzzerOff();

  if (ENABLE_THINGSPEAK) {
    connectWiFi();
  }

  // Ghi mau dau tien
  unsigned long now = millis();
  TempAndHumidity th = dht.getTempAndHumidity();
  float t = isnan(th.temperature) ? 0 : th.temperature;
  pushTemperatureSample(t, now);

  lastLoopMs = now;
  lastThingSpeakMs = now;
}

void loop() {
  unsigned long now = millis();

  // Cap nhat mau nhiet do moi 2 giay
  if (now - lastLoopMs >= 2000) {
    lastLoopMs = now;

    TempAndHumidity th = dht.getTempAndHumidity();
    float t = isnan(th.temperature) ? 0 : th.temperature;
    pushTemperatureSample(t, now);

    DetectorState state = readDetectors();
    handleAlarm(state);
    printState(state);

    if (ENABLE_THINGSPEAK && (now - lastThingSpeakMs >= THINGSPEAK_INTERVAL_MS)) {
      lastThingSpeakMs = now;
      sendToThingSpeak(state);
    }
  }
}