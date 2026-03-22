#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

/* ================================================================
 * CẤU HÌNH MẠNG & TELEGRAM
 * ================================================================ */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BOTtoken     "8574319130:AAFSzp008cM4RiDGiweQeY6sFwOWwjifx4o" 
#define CHAT_ID      "-1003559275543"

/* ================================================================
 * ĐỊNH NGHĨA CHÂN & KIỂU CẢM BIẾN
 * ================================================================ */
#define DHTPIN    D3
#define DHTTYPE   DHT22
#define LED_PIN   D4
#define MQ2_PIN   A0

/* ================================================================
 * NGƯỠNG CẢNH BÁO
 * ================================================================ */
#define GAS_THRESHOLD     600 
#define TEMP_ALERT_HIGH   35.0f
#define TEMP_DELTA        0.5f 
#define HUM_DELTA         2.0f 

#define LED_TIMER_MAX_MIN  60 
#define LOG_SIZE 10

struct SensorLog {
    float temp;
    float hum;
    int   gas;
    unsigned long timestamp;
};

/* ================================================================
 * BIẾN TOÀN CỤC
 * ================================================================ */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

float t = 0, h = 0;
int   gas = 0;
float last_t = 0, last_h = 0;
bool  gasAlerted = false;

bool          ledTimerActive = false;
unsigned long ledOffTime     = 0;

unsigned long lastBotCheck   = 0;
unsigned long lastSensorRead = 0;

SensorLog sensorLog[LOG_SIZE];
int       logIndex = 0;
int       logCount = 0;

/* ================================================================
 * MODULE: TIỆN ÍCH & LOG
 * ================================================================ */
String getUptime() {
    unsigned long sec = millis() / 1000;
    char buf[12];
    sprintf(buf, "%02lu:%02lu:%02lu", sec / 3600, (sec % 3600) / 60, sec % 60);
    return String(buf);
}

String fmtTimestamp(unsigned long ms) {
    unsigned long sec = ms / 1000;
    char buf[12];
    sprintf(buf, "[%luh%lum]", sec / 3600, (sec % 3600) / 60);
    return String(buf);
}

void pushLog(float _t, float _h, int _gas) {
    sensorLog[logIndex] = { _t, _h, _gas, millis() };
    logIndex = (logIndex + 1) % LOG_SIZE;
    if (logCount < LOG_SIZE) logCount++;
}

String buildLogSummary() {
    if (logCount == 0) return "Chưa có dữ liệu log.";
    String msg = "📋 *Log " + String(logCount) + " mẫu gần nhất:*\n";
    for (int i = 0; i < logCount; i++) {
        int idx = (logIndex - logCount + i + LOG_SIZE) % LOG_SIZE;
        SensorLog& e = sensorLog[idx];
        msg += fmtTimestamp(e.timestamp) + " T:" + String(e.temp, 1) + " H:" + String(e.hum, 1) + " G:" + String(e.gas) + "\n";
    }
    return msg;
}

/* ================================================================
 * MODULE: OLED
 * ================================================================ */
void updateOLED() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 7, "=== IoT MONITOR v2 ===");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(0, 19);
    u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");
    u8g2.setCursor(0, 31);
    u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");
    u8g2.setCursor(0, 43);
    u8g2.print("Gas : "); u8g2.print(gas);
    if (gas >= GAS_THRESHOLD) u8g2.drawStr(72, 43, "<!ALERT>");
    u8g2.setCursor(0, 55);
    bool ledOn = (digitalRead(LED_PIN) == LOW);
    u8g2.print("LED : "); u8g2.print(ledOn ? "ON" : "OFF");
    if (ledTimerActive) {
        unsigned long remain = (ledOffTime > millis()) ? (ledOffTime - millis()) / 1000 : 0;
        u8g2.print(" ["); u8g2.print(remain); u8g2.print("s]");
    }
    u8g2.setFont(u8g2_font_4x6_tf);
    char footer[32];
    sprintf(footer, "Up:%s  Team12-IoT", getUptime().c_str());
    u8g2.drawStr(0, 64, footer);
    u8g2.sendBuffer();
}

/* ================================================================
 * MODULE: CẢM BIẾN & XỬ LÝ
 * ================================================================ */
