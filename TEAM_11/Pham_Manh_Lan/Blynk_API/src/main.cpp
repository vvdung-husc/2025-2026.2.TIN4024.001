#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6OgZOLWBS"
#define BLYNK_TEMPLATE_NAME "Blynk API"
#define BLYNK_AUTH_TOKEN "TduV16pFbuAqCKgWjWD0MyMKzkXXpHZg"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   // Thư viện gọi API
#include <ArduinoJson.h>  // Thư viện xử lý JSON

// --- THÔNG SỐ WIFI VÀ THỜI TIẾT ---
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// LƯU Ý: Không để dấu chấm phẩy (;) ở cuối dòng #define này
#define OPENWEATHERMAP_KEY "key_api github khong cho phep day len!!!!" 

// --- CẤU TRÚC LƯU DỮ LIỆU ---
struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info; 
unsigned long currentMiliseconds = 0; 
String urlWeather;  

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm định dạng chuỗi giống sprintf
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

// Phân tích chuỗi trả về từ ip4.iothings.vn
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
  ipInfo.longtitude = values[5];
  ipInfo.latitude = values[6];
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Country: %s\r\n", values[2].c_str());
  Serial.printf("City: %s\r\n", values[4].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

// Lấy IP và Tọa độ, sau đó tạo link
void getAPI() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Lỗi: Chưa kết nối WiFi"); return;
  }
  
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("--- DỮ LIỆU VỊ TRÍ ---");
          
    parseGeoInfo(response, ip4Info);

    // [ĐÃ SỬA LỖI] Tạo link Google Maps chuẩn với %s
    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("Link Bản đồ: %s \r\n", urlGooleMaps.c_str());

    // [ĐÃ SỬA LỖI] Sửa lỗi đánh máy %ss thành %s ở phần appid
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);
    Serial.printf("Link Thời tiết: %s \r\n", urlWeather.c_str());      
  } else {
    Serial.printf("Lỗi HTTP GET (Vị trí): %d\n", httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ
void updateTemp() {
  static unsigned long lastTime = 0;
  static float temp_ = 0.0;

  // Cập nhật mỗi 10 giây
  if (!IsReady(lastTime, 10000)) return; 
  if (WiFi.status() != WL_CONNECTED) return;
  if (urlWeather == "") return; // Tránh gọi khi chưa có link

  HTTPClient http;   
  http.begin(urlWeather);
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0) {
    String response = http.getString();
          
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
      Serial.println("Lỗi: Phân tích JSON thất bại");
    } else {
      float temp = doc["main"]["temp"];
      
      if (temp_ != temp) {
        temp_ = temp;
        Serial.print("Nhiệt độ hiện tại: "); Serial.print(temp); Serial.println(" °C");
        Blynk.virtualWrite(V3, temp_);
      }
    }
  } else {
    Serial.printf("Lỗi HTTP GET (Thời tiết): %d\n", httpResponseCode);
  }
  http.end();
}

// Gửi IP và Link Maps lên Blynk 1 lần duy nhất
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  // [ĐÃ SỬA LỖI] Cập nhật lại format link Google Maps
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str()); 
  Blynk.virtualWrite(V2, link.c_str()); 
}

// Cập nhật thời gian hoạt động lên Blynk
void uptimeBlynk() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  
  unsigned long value = lastTime / 1000;
  Blynk.virtualWrite(V0, value); 
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  
  Serial.print("Đang kết nối WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối!");

  Blynk.config(BLYNK_AUTH_TOKEN); 
  Blynk.connect();                

  // Gọi API lấy vị trí ngay khi khởi động
  getAPI();
}

void loop(void) {
  // [ĐÃ SỬA LỖI] Xóa bỏ lệnh return; ở đây để vòng lặp có thể hoạt động
  Blynk.run();  
  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}