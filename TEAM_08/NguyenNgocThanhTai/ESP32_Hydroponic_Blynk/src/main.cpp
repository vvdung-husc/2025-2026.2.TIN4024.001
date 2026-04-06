#define BLYNK_TEMPLATE_ID "TMPL62OwFyyL0"
#define BLYNK_TEMPLATE_NAME "Hydroponic ESP32"
#define BLYNK_AUTH_TOKEN "YourAuthTokenHere"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

#define NTC_PIN 32   
#define LDR_PIN 34   
#define PH_PIN 35    

#define RELAY_FAN 18 
#define RELAY_PUMP 19
#define RELAY_LAMP 21

BlynkTimer timer;

void sendSensorData() {
  int rawTemp = analogRead(NTC_PIN);
  int rawLight = analogRead(LDR_PIN);
  int rawPH = analogRead(PH_PIN);

  float temp = rawTemp * (50.0 / 4095.0); 
  int light = rawLight * (100.0 / 4095.0); 
  float ph = rawPH * (14.0 / 4095.0);     

  int fanState = (temp > 28.0) ? 1 : 0;
  int lampState = (light < 30) ? 1 : 0;
  int pumpState = (ph < 6.0) ? 1 : 0;

  digitalWrite(RELAY_FAN, fanState);
  digitalWrite(RELAY_LAMP, lampState);
  digitalWrite(RELAY_PUMP, pumpState);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V2, light);
    Blynk.virtualWrite(V3, ph);
    Blynk.virtualWrite(V4, fanState);   
    Blynk.virtualWrite(V0, lampState);  
    Blynk.virtualWrite(V5, pumpState); 
  }
  
  Serial.printf("Nhiet do: %.1f C | Anh sang: %d %% | pH: %.1f\n", temp, light, ph);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_LAMP, OUTPUT);

  Serial.println("Dang ket noi WiFi ao Wokwi-GUEST...");
  WiFi.begin(ssid, pass);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nDa vao WiFi! Dang tim may chu Blynk...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
  } else {
    Serial.println("\nKhong the vao mang. Mach se chay offline!");
  }

  timer.setInterval(1000L, sendSensorData);
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}