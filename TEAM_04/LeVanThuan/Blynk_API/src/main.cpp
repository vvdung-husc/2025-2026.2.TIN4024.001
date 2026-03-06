#define BLYNK_TEMPLATE_ID "TMPL6oOVHVFHX"
#define BLYNK_TEMPLATE_NAME "Blynk API"
#define BLYNK_AUTH_TOKEN "WozTCNuGPA0sz2nY95_1KnS3SgSpko-Q"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

String API_KEY = "d612ee61f70b2d92c09e14763ad66228";

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);
BlynkTimer timer;

String ipAddress;
String latitude;
String longitude;
String googleLink;

long uptime = 0;

void getIPLocation() {

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

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

    ipAddress = data[0];
    longitude = data[5];
    latitude = data[6];

    googleLink =
      "https://www.google.com/maps/place/" +
      latitude + "," + longitude;

    Serial.println("IP: " + ipAddress);
    Serial.println("Lat: " + latitude);
    Serial.println("Lon: " + longitude);
    Serial.println("Map: " + googleLink);

    Blynk.virtualWrite(V1, ipAddress);
    Blynk.virtualWrite(V2, googleLink);
  }

  http.end();
}

void getWeather() {

  HTTPClient http;

  String url =
    "https://api.openweathermap.org/data/2.5/weather?lat=" +
    latitude +
    "&lon=" +
    longitude +
    "&appid=" +
    API_KEY +
    "&units=metric";

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temp: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3, temp);
  }

  http.end();
}

void updateUptime() {

  uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);

  display.showNumberDec(uptime);
}
void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  display.setBrightness(7);

  getIPLocation();

  timer.setInterval(1000L, updateUptime);
  timer.setInterval(10000L, getWeather);
}

void loop() {

  Blynk.run();
  timer.run();
}