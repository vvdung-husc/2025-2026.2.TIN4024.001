/*
THÔNG TIN NHÓM X
1. Trần Hữu Bảo Anh
2. Lương Gia Mẫn
3. Nguyễn Trung Quân
*/

#define BLYNK_TEMPLATE_ID "TMPL60f44c9Ug"
#define BLYNK_TEMPLATE_NAME "TEAM3ESP8266BLYNKTELEGRAM"
#define BLYNK_AUTH_TOKEN "3MdKdoIvVZG08QCZTKddpVssBgPR7fnh"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>

// --- THÔNG TIN KẾT NỐI ---
char ssid[] = "CNTT-MMT"; 
char pass[] = "13572468";
#define BOTtoken "8741572482:AAHqrH7NIV7Eg9A51THsuuCbqV3T_VOKF1w"
#define CHAT_ID "-5274752582" 

// --- CẤU HÌNH LINH KIỆN ---
#define DHTPIN 2      // D4
#define DHTTYPE DHT11 
#define LED_PIN 14    // D5
#define MQ2_PIN A0    // Chân Analog đo khí Gas

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

// Biến lưu trữ
float lastTemp = 0, lastHumi = 0;
bool ledState = LOW;

// --- HÀM XỬ LÝ TELEGRAM ---
void handleTelegramMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue;

    String text = bot.messages[i].text;
    if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V4, HIGH); // Cập nhật Switch trên Blynk
      bot.sendMessage(chat_id, "Đèn LED đã BẬT ", "");
    } 
    else if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V4, LOW); // Cập nhật Switch trên Blynk
      bot.sendMessage(chat_id, "Đèn LED đã TẮT ", "");
    } 
    else if (text == "/led_status") {
      String status = (digitalRead(LED_PIN)) ? "Đèn đang BẬT " : "Đèn đang TẮT ";
      bot.sendMessage(chat_id, status, "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "Thời tiết hiện tại:\n🌡 Nhiệt độ: " + String(t) + "°C\n Độ ẩm: " + String(h) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// --- HÀM GỬI DỮ LIỆU ĐỊNH KỲ ---
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gasValue = analogRead(MQ2_PIN); // Đọc MQ2 từ kit

  if (isnan(h) || isnan(t)) return;

  // 1. Cập nhật Blynk
  Blynk.virtualWrite(V0, millis() / 1000); // Uptime
  Blynk.virtualWrite(V1, t);               // Nhiệt độ
  Blynk.virtualWrite(V2, h);               // Độ ẩm
  Blynk.virtualWrite(V3, gasValue);        // Khí Gas
  Blynk.virtualWrite(V5, "Team 03 - Anh, Mẫn, Quân");

  // 2. Cập nhật OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.printf("T: %.1fC | H: %.1f%%", t, h);
  display.setCursor(0, 20);
  display.printf("Gas: %d", gasValue);
  display.setCursor(0, 45);
  display.print("Team 03");
  display.display();

  // 3. Telegram: Tự động gửi khi nhiệt độ/độ ẩm thay đổi
  if (abs(t - lastTemp) > 1.0 || abs(h - lastHumi) > 2.0) {
    String alert = "Cập nhật thay đổi:\nNhiệt độ: " + String(t) + "°C\nĐộ ẩm: " + String(h) + "%";
    bot.sendMessage(CHAT_ID, alert, "");
    lastTemp = t;
    lastHumi = h;
  }
}

// Điều khiển LED từ Blynk Switch
BLYNK_WRITE(V4) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  String msg = ledState ? "LED bật (từ Blynk)" : "LED tắt (từ Blynk)";
  bot.sendMessage(CHAT_ID, msg, "");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  Wire.begin(4, 5); // D2, D1 cho OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);

  WiFi.begin(ssid, pass);
  client.setInsecure(); // Cần thiết cho Telegram trên ESP8266
  client.setBufferSizes(512, 512);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendSensorData);
  
  Serial.println("System Ready!");
}

void loop() {
  Blynk.run();
  timer.run();

  static unsigned long lastBotTime;
  // Tăng thời gian kiểm tra lên 3 giây để giảm tải cho chip
  if (millis() - lastBotTime > 3000) { 
    // Serial.println("Checking Telegram..."); // Bỏ comment nếu muốn theo dõi log
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotTime = millis();
  }
}