#define BLYNK_TEMPLATE_ID "TMPL6vwxTSqx4"
#define BLYNK_TEMPLATE_NAME "API ESP32"
#define BLYNK_AUTH_TOKEN "6mOpfMSuA61yzGX7QWFZ9sgmeyLY0kPV"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

#define OPENWEATHERMAP_KEY ""

//==============================
// STRUCT LƯU THÔNG TIN GEO
//==============================

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

unsigned long currentMiliseconds = 0;
String urlWeather;

//==============================
// TIMER KHÔNG BLOCK
//==============================

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//==============================
// FORMAT STRING
//==============================

String StringFormat(const char* fmt, ...) {

  va_list args;
  va_start(args, fmt);

  int len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char buffer[len + 1];

  va_start(args, fmt);
  vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  return String(buffer);
}

//==============================
// PARSE GEO API
//==============================

void parseGeoInfo(String payload, IP4_Info& ipInfo) {

  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {

    int delimiter = payload.indexOf('|');

    if (delimiter == -1) {
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0, delimiter);
    payload = payload.substring(delimiter + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5];
  ipInfo.latitude = values[6];

  Serial.println("----- GEO INFO -----");
  Serial.println("IP: " + ipInfo.ip4);
  Serial.println("Longitude: " + ipInfo.longtitude);
  Serial.println("Latitude: " + ipInfo.latitude);
}

//==============================
// GET IP + LOCATION
//==============================

void getAPI() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode > 0) {

    String response = http.getString();

    parseGeoInfo(response, ip4Info);

    String mapLink = StringFormat(
      "https://www.google.com/maps/place/%s,%s",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str()
    );

    Serial.println("Google Maps:");
    Serial.println(mapLink);

    urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str(),
      OPENWEATHERMAP_KEY
    );

    Serial.println("Weather API:");
    Serial.println(urlWeather);

  } else {

    Serial.print("HTTP error: ");
    Serial.println(httpCode);

  }

  http.end();
}

//==============================
// UPDATE NHIỆT ĐỘ
//==============================

void updateTemp() {

  static unsigned long lastTime = 0;
  static float oldTemp = 0;

  if (!IsReady(lastTime, 10000)) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (urlWeather == "") return;

  HTTPClient http;

  http.begin(urlWeather);

  int httpCode = http.GET();

  if (httpCode > 0) {

    String response = http.getString();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, response);

    if (!error) {

      float temp = doc["main"]["temp"];

      if (temp != oldTemp) {

        oldTemp = temp;

        Serial.print("Temperature: ");
        Serial.println(temp);

        Blynk.virtualWrite(V3, temp);

      }
    }

  }

  http.end();
}

//==============================
// GỬI IP + GOOGLE MAPS
//==============================

void onceCalled() {

  static bool done = false;

  if (done) return;
  if (ip4Info.ip4 == "") return;

  done = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

//==============================
// UPTIME
//==============================

void uptimeBlynk() {

  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  unsigned long uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);
}

//==============================
// SETUP
//==============================

void setup() {

  Serial.begin(115200);

  Serial.println("Connecting WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");

  }

  Serial.println(" Connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connected to Blynk!");

  getAPI();
}

//==============================
// LOOP
//==============================

void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();
}