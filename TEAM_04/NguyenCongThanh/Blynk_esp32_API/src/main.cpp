// #define BLYNK_TEMPLATE_ID "TMPL6mBDBnCjs"
// #define BLYNK_TEMPLATE_NAME "blynk API"
// #define BLYNK_AUTH_TOKEN "DEIxFsy44gtFcRygTMRaFMiwV6tblSt1"

// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include <BlynkSimpleEsp32.h>
// #include <DHT.h>
// #include <TM1637Display.h>

// // WIFI
// char ssid[] = "Wokwi-GUEST";
// char pass[] = "";

// // DHT22
// #define DHTPIN 16
// #define DHTTYPE DHT22
// DHT dht(DHTPIN, DHTTYPE);

// // TM1637
// #define CLK 18
// #define DIO 19
// TM1637Display display(CLK, DIO);

// // biến toàn cục
// String ip;
// float lat;
// float lon;
// float weatherTemp;

// BlynkTimer timer;

// // prototype function
// void sendData();
// void getLocation();
// void getWeather();

// void setup()
// {
//   Serial.begin(115200);

//   dht.begin();

//   display.setBrightness(7);

//   Serial.println("Connecting WiFi...");

//   WiFi.begin(ssid, pass);

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("\nWiFi Connected");

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

//   // chạy mỗi 5 giây
//   timer.setInterval(5000L, sendData);
// }

// void loop()
// {
//   Blynk.run();
//   timer.run();
// }

// void sendData()
// {
//   // lấy IP + location
//   getLocation();

//   // lấy nhiệt độ thời tiết
//   getWeather();

//   // đọc nhiệt độ DHT
//   float localTemp = dht.readTemperature();

//   if (!isnan(localTemp))
//   {
//     display.showNumberDec((int)localTemp);
//   }

//   // uptime
//   Blynk.virtualWrite(V0, millis() / 1000);

//   // IPv4
//   Blynk.virtualWrite(V1, ip);

//   // google maps link
//   String mapLink =
//       "https://www.google.com/maps/place/" +
//       String(lat, 6) + "," +
//       String(lon, 6);

//   Blynk.virtualWrite(V2, mapLink);

//   // nhiệt độ từ OpenWeather
//   Blynk.virtualWrite(V3, weatherTemp);

//   Serial.println("---- DATA SENT ----");
// }

// void getLocation()
// {
//   HTTPClient http;

//   http.begin("http://ip4.iothings.vn?geo=1");

//   int httpCode = http.GET();

//   if (httpCode > 0)
//   {
//     String payload = http.getString();

//     Serial.println("Location API:");
//     Serial.println(payload);

//     // format
//     // ip|country|countryName|region|city|lon|lat

//     int p1 = payload.indexOf('|');
//     int p2 = payload.indexOf('|', p1 + 1);
//     int p3 = payload.indexOf('|', p2 + 1);
//     int p4 = payload.indexOf('|', p3 + 1);
//     int p5 = payload.indexOf('|', p4 + 1);
//     int p6 = payload.indexOf('|', p5 + 1);

//     ip = payload.substring(0, p1);

//     lon = payload.substring(p5 + 1, p6).toFloat();
//     lat = payload.substring(p6 + 1).toFloat();

//     Serial.print("IP: ");
//     Serial.println(ip);

//     Serial.print("Latitude: ");
//     Serial.println(lat);

//     Serial.print("Longitude: ");
//     Serial.println(lon);
//   }

//   http.end();
// }

// void getWeather()
// {
//   HTTPClient http;

//   String url =
//       "https://api.openweathermap.org/data/2.5/weather?lat=" +
//       String(lat, 6) +
//       "&lon=" +
//       String(lon, 6) +
//       "&appid=152fabd9fda5f1766d54e461cd9e590f&units=metric";

//   http.begin(url);

//   int httpCode = http.GET();

//   if (httpCode > 0)
//   {
//     String payload = http.getString();

//     Serial.println("Weather API:");
//     Serial.println(payload);

//     DynamicJsonDocument doc(2048);
//     deserializeJson(doc, payload);

//     weatherTemp = doc["main"]["temp"];

//     Serial.print("Temperature: ");
//     Serial.println(weatherTemp);
//   }

//   http.end();
// }


#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// === Thông tin kết nối Blynk (PHẢI ĐẶT TRƯỚC include) ===
#define BLYNK_TEMPLATE_ID "TMPL6mBDBnCjs"
#define BLYNK_TEMPLATE_NAME "blynk API"
#define BLYNK_AUTH_TOKEN "DEIxFsy44gtFcRygTMRaFMiwV6tblSt1"

// === Thông tin WiFi ===
const char* ssid = "WIFI_SSID";         // Thay bằng SSID WiFi của bạn
const char* password = "WIFI_PASSWORD"; // Thay bằng mật khẩu WiFi

