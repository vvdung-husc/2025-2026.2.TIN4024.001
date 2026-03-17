/*
	THÔNG TIN NHÓM 11
	1. Đào Văn Lợi
	2. 
	3.
	*/
#define BLYNK_TEMPLATE_ID "TMPL6g5wadXBg"
#define BLYNK_TEMPLATE_NAME "iOTTeam11"
#define BLYNK_AUTH_TOKEN "JuA3odqyzr2x0D5TgFWWUV1OHdNbR5tz"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// --- CẤU HÌNH HỆ THỐNG - ĐÀO VĂN LỢI - HUE ---
#define LED_BOARD 2    // D4 (GPIO2)
#define DHTPIN 0       // D3 (GPIO0)
#define PIRPIN 4       // D2 (GPIO4)
#define GASPIN A0      // A0
#define DHTTYPE DHT22

// Cấu hình mạng và Telegram
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "loi"; 
char pass[] = "11111111";
#define BOTtoken "8482351137:AAG3HGrN3ofuROEq9GzZgOMKYjh4nTKgft8"
#define GROUP_ID "-5204265134" 

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

float t = 0, h = 0;
int gasValue = 0;
int motion = 0;
bool ledStatus = false;
unsigned long lastTimeBotRan;

// --- 1. Hiển thị OLED ---
void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 10, "DAO VAN LOI - HUE"); 
  
  u8g2.setCursor(0, 24); u8g2.print("Temp: "); u8g2.print(t, 1); u8g2.print(" C");
  u8g2.setCursor(0, 34); u8g2.print("Humi: "); u8g2.print(h, 1); u8g2.print(" %");
  u8g2.setCursor(0, 44); u8g2.print("GAS : "); u8g2.print(gasValue);
  u8g2.setCursor(0, 54); u8g2.print("PIR : "); u8g2.print(motion ? "DETECTED!" : "Safe");
  
  u8g2.setCursor(0, 64);
  u8g2.print("LED Status: "); u8g2.print(ledStatus ? "ON" : "OFF");
  u8g2.sendBuffer();
}

// --- 2. Xử lý Telegram (4 Lệnh chính) ---
void handleTelegramMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; 

    String text = bot.messages[i].text;

    if (text == "/led_on") {
      ledStatus = true;
      digitalWrite(LED_BOARD, LOW); // Bật LED (logic âm)
      Blynk.virtualWrite(V3, 1);
      bot.sendMessage(GROUP_ID, "Lợi thông báo: Đèn đã BẬT ✅", "");
    } 
    else if (text == "/led_off") {
      ledStatus = false;
      digitalWrite(LED_BOARD, HIGH); // Tắt LED
      Blynk.virtualWrite(V3, 0);
      bot.sendMessage(GROUP_ID, "Lợi thông báo: Đèn đã TẮT 🌑", "");
    }
    else if (text == "/get_weather") {
      String msg = "🌤 Thời tiết (Đào Văn Lợi):\n🌡 Nhiệt độ: " + String(t, 1) + "°C\n💧 Độ ẩm: " + String(h, 1) + "%";
      bot.sendMessage(GROUP_ID, msg, "");
    }
    else if (text == "/status") {
      String msg = "Hệ thống Đào Văn Lợi:\n";
      msg += "🌡 Nhiệt độ: " + String(t, 1) + "°C\n";
      msg += "💧 Độ ẩm: " + String(h, 1) + "%\n";
      msg += "💨 Gas: " + String(gasValue) + "\n";
      msg += "🏃 Chuyển động: " + String(motion ? "CÓ NGƯỜI!" : "An toàn") + "\n";
      msg += "💡 Đèn LED: " + String(ledStatus ? "Đang BẬT" : "Đang TẮT");
      bot.sendMessage(GROUP_ID, msg, "");
    }
  }
}

// --- 3. Đọc cảm biến & Cảnh báo PIR ---
void sensorJob() {
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  if (!isnan(newT) && !isnan(newH)) {
    t = newT;
    h = newH;
  }
  gasValue = analogRead(GASPIN);
  int currentMotion = digitalRead(PIRPIN);

  // Đẩy dữ liệu lên Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, gasValue);
  Blynk.virtualWrite(V4, currentMotion ? 255 : 0);

  // Tự động báo Telegram khi phát hiện người
  if (currentMotion == HIGH && motion == LOW) {
    bot.sendMessage(GROUP_ID, "⚠️ CẢNH BÁO: Phát hiện người xâm nhập! (Đào Văn Lợi)", "");
  }
  motion = currentMotion;

  // In ra Serial Monitor để kiểm tra
  Serial.printf("T:%.1f H:%.1f Gas:%d PIR:%d LED:%d\n", t, h, gasValue, motion, ledStatus);
}

// --- 4. Đồng bộ từ App Blynk ---
BLYNK_WRITE(V3) {
  ledStatus = param.asInt();
  digitalWrite(LED_BOARD, ledStatus ? LOW : HIGH);
  bot.sendMessage(GROUP_ID, ledStatus ? "Đèn vừa BẬT từ Blynk ✅" : "Đèn vừa TẮT từ Blynk 🌑", "");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BOARD, OUTPUT);
  digitalWrite(LED_BOARD, HIGH); // Mặc định tắt
  pinMode(PIRPIN, INPUT);
  
  dht.begin();
  u8g2.begin();
  
  Blynk.begin(auth, ssid, pass);
  client.setInsecure(); // Cần thiết cho Telegram Bot
  
  timer.setInterval(2000L, sensorJob); // Đọc cảm biến mỗi 2s
  timer.setInterval(1000L, updateOLED); // Cập nhật OLED mỗi 1s
  
  Serial.println("\n--- HE THONG DAO VAN LOI READY ---");
}

void loop() {
  Blynk.run();
  timer.run();

  // Kiểm tra tin nhắn Telegram mỗi 3 giây
  if (millis() - lastTimeBotRan > 3000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}