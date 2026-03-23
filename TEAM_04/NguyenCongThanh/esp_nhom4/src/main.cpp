#include <Arduino.h>

/* ================================================================
 *  CẤU HÌNH BLYNK
 * ================================================================ */
// #define BLYNK_TEMPLATE_ID   "TMPL6AoscMprl"
// #define BLYNK_TEMPLATE_NAME "ESP8266 LYNK TELEGRAM"
// #define BLYNK_AUTH_TOKEN    "doYo230uoTcCcEbp0-BoBFKeCuP5d8sq"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

/* ================================================================
 *  CẤU HÌNH MẠNG & TELEGRAM
 * ================================================================ */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BOTtoken  "8629442956:AAF8YBmH05p94b6TYRJh29mo_Fpgvs4pPuI"
#define CHAT_ID   "6839585914"

/* ================================================================
 *  ĐỊNH NGHĨA CHÂN & KIỂU CẢM BIẾN
 * ================================================================ */
#define DHTPIN    D3
#define DHTTYPE   DHT22
#define LED_PIN   D4
#define MQ2_PIN   A0

/* ================================================================
 *  NGƯỠNG CẢNH BÁO
 * ================================================================ */
#define GAS_THRESHOLD     600     // Ngưỡng Gas nguy hiểm (ADC 0-1023)
#define TEMP_ALERT_HIGH   35.0f   // Nhiệt độ cao bất thường (°C)
#define TEMP_DELTA        0.5f    // Độ lệch nhiệt độ để thông báo
#define HUM_DELTA         2.0f    // Độ lệch độ ẩm để thông báo

/* ================================================================
 *  THÔNG SỐ HẸN GIỜ LED
 * ================================================================ */
#define LED_TIMER_MAX_MIN  60     // Hẹn giờ tối đa (phút)

/* ================================================================
 *  CẤU TRÚC DỮ LIỆU LOG
 * ================================================================ */
#define LOG_SIZE 10

struct SensorLog {
    float temp;
    float hum;
    int   gas;
    unsigned long timestamp; // millis()
};

/* ================================================================
 *  BIẾN TOÀN CỤC
 * ================================================================ */
// --- Đối tượng phần cứng ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DHT           dht(DHTPIN, DHTTYPE);
BlynkTimer    timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// --- Dữ liệu cảm biến ---
float t = 0, h = 0;
int   gas = 0;

// --- Theo dõi thay đổi ---
float last_t = 0, last_h = 0;
bool  gasAlerted = false;   // Tránh spam cảnh báo gas

// --- Hẹn giờ LED ---
bool           ledTimerActive = false;
unsigned long  ledOffTime     = 0;   // millis() lúc cần tắt LED

// --- Polling Telegram ---
unsigned long lastBotCheck = 0;

// --- Log vòng tròn ---
SensorLog sensorLog[LOG_SIZE];
int       logIndex = 0;
int       logCount = 0;

/* ================================================================
 *  MODULE: TIỆN ÍCH
 * ================================================================ */

// Trả về chuỗi thời gian hoạt động "HH:MM:SS"
String getUptime() {
    unsigned long sec = millis() / 1000;
    char buf[12];
    sprintf(buf, "%02lu:%02lu:%02lu", sec / 3600, (sec % 3600) / 60, sec % 60);
    return String(buf);
}

// Trả về timestamp dạng "[XhYm]" từ millis
String fmtTimestamp(unsigned long ms) {
    unsigned long sec = ms / 1000;
    char buf[12];
    sprintf(buf, "[%luh%lum]", sec / 3600, (sec % 3600) / 60);
    return String(buf);
}

/* ================================================================
 *  MODULE: OLED
 * ================================================================ */
void updateOLED() {
    u8g2.clearBuffer();

    // --- Tiêu đề ---
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0, 7, "=== IoT MONITOR v2 ===");

    // --- Dữ liệu cảm biến ---
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(0, 19);
    u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");

    u8g2.setCursor(0, 31);
    u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");

    // --- Gas: hiển thị thanh cảnh báo ---
    u8g2.setCursor(0, 43);
    u8g2.print("Gas : "); u8g2.print(gas);
    if (gas >= GAS_THRESHOLD) {
        u8g2.drawStr(72, 43, "<!ALERT>");
    }

    // --- LED + Hẹn giờ ---
    u8g2.setCursor(0, 55);
    bool ledOn = (digitalRead(LED_PIN) == LOW);
    u8g2.print("LED : ");
    u8g2.print(ledOn ? "ON" : "OFF");
    if (ledTimerActive) {
        unsigned long remain = (ledOffTime - millis()) / 1000;
        u8g2.print(" ["); u8g2.print(remain); u8g2.print("s]");
    }

    // --- Footer: uptime + team ---
    u8g2.setFont(u8g2_font_4x6_tf);
    char footer[32];
    sprintf(footer, "Up:%s  Team12-IoT", getUptime().c_str());
    u8g2.drawStr(0, 64, footer);

    u8g2.sendBuffer();
}

/* ================================================================
 *  MODULE: LOG DỮ LIỆU
 * ================================================================ */
