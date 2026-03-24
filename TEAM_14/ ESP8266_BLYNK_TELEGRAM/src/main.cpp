/*
=========================================
      THONG TIN NHOM 14 - IoT
=========================================
1. Phan Đại Tấn
2. Võ Phúc Đạt
3. Nguyễn Trí Nhân
...
=========================================
*/

#define BLYNK_TEMPLATE_ID "TMPL61TMBQWcc"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "D3rVJiuXLBL6njHeiWBqi74qwxaGTEkk"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

char ssid[] = "OPPO A58";
char pass[] = "44444444";

// ===================== TELEGRAM =====================
#define BOT_TOKEN "8320632205:AAETm2lhX3N0Wsr2B9CKaMZYnrvZVBbOVTk"
#define ADMIN_CHAT_ID "-5138628831"

// ===================== PIN CONFIG =====================
#define DHTPIN D3
#define DHTTYPE DHT11 // Chinh lai DHT11 cho phu hop thiet bi pho bien
#define RELAY_PIN D4  // Su dung D4 (LED build-in) hoac chan Relay
#define MQ2_PIN A0
#define OLED_SDA D2
#define OLED_SCL D1

// ===================== TEAM INFO =====================
const char *TEAM_NAME = "IoT - Team 14.001";

// ===================== THRESHOLD =====================
const int GAS_MEDIUM_THRESHOLD = 400;
const int GAS_HIGH_THRESHOLD = 700;

// ===================== OBJECTS =====================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display(128, 64, &Wire, -1);
BlynkTimer timer;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ===================== GLOBAL VARIABLES =====================
float temperature = 0.0f;
float humidity = 0.0f;
int gasValue = 0;
bool relayState = false;

unsigned long lastTelegramCheck = 0;
unsigned long lastGasNotify = 0;
unsigned long lastWeatherNotify = 0;
unsigned long lastDisplayUpdate = 0;

float lastSentTemp = -999.0f;
float lastSentHum = -999.0f;
int lastSentGas = -1;

// ===================== HELPER =====================
String formatUptime(unsigned long ms)
{
  unsigned long totalSec = ms / 1000;
  unsigned long days = totalSec / 86400;
  unsigned long hours = (totalSec % 86400) / 3600;
  unsigned long mins = (totalSec % 3600) / 60;
  unsigned long secs = totalSec % 60;
  char buf[32];
  snprintf(buf, sizeof(buf), "%luD %02lu:%02lu:%02lu", days, hours, mins, secs);
  return String(buf);
}

void setRelay(bool state)
{
  relayState = state;
  // ESP8266 Build-in LED thuong active LOW
  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
  if (Blynk.connected())
  {
    Blynk.virtualWrite(V1, relayState ? 1 : 0);
  }
}

// ===================== TELEGRAM TEXT =====================
String getFormattedSensorData()
{
  String msg = "📊 BAO CAO TEAM 14\n";
  msg += "----------------------\n";
  msg += "🌡 Nhiet do: " + String(temperature, 1) + " C\n";
  msg += "💧 Do am   : " + String(humidity, 0) + " %\n";
  msg += "⛽ Khi gas : " + String(gasValue) + "\n";
  msg += "💡 Thiet bi: " + String(relayState ? "DANG BAT" : "DANG TAT") + "\n";
  msg += "⏱ Uptime  : " + formatUptime(millis()) + "\n";
  msg += "----------------------\n";
  msg += TEAM_NAME;
  return msg;
}

// ===================== NOTIFICATIONS =====================
void notifyTelegramSensors()
{
  if (WiFi.status() != WL_CONNECTED)
    return;

  // Tu dong bao khi nhiet do thay doi manh
  if (abs(temperature - lastSentTemp) > 1.0 || abs(humidity - lastSentHum) > 5.0)
  {
    if (millis() - lastWeatherNotify > 60000UL)
    {
      String alert = "⚠️ CANH BAO MOI TRUONG\n";
      alert += "Nhiet do: " + String(temperature, 1) + " C\n";
      alert += "Do am: " + String(humidity, 0) + " %\n";
      alert += TEAM_NAME;
      bot.sendMessage(ADMIN_CHAT_ID, alert, "");
      lastSentTemp = temperature;
      lastSentHum = humidity;
      lastWeatherNotify = millis();
    }
  }

  // Canh bao Gas
  if (gasValue > GAS_HIGH_THRESHOLD && (millis() - lastGasNotify > 60000UL))
  {
    bot.sendMessage(ADMIN_CHAT_ID, "🚨 NGUY HIEM: PHAT HIEN RO RI KHI GAS!\nTeam 14", "");
    lastGasNotify = millis();
  }
}

// ===================== OLED =====================
void updateOLED()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("NHOM 14 - IoT");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);

  display.setCursor(0, 15);
  display.printf("Temp: %.1f C", temperature);
  display.setCursor(0, 25);
  display.printf("Humi: %.1f %%", humidity);
  display.setCursor(0, 35);
  display.printf("Gas : %d ppm", gasValue);
  display.setCursor(0, 45);
  display.printf("LED : %s", relayState ? "ON" : "OFF");

  display.setCursor(0, 55);
  display.print("UP: ");
  display.print(formatUptime(millis()));
  display.display();
}

// ===================== TELEGRAM HANDLE =====================
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (text == "/start")
    {
      String welcome = "Chao mung den voi " + String(TEAM_NAME) + "\n";
      welcome += "/led_on : Bat den\n";
      welcome += "/led_off : Tat den\n";
      welcome += "/get_all : Xem cam bien\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if (text == "/led_on")
    {
      setRelay(true);
      bot.sendMessage(chat_id, "đã bật đèn LED 🟢", "");
    }
    if (text == "/led_off")
    {
      setRelay(false);
      bot.sendMessage(chat_id, "đã tắt đèn LED 🔴", "");
    }
    if (text == "/get_all")
    {
      bot.sendMessage(chat_id, getFormattedSensorData(), "");
    }
  }
}

// ===================== MAIN TASKS =====================
void taskReadData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  gasValue = analogRead(MQ2_PIN); // Hoac random(200,500) neu ko co cam bien

  // Update Blynk
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, formatUptime(millis()));
}

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);
  if (display.begin(0x3C, true))
  {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.display();
  }

  WiFi.begin(ssid, pass);
  secured_client.setInsecure();

  Blynk.config(BLYNK_AUTH_TOKEN);

  timer.setInterval(2000L, taskReadData);
  timer.setInterval(5000L, notifyTelegramSensors);

  Serial.println("He thong Nhom 14 san sang.");
}

void loop()
{
  Blynk.run();
  timer.run();

  if (millis() - lastTelegramCheck > 1000)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTelegramCheck = millis();
  }

  if (millis() - lastDisplayUpdate > 500)
  {
    updateOLED();
    lastDisplayUpdate = millis();
  }
}