#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"

#define DHTPIN 4       
#define DHTTYPE DHT22  
#define SD_CS_PIN 5    
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

String serverName = "http://api.thingspeak.com/update?api_key=4W1U8K9V1Y5M3P1T";
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== TRAM GHI NHAT KY MOI TRUONG ===");
 
  Serial.println("[1] Dang khoi tao DHT22...");
  dht.begin();

  Serial.print("[2] Dang khoi tao the SD... ");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("LOI: Khong tim thay the SD!");
  } else {
    Serial.println("OK!");
  
    File dataFile = SD.open("/nhatky.csv", FILE_APPEND);
    if (dataFile) {
      if(dataFile.size() == 0) dataFile.println("Nhiet do (*C), Do am (%)");
      dataFile.close();
    }
  }

  Serial.print("[3] Dang ket noi Wi-Fi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n -> Ket noi Wi-Fi thanh cong!");
  Serial.println("-------------------------------------------");
}
void loop() {

  float do_am = dht.readHumidity();
  float nhiet_do = dht.readTemperature();
  if (isnan(do_am) || isnan(nhiet_do)) {
    Serial.println("LOI: Khong doc duoc DHT22!");
    delay(2000);
    return;
  }
  Serial.printf("Nhiet do: %.1f *C  |  Do am: %.1f %%\n", nhiet_do, do_am);
 
  File dataFile = SD.open("/nhatky.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%.2f,%.2f\n", nhiet_do, do_am);
    dataFile.close();
    Serial.println(" -> Luu the SD: Thanh cong");
  } else {
    Serial.println(" -> Luu the SD: Loi!");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverName + "&field1=" + String(nhiet_do) + "&field2=" + String(do_am);
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.printf(" -> ThingSpeak: Thanh cong (Ma: %d)\n", httpResponseCode);
    } else {
      Serial.printf(" -> ThingSpeak: Loi (Ma: %d)\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println(" -> ThingSpeak: Mat mang!");
  }

  Serial.println("-------------------------------------------");
  delay(15000);
}
