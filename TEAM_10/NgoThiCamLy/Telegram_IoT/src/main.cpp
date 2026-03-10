#include <Arduino.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "8235124253:AAHt8WxIYQEdb9a_EwdyQEUjPe2jKgYrpmU"  // your Bot Token (Get from Botfather)

// tìm giá trị GROUP_ID này
#define GROUP_ID "-4663178968" // thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
const int ledPin = 23;       // LED

bool motionDetected = false;
bool ledState = false;

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;

//Định dạng chuỗi %s,%d,...
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

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// ===== XỬ LÝ LỆNH TELEGRAM =====
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {
      String welcome = "Xin chào, Cẩm Ly.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED đã tắt", "");
    }

    if (text == "/get_state") {

      if (ledState) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }

    }

  }
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);

  // LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Set motionSensor pin as interrupt
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");

  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}

void loop() {
  static uint count_ = 0;

  // ===== PIR phát hiện chuyển động =====
  if(motionDetected){
    ++count_;
    Serial.print(count_);
    Serial.println(". MOTION DETECTED => Waiting to send to Telegram");

    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());

    Serial.print(count_);
    Serial.println(". Sent successfully to Telegram: Motion Detected");

    motionDetected = false;
  }

  // ===== Kiểm tra lệnh Telegram =====
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

while (numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {

      String welcome = "Xin chào.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "/led_on bật đèn\n";
      welcome += "/led_off tắt đèn\n";

      bot.sendMessage(chat_id, welcome, "");
    }

  }

  numNewMessages = bot.getUpdates(bot.last_message_received + 1);
}
}