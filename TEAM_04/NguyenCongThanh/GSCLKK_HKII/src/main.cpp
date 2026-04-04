/*
 * ĐỀ TÀI: Giám sát chất lượng không khí với ESP32 và cảm biến MQ
 * Phiên bản: Wokwi Optimized v2.0
 * Fix: Telegram nhanh hơn + Serial Terminal hiển thị đúng trên Wokwi/VS Code
 */

<<<<<<< HEAD
#define BLYNK_TEMPLATE_ID "TMPL6nn2gjF7c"       
#define BLYNK_TEMPLATE_NAME "AIR QUALITY" 
#define BLYNK_AUTH_TOKEN "m_JnU9yh0VsAMW8u51VtEfWeBD2cbSx_"    
=======
#define BLYNK_TEMPLATE_ID "TMPL_ID_CUA_BAN"       
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor" 
#define BLYNK_AUTH_TOKEN "TOKEN_BLYNK_CUA_BAN"    
>>>>>>> 0765b0d2511d474a75a85dab515cc51a1b80ff2d

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CẤU HÌNH MẠNG & TELEGRAM ----------------
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";
#define BOT_TOKEN "8629442956:AAF8YBmH05p94b6TYRJh29mo_Fpgvs4pPuI"
#define CHAT_ID "6839585914"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ---------------- CẤU HÌNH CHÂN (PINS) ----------------
#define PIN_CO_SENSOR 39
#define PIN_PM_SENSOR 36
#define PIN_LED_GREEN 4
#define PIN_LED_YELLOW 5
#define PIN_LED_RED 2

// OLED SSD1306
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- BIẾN TOÀN CỤC & NGƯỠNG ----------------
WebServer server(80);
int coValue = 0;
int pmValue = 0;
int currentStatus = 0; // 0: Tốt, 1: Kém, 2: Nguy hiểm
unsigned long lastUpdate = 0;
const int updateInterval = 2000;

bool useSimulatedData = false; 
bool ledState = false;

// ⚡ TELEGRAM OPTIMIZED: Giảm interval + thêm biến static
#define BOT_CHECK_INTERVAL 800  // ← Giảm từ 2000ms → 800ms

// ---------------- HÀM HỖ TRỢ ----------------

// ✅ Fix Serial: Có flush() để hiển thị ngay trên Wokwi
void printLog(const String &msg) {
  Serial.println(msg);
  Serial.flush();  // ← Quan trọng cho Wokwi simulator!
}

bool isTelegramEnabled() {
  return String(BOT_TOKEN) != "ĐIỀN_BOT_TOKEN_VÀO_ĐÂY" && 
         String(BOT_TOKEN).length() > 20;
}

// ---------------- TASK LED NHẤP NHÁY (FreeRTOS) ----------------
void blinkTask(void * pvParameters) {
  for (;;) {
    int blinkInterval = 1000;
    if (currentStatus == 1) blinkInterval = 700;
    else if (currentStatus == 2) blinkInterval = 500;

    ledState = !ledState; 

    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, LOW);

    if (ledState) {
      if (currentStatus == 0) digitalWrite(PIN_LED_GREEN, HIGH);
      else if (currentStatus == 1) digitalWrite(PIN_LED_YELLOW, HIGH);
      else if (currentStatus == 2) digitalWrite(PIN_LED_RED, HIGH);
    }
    
    vTaskDelay(blinkInterval / portTICK_PERIOD_MS);
    yield(); // ← Tránh watchdog reset trên Wokwi
  }
}

// ---------------- XỬ LÝ LỆNH TELEGRAM ----------------
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    printLog("💬 Telegram: " + from_name + " > " + text);

    if (text == "/start") {
      String welcome = "👋 Xin chào " + from_name + "!\n";
      welcome += "🤖 Bot Giám sát Không Khí - Wokwi Edition\n\n";
      welcome += "📋 LỆNH:\n";
      welcome += "/status : Xem thông số\n";
      welcome += "/pot_good : Mode TỐT 🟢\n";
      welcome += "/pot_bad : Mode KÉM 🟡\n";
      welcome += "/pot_danger : Mode NGUY HIỂM 🔴\n";
      welcome += "/pot_real : Đọc cảm biến thật";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/status") {
      String statMsg = "📊 THÔNG SỐ HIỆN TẠI:\n";
      statMsg += "🌫️ CO: " + String(coValue) + " ppm\n";
      statMsg += "💨 PM2.5: " + String(pmValue) + " µg/m³\n";
      
      if (currentStatus == 0) statMsg += "✅ Trạng thái: TỐT";
      else if (currentStatus == 1) statMsg += "⚠️ Trạng thái: KÉM";
      else if (currentStatus == 2) statMsg += "🚨 Trạng thái: NGUY HIỂM!";
      bot.sendMessage(chat_id, statMsg, "");
    }
    else if (text == "/pot_good") {
      useSimulatedData = true;
      coValue = random(10, 350);
      pmValue = random(5, 18);
      bot.sendMessage(chat_id, "🔧 Mode: TỐT 🟢", "");
      printLog(">>> [TELEGRAM] Mode: /pot_good");
    }
    else if (text == "/pot_bad") {
      useSimulatedData = true;
      coValue = random(450, 900);
      pmValue = random(25, 45);
      bot.sendMessage(chat_id, "🔧 Mode: KÉM 🟡", "");
      printLog(">>> [TELEGRAM] Mode: /pot_bad");
    }
    else if (text == "/pot_danger") {
      useSimulatedData = true;
      coValue = random(1200, 1900);
      pmValue = random(70, 140);
      bot.sendMessage(chat_id, "🔧 Mode: NGUY HIỂM 🔴", "");
      printLog(">>> [TELEGRAM] Mode: /pot_danger");
    }
    else if (text == "/pot_real") {
      useSimulatedData = false;
      bot.sendMessage(chat_id, "🔧 Quay lại đọc cảm biến thật!", "");
      printLog(">>> [TELEGRAM] Mode: /pot_real");
    }
  }
  yield(); // ← Giải phóng CPU sau khi xử lý
}

