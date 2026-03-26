#define BLYNK_TEMPLATE_ID "TMPL6iJh7kbPx"
#define BLYNK_TEMPLATE_NAME "Blynk API"
#define BLYNK_AUTH_TOKEN "EaAUgSqeTMbJPmTFHI4cyIBblHHC8TnP"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define OPENWEATHERMAP_KEY "729ff1bcff5cd0c2e58362ee8faa2273"

struct IP4_Info {
  String ip4 = "0.0.0.0";
  String latitude = "0";
  String longitude = "0";
};

IP4_Info ip4Info;
unsigned long currentMillis = 0;
String urlWeather = "";

// Hàm quản lý thời gian (Non-blocking)
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// Lấy dữ liệu IP và Vị trí THẬT từ ip-api
void getGeoAPI() {
  if(WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  http.setTimeout(8000);
  http.begin("http://ip-api.com/json/");
  
  int httpCode = http.GET();
  if(httpCode == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    
    ip4Info.ip4 = doc["query"].as<String>();
    ip4Info.latitude = doc["lat"].as<String>();
    ip4Info.longitude = doc["lon"].as<String>();

    // Gửi dữ liệu thật lên các chân V4, V5
    Blynk.virtualWrite(V4, ip4Info.ip4); 
    String mapsLink = "http://www.google.com/maps/place/" + ip4Info.latitude + "," + ip4Info.longitude;
    Blynk.virtualWrite(V5, mapsLink);   
    
    urlWeather = "http://api.openweathermap.org/data/2.5/weather?lat=" + ip4Info.latitude + "&lon=" + ip4Info.longitude + "&appid=" + OPENWEATHERMAP_KEY + "&units=metric";
    Serial.println("Đã lấy được IP THẬT: " + ip4Info.ip4);
  }
  http.end();
}

void updateWeather() {
  static unsigned long lastWeatherTime = 0;
  if (!IsReady(lastWeatherTime, 30000)) return; 
  if (urlWeather == "" || WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(urlWeather);
  if(http.GET() == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    float temp = doc["main"]["temp"];
    Blynk.virtualWrite(V1, temp); 
  }
  http.end();
}

void uptimeBlynk() {
  static unsigned long lastUptimeTime = 0;
  if (!IsReady(lastUptimeTime, 1000)) return;
  Blynk.virtualWrite(V0, millis() / 1000); // Chân Uptime
  Blynk.virtualWrite(V6, "ĐÀO VĂN LỢI"); 
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getGeoAPI(); 
}

void loop() {
  Blynk.run();
  uptimeBlynk();
  updateWeather();

  // Nếu chưa có IP thì thử lại mỗi 10s
  static unsigned long retryGeo = 0;
  if (ip4Info.ip4 == "0.0.0.0" && IsReady(retryGeo, 10000)) {
    getGeoAPI();
  }
}