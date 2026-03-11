#include <Arduino.h>

//Thay thông số BLYNK của bạn vào đây
#define BLYNK_TEMPLATE_ID "TMPL6LKpYf786"
#define BLYNK_TEMPLATE_NAME "API"
#define BLYNK_AUTH_TOKEN "VAKrBiknNPC6M6CJ2T_LBhC4NAqAAdQA"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

ulong currentMiliseconds = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

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
  ipInfo.latitude = values[6].c_str();
  ipInfo.longtitude = values[5].c_str();
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Country Code: %s\r\n", values[1].c_str());
  Serial.printf("Country: %s\r\n", values[2].c_str());
  Serial.printf("Region: %s\r\n", values[3].c_str());
  Serial.printf("City: %s\r\n", values[4].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

#define OPENWEATHERMAP_KEY "520fa16f7f8e412a014913dc8921b651"
String urlWeather;

void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); 
    return;
  }

  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();

  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

    Serial.printf("IPv4 => %s \r\n",ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    urlWeather = StringFormat(
    "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str(),
    OPENWEATHERMAP_KEY);

    Serial.printf("URL => %s \r\n",urlWeather.c_str());      

  }else{
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; // cập nhật mỗi 10 giây

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error in WiFi connection"); 
    return;
  }

  HTTPClient http;   
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();

  if(httpResponseCode>0){

    String response = http.getString();

    Serial.println(httpResponseCode);
    Serial.println(response);

    // SỬA Ở ĐÂY
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, response);

    if (!error) {

      float temp = doc["main"]["temp"];

      if (temp_ != temp){

        temp_ = temp;

        Serial.print("Nhiet do: ");
        Serial.println(temp);

        Blynk.virtualWrite(V3, temp_);
      }

    }
    else{
      Serial.println("Failed to parse JSON");
    }

  }else{
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void onceCalled(){
  static bool done_ = false;

  if (done_) return;

  done_ = true;

  String link = StringFormat("https://www.google.com/maps/place/%s,%s",
  ip4Info.latitude.c_str(),ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());
  Blynk.virtualWrite(V2, link.c_str());
}

void uptimeBlynk(){

  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;

  Blynk.virtualWrite(V0, value);
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

  Blynk.config(BLYNK_AUTH_TOKEN);

  Blynk.connect();

  getAPI();
}

void loop(void) {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();

  updateTemp();

  uptimeBlynk();
}