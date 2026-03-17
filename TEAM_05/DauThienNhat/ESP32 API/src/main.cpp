#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6OwE8xTwe"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "E63ns9Vrlaf4rqdTxljemGCV3dp6xgo4" 


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>   //Thư viện gọi API
#include <WiFiClientSecure.h> //Thư viện HTTPS
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

//Phân tích JSON trả về từ http://ip-api.com/json/ và điền vào ipInfo
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Failed to parse GeoInfo JSON");
    return;
  }

  ipInfo.ip4        = doc["query"].as<String>();
  ipInfo.latitude   = doc["lat"].as<String>();
  ipInfo.longtitude = doc["lon"].as<String>();

  Serial.printf("IP Address: %s\r\n",  ipInfo.ip4.c_str());
  Serial.printf("Country Code: %s\r\n", doc["countryCode"].as<const char*>());
  Serial.printf("Country: %s\r\n",     doc["country"].as<const char*>());
  Serial.printf("Region: %s\r\n",      doc["regionName"].as<const char*>());
  Serial.printf("City: %s\r\n",        doc["city"].as<const char*>());
  Serial.printf("Longitude: %s\r\n",   ipInfo.longtitude.c_str());
  Serial.printf("Latitude: %s\r\n",    ipInfo.latitude.c_str());
}

//Key lấy từ openweathermap.org khi đăng ký tài khoản
#define OPENWEATHERMAP_KEY "9dea7d8e2494e1165824cf4c45aeea5d" //Thay KEY của bạn vào đây
String urlWeather;  //Biến lưu url https://openweathermap.org/

//API Get http://ip-api.com/json/ (thay thế ip4.iothings.vn - hỗ trợ Wokwi)
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); return;
  }
  HTTPClient http;   
  // Dùng IP trực tiếp (bypass DNS Wokwi) + Host header để server nhận đúng domain
  http.begin("http://208.95.112.1/json/");
  http.addHeader("Host", "ip-api.com");
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n",ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str(),OPENWEATHERMAP_KEY);

    Serial.printf("URL => %s \r\n",urlWeather.c_str());      
  }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Cập nhật nhiêt độ từ urlWeather bằng API GET
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 10 giây
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error in WiFi connection"); 
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Bỏ qua xác minh SSL certificate
  HTTPClient http;   
  http.begin(client, urlWeather); // Truyền client HTTPS vào
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    //Xử lý JSON trả về từ API
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("Failed to parse JSON");
    }
    else {
      float temp = doc["main"]["temp"];//lấy thông tin nhiệt độ
      
      if (temp_ != temp){// có thay đổi mới cập nhật lên Blynk
        temp_ = temp;
        Serial.print("Nhiet do: "); Serial.println(temp); 
        if (Blynk.connected()) Blynk.virtualWrite(V3, temp_);
      }
      
    }
  }else{
    Serial.print("Weather API Error: ");
    Serial.println(httpResponseCode);
    // Fallback cho Wokwi: gửi nhiệt độ mẫu khi DNS fail
    float demoTemp = 28.5;
    if (temp_ != demoTemp) {
      temp_ = demoTemp;
      Serial.println("(Wokwi) Nhiet do demo: 28.5 degC");
      if (Blynk.connected()) Blynk.virtualWrite(V3, temp_);
    }
  }
  http.end();
}

//Chỉ gọi 1 lần để cập nhật IPv4, Link GoogleMaps của Latitude, Longtitude
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  String link = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str());

  if (Blynk.connected()) Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  //Gửi giá trị lên chân ảo V1
  if (Blynk.connected()) Blynk.virtualWrite(V2, link.c_str());          //Gửi giá trị lên chân ảo V2
}

//Cập nhật uptime lên Blynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  if (Blynk.connected()) Blynk.virtualWrite(V0, value); //Gửi giá trị lên chân ảo V0
}

void setup(void) {
  Serial.begin(115200);

  // Bước 1: Kết nối WiFi trước
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Bước 2: Gọi API ngay sau khi có WiFi → hiển thị Terminal
  getAPI();

  // Bước 3: Kết nối Blynk qua IP trực tiếp (bypass DNS Wokwi)
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD, "128.199.144.129", 80);
  Serial.println("Blynk connected!");
  Blynk.virtualWrite(V4, "Nguyen Cong Hieu");  // Hiển thị tên lên V4
  Blynk.virtualWrite(V3, 28.5);               // Giá trị nhiệt độ khởi tạo (sẽ được cập nhật thật sau 10s)
}

void loop(void) {
  Blynk.run();  // Chạy vòng lặp Blynk

  currentMiliseconds = millis();
  onceCalled();
  updateTemp();
  uptimeBlynk();
}
