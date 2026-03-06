#include <Arduino.h>

// --- THAY THÔNG SỐ BLYNK CỦA BẠN VÀO ĐÂY ---
#define BLYNK_TEMPLATE_ID "TMPL6uNMmwyUq"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "nGC0dD_EYsPyLopJPgRn9FdqXXoZ8JJB" 

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// --- THAY API KEY THỜI TIẾT CỦA BẠN VÀO ĐÂY ---
#define OPENWEATHERMAP_KEY "API_KEY_CUA_TAINGUYEN" // Đã sửa lỗi dư dấu chấm phẩy

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;
String urlWeather;  
ulong currentMiliseconds = 0; 

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Định dạng chuỗi
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Phân tích chuỗi trả về từ iothings.vn
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;
  
  while (payload.length() > 0 && index < 7) {
      int delimiterIndex = payload.indexOf('|');
      if (delimiterIndex == -1) {
          values[index++] = payload;
          break;
      }
      values[index++] = payload.substring(0, delimiterIndex);
      payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5].c_str();
  ipInfo.latitude = values[6].c_str(); 
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

// API Get lấy IP và Tọa độ
void getAPI() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Lỗi WiFi kết nối"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);

    // Đã sửa lại định dạng link Google Maps cho chuẩn xác
    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n", ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    // Đã sửa lại lỗi cú pháp URL của OpenWeatherMap
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);
    Serial.printf("URL Thời tiết => %s \r\n", urlWeather.c_str());      
  } else {
    Serial.print("Lỗi HTTP GET: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; 
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin(urlWeather);
  int httpResponseCode = http.GET();
  
  if(httpResponseCode > 0) {
    String response = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      float temp = doc["main"]["temp"];
      if (temp_ != temp) {
        temp_ = temp;
        Serial.print("Nhiet do moi: "); Serial.println(temp); 
        Blynk.virtualWrite(V3, temp_);
      }
    }
  }
  http.end();
}

// Gửi IP và Link Maps lên Blynk
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
}

// Cập nhật Uptime
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Dang ket noi WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Thanh cong!");

  Blynk.config(BLYNK_AUTH_TOKEN); 
  Blynk.connect();                

  getAPI(); // Gọi API lấy vị trí ngay khi khởi động
}

void loop() {
  // Đã bỏ dòng "return;" để vòng lặp được phép chạy
  Blynk.run();  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}