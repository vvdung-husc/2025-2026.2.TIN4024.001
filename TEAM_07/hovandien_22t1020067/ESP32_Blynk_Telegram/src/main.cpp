#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>
#include "DHTesp.h"


//=========== BLYNK ===========
#define BLYNK_TEMPLATE_ID   "TMPL6PPonUtRv"
#define BLYNK_TEMPLATE_NAME "blynk telegram"
#define BLYNK_AUTH_TOKEN    "793CtBqmPKSmHW4CJqNXl_Auc2AnYeDT"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>



//=========== TELEGRAM ===========
#define BOT_TOKEN "8778054811:AAGWgMPMd8fZudPfIbV4WQYzw5s_kVsYtgg"
#define CHAT_ID   "-5156482232"

//=========== WIFI ===========
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//=========== PIN ===========
#define btnBLED  23
#define pinBLED  21
#define CLK      18
#define DIO      19
#define DHT_PIN  16
#define MQ2_PIN  34
#define PIR_PIN  27          // [PIR] định nghĩa pin PIR
#define OLED_SDA 13
#define OLED_SCL 12

//=========== OBJECT ===========
TM1637Display display(CLK, DIO);
DHTesp dht;

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

//=========== VARIABLE ===========
ulong currentMiliseconds = 0;
bool  blueButtonON       = false;

float temperature = 0;
float humidity    = 0;
int   gasValue    = 0;

float prevTemperature = -999;
float prevHumidity    = -999;
int   prevGasValue    = -999;

#define TEMP_THRESHOLD 0.5f
#define HUM_THRESHOLD  1.0f
#define GAS_THRESHOLD  50

bool  needSendTelegram = false;
float tgTemp           = 0;
float tgHum            = 0;
int   tgGas            = 0;

bool dhtReady = false;
bool gasReady = false;

// [PIR] biến trạng thái chuyển động
bool motionDetected   = false;

//=========== PROTOTYPE ===========
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void printEvent(String msg);
void checkSensorChange();
void printSensor();
void updateBlueButton();
void uptimeBlynk();
void readDHT22();
void readGas();
void readPIR();                 // [PIR] prototype
void displayOLED();
void handleTelegramMessages();
void sendTelegramWeather();

//=========== SETUP ===========
void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);     // [PIR] khai báo chân PIR

  digitalWrite(pinBLED, LOW);
  display.setBrightness(0, false);
  display.clear();

  display.setBrightness(7);
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);

  Serial.println("Connecting to " + String(ssid));
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

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
  readPIR();                   // [PIR] gọi hàm đọc PIR trong loop
  displayOLED();
  handleTelegramMessages();
  printSensor();
}

//=========== PRINT EVENT ===========
void printEvent(String msg) {
  Serial.println(msg);
}

//=========== CHECK SENSOR CHANGE ===========
void checkSensorChange() {
  bool tempChanged = fabs(temperature - prevTemperature) >= TEMP_THRESHOLD;
  bool humChanged  = fabs(humidity    - prevHumidity)    >= HUM_THRESHOLD;
  bool gasChanged  = abs(gasValue     - prevGasValue)    >= GAS_THRESHOLD;

  if (tempChanged || humChanged || gasChanged) {
    prevTemperature = temperature;
    prevHumidity    = humidity;
    prevGasValue    = gasValue;

    needSendTelegram = true;
    tgTemp = temperature;
    tgHum  = humidity;
    tgGas  = gasValue;
  }
}

//=========== PRINT SENSOR ===========
void printSensor() {
  if (!dhtReady || !gasReady) return;

  checkSensorChange();

  if (needSendTelegram) {
    char buf[70];
    snprintf(buf, sizeof(buf),
      "[SENSOR] Temp: %5.1f C | Hum: %5.1f %% | Gas: %4d",
      temperature, humidity, gasValue);
    Serial.println(buf);

    char tgBuf[65];
    snprintf(tgBuf, sizeof(tgBuf),
      "[TG    ] >> Canh bao (%.1f C, %.1f %%, Gas: %d)",
      tgTemp, tgHum, tgGas);
    Serial.println(tgBuf);

    String msg = "⚠️ *Cập nhật môi trường*\n";
    msg += "🌡 Nhiệt độ: " + String(tgTemp, 1) + " °C\n";
    msg += "💧 Độ ẩm: "    + String(tgHum, 1)  + " %\n";
    msg += "💨 Gas: "       + String(tgGas);
    bot.sendMessage(CHAT_ID, msg, "Markdown");

    needSendTelegram = false;
  }

  dhtReady = false;
  gasReady = false;
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

  // phát hiện cạnh xuống (HIGH -> LOW)
  if (lastValue == HIGH && v == LOW) {

    blueButtonON = !blueButtonON;

    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

    if (blueButtonON) {
      display.setBrightness(7, true);   // bật display
    } else {
      display.setBrightness(0, false);  // tắt display
      display.clear();
    }

    Blynk.virtualWrite(V1, blueButtonON);

    printEvent("[BTN   ] Toggle -> " + String(blueButtonON ? "ON" : "OFF"));
  }

  lastValue = v;
}

//=========== UPTIME ===========
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);

  if (blueButtonON) display.showNumberDec(value);
}

