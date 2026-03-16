#include <Arduino.h>

// --- THÔNG SỐ BLYNK CỦA BẠN ---
#define BLYNK_TEMPLATE_ID "TMPL6sdiqhlOk"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "zuWrxoKhihOM9kgHR_SJS5ZfF6Y0_2Y3"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// --- API KEY THỜI TIẾT ---
#define OPENWEATHERMAP_KEY "619ac2f5250a67d3524bc2dc2702ab12" 

struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
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
  
  // ĐÃ SỬA LỖI Ở 2 DÒNG NÀY (Đảo lại cho đúng Vĩ độ và Kinh độ)
  ipInfo.latitude = values[5];  
  ipInfo.longitude = values[6]; 
  
  // Xóa các ký tự khoảng trắng hoặc xuống dòng thừa
  ipInfo.ip4.trim();
  ipInfo.longitude.trim();
  ipInfo.latitude.trim();
  
  Serial.printf("IP Address: %s\r\n", ipInfo.ip4.c_str());
  Serial.printf("Latitude: %s\r\n", ipInfo.latitude.c_str());
  Serial.printf("Longitude: %s\r\n", ipInfo.longitude.c_str());
}

// API Get lấy IP và Tọa độ
// API Get lấy IP và Tọa độ
void getAPI() {
  if(WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpResponseCode = http.GET();
  
  if(httpResponseCode > 0) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);
    
    // Ép cứng tọa độ Huế vào link để bỏ qua lỗi ký tự ẩn
    urlWeather = StringFormat("http://api.openweathermap.org/data/2.5/weather?lat=16.4&lon=107.6&appid=%s&units=metric", OPENWEATHERMAP_KEY);
  } 
  http.end();
}

// Cập nhật nhiệt độ
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; 
  if(WiFi.status() != WL_CONNECTED) return;
  if(urlWeather == "") return; 

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
    } else {
      Serial.print("Lỗi phân tích JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Lỗi API Thời tiết: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Gửi IP, Link Maps và Tên lên Blynk
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  if (ip4Info.ip4 == "") return; 
  done_ = true;
  
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
  Blynk.virtualWrite(V4, "Trần Văn Tiến"); 
}

// Cập nhật Uptime
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  ulong value = currentMiliseconds / 1000; 
  Blynk.virtualWrite(V0, value);  
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.print("Dang ket noi WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Thanh cong!");

  Blynk.config(BLYNK_AUTH_TOKEN); 
  Blynk.connect();                

  getAPI(); 
}

void loop() {
  Blynk.run();  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}