#include <Arduino.h>
#include <stdarg.h>

// --- THÔNG SỐ BLYNK ---
#define BLYNK_TEMPLATE_ID "TMPL6sdiqhlOk"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// --- API KEY THỜI TIẾT ---
#define OPENWEATHERMAP_KEY "YOUR_OPENWEATHER_API_KEY"

struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

String urlWeather;
unsigned long currentMiliseconds = 0;

// ------------------------------------------------
// NON-BLOCKING TIMER
// ------------------------------------------------
bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// ------------------------------------------------
// FORMAT STRING
// ------------------------------------------------
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);

  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), fmt, vaArgs);

  va_end(vaArgs);

  return String(buffer);
}

// ------------------------------------------------
// PARSE GEO LOCATION
// ------------------------------------------------
void parseGeoInfo(String payload, IP4_Info& ipInfo) {

  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {

    int delimiterIndex = payload.indexOf('|');

    if (delimiterIndex == -1) {
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0, delimiterIndex);
    payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[5];
  ipInfo.longitude = values[6];

  ipInfo.ip4.trim();
  ipInfo.latitude.trim();
  ipInfo.longitude.trim();

  Serial.printf("IP: %s\n", ipInfo.ip4.c_str());
  Serial.printf("LAT: %s\n", ipInfo.latitude.c_str());
  Serial.printf("LON: %s\n", ipInfo.longitude.c_str());
}

// ------------------------------------------------
// GET IP + GEO LOCATION
// ------------------------------------------------
void getAPI() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {

    String response = http.getString();

    parseGeoInfo(response, ip4Info);

    urlWeather = StringFormat(
      "http://api.openweathermap.org/data/2.5/weather?lat=16.4&lon=107.6&appid=%s&units=metric",
      OPENWEATHERMAP_KEY
    );
  }

  http.end();
}

// ------------------------------------------------
// UPDATE TEMPERATURE
// ------------------------------------------------
void updateTemp() {

  static unsigned long lastTime = 0;
  static float temp_ = 0;

  if (!IsReady(lastTime, 10000)) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (urlWeather == "") return;

  HTTPClient http;

  http.begin(urlWeather);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {

    String response = http.getString();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, response);

    if (!error) {

      float temp = doc["main"]["temp"];

      if (temp_ != temp) {

        temp_ = temp;

        Serial.print("Nhiet do: ");
        Serial.println(temp_);

        Blynk.virtualWrite(V3, temp_);
      }

    } else {

      Serial.print("JSON Error: ");
      Serial.println(error.c_str());
    }

  } else {

    Serial.print("Weather API Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// ------------------------------------------------
// SEND DATA TO BLYNK (ONCE)
// ------------------------------------------------
void onceCalled() {

  static bool done_ = false;

  if (done_) return;
  if (ip4Info.ip4 == "") return;

  done_ = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
  Blynk.virtualWrite(V4, "Tran Van Tien");
}

// ------------------------------------------------
// SEND UPTIME
// ------------------------------------------------
void uptimeBlynk() {

  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  unsigned long value = currentMiliseconds / 1000;

  Blynk.virtualWrite(V0, value);
}

// ------------------------------------------------
// SETUP
// ------------------------------------------------
void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println(" OK");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

// ------------------------------------------------
// LOOP
// ------------------------------------------------
void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();
}