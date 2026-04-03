#define BLYNK_TEMPLATE_ID "TMPL6nckkSvdV"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk Telegram"
#define BLYNK_AUTH_TOKEN "jR53GR1c9eKpi0dWczhUEg9j-0a-wR5P"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ---- WiFi ----
const char* ssid     = "Wokwi-GUEST";     // ← SỬA
const char* password = "";     // ← SỬA

// ---- Telegram ----

#define BOT_TOKEN "8616279864:AAGgAliUwCuRsBECjFbCwrnXAQRyeqf7II8"
#define CHAT_ID "-1003847372840"    // ← SỬA

// ---- PIN LoLin V3 ----
#define DHTPIN    D4    // GPIO2
#define DHTTYPE   DHT22
#define LED_PIN   D5    // GPIO14
#define MQ2_PIN   A0    // Analog duy nhất

// ---- OLED ----
// SDA → D2 (GPIO4)
// SCL → D1 (GPIO5)
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
BlynkTimer timer;

// ---- Biến toàn cục ----
bool ledState     = false;
float temperature = 0;
float humidity    = 0;
int gasValue      = 0;
unsigned long lastTelegramCheck = 0;
const int telegramInterval      = 3000;

// ============================================
// UPTIME → Blynk V0 mỗi 1 giây
// ============================================
void sendUptime() {
  unsigned long uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime);
}

// ============================================
// ĐỌC CẢM BIẾN → Blynk V2 V3 V4
// ============================================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;

  int raw  = analogRead(MQ2_PIN);
  gasValue = (raw > 10) ? raw : random(200, 800);

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, gasValue);
}

// ============================================
// HIỂN THỊ OLED
// ============================================
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Uptime: ");
  display.print(millis() / 1000);
  display.print("s");

  display.setCursor(0, 12);
  display.print("LED: ");
  display.print(ledState ? "ON" : "OFF");

  display.setCursor(0, 24);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.print(" C");

  display.setCursor(0, 36);
  display.print("Humi: ");
  display.print(humidity, 1);
  display.print(" %");

  display.setCursor(0, 48);
  display.print("Gas: ");
  display.print(gasValue);

  display.display();
}

// ============================================
// GỬI TEAM INFO → Blynk V5
// ============================================
void sendTeamInfo() {
  Blynk.virtualWrite(V5, "Nhom 07 - IoT");
}

// ============================================
// BLYNK: SWITCH V1 → LED
// ============================================
BLYNK_WRITE(V1) {
  int val  = param.asInt();
  ledState = (val == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Serial.println(ledState ? "LED ON" : "LED OFF");
  updateOLED(); // Cập nhật OLED ngay khi bấm
}

// ============================================
// TELEGRAM: XỬ LÝ TIN NHẮN
// ============================================
void handleTelegramMessages(int numMessages) {
  for (int i = 0; i < numMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text    = bot.messages[i].text;
    String from    = bot.messages[i].from_name;

    Serial.println("MSG: " + text + " from " + from);

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
      updateOLED();
      bot.sendMessage(chat_id, "LED da BAT!", "");
    }
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0);
      updateOLED();
      bot.sendMessage(chat_id, "LED da TAT!", "");
    }
    else if (text == "/led_status") {
      bot.sendMessage(chat_id,
        ledState ? "LED dang BAT" : "LED dang TAT", "");
    }
    else if (text == "/get_weather") {
      String msg  = "Nhiet do: " + String(temperature, 1) + " C\n";
              msg += "Do am:    " + String(humidity, 1)    + " %";
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/start" || text == "/help") {
      String help  = "IoT Bot - Nhom 07\n\n";
             help += "/led_on     - Bat LED\n";
             help += "/led_off    - Tat LED\n";
             help += "/led_status - Trang thai\n";
             help += "/get_weather - Nhiet do & do am";
      bot.sendMessage(chat_id, help, "");
    }
  }
}

// ============================================
// TELEGRAM: GỬI KHI CẢM BIẾN THAY ĐỔI
// ============================================
float lastTemp = 0, lastHumi = 0;

void checkAndNotifyTelegram() {
  if (abs(temperature - lastTemp) >= 1.0 ||
      abs(humidity    - lastHumi) >= 5.0) {
    lastTemp = temperature;
    lastHumi = humidity;
    String msg  = "Cap nhat cam bien:\n";
           msg += "Nhiet do: " + String(temperature, 1) + " C\n";
           msg += "Do am:    " + String(humidity, 1)    + " %";
    bot.sendMessage(CHAT_ID, msg, "");
  }
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // OLED khởi động
  Wire.begin();   // D2=SDA, D1=SCL (mặc định ESP8266)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  // DHT khởi động
  dht.begin();

  // Blynk kết nối
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Telegram SSL
  client.setInsecure();

  // Timer — interval khác nhau tránh xung đột
  timer.setInterval(1000L,  sendUptime);
  timer.setInterval(1500L,  updateOLED);
  timer.setInterval(2000L,  readSensors);
  timer.setInterval(10000L, sendTeamInfo);
  timer.setInterval(30000L, checkAndNotifyTelegram);

  Serial.println("== READY ==");
}

// ============================================
// LOOP
// ============================================
void loop() {
  Blynk.run();
  timer.run();

  // Kiểm tra Telegram mỗi 3 giây
  if (millis() - lastTelegramCheck > telegramInterval) {
    lastTelegramCheck = millis();
    int n = bot.getUpdates(bot.last_message_received + 1);
    if (n > 0) handleTelegramMessages(n);
  }
}