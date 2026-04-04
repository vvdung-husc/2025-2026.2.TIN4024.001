#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== THINGSPEAK =====
String apiKey = "YOUR_API_KEY";
const char* server = "http://api.thingspeak.com/update";

// ===== DHT =====
#define DHTPIN 4
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_PIN 2

void connectWiFi() {
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  connectWiFi();
}

void loop() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT error!");
    delay(2000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" °C | Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  // ===== LED cảnh báo =====
  if (temp > 30) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // ===== Gửi ThingSpeak =====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(server) +
                 "?api_key=" + apiKey +
                 "&field1=" + String(temp) +
                 "&field2=" + String(hum);

    http.begin(url);
    int httpCode = http.GET();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    http.end();
  }

  delay(15000); // bắt buộc >=15s
}