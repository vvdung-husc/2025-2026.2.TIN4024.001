#define BLYNK_TEMPLATE_ID "TMPL62kk3uTHb"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "_wN3X6vyLAMbVGSniJjSAIQDCKAwtCbe"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// OpenWeather API key
String weatherAPI = "86f74cefc34d2d0ff415f6e31893a1d4";

unsigned long lastUptime = 0;
unsigned long lastAPI = 0;

void getGeoInfo();
void getTemperature(float lat, float lon);

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  // Fix DNS (quan trọng với Wokwi)
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8,8,8,8));

  // Kết nối Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
}

void loop() {

  if (!Blynk.connected()) {
    Blynk.connect();
  }

  Blynk.run();

  // Uptime mỗi 1 giây
  if (millis() - lastUptime > 1000) {

    lastUptime = millis();

    unsigned long uptime = millis() / 1000;

    Serial.print("Uptime: ");
    Serial.println(uptime);

    Blynk.virtualWrite(V0, uptime);
  }

  // Gọi API mỗi 10 giây
  if (millis() - lastAPI > 10000) {

    lastAPI = millis();

    getGeoInfo();
  }
}

void getGeoInfo() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    Serial.println("\nAPI Response:");
    Serial.println(payload);

    String data[7];
    int index = 0;

    while (payload.length() > 0 && index < 7) {

      int pos = payload.indexOf('|');

      if (pos == -1) {
        data[index++] = payload;
        break;
      }

      data[index++] = payload.substring(0, pos);
      payload = payload.substring(pos + 1);
    }

    String ip = data[0];
    String longitude = data[5];
    String latitude = data[6];

    String maps =
    "https://www.google.com/maps/place/" +
    latitude + "," + longitude;

    Serial.println("\n===== GEO INFO =====");

    Serial.print("IPv4: ");
    Serial.println(ip);

    Serial.print("Latitude: ");
    Serial.println(latitude);

    Serial.print("Longitude: ");
    Serial.println(longitude);

    Serial.print("Google Maps: ");
    Serial.println(maps);

    Serial.println("====================");

    // Gửi lên Blynk
    Blynk.virtualWrite(V1, ip);
    Blynk.virtualWrite(V2, maps);

    // Lấy nhiệt độ
    float lat = latitude.toFloat();
    float lon = longitude.toFloat();

    getTemperature(lat, lon);
  }

  else {

    Serial.print("HTTP Error: ");
    Serial.println(httpCode);

  }

  http.end();
}

void getTemperature(float lat, float lon) {

  HTTPClient http;

  String url =
  "https://api.openweathermap.org/data/2.5/weather?lat=" +
  String(lat) +
  "&lon=" +
  String(lon) +
  "&appid=" +
  weatherAPI +
  "&units=metric";

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    JsonDocument doc;
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    // Gửi lên Blynk
    Blynk.virtualWrite(V3, temp);
  }

  else {

    Serial.print("Weather API error: ");
    Serial.println(httpCode);

  }

  http.end();
}