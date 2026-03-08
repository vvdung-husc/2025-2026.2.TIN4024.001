#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected");

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    Serial.println(payload);

    DynamicJsonDocument doc(1024);

    deserializeJson(doc, payload);

    String ip = doc["ip"];

    float lat = doc["geo"]["latitude"];
    float lon = doc["geo"]["longitude"];

    Serial.println("IPv4:");
    Serial.println(ip);

    Serial.println("Latitude:");
    Serial.println(lat);

    Serial.println("Longitude:");
    Serial.println(lon);

    String mapLink =
      "https://www.google.com/maps/place/" +
      String(lat) + "," + String(lon);

    Serial.println("Google Maps:");
    Serial.println(mapLink);
  }

  http.end();
}

void loop() {

}