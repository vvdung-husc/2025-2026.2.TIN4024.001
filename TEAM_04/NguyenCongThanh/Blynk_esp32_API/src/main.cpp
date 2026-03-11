// #define BLYNK_TEMPLATE_ID "TMPL6mBDBnCjs"
// #define BLYNK_TEMPLATE_NAME "blynk API"
// #define BLYNK_AUTH_TOKEN "DEIxFsy44gtFcRygTMRaFMiwV6tblSt1"

// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include <TM1637Display.h>

// char ssid[] = "Wokwi-GUEST";
// char pass[] = "";

// const char* API_KEY = "152fabd9fda5f1766d54e461cd9e590f";

// #define CLK 18
// #define DIO 19

// TM1637Display display(CLK, DIO);
// BlynkTimer timer;

// String ipAddress;
// String latitude;
// String longitude;
// String googleLink;

// unsigned long uptime = 0;

// void getIPLocation();
// void getWeather();
// void updateUptime();

// void connectWiFi()
// {
//   Serial.print("Connecting WiFi");

//   WiFi.begin(ssid, pass);

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("\nWiFi Connected");
//   Serial.println(WiFi.localIP());
// }

// void getIPLocation()
// {
//   if (WiFi.status() != WL_CONNECTED) return;

//   HTTPClient http;
//   http.begin("http://ip4.iothings.vn/?geo=1");

//   int httpCode = http.GET();

//   if (httpCode == HTTP_CODE_OK)
//   {
//     String payload = http.getString();

//     Serial.println("Location API:");
//     Serial.println(payload);

//     String data[7];
//     int index = 0;

//     while (payload.length() > 0 && index < 7)
//     {
//       int pos = payload.indexOf('|');

//       if (pos == -1)
//       {
//         data[index++] = payload;
//         break;
//       }

//       data[index++] = payload.substring(0, pos);
//       payload.remove(0, pos + 1);
//     }

//     ipAddress = data[0];
//     longitude = data[5];
//     latitude = data[6];

//     googleLink =
//       "https://www.google.com/maps/place/" +
//       latitude + "," + longitude;

//     Serial.println("IP: " + ipAddress);
//     Serial.println("Lat: " + latitude);
//     Serial.println("Lon: " + longitude);

//     Blynk.virtualWrite(V1, ipAddress);
//     Blynk.virtualWrite(V2, googleLink);
//   }
//   else
//   {
//     Serial.print("Location HTTP Error: ");
//     Serial.println(httpCode);
//   }

//   http.end();
// }

// void getWeather()
// {
//   if (WiFi.status() != WL_CONNECTED) return;

//   HTTPClient http;

//   String url =
//     "https://api.openweathermap.org/data/2.5/weather?lat=" +
//     latitude +
//     "&lon=" +
//     longitude +
//     "&appid=" +
//     API_KEY +
//     "&units=metric";

//   http.begin(url);

//   int httpCode = http.GET();

//   if (httpCode == HTTP_CODE_OK)
//   {
//     String payload = http.getString();

//     DynamicJsonDocument doc(1024);

//     DeserializationError error = deserializeJson(doc, payload);

//     if (!error)
//     {
//       float temp = doc["main"]["temp"];

//       Serial.print("Temperature: ");
//       Serial.println(temp);

//       Blynk.virtualWrite(V3, temp);
//     }
//     else
//     {
//       Serial.println("JSON parse error");
//     }
//   }
//   else
//   {
//     Serial.print("Weather HTTP Error: ");
//     Serial.println(httpCode);
//   }

//   http.end();
// }

// void updateUptime()
// {
//   uptime = millis() / 1000;

//   Blynk.virtualWrite(V0, uptime);

//   display.showNumberDec(uptime);
// }

// void setup()
// {
//   Serial.begin(115200);

//   display.setBrightness(7);

//   connectWiFi();

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

//   getIPLocation();

//   timer.setInterval(1000L, updateUptime);
//   timer.setInterval(10000L, getWeather);
// }

// void loop()
// {
//   Blynk.run();
//   timer.run();
// }

#define BLYNK_TEMPLATE_ID   "TMPL6mBDBnCjs"
#define BLYNK_TEMPLATE_NAME "blynk API"
#define BLYNK_AUTH_TOKEN    "DEIxFsy44gtFcRygTMRaFMiwV6tblSt1"
#define BLYNK_PRINT         Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TM1637Display.h>
#include <DHT.h>

#define DHTTYPE DHT22

// ─── Pin Definition ───────────────────────────────────────────────
#define CLK_PIN   18
#define DIO_PIN   19
#define DHT_PIN   16

// ─── WiFi Credentials ─────────────────────────────────────────────
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ─── API Keys ─────────────────────────────────────────────────────
const char* OWM_API_KEY  = "152fabd9fda5f1766d54e461cd9e590f";
const char* GEO_API_URL  = "http://ip4.iothings.vn/?geo=1";

// ─── Intervals (ms) ───────────────────────────────────────────────
const unsigned long UPTIME_INTERVAL  = 1000UL;
const unsigned long WEATHER_INTERVAL = 30000UL;   // tăng lên 30s để tránh rate-limit
const unsigned long DHT_INTERVAL     = 5000UL;

// ─── Objects ──────────────────────────────────────────────────────
TM1637Display display(CLK_PIN, DIO_PIN);
DHT           dht(DHT_PIN, DHTTYPE);
BlynkTimer    timer;

// ─── State ────────────────────────────────────────────────────────
struct GeoInfo {
  String ip;
  String lat;
  String lon;
  String mapsLink;
  bool   valid = false;
} geo;

