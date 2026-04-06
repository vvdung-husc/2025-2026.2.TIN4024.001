#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6Id1NZwVf"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK API"
#define BLYNK_AUTH_TOKEN "bDQsW-2_Jey7IEFaZi2YtsToOHYrCMjE"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>   //Thư viện gọi API
#include <ArduinoJson.h>  //Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

//Cấu trúc lưu thông tin IPv4, lat, long từ http://ip4.iothings.vn/?geo=1
struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info; //Biến lưu trữ cấu trúc nhận được từ GET http://ip4.iothings.vn/?geo=1

ulong currentMiliseconds = 0; //Thời gian hiện tại - miliseconds 

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//Định dạng chuỗi %s,%s,...
String StringFormat(const char* fmt, ...){
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

//Phân tích chuỗi trả về từ http://ip4.iothings.vn/?geo=1 và điền vào ipInfo
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
  
  // QUAN TRỌNG: Loại bỏ các ký tự xuống dòng (\r, \n) hoặc khoảng trắng thừa
  // Nếu không có hàm này, URL OpenWeather sẽ bị hỏng do chứa ký tự lạ
  ipInfo.ip4.trim();
  ipInfo.longtitude.trim();
  ipInfo.latitude.trim();
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Country Code: %s\r\n", values[1].c_str());
  Serial.printf("Country: %s\r\n", values[2].c_str());
  Serial.printf("Region: %s\r\n", values[3].c_str());
  Serial.printf("City: %s\r\n", values[4].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

//Key lấy từ openweathermap.org khi đăng ký tài khoản
#define OPENWEATHERMAP_KEY "afbfa4e49f141ff9590a0b9637e3a1b0" //Thay KEY của bạn vào đây
String urlWeather;  //Biến lưu url https://openweathermap.org/

//API Get http://ip4.iothings.vn/?geo=1
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");
  // Thêm User-Agent để API không chặn Wokwi
  http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    // ĐÃ SỬA: Thêm định dạng %s,%s vào URL và đổi sang chuẩn link Google Maps
    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n", ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    // Tạo URL Thời tiết
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);

    Serial.printf("URL Weather => %s \r\n", urlWeather.c_str());      
  }else{
    Serial.print("Error on sending GET iothings: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Cập nhật nhiêt độ từ urlWeather bằng API GET
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; //Kiểm tra và cập nhật sau mỗi 10 giây
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error in WiFi connection"); 
    return;
  }
  
  // Nếu urlWeather trống (chưa lấy được tọa độ), không gọi API
  if(urlWeather == "") return;

  HTTPClient http;   
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
          
    //Xử lý JSON trả về từ API
    JsonDocument doc; // Hỗ trợ trên ArduinoJson v7
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("Failed to parse JSON Weather");
    }
    else {
      float temp = doc["main"]["temp"];//lấy thông tin nhiệt độ
      
      if (temp_ != temp){// có thay đổi mới cập nhật lên Blynk
        temp_ = temp;
        Serial.print("Nhiet do: "); Serial.println(temp); 
        Blynk.virtualWrite(V3, temp_);
      }
    }
  }else{
    Serial.print("Error on sending GET Weather: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Chỉ gọi 1 lần để cập nhật IPv4, Link GoogleMaps của Latitude, Longtitude
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  // Đảm bảo ip4Info đã có dữ liệu trước khi đẩy lên Blynk
  if (ip4Info.ip4 == "") return; 
  
  done_ = true;
  // ĐÃ SỬA: Thêm định dạng %s,%s vào URL
  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  //Gửi giá trị lên chân ảo V1
  Blynk.virtualWrite(V2, link.c_str());  //Gửi giá trị lên chân ảo V2
}

//Cập nhật uptime lên Blynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật sau mỗi 1 giây
  ulong value = currentMiliseconds / 1000; // Đổi lastTime thành currentMiliseconds để số giây tăng dần chính xác
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN); // Cấu hình Blynk với mã token
  Blynk.connect();                // Kết nối Blynk

  getAPI();
}

void loop(void) {
  Blynk.run();  // Chạy vòng lặp Blynk
  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}