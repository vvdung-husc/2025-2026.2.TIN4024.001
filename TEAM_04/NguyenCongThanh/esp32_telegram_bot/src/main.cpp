#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WIFI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// TELEGRAM
#define BOT_TOKEN "8629442956:AAF8YBmH05p94b6TYRJh29mo_Fpgvs4pPuI"
#define CHAT_ID "-4992905436"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// PIN
const int LED_PIN = 23;
const int PIR_PIN = 27;

bool ledState = false;
bool motionState = false;

unsigned long lastTimeBotRan;
unsigned long lastMotionTime = 0;

const int botDelay = 1000;
const int motionCooldown = 10000;

// =========================
// Xử lý lệnh Telegram
// =========================
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Command received: " + text);

    if (text == "/start") {

      String welcome = "Xin chào 👋\n";
      welcome += "ESP32 Telegram Control\n\n";
      welcome += "/led2_on : bật LED\n";
      welcome += "/led2_off : tắt LED\n";
      welcome += "/get_state : trạng thái LED\n";

      bot.sendMessage(chat_id, welcome, "");

      Serial.println("Sent welcome message");
    }

    if (text == "/led2_on") {

      digitalWrite(LED_PIN, HIGH);
      ledState = true;

      bot.sendMessage(chat_id, "💡 LED đã bật", "");

      Serial.println("LED ON");
    }

    if (text == "/led2_off") {

      digitalWrite(LED_PIN, LOW);
      ledState = false;

      bot.sendMessage(chat_id, "🌑 LED đã tắt", "");

      Serial.println("LED OFF");
    }

    if (text == "/get_state") {

      if (ledState) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }

      Serial.println("State requested");
    }
  }
}

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  client.setInsecure();

  bot.sendMessage(CHAT_ID, "ESP32 BOT ONLINE 🚀", "");

}

// =========================
// LOOP
// =========================
void loop() {

  // ===== TELEGRAM =====
  if (millis() - lastTimeBotRan > botDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }

  // ===== PIR SENSOR =====

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH && millis() - lastMotionTime > motionCooldown) {

    Serial.println("🚨 Motion detected!");

    bot.sendMessage(CHAT_ID, "🚨 Motion detected by PIR sensor!", "");

    lastMotionTime = millis();
  }
}