void readSensors() {
    float new_h = dht.readHumidity();
    float new_t = dht.readTemperature();
    if (isnan(new_h) || isnan(new_t)) return;

    t = new_t; h = new_h;
    gas = analogRead(MQ2_PIN);
    if (gas < 50) gas = random(180, 480);

    pushLog(t, h, gas);

    if (abs(t - last_t) >= TEMP_DELTA || abs(h - last_h) >= HUM_DELTA) {
        String alert = "🔔 *Cập nhật môi trường*\n🌡 T: " + String(t, 1) + "°C | 💧 H: " + String(h, 1) + "%\n💨 Gas: " + String(gas);
        if (t >= TEMP_ALERT_HIGH) alert += "\n⚠️ NHIỆT ĐỘ CAO!";
        bot.sendMessage(CHAT_ID, alert, "Markdown");
        last_t = t; last_h = h;
    }

    if (gas >= GAS_THRESHOLD && !gasAlerted) {
        bot.sendMessage(CHAT_ID, "🚨 *CẢNH BÁO KHÍ GAS!*\nMức: " + String(gas), "Markdown");
        gasAlerted = true;
    }
    if (gas < GAS_THRESHOLD - 50) gasAlerted = false;

    updateOLED();
}

void checkLedTimer() {
    if (ledTimerActive && millis() >= ledOffTime) {
        digitalWrite(LED_PIN, HIGH); // Tắt LED
        ledTimerActive = false;
        bot.sendMessage(CHAT_ID, "⏰ Hẹn giờ kết thúc — Đèn LED đã TẮT.", "");
    }
}

/* ================================================================
 * MODULE: TELEGRAM HANDLER
 * ================================================================ */
void handleTelegram(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        text.trim();

        if (text.startsWith("/led_on")) {
            digitalWrite(LED_PIN, LOW);
            ledTimerActive = false;
            String reply = "💡 Đèn LED đã *BẬT* ✅";
            if (text.length() > 7) {
                int minutes = text.substring(8).toInt();
                if (minutes > 0 && minutes <= LED_TIMER_MAX_MIN) {
                    ledOffTime = millis() + (unsigned long)minutes * 60000UL;
                    ledTimerActive = true;
                    reply += "\n⏱ Tắt sau *" + String(minutes) + " phút*";
                }
            }
            bot.sendMessage(chat_id, reply, "Markdown");
        }
        else if (text == "/led_off") {
            digitalWrite(LED_PIN, HIGH);
            ledTimerActive = false;
            bot.sendMessage(chat_id, "💡 Đèn LED đã *TẮT* ❌", "Markdown");
        }
        else if (text == "/status") {
            bool ledOn = (digitalRead(LED_PIN) == LOW);
            String msg = "📊 *TRẠNG THÁI*\n🌡 T: " + String(t, 1) + "°C\n💧 H: " + String(h, 1) + "%\n💨 G: " + String(gas) + "\n💡 LED: " + (ledOn ? "BẬT" : "TẮT");
            bot.sendMessage(chat_id, msg, "Markdown");
        }
        else if (text == "/log") {
            bot.sendMessage(chat_id, buildLogSummary(), "Markdown");
        }
        else if (text == "/help" || text == "/start") {
            bot.sendMessage(chat_id, "🤖 *Lệnh:*\n/status, /log, /led_on, /led_off, /led_on [phút]", "Markdown");
        }
    }
}

/* ================================================================
 * SETUP & LOOP
 * ================================================================ */
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    u8g2.begin();
    dht.begin();

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

    client.setInsecure();
    bot.sendMessage(CHAT_ID, "✅ *Hệ thống IoT v2 Online (No Blynk)*", "Markdown");
}

void loop() {
    // Đọc cảm biến mỗi 3 giây
    if (millis() - lastSensorRead > 3000) {
        readSensors();
        lastSensorRead = millis();
    }

    checkLedTimer();

    // Kiểm tra Telegram mỗi 1 giây
    if (millis() - lastBotCheck > 1000) {
        int n = bot.getUpdates(bot.last_message_received + 1);
        if (n > 0) handleTelegram(n);
        lastBotCheck = millis();
    }
}