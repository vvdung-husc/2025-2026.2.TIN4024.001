#define BLYNK_TEMPLATE_ID "TMPL6OYE0S58j"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "8MlGu-9s0AtZbVb4dBwwzg1L8GDHYgfZ"

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// WIFI
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Timer
BlynkTimer timer;

// API
String ipAPI = "http://ip-api.com/json";

String weatherAPI =
"https://api.openweathermap.org/data/2.5/weather?lat=22.633329&lon=106.400002&appid=a8e28eb3fa6627332947b2d3e8b83d2d&units=metric";

// Variables
String ipv4;
float lat;
float lon;
float temperature;

unsigned long uptimeSeconds = 0;

// ================= UPTIME =================
void sendUptime() {

  uptimeSeconds = millis() / 1000;

  Serial.print("Uptime: ");
  Serial.println(uptimeSeconds);

  Blynk.virtualWrite(V0, uptimeSeconds);
}

// ================= GET IP + LOCATION =================
void getIPLocation() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(ipAPI);

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    Serial.println("IP API Response:");
    Serial.println(payload);

    JsonDocument doc;
    deserializeJson(doc, payload);

    ipv4 = doc["query"].as<String>();
    lat = doc["lat"];
    lon = doc["lon"];

    Serial.print("IPv4: ");
    Serial.println(ipv4);

    Serial.print("Latitude: ");
    Serial.println(lat);

    Serial.print("Longitude: ");
    Serial.println(lon);

    String mapLink =
      "https://www.google.com/maps/@" +
      String(lat,6) + "," + String(lon,6);

    Serial.print("Google Maps: ");
    Serial.println(mapLink);

    Blynk.virtualWrite(V1, ipv4);
    Blynk.virtualWrite(V2, mapLink);
  }

  http.end();
}

// ================= GET WEATHER =================
void getWeather() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(weatherAPI);

  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    Serial.println("Weather API:");
    Serial.println(payload);

    JsonDocument doc;
    deserializeJson(doc, payload);

    temperature = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temperature);

    Blynk.virtualWrite(V3, temperature);
  }

  http.end();
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer tasks
  timer.setInterval(1000L, sendUptime);
  timer.setInterval(15000L, getIPLocation);
  timer.setInterval(20000L, getWeather);
}

// ================= LOOP =================
void loop() {

  Blynk.run();
  timer.run();

}