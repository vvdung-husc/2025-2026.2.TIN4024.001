#define BLYNK_TEMPLATE_ID "TMPL6n34QV4mT"
#define BLYNK_TEMPLATE_NAME "SmallKON"
#define BLYNK_AUTH_TOKEN "d91-oczGWZ5Vm8Z-Suhl4KcmJvanTJbo"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define DHTPIN D3
#define DHTTYPE DHT22
#define LED_PIN D4 
#define GAS_PIN A0

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";
String botToken = "8618224007:AAFkL2FH9sNNCbmfc0fsHXIYqv2Qy3KW1Yg";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

BlynkTimer timer;
float t, h;
int gas;
unsigned long lastTimeBotRan;
const unsigned long botRequestDelay = 2000; // Tăng lên 2 giây để tránh treo

bool ledState = LOW;

void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("TEAM: ITHACA ENERGY");
  display.drawLine(0, 9, 128, 9, SH110X_WHITE);
  display.setCursor(0, 12);
  display.printf("Temp: %.1f C | Hum: %.1f%%", t, h);
  display.setCursor(0, 24);
  display.printf("Gas: %d", gas);
  display.setCursor(0, 36);
  display.printf("Uptime: %lu s", millis() / 1000);
  display.setCursor(0, 54);
  display.println("Status: Online");
  display.display();
}

void sendSensorData() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  gas = analogRead(GAS_PIN);

  if (isnan(h) || isnan(t)) return;

  Blynk.virtualWrite(V1, t);   
  Blynk.virtualWrite(V2, h);   
  Blynk.virtualWrite(V3, gas); 
  Blynk.virtualWrite(V0, millis() / 1000); 
  updateOLED();
}

BLYNK_WRITE(V4) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? LOW : HIGH); 
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/led_on") {
      digitalWrite(LED_PIN, LOW);
      ledState = HIGH;
      bot.sendMessage(chat_id, "LED ON", "");
    } else if (text == "/led_off") {
      digitalWrite(LED_PIN, HIGH);
      ledState = LOW;
      bot.sendMessage(chat_id, "LED OFF", "");
    } else if (text == "/get_weather") {
      bot.sendMessage(chat_id, "Temp: " + String(t) + "C, Hum: " + String(h) + "%", "");
    }
    yield(); // Quan trọng: Trả lại quyền cho hệ thống
  }
}

void setup() {
  Serial.begin(115200); // Đảm bảo Monitor cũng để 115200
  delay(100);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 
  
  dht.begin();
  Wire.begin(D2, D1);
  display.begin(0x3C, true);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure(); 
  
  timer.setInterval(3000L, sendSensorData); // Tăng thời gian lấy mẫu lên 3s
}

void loop() {
  Blynk.run();
  timer.run();

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  yield(); // Giữ cho WiFi ổn định
}