void pushLog(float _t, float _h, int _gas) {
    sensorLog[logIndex] = { _t, _h, _gas, millis() };
    logIndex = (logIndex + 1) % LOG_SIZE;
    if (logCount < LOG_SIZE) logCount++;
}

// Tạo chuỗi tóm tắt log gần nhất gửi Telegram
String buildLogSummary() {
    if (logCount == 0) return "Chưa có dữ liệu log.";
    String msg = "📋 *Log " + String(logCount) + " mẫu gần nhất:*\n";
    // Duyệt từ cũ đến mới
    for (int i = 0; i < logCount; i++) {
        int idx = (logIndex - logCount + i + LOG_SIZE) % LOG_SIZE;
        SensorLog& e = sensorLog[idx];
        msg += fmtTimestamp(e.timestamp)
             + " T:" + String(e.temp, 1)
             + " H:" + String(e.hum, 1)
             + " G:" + String(e.gas) + "\n";
    }
    return msg;
}

/* ================================================================
 *  MODULE: CẢM BIẾN & BLYNK
 * ================================================================ */
void sendSensorData() {
    float new_h = dht.readHumidity();
    float new_t = dht.readTemperature();

    if (isnan(new_h) || isnan(new_t)) {
        Serial.println("[DHT] Lỗi đọc cảm biến!");
        return;
    }

    t   = new_t;
    h   = new_h;
    gas = analogRead(MQ2_PIN);
    // Giả lập nếu Wokwi / không có MQ2 thật
    if (gas < 50) gas = random(180, 480);

    // Ghi log
    pushLog(t, h, gas);

    // Đẩy lên Blynk
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
    Blynk.virtualWrite(V3, gas);
    Blynk.virtualWrite(V4, getUptime());

    // --- Cảnh báo thay đổi nhiệt độ / độ ẩm ---
    if (abs(t - last_t) >= TEMP_DELTA || abs(h - last_h) >= HUM_DELTA) {
        String alert = "🔔 *Cập nhật môi trường*\n"
                       "🌡 Nhiệt độ : " + String(t, 1) + " °C\n"
                       "💧 Độ ẩm    : " + String(h, 1) + " %\n"
                       "💨 Gas      : " + String(gas);
        // Thêm cảnh báo nhiệt độ cao
        if (t >= TEMP_ALERT_HIGH) alert += "\n⚠️ NHIỆT ĐỘ CAO BẤT THƯỜNG!";
        bot.sendMessage(CHAT_ID, alert, "Markdown");
        last_t = t;
        last_h = h;
    }

    // --- Cảnh báo Gas ---
    if (gas >= GAS_THRESHOLD && !gasAlerted) {
        String gasMsg = "🚨 *CẢNH BÁO KHÍ GAS!*\n"
                        "📊 Mức Gas: " + String(gas) + " / 1023\n"
                        "📍 Vượt ngưỡng: " + String(GAS_THRESHOLD) + "\n"
                        "➡️ Kiểm tra khu vực ngay!";
        bot.sendMessage(CHAT_ID, gasMsg, "Markdown");
        gasAlerted = true;
    }
    // Reset cờ khi gas giảm về an toàn
    if (gas < GAS_THRESHOLD - 50) gasAlerted = false;

    updateOLED();
}

/* ================================================================
 *  MODULE: HẸN GIỜ LED
 * ================================================================ */
void checkLedTimer() {
    if (ledTimerActive && millis() >= ledOffTime) {
        digitalWrite(LED_PIN, HIGH);   // Tắt LED
        ledTimerActive = false;
        Blynk.virtualWrite(V0, 0);
        bot.sendMessage(CHAT_ID, "⏰ Hẹn giờ kết thúc — Đèn LED đã TẮT tự động.", "");
    }
}

/* ================================================================
 *  MODULE: TELEGRAM HANDLER
 * ================================================================ */
