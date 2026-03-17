/*
//Blynk
#define BLYNK_TEMPLATE_ID "TMPL6hvAQ6euC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "fWvJyzBUr1c1RbiD6fn0BfGSs3orBNU9"

// TELEGRAM
#define BOTtoken "8665074207:AAEl9awOa1D0j8Jp-M2bWq9zlgM4HIscF-s"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
String chat_id = "8636449732";
*/
/*
THÔNG TIN TEAM 10.001
1. Lê Thị Khánh Ly
2.
3.
*/
// ===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL6hvAQ6euC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "fWvJyzBUr1c1RbiD6fn0BfGSs3orBNU9"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== TELEGRAM =====
#define BOTtoken "8665074207:AAEl9awOa1D0j8Jp-M2bWq9zlgM4HIscF-s"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
String chat_id = "8636449732";

// ===== DHT =====
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_PIN D5   // TRÁNH D1, D2 (I2C)
bool ledState = false;

// ===== MQ2 =====
int gasValue = 0;

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// ===== TIME =====
unsigned long startTime;

// ===== BLYNK VPIN =====
#define VPIN_LED V0
#define VPIN_TEMP V1
#define VPIN_HUMI V2
#define VPIN_GAS V3
#define VPIN_UPTIME V4
#define VPIN_TEAM V5

BlynkTimer timer;

// ===== BLYNK CONTROL =====
BLYNK_WRITE(VPIN_LED) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// ===== SENSOR + OLED =====
void sendSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) return;

  gasValue = random(200, 800);
  Blynk.virtualWrite(VPIN_TEAM, "TEAM 10.001");
  Blynk.virtualWrite(VPIN_TEMP, t);
  Blynk.virtualWrite(VPIN_HUMI, h);
  Blynk.virtualWrite(VPIN_GAS, gasValue);

  unsigned long uptime = (millis() - startTime) / 1000;
  Blynk.virtualWrite(VPIN_UPTIME, uptime);

  // OLED hiển thị
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  u8g2.drawStr(0,10,"TEAM XY.001");

  u8g2.setCursor(0,25);
  u8g2.print("Temp: "); u8g2.print(t);

  u8g2.setCursor(0,35);
  u8g2.print("Humi: "); u8g2.print(h);

  u8g2.setCursor(0,45);
  u8g2.print("Gas : "); u8g2.print(gasValue);

  u8g2.setCursor(0,60);
  u8g2.print("Up: "); u8g2.print(uptime);

  u8g2.sendBuffer();
}

// ===== TELEGRAM =====
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;

      if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
        bot.sendMessage(chat_id, "LED ON", "");
      }
      else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        bot.sendMessage(chat_id, "LED OFF", "");
      }
      else if (text == "/led_status") {
        bot.sendMessage(chat_id, ledState ? "LED ON" : "LED OFF", "");
      }
      else if (text == "/get_weather") {
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        String msg = "Temp: " + String(t) + "\nHumi: " + String(h);
        bot.sendMessage(chat_id, msg, "");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  // FIX I2C CHẮC CHẮN
  Wire.begin(D2, D1); // SDA, SCL
  u8g2.begin();

  // Hiển thị thông tin nhóm khi khởi động
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0,10,"TEAM 10.001");
  u8g2.drawStr(0,20,"1. Khanh Ly");
  u8g2.drawStr(0,30,"2. Cam Ly");
  u8g2.drawStr(0,40,"3. Pham Nang");
  u8g2.drawStr(0,50,"4. Quynh Anh");
  u8g2.drawStr(0,60,"5. Yen Nhi");
  u8g2.sendBuffer();
  delay(3000);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Telegram HTTPS fix
  client.setInsecure();

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  startTime = millis();

  timer.setInterval(1000L, handleTelegram);
  timer.setInterval(2000L, sendSensor);
}

// ===== LOOP =====
void loop() {
  Blynk.run();
  timer.run();
}

