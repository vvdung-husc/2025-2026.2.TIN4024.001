#define BLYNK_TEMPLATE_ID "TMPL6Q9z66kmy"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ===== WIFI =====
char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

// ===== TELEGRAM =====
#define BOT_TOKEN "YOUR_NEW_TOKEN"
#define CHAT_ID  "-5237059167"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ===== PIN =====
#define DHTPIN D3
#define DHTTYPE DHT22
#define LED_PIN D4

DHT dht(DHTPIN, DHTTYPE);

// ===== BIẾN =====
unsigned long lastRead = 0;
unsigned long lastCheck = 0;
int ledState = LOW;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  WiFi.begin(ssid, pass);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  client.setInsecure();

  bot.sendMessage(CHAT_ID, "✅ ESP8266 da san sang!", "");
}

// ===== XỬ LÝ TELEGRAM =====
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    Serial.println("Nhan lenh: " + text);

    // ===== LED ON =====
    if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      ledState = HIGH;
      bot.sendMessage(chat_id, "💡 LED DA BAT", "");
    }

    // ===== LED OFF =====
    else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      ledState = LOW;
      bot.sendMessage(chat_id, "💡 LED DA TAT", "");
    }

    // ===== LED STATUS =====
    else if (text == "/led_status") {
      if (ledState == HIGH)
        bot.sendMessage(chat_id, "💡 LED dang BAT", "");
      else
        bot.sendMessage(chat_id, "💡 LED dang TAT", "");
    }

    // ===== WEATHER =====
    else if (text == "/get_weather") {
      float temp = dht.readTemperature();
      float hum  = dht.readHumidity();

      if (isnan(temp) || isnan(hum)) {
        bot.sendMessage(chat_id, "❌ Loi doc cam bien!", "");
      } else {
        String msg = "🌡 Nhiet do: " + String(temp) + "°C\n";
        msg += "💧 Do am: " + String(hum) + "%";
        bot.sendMessage(chat_id, msg, "");
      }
    }
  }
}

// ===== LOOP =====
void loop() {
  Blynk.run();

  // đọc sensor mỗi 2s (gửi lên Blynk)
  if (millis() - lastRead > 2000) {
    lastRead = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      Blynk.virtualWrite(V2, temp);
      Blynk.virtualWrite(V3, hum);
    }

    Serial.print("Temp: "); Serial.print(temp);
    Serial.print(" | Hum: "); Serial.println(hum);
  }

  // check Telegram mỗi 1s
  if (millis() - lastCheck > 1000) {
    lastCheck = millis();

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}