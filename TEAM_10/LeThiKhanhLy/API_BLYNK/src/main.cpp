#define BLYNK_TEMPLATE_ID "TMPL6PdUm-2nl"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "-e2rofrvgLj6WHL82Y6XWkSh7kfTl9mp"
#define BLYNK_HOST "blynk.cloud"
#define BLYNK_PORT 80

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

String ip;
String lat;
String lon;
float temperature;

unsigned long startTime;

BlynkTimer timer;

void getIPLocation()
{
  HTTPClient http;

  http.begin("http://ip4.ithings.vn/?geo=1");

  int httpCode = http.GET();

  if(httpCode > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    ip = doc["ip"].as<String>();
    lat = doc["latitude"].as<String>();
    lon = doc["longitude"].as<String>();

    String mapLink = "https://www.google.com/maps/place/" + lat + "," + lon;

    Serial.println("IPv4: " + ip);
    Serial.println("Google Maps: " + mapLink);

    Blynk.virtualWrite(V1, ip);
    Blynk.virtualWrite(V2, mapLink);
  }

  http.end();
}

void getWeather()
{
  HTTPClient http;

  String url = "https://api.openweathermap.org/data/2.5/weather?lat=" 
  + lat + "&lon=" + lon + "&appid=YOUR_API_KEY&units=metric";

  http.begin(url);

  int httpCode = http.GET();

  if(httpCode > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    temperature = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temperature);

    Blynk.virtualWrite(V3, temperature);
  }

  http.end();
}

void sendUptime()
{
  int uptime = (millis() - startTime) / 1000;

  Blynk.virtualWrite(V0, uptime);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

WiFi.begin("Wokwi-GUEST", "");

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
}

Blynk.config(BLYNK_AUTH_TOKEN);
Blynk.connect();

  startTime = millis();

  timer.setInterval(10000L, getIPLocation);
  timer.setInterval(15000L, getWeather);
  timer.setInterval(1000L, sendUptime);
}

void loop()
{
  Blynk.run();
  timer.run();
}