unsigned long uptimeSec = 0;

// ══════════════════════════════════════════════════════════════════
//  WiFi
// ══════════════════════════════════════════════════════════════════
void connectWiFi()
{
  Serial.print("[WiFi] Connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - start > 15000UL)
    {
      Serial.println("\n[WiFi] Timeout – restarting");
      ESP.restart();
    }
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\n[WiFi] Connected  IP: %s\n", WiFi.localIP().toString().c_str());
}

// ══════════════════════════════════════════════════════════════════
//  HTTP helper – trả về body hoặc "" nếu lỗi
// ══════════════════════════════════════════════════════════════════
String httpGET(const String& url)
{
  if (WiFi.status() != WL_CONNECTED) return "";

  HTTPClient http;
  http.setTimeout(8000);
  http.begin(url);

  int code = http.GET();
  String body = "";

  if (code == HTTP_CODE_OK)
    body = http.getString();
  else
    Serial.printf("[HTTP] GET %s  code=%d\n", url.c_str(), code);

  http.end();
  return body;
}

// ══════════════════════════════════════════════════════════════════
//  Lấy IP + Geolocation
// ══════════════════════════════════════════════════════════════════
void getIPLocation()
{
  String raw = httpGET(GEO_API_URL);
  if (raw.isEmpty()) return;

  Serial.printf("[GEO] Raw: %s\n", raw.c_str());

  // Format: IP|CC|Country|Region|City|Lon|Lat
  String fields[7];
  int    idx = 0;
  String tmp = raw;

  while (tmp.length() && idx < 7)
  {
    int p = tmp.indexOf('|');
    if (p == -1) { fields[idx++] = tmp; break; }
    fields[idx++] = tmp.substring(0, p);
    tmp.remove(0, p + 1);
  }

  geo.ip       = fields[0];
  geo.lon      = fields[5];
  geo.lat      = fields[6];
  geo.mapsLink = "https://www.google.com/maps/place/" + geo.lat + "," + geo.lon;
  geo.valid    = true;

  Serial.printf("[GEO] IP=%s  Lat=%s  Lon=%s\n",
                geo.ip.c_str(), geo.lat.c_str(), geo.lon.c_str());
  Serial.printf("[GEO] Maps: %s\n", geo.mapsLink.c_str());

  Blynk.virtualWrite(V1, geo.ip);
  Blynk.virtualWrite(V2, geo.mapsLink);
}

// ══════════════════════════════════════════════════════════════════
//  Lấy nhiệt độ từ OpenWeatherMap
// ══════════════════════════════════════════════════════════════════
void getWeather()
{
  if (!geo.valid)
  {
    getIPLocation();   // thử lấy lại nếu lần đầu thất bại
    if (!geo.valid) return;
  }

  String url = "https://api.openweathermap.org/data/2.5/weather"
               "?lat=" + geo.lat +
               "&lon=" + geo.lon +
               "&appid=" + OWM_API_KEY +
               "&units=metric";

  String body = httpGET(url);
  if (body.isEmpty()) return;

  // Dùng filter để chỉ parse field cần thiết → tiết kiệm RAM
  StaticJsonDocument<64> filter;
  filter["main"]["temp"] = true;

  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, body, DeserializationOption::Filter(filter));

  if (err)
  {
    Serial.printf("[OWM] JSON error: %s\n", err.c_str());
    return;
  }

  float temp = doc["main"]["temp"];
  Serial.printf("[OWM] Temp: %.1f °C\n", temp);
  Blynk.virtualWrite(V3, temp);
}

// ══════════════════════════════════════════════════════════════════
//  Đọc DHT22 (cảm biến gắn trực tiếp trên ESP32)
// ══════════════════════════════════════════════════════════════════
void readDHT()
{
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("[DHT] Read failed!");
    return;
  }

  Serial.printf("[DHT] Temp=%.1f°C  Hum=%.1f%%\n", temp, hum);

  // Nếu muốn ghi lên Blynk thêm V4/V5 thì uncomment:
  // Blynk.virtualWrite(V4, temp);
  // Blynk.virtualWrite(V5, hum);
}

// ══════════════════════════════════════════════════════════════════
//  Cập nhật uptime + màn hình LED
// ══════════════════════════════════════════════════════════════════
void updateUptime()
{
  uptimeSec = millis() / 1000UL;

  // TM1637 chỉ hiển thị 4 chữ số → mod 10000 để tránh tràn
  display.showNumberDec(uptimeSec % 10000, false);

  Blynk.virtualWrite(V0, uptimeSec);
}

// ══════════════════════════════════════════════════════════════════
//  Setup
// ══════════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  Serial.println("\n[SYS] ESP32 Blynk API Demo – starting…");

  // Màn hình LED
  display.setBrightness(7);
  display.showNumberDec(0);

  // Cảm biến DHT22
  dht.begin();

  // Kết nối WiFi
  connectWiFi();

  // Khởi động Blynk (non-blocking)
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);

  // Lấy dữ liệu lần đầu
  getIPLocation();
  getWeather();

  // Đăng ký timer
  timer.setInterval(UPTIME_INTERVAL,  updateUptime);
  timer.setInterval(WEATHER_INTERVAL, getWeather);
  timer.setInterval(DHT_INTERVAL,     readDHT);
}

// ══════════════════════════════════════════════════════════════════
//  Loop
// ══════════════════════════════════════════════════════════════════
void loop()
{
  Blynk.run();
  timer.run();
}
