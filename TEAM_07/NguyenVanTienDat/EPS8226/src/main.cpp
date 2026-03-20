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

bool motionDetected = false;
bool gasAlertSent = false;

unsigned long lastTelegramCheck = 0;

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

  Serial.print("Connecting to ");
  Serial.println(ssid);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // TELEGRAM
  client.setInsecure();
  bot.sendMessage(CHAT_ID, "ESP32 Started", "");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==");
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

  // PIR
  if (digitalRead(PIR_PIN) == HIGH) {
    if (!motionDetected) {
      bot.sendMessage(CHAT_ID, "⚠ Phát hiện chuyển động!");
      motionDetected = true;
    }
  } else {
    motionDetected = false;
  }
}

//=========== TELEGRAM ===========
void handleTelegram() {

  if (millis() - lastTelegramCheck < 1000) return;
  lastTelegramCheck = millis();

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {

    for (int i = 0; i < numNewMessages; i++) {

      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;
      String from_name = bot.messages[i].from_name;

      int botname = text.indexOf("@");
      if (botname != -1) text = text.substring(0, botname);

      // START
      if (text == "/start") {

        String welcome = "Xin chào, " + from_name + ".\n";
        welcome += "Sử dụng các lệnh sau để điều khiển hệ thống.\n\n";
        welcome += "/led_on bật đèn\n";
        welcome += "/led_off tắt đèn\n";
        welcome += "/status xem trạng thái hệ thống\n";

        bot.sendMessage(chat_id, welcome, "");
      }

      // LED ON
      if (text == "/led_on") {
        digitalWrite(pinBLED, HIGH);
        blueButtonON = true;
        Blynk.virtualWrite(V1, blueButtonON);
        bot.sendMessage(chat_id, "💡 LED ĐÃ BẬT", "");
      }

      // LED OFF
      if (text == "/led_off") {
        digitalWrite(pinBLED, LOW);
        blueButtonON = false;
        Blynk.virtualWrite(V1, blueButtonON);
        bot.sendMessage(chat_id, "💡 LED ĐÃ TẮT", "");
      }

      // STATUS
      if (text == "/status") {

        String msg = "📊 TRẠNG THÁI HỆ THỐNG\n";
        msg += "----------------------\n";
        msg += "🌡 Nhiệt độ: " + String(temperature) + " °C\n";
        msg += "💧 Độ ẩm: " + String(humidity) + " %\n";
        msg += "💨 Gas: " + String(gasValue) + "\n";

        if (gasValue > 2000) {
          msg += "⚠ Trạng thái: NGUY HIỂM\n";
        } else {
          msg += "✅ Trạng thái: An toàn\n";
        }

        msg += "💡 LED: ";
        msg += (blueButtonON ? "ĐANG BẬT\n" : "ĐANG TẮT\n");

        bot.sendMessage(chat_id, msg, "");
      }
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

//=========== TIMER ===========
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//=========== BUTTON ===========
void updateBlueButton() {

  static ulong lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;

  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON) {
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    digitalWrite(pinBLED, LOW);
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }
}

//=========== UPTIME ===========
void uptimeBlynk() {

  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;

  Blynk.virtualWrite(V0, value);

  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

//=========== DHT22 ===========
void readDHT22() {

  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  TempAndHumidity data = dht.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) return;

  temperature = data.temperature;
  humidity = data.humidity;

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

//=========== GAS ===========
void readGas() {

  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  gasValue = analogRead(MQ2_PIN);

  Blynk.virtualWrite(V4, gasValue);

  if (gasValue > 2000) {

    if (!gasAlertSent) {
      bot.sendMessage(CHAT_ID, "⚠ Cảnh báo khí GAS NGUY HIỂM!");
      gasAlertSent = true;
    }

  } else {

    if (gasAlertSent) {
      bot.sendMessage(CHAT_ID, "✅ Khí GAS đã trở lại an toàn");
      gasAlertSent = false;
    }
  }
}

//=========== OLED ===========
void displayOLED() {

  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  oled.clearDisplay();

  oled.setCursor(0,0);
  oled.print("Nhiet Do: ");
  oled.print(temperature);
  oled.println(" C");

  oled.setCursor(0,20);
  oled.print("Do am: ");
  oled.print(humidity);
  oled.println(" %");

  oled.setCursor(0,40);
  oled.print("Gas: ");
  oled.println(gasValue);

  oled.display();
}

//=========== BLYNK ===========
BLYNK_WRITE(V1) {

  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
}