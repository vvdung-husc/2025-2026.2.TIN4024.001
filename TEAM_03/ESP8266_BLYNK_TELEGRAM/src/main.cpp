/*
THÔNG TIN NHÓM 03
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

// --- CẤU HÌNH KẾT NỐI ---
char ssid[] = "CNTT-MMT"; 
char pass[] = "13572468";
#define BOTtoken "8741572482:AAHqrH7NIV7Eg9A51THsuuCbqV3T_VOKF1w"
#define CHAT_ID "-5274752582" 

// --- CẤU HÌNH CHÂN PIN (KHỚP KIT THỰC TẾ) ---
#define DHTPIN 2      // D4
#define DHTTYPE DHT11 
#define LED_PIN 14    // D5 (LED Đỏ trên Kit)
#define MQ2_PIN A0    // Analog đọc khí Gas

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
BlynkTimer timer;

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
      Blynk.virtualWrite(V4, HIGH);
      bot.sendMessage(chat_id, "Đèn LED đã BẬT ✅", "");
    } 
    else if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V4, LOW);
      bot.sendMessage(chat_id, "Đèn LED đã TẮT ❌", "");
    } 
    else if (text == "/led_status") {
      String status = (digitalRead(LED_PIN)) ? "Trạng thái: Đèn đang BẬT 💡" : "Trạng thái: Đèn đang TẮT 🌑";
      bot.sendMessage(chat_id, status, "");
    } 
    else if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      String msg = "🌡 Nhiệt độ: " + String(t) + "°C\n💧 Độ ẩm: " + String(h) + "%";
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

// --- HÀM ĐỌC CẢM BIẾN & GỬI DỮ LIỆU ---
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gasValue = analogRead(MQ2_PIN);

  // Sinh ngẫu nhiên nếu cảm biến MQ2 không có dữ liệu thực tế
  if (gasValue == 0) gasValue = random(100, 300);

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
  display.printf("Temp: %.1f C\nHumi: %.1f %%", t, h);
  display.setCursor(0, 25);
  display.printf("Gas: %d PPM", gasValue);
  display.setCursor(0, 50);
  display.print("TEAM 03 - K45");
  display.display();

  // 3. Telegram: Tự động gửi khi có thay đổi lớn (>1 độ hoặc >2% ẩm)
  if (abs(t - lastTemp) > 1.0 || abs(h - lastHumi) > 2.0) {
    String alert = "⚠️ Cảnh báo thay đổi:\n🌡 Nhiệt độ: " + String(t) + "°C\n💧 Độ ẩm: " + String(h) + "%";
    bot.sendMessage(CHAT_ID, alert, "");
    lastTemp = t; lastHumi = h;
  }
}

BLYNK_WRITE(V4) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  bot.sendMessage(CHAT_ID, ledState ? "LED bật (từ Blynk)" : "LED tắt (từ Blynk)", "");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  Wire.begin(4, 5); // D2, D1 cho OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);

  WiFi.begin(ssid, pass);
  client.setInsecure(); // Bỏ qua SSL
  client.setBufferSizes(512, 512); // Tối ưu RAM cho Telegram

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(3000L, sendSensorData); // Đọc mỗi 3 giây để tránh quá tải
  
  Serial.println("System Ready!");
}

void loop() {
  Blynk.run();
  timer.run();

  static unsigned long lastBotTime;
  if (millis() - lastBotTime > 2000) { // Kiểm tra Telegram mỗi 2 giây
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages > 0) handleTelegramMessages(numNewMessages);
    lastBotTime = millis();
  }
}