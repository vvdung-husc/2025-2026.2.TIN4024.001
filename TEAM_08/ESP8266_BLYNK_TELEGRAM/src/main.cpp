/*
THÔNG TIN NHÓM 08
1. Nguyễn Ngọc Thành Tài
2. [Điền tên thành viên 2]
3. [Điền tên thành viên 3]
*/

// --- THÔNG SỐ BLYNK ---
#define BLYNK_TEMPLATE_ID "TMPL_CUA_BAN"
#define BLYNK_TEMPLATE_NAME "Team 08 Project"
#define BLYNK_AUTH_TOKEN "TOKEN_BLYNK_CUA_BAN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- THÔNG SỐ WIFI & TELEGRAM ---
const char* ssid = "TEN_WIFI_NHA_BAN";
const char* password = "MAT_KHAU_WIFI";

#define BOT_TOKEN "TOKEN_TELEGRAM_BOT_CUA_BAN"
#define GROUP_ID "-5108587371"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH PHẦN CỨNG ---
#define DHTPIN D3       // Cảm biến DHT
#define DHTTYPE DHT11   // Đổi thành DHT22 nếu cần
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN D5      // Chân đèn LED (Đổi thành D4 nếu dùng LED tích hợp)
#define MQ2_PIN A0      // Chân Analog đọc khí Gas

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
BlynkTimer timer;

// Biến lưu trữ trạng thái
float lastTemp = -999.0;
float lastHum = -999.0;
bool ledState = false;

// 1. NHẬN LỆNH ĐIỀU KHIỂN LED TỪ BLYNK (NÚT V1)
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  bot.sendMessage(GROUP_ID, ledState ? "Đèn LED vừa được BẬT từ Blynk!" : "Đèn LED vừa được TẮT từ Blynk!");
}

// 2. XỬ LÝ LỆNH TỪ TELEGRAM
void handleTelegramMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/led_on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1); // Đồng bộ trạng thái lên nút Blynk
      bot.sendMessage(chat_id, "🟢 Đèn LED đã được BẬT.", "");
    } 
    else if (text == "/led_off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0); 
      bot.sendMessage(chat_id, "🔴 Đèn LED đã được TẮT.", "");
    } 
    else if (text == "/led_status") {
      bot.sendMessage(chat_id, ledState ? "Trạng thái: Đèn đang BẬT 🟢" : "Trạng thái: Đèn đang TẮT 🔴", "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "🌤 Thời tiết hiện tại:\n- Nhiệt độ: " + String(t, 1) + "°C\n- Độ ẩm: " + String(h, 1) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// 3. ĐỌC CẢM BIẾN, CẬP NHẬT OLED & GỬI LÊN BLYNK
void sensorRoutine() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Xử lý khí Gas MQ2 (Sinh ngẫu nhiên nếu chân A0 không cắm mạch)
  int gasValue = analogRead(MQ2_PIN);
  if (gasValue < 10) gasValue = random(100, 300); // Sinh ngẫu nhiên theo yêu cầu đề bài
  
  // Gửi lên Blynk
  Blynk.virtualWrite(V0, millis() / 1000); // Uptime
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, "Team 08");

  // Hiển thị lên OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  if (isnan(h) || isnan(t)) {
    u8g2.drawStr(10, 30, "DHT Sensor Error!");
  } else {
    u8g2.setCursor(0, 12); u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");
    u8g2.setCursor(0, 26); u8g2.print("Hum:  "); u8g2.print(h, 1); u8g2.print(" %");
    u8g2.setCursor(0, 40); u8g2.print("Gas:  "); u8g2.print(gasValue);
    u8g2.setCursor(0, 54); u8g2.print(ledState ? "LED: ON" : "LED: OFF");
    u8g2.setCursor(40, 64); u8g2.print("TEAM 08");
  }
  u8g2.sendBuffer();

  // Báo cáo Telegram nếu nhiệt độ/độ ẩm thay đổi
  if (!isnan(t) && !isnan(h)) {
    if (abs(t - lastTemp) > 1.0 || abs(h - lastHum) > 2.0) { // Chênh lệch 1 độ hoặc 2% độ ẩm
      if (lastTemp != -999.0) { // Không báo lúc mới khởi động
        String alert = "⚠️ Cảnh báo thay đổi môi trường:\n- Nhiệt độ mới: " + String(t, 1) + "°C\n- Độ ẩm mới: " + String(h, 1) + "%";
        bot.sendMessage(GROUP_ID, alert, "");
      }
      lastTemp = t;
      lastHum = h;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Khởi tạo phần cứng
  dht.begin();
  u8g2.begin();

  // Kết nối WiFi & Blynk
  configTime(0, 0, "pool.ntp.org"); // Cần thiết cho Telegram ESP8266
  secured_client.setTrustAnchors(&cert); 
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Thiết lập Timer chạy hàm sensorRoutine mỗi 2 giây
  timer.setInterval(2000L, sensorRoutine);
  
  bot.sendMessage(GROUP_ID, "🚀 Thiết bị ESP8266 của Team 08 đã khởi động thành công!");
}

void loop() {
  Blynk.run();
  timer.run();

  // Quét tin nhắn Telegram mỗi giây (Tránh dùng delay gây đơ Blynk)
  static unsigned long lastBotCheck = 0;
  if (millis() - lastBotCheck > 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotCheck = millis();
  }
}