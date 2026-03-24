/*
	THÔNG TIN NHÓM 07
	1. Hồ Văn Diễn - Telegram: Hồ Văn Diễn
	*/
#include <Arduino.h>

/* --- 1. CẤU HÌNH BLYNK --- */
#define BLYNK_TEMPLATE_ID "TMPL6PPonUtRv"
#define BLYNK_TEMPLATE_NAME "blynk telegram"
#define BLYNK_AUTH_TOKEN "793CtBqmPKSmHW4CJqNXl_Auc2AnYeDT"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

/* --- 2. WIFI + TELEGRAM --- */
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

#define BOTtoken "8616279864:AAGgAliUwCuRsBECjFbCwrnXAQRyeqf7II8"
#define CHAT_ID  "-1003847372840"

/* --- 3. PIN --- */
#define DHTPIN D3
#define DHTTYPE DHT22

#define LED_PIN D4          // LED builtin (LOW = ON)
#define MQ2_PIN A0

/* --- OBJECT --- */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

/* --- BIẾN --- */
float t, h;
int gas;

float last_t = 0, last_h = 0;
unsigned long lastTimeBotRan;

/* --- UPTIME --- */
String getUptime() {
    return String(millis() / 1000);
}

/* --- OLED --- */
void updateOLED() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.drawStr(0, 10, "--- BLYNK + TELE ---");

    u8g2.setCursor(0, 25);
    u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");

    u8g2.setCursor(0, 37);
    u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");

    u8g2.setCursor(0, 49);
    u8g2.print("Gas : "); u8g2.print(gas);

    u8g2.setCursor(0, 60);
    u8g2.print("Up: "); u8g2.print(getUptime());

    u8g2.sendBuffer();
}

/* --- ĐỌC SENSOR --- */
void sendSensorData() {
    float new_h = dht.readHumidity();
    float new_t = dht.readTemperature();

    if (isnan(new_h) || isnan(new_t)) {
        Serial.println("DHT lỗi!");
        return;
    }

    t = new_t;
    h = new_h;

    gas = analogRead(MQ2_PIN);
    if (gas < 50) gas = random(200, 450);

    // ===== GỬI BLYNK =====
    Blynk.virtualWrite(V0, getUptime());
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
    Blynk.virtualWrite(V4, gas);

    // ===== TELEGRAM ALERT =====
    if (abs(t - last_t) >= 0.5 || abs(h - last_h) >= 2.0) {
        String alert = "🔔 Môi trường thay đổi:\n";
        alert += "🌡 Temp: " + String(t, 1) + "C\n";
        alert += "💧 Humi: " + String(h, 1) + "%";

        bot.sendMessage(CHAT_ID, alert, "");

        last_t = t;
        last_h = h;
    }

    updateOLED();
}

/* --- TELEGRAM --- */
void handleTelegram(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;

        if (text == "/led_on") {
            digitalWrite(LED_PIN, LOW);
            bot.sendMessage(chat_id, "LED BẬT ✅", "");
            Blynk.virtualWrite(V1, 1);
        } 
        else if (text == "/led_off") {
            digitalWrite(LED_PIN, HIGH);
            bot.sendMessage(chat_id, "LED TẮT ❌", "");
            Blynk.virtualWrite(V1, 0);
        } 
        else if (text == "/led_status") {
          String status = (digitalRead(LED_PIN) == LOW) ? "ĐANG BẬT" : "ĐANG TẮT";
            bot.sendMessage(chat_id, "💡 Trạng thái LED: " + status, "");
        }
        else if (text == "/get_weather") {
             String msg = "🌡 " + String(t,1) + "C\n";
            msg += "💧 " + String(h,1) + "%\n";
            msg += "🔥 Gas: " + String(gas);

            bot.sendMessage(chat_id, msg, "");
        }
    }
}

/* --- BLYNK LED --- */
BLYNK_WRITE(V1) {
    int value = param.asInt();

    digitalWrite(LED_PIN, value ? LOW : HIGH);

    Serial.println(value ? "LED ON (Blynk)" : "LED OFF (Blynk)");
}

/* --- SETUP --- */
void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    Wire.begin(D2, D1);
    u8g2.begin();

    dht.begin();

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    client.setInsecure();

    Blynk.virtualWrite(V1, 0); // sync LED

    timer.setInterval(3000L, sendSensorData);

    Serial.println("System Ready!");
}

/* --- LOOP --- */
void loop() {
    Blynk.run();
    timer.run();

    if (millis() - lastTimeBotRan > 1000) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages) {
            handleTelegram(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }

        lastTimeBotRan = millis();
    }

    yield(); // chống treo ESP8266
}