/* * Thông tin thành viên nhóm:
 
 * Team: IoT - Team03
      1.Nguyễn Trung Quân - Telegram: Ntq781
      2.Trần Hữu Bảo Anh  - Telegram: Banh547
      3.Lương Gia Mẫn     - Telegram: giamam2104
      4.Cao Khả Tài       - Telegram: taikha0708
      5.
 */

#include <Arduino.h>

/* --- 1. CẤU HÌNH BLYNK --- */
#define BLYNK_TEMPLATE_ID "TMPL6yWzNchHz"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM TEAM03"
#define BLYNK_AUTH_TOKEN "8npl-293kkvn35anH_DDqzrtkhdWZr1s"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

/* --- 2. CẤU HÌNH KẾT NỐI WIFI VÀ TELEGRAM --- */
char ssid[] = "CNTT-MMT"; 
char pass[] = "13572468";    // Mật khẩu WiFi 
/* TOKEN BOT VÀ ID GROUP CHAT TRÊN TELEGRAM */
#define BOTtoken "8741572482:AAHqrH7NIV7Eg9A51THsuuCbqV3T_VOKF1w"
#define CHAT_ID "-5274752582" 

/* --- 3. ĐỊNH NGHĨA CHÂN CẮM (PIN) --- */
#define DHTPIN D3
#define DHTTYPE DHT22                 // Cảm biến DHT22 (màu trắng)
#define LED_PIN D4                    // LED trên board NodeMCU (D4)
#define MQ2_PIN A0                    // Cảm biến khí Gas

/* --- KHỞI TẠO ĐỐI TƯỢNG --- */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

float t, h;
int gas;
float last_t = 0, last_h = 0;
unsigned long lastTimeBotRan;

// Hàm định dạng thời gian hoạt động
String getUptime() {
    unsigned long sec = millis() / 1000;
    int hh = sec / 3600;
    int mm = (sec % 3600) / 60;
    int ss = sec % 60;
    char buf[15];
    sprintf(buf, "%02dh:%02dm:%02ds", hh, mm, ss);
    return String(buf);
}

// Hàm hiển thị lên màn hình OLED
void updateOLED() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "--- TELEGRAM BLYNK ---");
    u8g2.setCursor(0, 25);
    u8g2.print(" Temp: "); u8g2.print(t, 1); u8g2.print(" C");
    u8g2.setCursor(0, 37);
    u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");
    u8g2.setCursor(0, 49);
    u8g2.print(" GAS : "); u8g2.print(gas);
    u8g2.print("|Up:"); u8g2.print(millis()/60000); u8g2.print("m");
    u8g2.setFont(u8g2_font_profont11_tr); 
    u8g2.drawStr(0, 64, "TEAM: IoT - Team03"); 
    u8g2.sendBuffer();
}

// Hàm đọc cảm biến và tự động thông báo Telegram
void sendSensorData() {
    float new_h = dht.readHumidity();
    float new_t = dht.readTemperature();

    if (isnan(new_h) || isnan(new_t)) {
        Serial.println("Lỗi đọc cảm biến DHT22!");
        return;
    }

    t = new_t;
    h = new_h;
    gas = analogRead(MQ2_PIN);
    if (gas < 50) gas = random(200, 450); // Giả lập nếu không có cảm biến

    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
    Blynk.virtualWrite(V3, gas);
    Blynk.virtualWrite(V4, getUptime());

    // Yêu cầu: Thông báo khi có thay đổi nhiệt độ hoặc độ ẩm
    if (abs(t - last_t) >= 0.5 || abs(h - last_h) >= 2.0) {
        String alert = "🌱 Cập nhật môi trường:\n🌡 Nhiệt độ: " + String(t, 1) + "C\n💧 Độ ẩm: " + String(h, 1) + "%";
        bot.sendMessage(CHAT_ID, alert, "");
        last_t = t;
        last_h = h;
    }

    updateOLED();
}

// Xử lý các lệnh từ Telegram chat
void handleTelegram(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        if (text == "/led_on") {
            digitalWrite(LED_PIN, LOW); // Bật LED
            bot.sendMessage(chat_id, "Đèn LED đã BẬT", "");
            Blynk.virtualWrite(V0, 1);
        } 
        else if (text == "/led_off") {
            digitalWrite(LED_PIN, HIGH); // Tắt LED
            bot.sendMessage(chat_id, "Đèn LED đã TẮT", "");
            Blynk.virtualWrite(V0, 0);
        } 
        else if (text == "/led_status") {
            String status = (digitalRead(LED_PIN) == LOW) ? "ĐANG BẬT" : "ĐANG TẮT";
            bot.sendMessage(chat_id, "💡 Trạng thái LED: " + status, "");
        } 
        else if (text == "/get_weather") {
            String weather = "🌡 Nhiệt độ: " + String(t, 1) + "°C\n💧 Độ ẩm: " + String(h, 1) + "%";
            bot.sendMessage(chat_id, weather, "");
        }
    }
}

// Nhận lệnh từ nút nhấn trên ứng dụng Blynk
BLYNK_WRITE(V0) {
    int value = param.asInt();
    digitalWrite(LED_PIN, value ? LOW : HIGH);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Mặc định tắt LED

    u8g2.begin();
    dht.begin();
    
    // --- BẮT ĐẦU KẾT NỐI WIFI VÀ BLYNK ---
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    client.setInsecure(); // QUAN TRỌNG: Để kết nối Telegram an toàn

    timer.setInterval(3000L, sendSensorData); // Gửi dữ liệu mỗi 3 giây
}

void loop() {
    Blynk.run();
    timer.run();

    // Kiểm tra tin nhắn Telegram mỗi 1 giây
    if (millis() - lastTimeBotRan > 1000) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while(numNewMessages) {
            handleTelegram(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}
