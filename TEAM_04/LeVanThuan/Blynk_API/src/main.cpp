#define BLYNK_TEMPLATE_ID "TMPL6oOVHVFHX"
#define BLYNK_TEMPLATE_NAME "Blynk API"
#define BLYNK_AUTH_TOKEN "1dhkBWBqksbH0X4XD2HabeJJMmPe3VzL"

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
    Serial.println("Dữ liệu vị trí: " + payload);

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

    googleLink = "https://www.google.com/maps/place/" + latitude + "," + longitude;

    Serial.println("IP: " + ipAddress);
    Serial.println("Lat: " + latitude);
    Serial.println("Lon: " + longitude);
    Serial.println("Map: " + googleLink);

    if (Blynk.connected()) {
      Blynk.virtualWrite(V1, ipAddress);
      Blynk.virtualWrite(V2, googleLink);
    }

  } else {

    Serial.print("Lỗi lấy IP/Location: ");
    Serial.println(httpCode);
  }

  http.end();
}

void getWeather() {

  if (latitude == "" || longitude == "") return;

  HTTPClient http;

  String url =
  "http://api.openweathermap.org/data/2.5/weather?lat=" +
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

    Serial.print("Nhiệt độ hiện tại: ");
    Serial.println(temp);

    if (Blynk.connected()) {
      Blynk.virtualWrite(V3, temp);
    }

  } else {

    Serial.print("Lỗi API thời tiết: ");
    Serial.println(httpCode);
  }

  http.end();
}

void updateUptime() {

  uptime = millis() / 1000;

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, uptime);
  }

  int minutes = (uptime / 60) % 100;
  int seconds = uptime % 60;

  int displayTime = (minutes * 100) + seconds;

  bool blinkColon = (uptime % 2 == 0);

  display.showNumberDecEx(displayTime, blinkColon ? 0x40 : 0, true);
}

void setup() {

  Serial.begin(115200);

  display.setBrightness(7);

  Serial.print("Đang kết nối WiFi");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Serial.print("Đang kết nối Blynk...");

  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);

  bool b = Blynk.connect(5000);

  Serial.print("OK?: ");
  Serial.println(b);

  getIPLocation();
  getWeather();

  timer.setInterval(1000L, updateUptime);
  timer.setInterval(60000L, getWeather);
}

void loop() {

  if (Blynk.connected()) {
    Blynk.run();
  }

  timer.run();
}