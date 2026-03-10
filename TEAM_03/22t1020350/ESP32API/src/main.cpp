#define BLYNK_TEMPLATE_ID "TMPL6PL870nVz"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "b__UR--5MwA11uaHuK18_v5xM7tWVvc8"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Link API thời tiết Huế
const char* serverName = "https://api.openweathermap.org/data/2.5/weather?lat=16.466669&lon=107.599998&appid=cca78d44ec7225425593b8b1b1d43751&units=metric";

BlynkTimer timer;

void sendDataToBlynk() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName); 
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);

      float temp = doc["main"]["temp"];
      
      Blynk.virtualWrite(V0, millis() / 1000); 
      Blynk.virtualWrite(V1, temp); 
      Blynk.virtualWrite(V2, WiFi.localIP().toString()); 
      
      String mapUrl = "https://www.google.com/maps/place/16.466669,107.599998";
      Blynk.virtualWrite(V3, mapUrl); 

      Serial.print("Da gui du lieu len Blynk. Nhiet do: ");
      Serial.println(temp);
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(10000L, sendDataToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}