// === API Keys ===
String weatherApiKey = "152fabd9fda5f1766d54e461cd9e590f"; // OpenWeatherMap
const char* geoApiUrl = "http://ip4.iotthings.vn?geo=1";   // API lấy IP + tọa độ

// === Biến toàn cục ===
String ipv4 = "";
float lat = 0.0, lon = 0.0;
String googleMapsLink = "";
float temperature = 0.0;
int humidity = 0;
String weatherDesc = "";

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000; // 60 giây
unsigned long uptimeSeconds = 0;

// Khai báo hàm (prototype) để tránh lỗi "not declared in this scope"
void layDuLieuVaCapNhat();

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi đã kết nối");
  Serial.print("Địa chỉ IP cục bộ: ");
  Serial.println(WiFi.localIP());

  // Kết nối Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("✅ Đã kết nối Blynk");
}

void loop() {
  Blynk.run();
  
  uptimeSeconds = millis() / 1000; // Tính uptime (giây)
  
  // Cập nhật dữ liệu từ API mỗi 60 giây
  if (millis() - lastUpdate > updateInterval) {
    layDuLieuVaCapNhat();
    lastUpdate = millis();
  }
  
  // Gửi uptime lên V0 mỗi 10 giây (tránh spam Blynk)
  static unsigned long lastUptimeSend = 0;
  if (millis() - lastUptimeSend > 10000) {
    Blynk.virtualWrite(V0, uptimeSeconds);
    lastUptimeSend = millis();
  }
}

void layDuLieuVaCapNhat() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ Mất kết nối WiFi");
    return;
  }

  HTTPClient http;
  
  // --- 1. Gọi API lấy IP và tọa độ ---
  http.begin(geoApiUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("📡 Geo API Response: " + payload);

    // Dùng StaticJsonDocument thay cho DynamicJsonDocument (tránh cảnh báo deprecated)
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      ipv4 = doc["ip"].as<String>();
      lat = doc["lat"].as<float>();
      lon = doc["lon"].as<float>();

      googleMapsLink = "http://www.google.com/maps/place/" + String(lat, 6) + "," + String(lon, 6);

      Serial.println("🌐 IPv4: " + ipv4);
      Serial.println("📍 Lat: " + String(lat, 6));
      Serial.println("📍 Lon: " + String(lon, 6));
      Serial.println("🔗 Google Maps: " + googleMapsLink);

      Blynk.virtualWrite(V1, ipv4);
      Blynk.virtualWrite(V2, googleMapsLink);
    } else {
      Serial.println("❌ Lỗi parse JSON geo: " + String(error.c_str()));
      // Fallback dữ liệu mẫu
      ipv4 = "113.161.8.219";
      lat = 16.466669;
      lon = 107.599998;
      googleMapsLink = "http://www.google.com/maps/place/16.466669,107.599998";
      Blynk.virtualWrite(V1, ipv4);
      Blynk.virtualWrite(V2, googleMapsLink);
    }
  } else {
    Serial.println("❌ Lỗi HTTP geo, code: " + String(httpCode));
    // Fallback dữ liệu mẫu
    ipv4 = "113.161.8.219";
    lat = 16.466669;
    lon = 107.599998;
    googleMapsLink = "http://www.google.com/maps/place/16.466669,107.599998";
    Blynk.virtualWrite(V1, ipv4);
    Blynk.virtualWrite(V2, googleMapsLink);
  }
  http.end();

  // --- 2. Gọi OpenWeatherMap nếu có tọa độ ---
  if (lat != 0.0 && lon != 0.0) {
    String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" +
                        String(lat, 6) + "&lon=" + String(lon, 6) +
                        "&appid=" + weatherApiKey + "&units=metric";

    http.begin(weatherUrl);
    httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("🌦 Weather API Response: " + payload);

      StaticJsonDocument<2048> doc; // Đủ cho response thời tiết
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        temperature = doc["main"]["temp"].as<float>();
        humidity = doc["main"]["humidity"].as<int>();
        weatherDesc = doc["weather"][0]["description"].as<String>();

        Serial.println("🌡 Nhiệt độ: " + String(temperature) + " °C");
        Serial.println("💧 Độ ẩm: " + String(humidity) + " %");
        Serial.println("☁️ Mô tả: " + weatherDesc);

        Blynk.virtualWrite(V3, temperature);
        // Có thể ghi thêm V4, V5 nếu muốn hiển thị độ ẩm và mô tả
      } else {
        Serial.println("❌ Lỗi parse JSON weather: " + String(error.c_str()));
      }
    } else {
      Serial.println("❌ Lỗi HTTP weather, code: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("⚠️ Không có tọa độ, bỏ qua lấy thời tiết.");
  }
}