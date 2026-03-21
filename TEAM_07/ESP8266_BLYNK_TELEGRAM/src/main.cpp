/*
Thông tin nhóm 7
1. Nguyễn Văn Tiến Đạt
2. Hồ Văn Diễn
3. Nguyễn Văn Phong
4. Lương Thanh Ngọc Như
5. Bùi Khắc Hiếu
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>
#include "DHTesp.h"

//=========== TELEGRAM ===========
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define BOT_TOKEN "8616279864:AAGgAliUwCuRsBECjFbCwrnXAQRyeqf7II8"
#define CHAT_ID "-1003847372840"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//=========== BLYNK ===========
#define BLYNK_TEMPLATE_ID "TMPL6PPonUtRv"
#define BLYNK_TEMPLATE_NAME "blynk telegram"
#define BLYNK_AUTH_TOKEN "793CtBqmPKSmHW4CJqNXl_Auc2AnYeDT"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//=========== PIN ===========
#define btnBLED 23
#define pinBLED 21

#define CLK 18
#define DIO 19

#define DHT_PIN 16
#define MQ2_PIN 34
#define PIR_PIN 27

#define OLED_SDA 13
#define OLED_SCL 12

//=========== OBJECT ===========
TM1637Display display(CLK, DIO);
DHTesp dht;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//=========== VARIABLE ===========
ulong currentMiliseconds = 0;
bool blueButtonON = true;

float temperature;
float humidity;
int gasValue;

float lastTemp = -1000;
float lastHumi = -1000;

bool motionDetected = false;
bool gasAlertSent = false;

unsigned long lastTelegramCheck = 0;

// TIMER LED
unsigned long savedTime = 0;
unsigned long startTime = 0;

//=========== HÀM TẠO MENU ===========
String getWelcome(String from_name = "User") {
  String welcome = "Xin chào, " + from_name + ".\n";
  welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
  welcome += "/led_on bật sáng đèn\n";
  welcome += "/led_off tắt đèn\n";
  welcome += "/led_status trạng thái đèn\n";
  welcome += "/get_weather thời tiết\n";

  // BONUS
  welcome = "🤖 ESP32 đã sẵn sàng!\n\n" + welcome;

  return welcome;
}

//=========== PROTOTYPE ===========
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();
void readGas();
void displayOLED();
void handleTelegram();

//=========== SETUP ===========
void setup() {

  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  display.setBrightness(7);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // TELEGRAM
  client.setInsecure();
  bot.sendMessage(CHAT_ID, getWelcome("ESP32"), "");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
}

//=========== LOOP ===========
void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHT22();
  readGas();
  displayOLED();

  handleTelegram();

  if (digitalRead(PIR_PIN) == HIGH) {
    if (!motionDetected) {
      bot.sendMessage(CHAT_ID, "⚠ Phát hiện chuyển động!");
      motionDetected = true;
    }
  } else {
    motionDetected = false;
  }
}