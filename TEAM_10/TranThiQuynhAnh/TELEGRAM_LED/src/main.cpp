#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

/* WIFI */
const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* TELEGRAM BOT */
#define BOTtoken "8668121676:AAE7AOv_XbPBKxesFAkI_5X15hHy9WUDN4k"
#define CHAT_ID "8654734301"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

/* BOT TIMER */
int bot_delay = 1000;
unsigned long lastTimeBotRan;

/* LED PIN */
const int ledPin = 23;
bool ledState = LOW;

/* HANDLE TELEGRAM MESSAGE */
void handleNewMessages(int numNewMessages) {

  Serial.println("Handling New Message");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String user_text = bot.messages[i].text;
    Serial.println(user_text);

    String your_name = bot.messages[i].from_name;

    /* START */
    if (user_text == "/start") {

      String welcome = "Welcome " + your_name + "\n";
      welcome += "ESP32 Telegram Control\n\n";
      welcome += "/led_on : Turn LED ON\n";
      welcome += "/led_off : Turn LED OFF\n";
      welcome += "/state : Check LED State\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    /* LED ON */
    if (user_text == "/led_on") {

      ledState = HIGH;
      digitalWrite(ledPin, ledState);

      bot.sendMessage(chat_id, "LED is ON", "");
    }

    /* LED OFF */
    if (user_text == "/led_off") {

      ledState = LOW;
      digitalWrite(ledPin, ledState);

      bot.sendMessage(chat_id, "LED is OFF", "");
    }

    /* CHECK STATE */
    if (user_text == "/state") {

      if (digitalRead(ledPin)) {
        bot.sendMessage(chat_id, "LED is currently ON", "");
      }
      else {
        bot.sendMessage(chat_id, "LED is currently OFF", "");
      }
    }
  }
}

/* SETUP */
void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);

  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  client.setInsecure();   // Important for Telegram
  client.setTimeout(15000);
}

/* LOOP */
void loop() {

  if (millis() > lastTimeBotRan + bot_delay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {

      Serial.println("Got Response!");

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    }

    lastTimeBotRan = millis();
  }
}