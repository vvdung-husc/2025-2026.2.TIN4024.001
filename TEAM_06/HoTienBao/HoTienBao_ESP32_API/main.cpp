#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define BLYNK_TEMPLATE_ID "TMPL6ZCP__m7F"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "Qay0XiSog6XOd6W1kNCpryVsuV0J3s6f"
#include <BlynkSimpleEsp32.h>

// BẮT BUỘC dùng mạng này cho Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = ""; 
String weatherKey = "aa75553a2028be176ca210c0b27e0b4d";

BlynkTimer timer;

void fetchData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://ip4.iothings.vn?geo=1");
    if (http.GET() > 0) {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, http.getString());
      String ip = doc["ip"].as<String>();
      String lat = doc["lat"].as<String>();
      String lon = doc["lon"].as<String>();
      String gMaps = "https://www.google.com/maps/place/" + lat + "," + lon;

      Serial.println("=============================");
      Serial.println("IP: " + ip);
      Serial.println("Link Maps: " + gMaps);

      Blynk.virtualWrite(V1, ip);
      Blynk.virtualWrite(V2, gMaps);
      http.end(); 

      String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + weatherKey + "&units=metric";
      http.begin(weatherUrl);
      if (http.GET() > 0) {
        DynamicJsonDocument weatherDoc(1024);
        deserializeJson(weatherDoc, http.getString());
        float temp = weatherDoc["main"]["temp"];
        Serial.printf("Nhiệt độ hiện tại: %.2f *C\n", temp);
        Blynk.virtualWrite(V3, temp);
      }
      http.end(); 
    } else {
      http.end();
    }
  }
}

void sendUptime() {
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nĐang kết nối WiFi Wokwi-GUEST...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Đã kết nối WiFi và Blynk!");
  fetchData(); 
  timer.setInterval(300000L, fetchData); 
  timer.setInterval(1000L, sendUptime);  
}

void loop() {
  Blynk.run();
  timer.run();
}