//=========== DHT22 ===========
void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  TempAndHumidity data = dht.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    printEvent("[DHT   ] Read failed!");
    dhtReady = false;
    return;
  }

  temperature = data.temperature;
  humidity    = data.humidity;

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);

  dhtReady = true;
}

//=========== GAS ===========
void readGas() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  gasValue = analogRead(MQ2_PIN);
  Blynk.virtualWrite(V4, gasValue);

  gasReady = true;
}

//=========== PIR ===========
// [PIR] Đọc cảm biến chuyển động, gửi Telegram khi phát hiện
// - Gửi 1 lần duy nhất khi bắt đầu có chuyển động (cạnh lên)
// - Gửi thông báo "hết chuyển động" khi trở về trạng thái nghỉ
// - Thêm cooldown 10s để tránh spam nếu PIR liên tục báo
void readPIR() {
  static ulong lastAlertTime = 0;           // cooldown chống spam

  bool pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH) {
    if (!motionDetected) {
      // cạnh lên: vừa phát hiện chuyển động
      if (millis() - lastAlertTime >= 10000UL) {
        bot.sendMessage(CHAT_ID, "🚨 *Phát hiện chuyển động!*", "Markdown");
        printEvent("[PIR   ] Phat hien chuyen dong → Telegram sent");
        lastAlertTime = millis();
      }
      motionDetected = true;
    }
  } else {
    if (motionDetected) {
      // cạnh xuống: chuyển động kết thúc
      bot.sendMessage(CHAT_ID, "✅ *Không còn chuyển động.*", "Markdown");
      printEvent("[PIR   ] Het chuyen dong → Telegram sent");
      motionDetected = false;
    }
  }
}

//=========== OLED ===========
void displayOLED() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  oled.clearDisplay();

  oled.setCursor(0, 0);
  oled.print("Nhiet Do: ");
  oled.print(temperature, 1);
  oled.println(" C");

  oled.setCursor(0, 20);
  oled.print("Do am: ");
  oled.print(humidity, 1);
  oled.println(" %");

  oled.setCursor(0, 40);
  oled.print("Gas: ");
  oled.println(gasValue);

  oled.display();
}

//=========== TELEGRAM: nhận lệnh ===========
void handleTelegramMessages() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1500)) return;

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages > 0) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = bot.messages[i].chat_id;
      String text    = bot.messages[i].text;

      printEvent("[TG    ] << [" + chat_id + "]: " + text);

      if (chat_id != CHAT_ID) {
        bot.sendMessage(chat_id, "⛔ Không có quyền truy cập.", "");
        printEvent("[TG    ] >> Tu choi: " + chat_id);
        continue;
      }

      if (text == "/led_on") {
        blueButtonON = true;
        digitalWrite(pinBLED, HIGH);
        Blynk.virtualWrite(V1, 1);
        bot.sendMessage(CHAT_ID, "💡 Đèn LED đã *BẬT*.", "Markdown");
        printEvent("[LED   ] ON  <- lenh Telegram");

      } else if (text == "/led_off") {
        blueButtonON = false;
        digitalWrite(pinBLED, LOW);
        display.clear();
        Blynk.virtualWrite(V1, 0);
        bot.sendMessage(CHAT_ID, "🌑 Đèn LED đã *TẮT*.", "Markdown");
        printEvent("[LED   ] OFF <- lenh Telegram");

      } else if (text == "/led_status") {
        String status = blueButtonON
          ? "💡 Đèn LED đang *BẬT*."
          : "🌑 Đèn LED đang *TẮT*.";
        bot.sendMessage(CHAT_ID, status, "Markdown");
        printEvent("[LED   ] Status: " + String(blueButtonON ? "ON" : "OFF"));

      } else if (text == "/get_weather") {
        sendTelegramWeather();

      } else {
        String help = "🤖 *Danh sách lệnh:*\n";
        help += "/led\\_on — Bật đèn LED\n";
        help += "/led\\_off — Tắt đèn LED\n";
        help += "/led\\_status — Trạng thái đèn\n";
        help += "/get\\_weather — Nhiệt độ & độ ẩm hiện tại";
        bot.sendMessage(CHAT_ID, help, "Markdown");
        printEvent("[TG    ] >> Gui huong dan su dung");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

//=========== TELEGRAM: gửi thời tiết ===========
void sendTelegramWeather() {
  String msg = "🌤 *Thông tin môi trường hiện tại*\n";
  msg += "🌡 Nhiệt độ: " + String(temperature, 1) + " °C\n";
  msg += "💧 Độ ẩm: "    + String(humidity, 1)    + " %\n";
  msg += "💨 Gas (MQ2): " + String(gasValue);
  bot.sendMessage(CHAT_ID, msg, "Markdown");

  char buf[65];
  snprintf(buf, sizeof(buf),
    "[TG    ] >> Weather: %.1f C | %.1f %% | Gas: %d",
    temperature, humidity, gasValue);
  printEvent(String(buf));
}

//=========== BLYNK ===========
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) display.clear();
  printEvent("[BLYNK ] Blue Light " + String(blueButtonON ? "ON" : "OFF"));
}
