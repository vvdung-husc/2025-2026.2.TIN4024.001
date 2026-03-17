#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6S4OtxtnF"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "4_SCgYPtPMbUFXDaW1Kw1R8FFLKH4u_T"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

#define OPENWEATHERMAP_KEY ""

String lat = "";
String lon = "";
String urlWeather = "";

unsigned long timerTemp = 0;
unsigned long timerUptime = 0;

void getLocation()
{
  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");

  int code = http.GET();

  if (code == 200)
  {
    String payload = http.getString();
    Serial.println(payload);

    int p1 = payload.lastIndexOf('|');
    int p2 = payload.substring(0, p1).lastIndexOf('|');

    lon = payload.substring(p2 + 1, p1);
    lat = payload.substring(p1 + 1);

    Serial.print("LAT: ");
    Serial.println(lat);
    Serial.print("LON: ");
    Serial.println(lon);

    urlWeather =
        "https://api.openweathermap.org/data/2.5/weather?lat=" +
        lat +
        "&lon=" +
        lon +
        "&appid=" +
        OPENWEATHERMAP_KEY +
        "&units=metric";

    Serial.println(urlWeather);
  }
  else
  {
    Serial.println("Lay location FAIL");
  }

  http.end();
}

void updateTemp()
{

  if (millis() - timerTemp < 10000)
    return;
  timerTemp = millis();

  if (WiFi.status() != WL_CONNECTED)
    return;
  if (urlWeather == "")
    return;

  HTTPClient http;
  http.begin(urlWeather);

  int code = http.GET();

  Serial.print("HTTP CODE: ");
  Serial.println(code);

  if (code == 200)
  {

    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {

      float temp = doc["main"]["temp"];

      Serial.print("Nhiet do: ");
      Serial.println(temp);

      Blynk.virtualWrite(V3, temp);
    }
    else
    {
      Serial.println("JSON FAIL");
    }
  }
  else
  {
    Serial.println("HTTP FAIL");
  }

  http.end();
}

void sendInfoOnce()
{
  static bool done = false;
  if (done)
    return;
  done = true;

  String link =
      "https://www.google.com/maps/place/" +
      lat +
      "," +
      lon;

  Blynk.virtualWrite(V1, WiFi.localIP().toString());
  Blynk.virtualWrite(V2, link);
}

void uptimeSend()
{

  if (millis() - timerUptime < 1000)
    return;
  timerUptime = millis();

  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" OK");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getLocation();
}

void loop()
{

  Blynk.run();

  sendInfoOnce();
  updateTemp();
  uptimeSend();
}