// ⚡ Hàm kiểm tra Telegram tối ưu cho Wokwi
void checkTelegramMessages() {
  if (!isTelegramEnabled()) return;
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < BOT_CHECK_INTERVAL) return;
  lastCheck = millis();
  
  yield(); // ← Tránh watchdog reset
  
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    yield();
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ---------------- GIAO DIỆN WEB ----------------
String getHTML(int co, int pm, int status) {
  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<meta http-equiv=\"refresh\" content=\"2\">"; 
  html += "<title>Trạm Giám sát Không Khí</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin-top: 50px; }";
  html += ".card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 8px rgba(0,0,0,0.2); display: inline-block; width: 300px; margin: 10px; }";
  html += "h1 { color: #0056b3; }";
  html += ".value { font-size: 2em; font-weight: bold; }";
  html += ".status0 { color: #28a745; font-weight: bold; font-size: 1.5em; }"; 
  html += ".status1 { color: #ffc107; font-weight: bold; font-size: 1.5em; }"; 
  html += ".status2 { color: #dc3545; font-weight: bold; font-size: 1.5em; animation: blinker 1s linear infinite; }"; 
  html += "@keyframes blinker { 50% { opacity: 0; } }";
  html += "</style></head><body>";
  
  html += "<h1>🌍 TRẠM GIÁM SÁT CHẤT LƯỢNG KHÔNG KHÍ</h1>";
  html += "<div class=\"card\"><h2>🌫️ Nồng độ CO</h2><p class=\"value\">" + String(co) + " <small>ppm</small></p></div>";
  html += "<div class=\"card\"><h2>💨 Bụi PM2.5</h2><p class=\"value\">" + String(pm) + " <small>µg/m³</small></p></div>";

  html += "<div><p>Trạng thái hệ thống:</p>";
  if(status == 0) html += "<p class=\"status0\">✅ CHẤT LƯỢNG: TỐT</p>";
  else if(status == 1) html += "<p class=\"status1\">⚠️ CHẤT LƯỢNG: KÉM</p>";
  else html += "<p class=\"status2\">🚨 CẢNH BÁO: NGUY HIỂM!</p>";
  html += "</div></body></html>";
  
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHTML(coValue, pmValue, currentStatus));
}

// ---------------- SETUP ----------------
void setup() {
  // ✅ Fix Serial cho Wokwi: baud + debug + flush
  Serial.begin(115200);
//  Serial.setDebugOutput(true);  // ← Bật debug output cho ESP32
  delay(1500);                   // ← Chờ Serial ổn định
  Serial.flush();                // ← Flush buffer cũ
  
  printLog("\n🚀 === AIR MONITOR - WOKWI EDITION ===");
  printLog("📡 Baud: 115200 | Chip: " + String(ESP.getChipModel()));
  printLog("🔧 Fix: Telegram fast + Serial flush enabled");
  
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  
  // Tạo task LED blink trên core 0
  xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 0);

  // Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    printLog("❌ Lỗi SSD1306 - Tiếp tục chạy không màn hình");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(F("Air Monitor IoT"));
    display.println(F("Booting..."));
    display.display();
  }

  // Kết nối WiFi
  printLog("📶 Đang kết nối WiFi: " + String(ssid));
  WiFi.begin(ssid, password);
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) { 
    delay(500); 
    Serial.print(".");  // Dùng Serial.print trực tiếp để thấy progress
    Serial.flush();
    wifiTimeout++;
  }
  Serial.println(); 
  Serial.flush();
  
  if (WiFi.status() == WL_CONNECTED) {
    printLog("🌐 WiFi OK! IP: " + WiFi.localIP().toString());
  } else {
    printLog("⚠️ WiFi timeout - Vẫn chạy ở chế độ offline");
  }
  
  // Web Server
  server.on("/", handleRoot);
  server.begin();
  printLog("🌐 Web Server: http://" + WiFi.localIP().toString());

  // Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  // ✅ Telegram: Cấu hình timeout nhanh hơn cho Wokwi
  secured_client.setInsecure(); 
  secured_client.setTimeout(8000);  // ← Timeout 8s thay vì 30s+
  
  // Cập nhật OLED
  if(display.width() > 0) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println(F("WiFi: OK"));
    display.println(F("Ready!"));
    display.display();
  }
  
  // Test Serial & Telegram
  printLog("✅ Setup hoàn tất! Đang chạy loop...");
  
  if (isTelegramEnabled()) {
    printLog("📤 Gửi test message tới Telegram...");
    bot.sendMessage(CHAT_ID, "🤖 [Wokwi] Bot đã khởi động!", "");
    delay(300);  // Tránh rate limit
  } else {
    printLog("⚠️ Telegram: Chưa cấu hình token hợp lệ");
  }
  
  Serial.flush();  // ← Đảm bảo tất cả log được đẩy ra terminal
}

