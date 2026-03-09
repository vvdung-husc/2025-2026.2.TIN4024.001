#define BLYNK_TEMPLATE_ID "TMPL6iJh7kbPx"
#define BLYNK_TEMPLATE_NAME "Blynk API"
#define BLYNK_AUTH_TOKEN "EaAUgSqeTMbJPmTFHI4cyIBblHHC8TnP"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
String apiKey = "729ff1bcff5cd0c2e58362ee8faa2273";

BlynkTimer timer;
int countdown = 0;

void fetchApiData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(5000);
    
    // 1. Dữ liệu mặc định (Chống null 100%)
    String ip = "123.25.115.141"; 
    float lat = 16.466669;
    float lon = 107.594444;

    // 2. Thử lấy dữ liệu thật từ API
    http.begin("http://ip4.iothings.vn/?geo=1");
    if (http.GET() == 200) {
      JsonDocument doc; // Chuẩn v7: Không cần (1024)
      DeserializationError error = deserializeJson(doc, http.getString());
      if (!error) {
        if (!doc["ip"].isNull()) ip = doc["ip"].as<String>();
        if (!doc["lat"].isNull()) lat = doc["lat"].as<float>();
        if (!doc["lon"].isNull()) lon = doc["lon"].as<float>();
      }
    }
    
    // Gửi lên Blynk
    Blynk.virtualWrite(V4, ip);
    Blynk.virtualWrite(V5, "https://www.google.com/maps/place/" + String(lat, 6) + "," + String(lon, 6));

    // 3. Lấy thời tiết
    String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + apiKey + "&units=metric";
    http.begin(weatherUrl);
    if (http.GET() == 200) {
      JsonDocument weatherDoc; // Chuẩn v7
      deserializeJson(weatherDoc, http.getString());
      if (!weatherDoc["main"]["temp"].isNull()) {
        Blynk.virtualWrite(V1, weatherDoc["main"]["temp"].as<float>());
      }
    }
    http.end();
  }
}

void updateTimer() {
  countdown++;
  Blynk.virtualWrite(V0, countdown);
  Blynk.virtualWrite(V6, "ĐÀO VĂN LỢI"); // Gửi tên định danh
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(60000L, fetchApiData); 
  timer.setInterval(1000L, updateTimer);
  
  fetchApiData(); 
}

void loop() {
  Blynk.run();
  timer.run();
}