void handleTelegram(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text    = bot.messages[i].text;
        text.trim();

        Serial.println("[TG] Lệnh: " + text);

        /* ---- /led_on [phút] ---- */
        if (text.startsWith("/led_on")) {
            digitalWrite(LED_PIN, LOW);
            Blynk.virtualWrite(V0, 1);
            ledTimerActive = false;

            // Kiểm tra có tham số hẹn giờ không: "/led_on 5"
            String reply = "💡 Đèn LED đã *BẬT* ✅";
            if (text.length() > 7) {
                int minutes = text.substring(8).toInt();
                if (minutes > 0 && minutes <= LED_TIMER_MAX_MIN) {
                    ledOffTime     = millis() + (unsigned long)minutes * 60000UL;
                    ledTimerActive = true;
                    reply += "\n⏱ Hẹn giờ TẮT sau *" + String(minutes) + " phút*";
                } else {
                    reply += "\n⚠️ Hẹn giờ không hợp lệ (1–" + String(LED_TIMER_MAX_MIN) + " phút)";
                }
            }
            bot.sendMessage(chat_id, reply, "Markdown");
        }

        /* ---- /led_off ---- */
        else if (text == "/led_off") {
            digitalWrite(LED_PIN, HIGH);
            ledTimerActive = false;
            Blynk.virtualWrite(V0, 0);
            bot.sendMessage(chat_id, "💡 Đèn LED đã *TẮT* ❌", "Markdown");
        }

        /* ---- /led_status ---- */
        else if (text == "/led_status") {
            bool ledOn = (digitalRead(LED_PIN) == LOW);
            String msg = "💡 LED: *" + String(ledOn ? "ĐANG BẬT" : "ĐANG TẮT") + "*";
            if (ledTimerActive) {
                unsigned long remain = (ledOffTime - millis()) / 1000;
                msg += "\n⏱ Tự tắt sau: *" + String(remain) + " giây*";
            }
            bot.sendMessage(chat_id, msg, "Markdown");
        }

        /* ---- /get_weather ---- */
        else if (text == "/get_weather") {
            String msg = "🌡 *Thời tiết hiện tại*\n"
                         "Nhiệt độ : *" + String(t, 1) + " °C*\n"
                         "Độ ẩm    : *" + String(h, 1) + " %*\n"
                         "Gas      : *" + String(gas) + "*";
            if (gas >= GAS_THRESHOLD) msg += "\n🚨 GAS NGUY HIỂM!";
            bot.sendMessage(chat_id, msg, "Markdown");
        }

        /* ---- /status (tổng hợp) ---- */
        else if (text == "/status") {
            bool ledOn = (digitalRead(LED_PIN) == LOW);
            String msg = "📊 *TRẠNG THÁI HỆ THỐNG*\n"
                         "─────────────────\n"
                         "🌡 Nhiệt độ : " + String(t, 1)  + " °C\n"
                         "💧 Độ ẩm    : " + String(h, 1)  + " %\n"
                         "💨 Gas      : " + String(gas)   + "\n"
                         "💡 LED      : " + (ledOn ? "BẬT" : "TẮT") + "\n"
                         "⏱ Uptime   : " + getUptime()   + "\n"
                         "─────────────────\n"
                         "🌐 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm";
            bot.sendMessage(chat_id, msg, "Markdown");
        }

        /* ---- /log ---- */
        else if (text == "/log") {
            bot.sendMessage(chat_id, buildLogSummary(), "Markdown");
        }

        /* ---- /help ---- */
        else if (text == "/help" || text == "/start") {
            String help = "🤖 *Danh sách lệnh hỗ trợ:*\n\n"
                          "/get\\_weather — Nhiệt độ & độ ẩm\n"
                          "/led\\_on — Bật LED\n"
                          "/led\\_on 5 — Bật LED, tắt sau 5 phút\n"
                          "/led\\_off — Tắt LED\n"
                          "/led\\_status — Trạng thái LED\n"
                          "/status — Tổng hợp hệ thống\n"
                          "/log — Xem log 10 mẫu gần nhất\n"
                          "/help — Trợ giúp";
            bot.sendMessage(chat_id, help, "Markdown");
        }

        /* ---- Lệnh không hợp lệ ---- */
        else {
            bot.sendMessage(chat_id, "❓ Lệnh không hợp lệ. Gõ /help để xem danh sách.", "");
        }
    }
}

/* ================================================================
 *  BLYNK: Nút bật/tắt LED từ app
 * ================================================================ */
BLYNK_WRITE(V0) {
    int value = param.asInt();
    digitalWrite(LED_PIN, value ? LOW : HIGH);
    ledTimerActive = false; // Huỷ hẹn giờ nếu dùng app
}

/* ================================================================
 *  SETUP
 * ================================================================ */
void setup() {
    Serial.begin(115200);
    Serial.println("\n[BOOT] Khởi động hệ thống IoT Monitor v2...");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Tắt LED mặc định

    u8g2.begin();
    dht.begin();

    // Hiển thị màn hình khởi động
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 25, "IoT Monitor v2");
    u8g2.drawStr(15, 40, "Dang khoi dong...");
    u8g2.sendBuffer();

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    client.setInsecure(); // Bỏ qua xác thực SSL cho Telegram

    // Đăng ký timer: đọc cảm biến mỗi 3 giây, kiểm tra hẹn giờ mỗi 500ms
    timer.setInterval(3000L,  sendSensorData);
    timer.setInterval(500L,   checkLedTimer);

    Serial.println("[BOOT] Hoàn tất! Bot sẵn sàng.");
    bot.sendMessage(CHAT_ID,
        "✅ *Hệ thống IoT Monitor v2 đã online!*\nGõ /help để xem lệnh.",
        "Markdown");
}

/* ================================================================
 *  LOOP
 * ================================================================ */
void loop() {
    Blynk.run();
    timer.run();

    // Kiểm tra tin nhắn Telegram mỗi 1 giây
    if (millis() - lastBotCheck > 1000) {
        int n = bot.getUpdates(bot.last_message_received + 1);
        while (n) {
            handleTelegram(n);
            n = bot.getUpdates(bot.last_message_received + 1);
        }
        lastBotCheck = millis();
    }
}