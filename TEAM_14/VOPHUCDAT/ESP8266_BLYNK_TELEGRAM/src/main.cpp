/*
=========================================
      NHOM 14 - IoT (WOKWI READY)
=========================================
*/

#define BLYNK_TEMPLATE_ID "TMPL6bcdtDBac"
#define BLYNK_TEMPLATE_NAME "ESP32Nhom14"
#define BLYNK_AUTH_TOKEN "gLnBvVhtCJmKTX6pAV_KTDB8_F9NrKNw"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI WOKWI =================
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ================= TELEGRAM =================
#define BOT_TOKEN "8379917780:AAF4OBlzWQVK5R_qiCKBbh8fxJPAogEKi1g"
#define ADMIN_CHAT_ID "8379917780"

// ================= PIN ESP32 =================
#define DHTPIN 15
#define DHTTYPE DHT11
#define RELAY_PIN 2
#define MQ2_PIN 34
#define OLED_SDA 21
#define OLED_SCL 22

// ================= OLED =================
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ================= OBJECT =================
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= GLOBAL =================
float temperature = 0;
float humidity = 0;
int gasValue = 0;
bool relayState = false;

unsigned long lastTelegramCheck = 0;
unsigned long lastNotify = 0;

// ================= HELPER =================
void setRelay(bool state)
{
  relayState = state;
  digitalWrite(RELAY_PIN, state ? LOW : HIGH);
  Blynk.virtualWrite(V1, state);
}

// ================= TELEGRAM =================
String getData()
{
  String msg = "📊 DATA TEAM 14\n";
  msg += "Temp: " + String(temperature) + "C\n";
  msg += "Humi: " + String(humidity) + "%\n";
  msg += "Gas: " + String(gasValue) + "\n";
  msg += "LED: " + String(relayState ? "ON" : "OFF");
  return msg;
}

void handleTelegram(int n)
{
  for (int i = 0; i < n; i++)
  {
    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start")
    {
      bot.sendMessage(chat_id, "/led_on\n/led_off\n/get", "");
    }

    if (text == "/led_on")
    {
      setRelay(true);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off")
    {
      setRelay(false);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/get")
    {
      bot.sendMessage(chat_id, getData(), "");
    }
  }
}

// ================= SENSOR =================
void readSensor()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // giả lập MQ2
  gasValue = random(200, 800);

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, gasValue);
}

// ================= OLED =================
void updateOLED()
{
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println("TEAM 14 IoT");

  display.setCursor(0, 15);
  display.printf("Temp: %.1f C", temperature);

  display.setCursor(0, 25);
  display.printf("Humi: %.0f %%", humidity);

  display.setCursor(0, 35);
  display.printf("Gas : %d", gasValue);

  display.setCursor(0, 45);
  display.printf("LED : %s", relayState ? "ON" : "OFF");

  display.display();
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  dht.begin();

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED fail");
    while (true);
  }
  display.setTextColor(SSD1306_WHITE);

  // WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");

  client.setInsecure();

  // Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  timer.setInterval(2000, readSensor);
}

// ================= LOOP =================
void loop()
{
  Blynk.run();
  timer.run();

  // Telegram
  if (millis() - lastTelegramCheck > 1000)
  {
    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n)
    {
      handleTelegram(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTelegramCheck = millis();
  }

  // OLED
  updateOLED();

  // Gas alert
  if (gasValue > 700 && millis() - lastNotify > 60000)
  {
    bot.sendMessage(ADMIN_CHAT_ID, "🚨 GAS ALERT!", "");
    lastNotify = millis();
  }
}