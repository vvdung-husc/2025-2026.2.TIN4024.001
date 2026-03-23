#define BLYNK_TEMPLATE_ID "TMPL6n34QV4mT"
#define BLYNK_TEMPLATE_NAME "SmallKON"
#define BLYNK_AUTH_TOKEN "d91-oczGWZ5Vm8Z-Suhl4KcmJvanTJbo"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h> 

#define DHTPIN D5 
#define DHTTYPE DHT22
#define LED_PIN D4 
#define GAS_PIN A0

char ssid[] = "abc"; 
char pass[] = "12345678";
String botToken = "8618224007:AAFkL2FH9sNNCbmfc0fsHXIYqv2Qy3KW1Yg";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

BlynkTimer timer;
float t = 0, h = 0;
int gas = 0;
unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 2000; 

void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "TEAM: ITHACA ENERGY");
  u8g2.drawLine(0, 12, 128, 12);
  u8g2.setCursor(0, 28);
  u8g2.printf("Temp: %.1f C", t);
  u8g2.setCursor(0, 42);
  u8g2.printf("Hum : %.1f %%", h);
  u8g2.setCursor(0, 56);
  u8g2.printf("Gas : %d", gas);
  u8g2.sendBuffer(); 
}

void sendSensorData() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  gas = analogRead(GAS_PIN);
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t);   
    Blynk.virtualWrite(V2, h);   
    Blynk.virtualWrite(V3, gas); 
    updateOLED();
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    if (text == "/led_on") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED ON", "");
    } else if (text == "/led_off") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED OFF", "");
    } else if (text == "/get_weather") {
      bot.sendMessage(chat_id, "Nhiet do: " + String(t) + "C, Do am: " + String(h) + "%", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 
  dht.begin();
  Wire.begin(D2, D1);
  u8g2.begin();
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(10, 35, "WiFi Connecting...");
  u8g2.sendBuffer();

  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  client.setInsecure(); 

  // Chờ kết nối và hiển thị dấu chấm ra Serial
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  u8g2.clearBuffer();
  u8g2.drawStr(10, 35, "WiFi OK!");
  u8g2.sendBuffer();
  delay(1000);

  Blynk.config(BLYNK_AUTH_TOKEN);
  timer.setInterval(3000L, sendSensorData);
}

void loop() {
  // Chỉ chạy khi có mạng
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    if (millis() - lastTimeBotRan > botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      if (numNewMessages > 0) {
        handleNewMessages(numNewMessages);
      }
      lastTimeBotRan = millis();
    }
  }
  timer.run();
}