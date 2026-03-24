#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram BOT
#define BOTtoken "8778054811:AAGWgMPMd8fZudPfIbV4WQYzw5s_kVsYtgg"
#define GROUP_ID "-5156482232"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// PIR
const int motionSensor = 27;
bool motionDetected = false;

// LED
const int ledPin = 23;
bool ledState = LOW;

// Telegram check
int bot_delay = 1000;
unsigned long lastTimeBotRan;

// format string
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

// PIR interrupt
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

// xử lý tin nhắn telegram
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println(text);

    if (text == "/start") {
      String welcome = "Xin chào \n";
      welcome += "/led_on : Bật đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/state : Trạng thái đèn\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = HIGH;
      bot.sendMessage(chat_id, "LED đã bật", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = LOW;
      bot.sendMessage(chat_id, "LED đã tắt", "");
    }

    if (text == "/state") {
      if (ledState) {
        bot.sendMessage(chat_id, "LED đang bật", "");
      } else {
        bot.sendMessage(chat_id, "LED đang tắt", "");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.println("Connecting WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected");

  bot.sendMessage(GROUP_ID, "ESP32 started", "");
}

void loop() {

  static uint count_ = 0;

  // PIR phát hiện chuyển động
  if (motionDetected) {

    count_++;

    Serial.println("Motion detected");

    String msg = StringFormat("%u => Motion detected!", count_);

    bot.sendMessage(GROUP_ID, msg, "");

    motionDetected = false;
  }

  // kiểm tra lệnh telegram
  if (millis() > lastTimeBotRan + bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}