// ---------------- VÒNG LẶP CHÍNH ----------------
void loop() {
  Blynk.run();
  server.handleClient();
  
  // ⚡ Kiểm tra Telegram với interval tối ưu
  checkTelegramMessages();
  
  // Đọc cảm biến và cập nhật
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    
    // Đọc cảm biến (hoặc dùng simulated data)
    if (!useSimulatedData) {
      int rawCO = analogRead(PIN_CO_SENSOR);
      coValue = map(rawCO, 0, 4095, 0, 2000); 
      
      int rawPM = analogRead(PIN_PM_SENSOR);
      pmValue = map(rawPM, 0, 4095, 0, 150);  
    }

    // ✅ Dùng printLog để có flush() tự động
    printLog("📊 CO: " + String(coValue) + " ppm | PM2.5: " + String(pmValue) + " µg/m³ | Mode: " + 
             (useSimulatedData ? "TELEGRAM" : "SLIDER"));

    // Cập nhật Blynk
    Blynk.virtualWrite(V1, coValue);
    Blynk.virtualWrite(V2, pmValue);

    // Xác định trạng thái
    int newStatus = 0;
    if (coValue > 1000 || pmValue > 50) newStatus = 2;
    else if (coValue > 400 || pmValue > 20) newStatus = 1;
    else newStatus = 0;
    
    // Cập nhật OLED
    if(display.width() > 0) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(F("--- AIR QUALITY ---"));
      display.setTextSize(2);
      display.setCursor(0, 16);
      display.printf("CO: %d\n", coValue);
      display.printf("PM: %d\n", pmValue);
      display.setTextSize(1);
      display.setCursor(0, 50);
      if (newStatus == 0) display.print(F("Status: GOOD"));
      else if (newStatus == 1) display.print(F("Status: BAD"));
      else display.print(F("Status: DANGER!"));
      display.display();
    }

    // Thông báo khi thay đổi trạng thái
    if (newStatus != currentStatus) {
      currentStatus = newStatus;
      
      String consoleMsg = "";
      String teleMsg = "";

      if (currentStatus == 0) {
        consoleMsg = "✅ [CHANGE] TỐT | CO:" + String(coValue) + " PM:" + String(pmValue);
        teleMsg = "✅ KHÔNG KHÍ ĐÃ TỐT!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
        Blynk.virtualWrite(V3, "An toàn");
      } 
      else if (currentStatus == 1) {
        consoleMsg = "⚠️ [CHANGE] KÉM | CO:" + String(coValue) + " PM:" + String(pmValue);
        teleMsg = "⚠️ CHẤT LƯỢNG KÉM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
        Blynk.virtualWrite(V3, "Cảnh báo");
      } 
      else if (currentStatus == 2) {
        consoleMsg = "🚨 [CHANGE] NGUY HIỂM | CO:" + String(coValue) + " PM:" + String(pmValue);
        teleMsg = "🚨 BÁO ĐỘNG ĐỎ!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³\n🏃‍♂️ Xử lý ngay!";
        Blynk.virtualWrite(V3, "NGUY HIỂM");
      }

      // In log với phân cách đẹp
      printLog("\n==============================================");
      printLog(consoleMsg);
      printLog("==============================================\n");
      
      // Gửi Telegram
      if (isTelegramEnabled()) {
        bot.sendMessage(CHAT_ID, teleMsg, "");
        delay(200);  // ← Tránh Telegram rate limit
        yield();
      }
    }
  }
  
  // ✅ Yield định kỳ để Wokwi không bị treo
  yield();
}