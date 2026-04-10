// /*
//  * ĐỀ TÀI: Giám sát chất lượng không khí với ESP32 và cảm biến MQ
//  * Các tính năng: 
//  * - Đọc cảm biến CO và Bụi PM2.5 (Mô phỏng)
//  * - Màn hình OLED SSD1306 hiển thị thông số tại chỗ
//  * - 3 Đèn LED cảnh báo mức độ (Xanh: Tốt, Vàng: Kém, Đỏ: Nguy hiểm)
//  * - Hiển thị qua Giao diện Web (Local)
//  * - Đẩy dữ liệu lên Blynk IoT và Cảnh báo qua Telegram
//  */

// #define BLYNK_TEMPLATE_ID "TMPL_ID_CUA_BAN"       
// #define BLYNK_TEMPLATE_NAME "Air Quality Monitor" 
// #define BLYNK_AUTH_TOKEN "TOKEN_BLYNK_CUA_BAN"    

// #include <WiFi.h>
// #include <WiFiClient.h>
// #include <WebServer.h>
// #include <BlynkSimpleEsp32.h>
// #include <WiFiClientSecure.h>
// #include <UniversalTelegramBot.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// // ---------------- CẤU HÌNH MẠNG & TELEGRAM ----------------
// const char* ssid = "Wokwi-GUEST"; 
// const char* password = "";
// #define BOT_TOKEN "ĐIỀN_BOT_TOKEN_VÀO_ĐÂY"
// #define CHAT_ID "ĐIỀN_CHAT_ID_VÀO_ĐÂY"
// WiFiClientSecure secured_client;
// UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// // ---------------- CẤU HÌNH CHÂN (PINS) ----------------
// #define PIN_CO_SENSOR 39  // Chân VN (pot1)
// #define PIN_PM_SENSOR 36  // Chân VP (pot2)
// #define PIN_LED_GREEN 4   // Đèn báo Tốt (led1)
// #define PIN_LED_YELLOW 5  // Đèn báo Xấu (led2)
// #define PIN_LED_RED 2     // Đèn báo Nguy hiểm (led3)

// // Khởi tạo màn hình OLED SSD1306 (Kích thước 128x64)
// #define SCREEN_WIDTH 128 
// #define SCREEN_HEIGHT 64 
// #define OLED_RESET    -1 
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // ---------------- BIẾN TOÀN CỤC & NGƯỠNG ----------------
// WebServer server(80);
// int coValue = 0;
// int pmValue = 0;
// int currentStatus = 0; // 0: Tốt, 1: Kém, 2: Nguy hiểm
// unsigned long lastUpdate = 0;
// const int updateInterval = 2000;

// // Các ngưỡng đo: 
// // CO < 400 & PM < 20 -> Tốt (Xanh)
// // CO < 1000 & PM < 50 -> Kém (Vàng)
// // Cao hơn -> Nguy hiểm (Đỏ)

// // ---------------- HÀM ĐIỀU KHIỂN LED ----------------
// void updateLEDs(int status) {
//   digitalWrite(PIN_LED_GREEN, status == 0 ? HIGH : LOW);
//   digitalWrite(PIN_LED_YELLOW, status == 1 ? HIGH : LOW);
//   digitalWrite(PIN_LED_RED, status == 2 ? HIGH : LOW);
// }

// // ---------------- GIAO DIỆN WEB ----------------
// String getHTML(int co, int pm, int status) {
//   String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
//   html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
//   html += "<meta http-equiv=\"refresh\" content=\"2\">"; 
//   html += "<title>Trạm Giám sát Không Khí</title>";
//   html += "<style>";
//   html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin-top: 50px; }";
//   html += ".card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 8px rgba(0,0,0,0.2); display: inline-block; width: 300px; margin: 10px; }";
//   html += "h1 { color: #0056b3; }";
//   html += ".value { font-size: 2em; font-weight: bold; }";
//   html += ".status0 { color: #28a745; font-weight: bold; font-size: 1.5em; }"; // Xanh
//   html += ".status1 { color: #ffc107; font-weight: bold; font-size: 1.5em; }"; // Vàng
//   html += ".status2 { color: #dc3545; font-weight: bold; font-size: 1.5em; animation: blinker 1s linear infinite; }"; // Đỏ
//   html += "@keyframes blinker { 50% { opacity: 0; } }";
//   html += "</style></head><body>";
  
//   html += "<h1>TRẠM GIÁM SÁT CHẤT LƯỢNG KHÔNG KHÍ</h1>";
//   html += "<div class=\"card\"><h2>Nồng độ CO</h2><p class=\"value\" style=\"color:#555\">" + String(co) + " <small>ppm</small></p></div>";
//   html += "<div class=\"card\"><h2>Bụi PM2.5</h2><p class=\"value\" style=\"color:#555\">" + String(pm) + " <small>µg/m³</small></p></div>";

//   html += "<div><p>Trạng thái hệ thống:</p>";
//   if(status == 0) html += "<p class=\"status0\">CHẤT LƯỢNG: TỐT</p>";
//   else if(status == 1) html += "<p class=\"status1\">CHẤT LƯỢNG: KÉM</p>";
//   else html += "<p class=\"status2\">CẢNH BÁO: NGUY HIỂM!</p>";
//   html += "</div></body></html>";
  
//   return html;
// }

// void handleRoot() {
//   server.send(200, "text/html", getHTML(coValue, pmValue, currentStatus));
// }

// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);
  
//   pinMode(PIN_LED_GREEN, OUTPUT);
//   pinMode(PIN_LED_YELLOW, OUTPUT);
//   pinMode(PIN_LED_RED, OUTPUT);
//   updateLEDs(0); // Mặc định bật xanh
  
//   // Khởi động OLED
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;);
//   }
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0, 10);
//   display.println(F("Air Monitor IoT"));
//   display.println(F("Starting WiFi..."));
//   display.display();

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
//   server.on("/", handleRoot);
//   server.begin();

//   Blynk.config(BLYNK_AUTH_TOKEN);
//   secured_client.setInsecure();
  
//   display.clearDisplay();
//   display.setCursor(0, 10);
//   display.println(F("WiFi Connected!"));
//   display.display();
//   delay(1000);
// }

// // ---------------- VÒNG LẶP CHÍNH ----------------
// void loop() {
//   Blynk.run();
//   server.handleClient();
  
//   if (millis() - lastUpdate > updateInterval) {
//     lastUpdate = millis();
    
//     // Đọc cảm biến
//     int rawCO = analogRead(PIN_CO_SENSOR);
//     coValue = map(rawCO, 0, 4095, 0, 2000); 
    
//     int rawPM = analogRead(PIN_PM_SENSOR);
//     pmValue = map(rawPM, 0, 4095, 0, 150);  

//     Blynk.virtualWrite(V1, coValue);
//     Blynk.virtualWrite(V2, pmValue);

//     // Xác định trạng thái
//     int newStatus = 0;
//     if (coValue > 1000 || pmValue > 50) newStatus = 2;      // Nguy hiểm
//     else if (coValue > 400 || pmValue > 20) newStatus = 1;  // Xấu
//     else newStatus = 0;                                     // Tốt
    
//     // Cập nhật màn hình OLED
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.println(F("--- AIR QUALITY ---"));
    
//     display.setTextSize(2);
//     display.setCursor(0, 16);
//     display.printf("CO: %d\n", coValue);
//     display.printf("PM: %d\n", pmValue);
    
//     display.setTextSize(1);
//     display.setCursor(0, 50);
//     if (newStatus == 0) display.print(F("Status: GOOD (SAFE)"));
//     else if (newStatus == 1) display.print(F("Status: BAD (WARN)"));
//     else display.print(F("Status: DANGER!"));
    
//     display.display();

//     // Nếu trạng thái thay đổi
//     if (newStatus != currentStatus) {
//       currentStatus = newStatus;
//       updateLEDs(currentStatus);
      
//       if (currentStatus == 2) {
//         Blynk.virtualWrite(V3, "NGUY HIỂM");
//         String msg = "⚠️ CẢNH BÁO Ô NHIỄM NGUY HIỂM!\nCO: " + String(coValue) + " ppm\nPM2.5: " + String(pmValue) + " µg/m³";
//         bot.sendMessage(CHAT_ID, msg, "");
//       } else if (currentStatus == 1) {
//         Blynk.virtualWrite(V3, "Chất lượng Kém");
//       } else {
//         Blynk.virtualWrite(V3, "An toàn");
//         bot.sendMessage(CHAT_ID, "✅ Không khí đã trở lại mức an toàn.", "");
//       }
//     }
//   }
// }





/*
 * ĐỀ TÀI: Giám sát chất lượng không khí với ESP32 và cảm biến MQ
 * Các tính năng nâng cấp: 
 * - Màn hình OLED SSD1306
 * - LED nhấp nháy đa tốc độ (Tốt: 1s, Kém: 0.7s, Nguy hiểm: 0.5s)
 * - Bot Telegram có thể tương tác (Lệnh: /start, /status)
 * - Ghi log sự kiện ra Terminal có Emoji
 */

#define BLYNK_TEMPLATE_ID "TMPL_ID_CUA_BAN"       
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor" 
#define BLYNK_AUTH_TOKEN "TOKEN_BLYNK_CUA_BAN"    

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
#define BOT_TOKEN "8629442956:AAF8YBmH05p94b6TYRJh29mo_Fpgvs4pPuI" // Điền token sau
#define CHAT_ID "6839585914"     // Điền ID của bạn sau
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);





// // ---------------- CẤU HÌNH CHÂN (PINS) ----------------
// #define PIN_CO_SENSOR 39  // Chân VN (pot1)
// #define PIN_PM_SENSOR 36  // Chân VP (pot2)
// #define PIN_LED_GREEN 4   // Đèn báo Tốt (led1)
// #define PIN_LED_YELLOW 5  // Đèn báo Xấu (led2)
// #define PIN_LED_RED 2     // Đèn báo Nguy hiểm (led3)

// // Khởi tạo màn hình OLED SSD1306 (Kích thước 128x64)
// #define SCREEN_WIDTH 128 
// #define SCREEN_HEIGHT 64 
// #define OLED_RESET    -1 
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // ---------------- BIẾN TOÀN CỤC & NGƯỠNG ----------------
// WebServer server(80);
// int coValue = 0;
// int pmValue = 0;
// int currentStatus = 0; // 0: Tốt, 1: Kém, 2: Nguy hiểm
// unsigned long lastUpdate = 0;
// const int updateInterval = 2000;





// // Biến cho LED nhấp nháy (Sử dụng millis() thay vì delay)
// unsigned long lastBlinkTime = 0;
// bool ledState = false;

// // Biến cho Telegram Bot
// unsigned long lastBotCheck = 0;
// const int botCheckInterval = 1000; // Kiểm tra tin nhắn mới mỗi 1 giây

// // ---------------- HÀM ĐIỀU KHIỂN LED NHẤP NHÁY ----------------
// void handleBlinkingLEDs() {
//   int blinkInterval = 1000; // Mặc định chớp 1s (Tốt)
//   if (currentStatus == 1) blinkInterval = 700;       // Kém: chớp 0.7s
//   else if (currentStatus == 2) blinkInterval = 500;  // Nguy hiểm: chớp 0.5s

//   if (millis() - lastBlinkTime >= blinkInterval) {
//     lastBlinkTime = millis();
//     ledState = !ledState; // Đảo trạng thái đèn (Bật -> Tắt, Tắt -> Bật)

//     // Tắt hết các đèn trước
//     digitalWrite(PIN_LED_GREEN, LOW);
//     digitalWrite(PIN_LED_YELLOW, LOW);
//     digitalWrite(PIN_LED_RED, LOW);

//     // Chỉ bật đèn tương ứng nếu ledState = true
//     if (ledState) {
//       if (currentStatus == 0) digitalWrite(PIN_LED_GREEN, HIGH);
//       else if (currentStatus == 1) digitalWrite(PIN_LED_YELLOW, HIGH);
//       else if (currentStatus == 2) digitalWrite(PIN_LED_RED, HIGH);
//     }
//   }
// }

// // ---------------- HÀM XỬ LÝ LỆNH TELEGRAM ----------------
// void handleNewMessages(int numNewMessages) {
//   for (int i = 0; i < numNewMessages; i++) {
//     String chat_id = String(bot.messages[i].chat_id);
//     String text = bot.messages[i].text;
//     String from_name = bot.messages[i].from_name;
    
//     Serial.println("💬 Nhận lệnh từ Telegram: " + text);

//     if (text == "/start") {
//       String welcome = "👋 Xin chào " + from_name + "!\n";
//       welcome += "🤖 Tôi là Bot Giám sát Không Khí.\n";
//       welcome += "👉 Gõ /status để xem thông số hiện tại.";
//       bot.sendMessage(chat_id, welcome, "");
//     }
    
//     if (text == "/status") {
//       String statMsg = "📊 THÔNG SỐ HIỆN TẠI:\n";
//       statMsg += "🌫️ Nồng độ CO: " + String(coValue) + " ppm\n";
//       statMsg += "💨 Bụi PM2.5: " + String(pmValue) + " µg/m³\n";
      
//       if (currentStatus == 0) statMsg += "✅ Trạng thái: TỐT (An toàn)";
//       else if (currentStatus == 1) statMsg += "⚠️ Trạng thái: KÉM (Cảnh báo)";
//       else if (currentStatus == 2) statMsg += "🚨 Trạng thái: NGUY HIỂM!";
      
//       bot.sendMessage(chat_id, statMsg, "");
//     }
//   }
// }

// // ---------------- GIAO DIỆN WEB ----------------
// String getHTML(int co, int pm, int status) {
//   String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
//   html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
//   html += "<meta http-equiv=\"refresh\" content=\"2\">"; 
//   html += "<title>Trạm Giám sát Không Khí</title>";
//   html += "<style>";
//   html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin-top: 50px; }";
//   html += ".card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 8px rgba(0,0,0,0.2); display: inline-block; width: 300px; margin: 10px; }";
//   html += "h1 { color: #0056b3; }";
//   html += ".value { font-size: 2em; font-weight: bold; }";
//   html += ".status0 { color: #28a745; font-weight: bold; font-size: 1.5em; }"; 
//   html += ".status1 { color: #ffc107; font-weight: bold; font-size: 1.5em; }"; 
//   html += ".status2 { color: #dc3545; font-weight: bold; font-size: 1.5em; animation: blinker 1s linear infinite; }"; 
//   html += "@keyframes blinker { 50% { opacity: 0; } }";
//   html += "</style></head><body>";
  
//   html += "<h1>🌍 TRẠM GIÁM SÁT CHẤT LƯỢNG KHÔNG KHÍ</h1>";
//   html += "<div class=\"card\"><h2>🌫️ Nồng độ CO</h2><p class=\"value\" style=\"color:#555\">" + String(co) + " <small>ppm</small></p></div>";
//   html += "<div class=\"card\"><h2>💨 Bụi PM2.5</h2><p class=\"value\" style=\"color:#555\">" + String(pm) + " <small>µg/m³</small></p></div>";

//   html += "<div><p>Trạng thái hệ thống:</p>";
//   if(status == 0) html += "<p class=\"status0\">✅ CHẤT LƯỢNG: TỐT</p>";
//   else if(status == 1) html += "<p class=\"status1\">⚠️ CHẤT LƯỢNG: KÉM</p>";
//   else html += "<p class=\"status2\">🚨 CẢNH BÁO: NGUY HIỂM!</p>";
//   html += "</div></body></html>";
  
//   return html;
// }

// void handleRoot() {
//   server.send(200, "text/html", getHTML(coValue, pmValue, currentStatus));
// }

// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);
  
//   pinMode(PIN_LED_GREEN, OUTPUT);
//   pinMode(PIN_LED_YELLOW, OUTPUT);
//   pinMode(PIN_LED_RED, OUTPUT);
  
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//     Serial.println(F("❌ Lỗi khởi tạo SSD1306"));
//     for(;;);
//   }
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0, 10);
//   display.println(F("Air Monitor IoT"));
//   display.println(F("Starting WiFi..."));
//   display.display();

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
//   server.on("/", handleRoot);
//   server.begin();

//   Blynk.config(BLYNK_AUTH_TOKEN);
//   secured_client.setInsecure(); // Bắt buộc cho ESP32 gọi API Telegram
  
//   display.clearDisplay();
//   display.setCursor(0, 10);
//   display.println(F("WiFi Connected!"));
//   display.display();
//   Serial.println("\n🌐 Đã kết nối WiFi. Sẵn sàng hoạt động!");
//   delay(1000);
// }

// // ---------------- VÒNG LẶP CHÍNH ----------------
// void loop() {
//   Blynk.run();
//   server.handleClient();
  
//   // 1. Chạy hiệu ứng LED nhấp nháy liên tục (Không bị chặn)
//   handleBlinkingLEDs();

//   // 2. Kiểm tra tin nhắn Telegram
//   if (millis() - lastBotCheck > botCheckInterval) {
//     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//     while (numNewMessages) {
//       handleNewMessages(numNewMessages);
//       numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//     }
//     lastBotCheck = millis();
//   }
  
//   // 3. Đọc cảm biến và cập nhật trạng thái
//   if (millis() - lastUpdate > updateInterval) {
//     lastUpdate = millis();
    
//     int rawCO = analogRead(PIN_CO_SENSOR);
//     coValue = map(rawCO, 0, 4095, 0, 2000); 
    
//     int rawPM = analogRead(PIN_PM_SENSOR);
//     pmValue = map(rawPM, 0, 4095, 0, 150);  

//     Blynk.virtualWrite(V1, coValue);
//     Blynk.virtualWrite(V2, pmValue);

//     // Xác định trạng thái
//     int newStatus = 0;
//     if (coValue > 1000 || pmValue > 50) newStatus = 2;      // Nguy hiểm
//     else if (coValue > 400 || pmValue > 20) newStatus = 1;  // Xấu
//     else newStatus = 0;                                     // Tốt
    
//     // Cập nhật OLED
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.println(F("--- AIR QUALITY ---"));
//     display.setTextSize(2);
//     display.setCursor(0, 16);
//     display.printf("CO: %d\n", coValue);
//     display.printf("PM: %d\n", pmValue);
//     display.setTextSize(1);
//     display.setCursor(0, 50);
//     if (newStatus == 0) display.print(F("Status: GOOD (SAFE)"));
//     else if (newStatus == 1) display.print(F("Status: BAD (WARN)"));
//     else display.print(F("Status: DANGER!"));
//     display.display();

//     // 4. KIỂM TRA SỰ THAY ĐỔI TRẠNG THÁI ĐỂ THÔNG BÁO
//     if (newStatus != currentStatus) {
//       currentStatus = newStatus;
      
//       String consoleMsg = "";
//       String teleMsg = "";

//       if (currentStatus == 0) {
//         consoleMsg = "✅ CHUYỂN TRẠNG THÁI: TỐT | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "✅ CHẤT LƯỢNG KHÔNG KHÍ ĐÃ TỐT LÊN!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
//         Blynk.virtualWrite(V3, "An toàn");
//       } 
//       else if (currentStatus == 1) {
//         consoleMsg = "⚠️ CHUYỂN TRẠNG THÁI: KÉM | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "⚠️ CHÚ Ý: CHẤT LƯỢNG KHÔNG KHÍ KÉM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
//         Blynk.virtualWrite(V3, "Cảnh báo");
//       } 
//       else if (currentStatus == 2) {
//         consoleMsg = "🚨 CHUYỂN TRẠNG THÁI: NGUY HIỂM | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "🚨 BÁO ĐỘNG ĐỎ: KHÔNG KHÍ NGUY HIỂM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³\n🏃‍♂️ Vui lòng có biện pháp xử lý ngay!";
//         Blynk.virtualWrite(V3, "NGUY HIỂM");
//       }

//       // In ra Terminal (VS Code / Arduino IDE)
//       Serial.println("\n==============================================");
//       Serial.println(consoleMsg);
//       Serial.println("==============================================");
      
//       // Gửi Telegram
//       bot.sendMessage(CHAT_ID, teleMsg, "");
//     }
//   }
// }





// -----------------------------------------





// // Biến cho LED nhấp nháy (Sử dụng millis() thay vì delay)
// unsigned long lastBlinkTime = 0;
// bool ledState = false;

// // Biến cho Telegram Bot
// unsigned long lastBotCheck = 0;
// const int botCheckInterval = 3000; // Tăng lên 3s để tránh lag

// // Hàm kiểm tra xem đã điền Telegram Token chưa
// bool isTelegramEnabled() {
//   return String(BOT_TOKEN) != "ĐIỀN_BOT_TOKEN_VÀO_ĐÂY";
// }

// // ---------------- TASK ĐA LUỒNG CHO LED NHẤP NHÁY ----------------
// // Thay thế hàm handleBlinkingLEDs bằng Task chạy trên luồng riêng biệt của ESP32
// void blinkTask(void * pvParameters) {
//   for (;;) {
//     int blinkInterval = 1000; // Mặc định chớp 1s (Tốt)
//     if (currentStatus == 1) blinkInterval = 700;       // Kém: chớp 0.7s
//     else if (currentStatus == 2) blinkInterval = 500;  // Nguy hiểm: chớp 0.5s

//     ledState = !ledState; // Đảo trạng thái đèn (Bật -> Tắt, Tắt -> Bật)

//     // Tắt hết các đèn trước
//     digitalWrite(PIN_LED_GREEN, LOW);
//     digitalWrite(PIN_LED_YELLOW, LOW);
//     digitalWrite(PIN_LED_RED, LOW);

//     // Chỉ bật đèn tương ứng nếu ledState = true
//     if (ledState) {
//       if (currentStatus == 0) digitalWrite(PIN_LED_GREEN, HIGH);
//       else if (currentStatus == 1) digitalWrite(PIN_LED_YELLOW, HIGH);
//       else if (currentStatus == 2) digitalWrite(PIN_LED_RED, HIGH);
//     }
    
//     vTaskDelay(blinkInterval / portTICK_PERIOD_MS); // Hàm delay không chặn của FreeRTOS
//   }
// }

// // ---------------- HÀM XỬ LÝ LỆNH TELEGRAM ----------------
// void handleNewMessages(int numNewMessages) {
//   for (int i = 0; i < numNewMessages; i++) {
//     String chat_id = String(bot.messages[i].chat_id);
//     String text = bot.messages[i].text;
//     String from_name = bot.messages[i].from_name;
    
//     Serial.println("💬 Nhận lệnh từ Telegram: " + text);

//     if (text == "/start") {
//       String welcome = "👋 Xin chào " + from_name + "!\n";
//       welcome += "🤖 Tôi là Bot Giám sát Không Khí.\n";
//       welcome += "👉 Gõ /status để xem thông số hiện tại.";
//       bot.sendMessage(chat_id, welcome, "");
//     }
    
//     if (text == "/status") {
//       String statMsg = "📊 THÔNG SỐ HIỆN TẠI:\n";
//       statMsg += "🌫️ Nồng độ CO: " + String(coValue) + " ppm\n";
//       statMsg += "💨 Bụi PM2.5: " + String(pmValue) + " µg/m³\n";
      
//       if (currentStatus == 0) statMsg += "✅ Trạng thái: TỐT (An toàn)";
//       else if (currentStatus == 1) statMsg += "⚠️ Trạng thái: KÉM (Cảnh báo)";
//       else if (currentStatus == 2) statMsg += "🚨 Trạng thái: NGUY HIỂM!";
      
//       bot.sendMessage(chat_id, statMsg, "");
//     }
//   }
// }

// // ---------------- GIAO DIỆN WEB ----------------
// String getHTML(int co, int pm, int status) {
//   String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
//   html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
//   html += "<meta http-equiv=\"refresh\" content=\"2\">"; 
//   html += "<title>Trạm Giám sát Không Khí</title>";
//   html += "<style>";
//   html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin-top: 50px; }";
//   html += ".card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 8px rgba(0,0,0,0.2); display: inline-block; width: 300px; margin: 10px; }";
//   html += "h1 { color: #0056b3; }";
//   html += ".value { font-size: 2em; font-weight: bold; }";
//   html += ".status0 { color: #28a745; font-weight: bold; font-size: 1.5em; }"; 
//   html += ".status1 { color: #ffc107; font-weight: bold; font-size: 1.5em; }"; 
//   html += ".status2 { color: #dc3545; font-weight: bold; font-size: 1.5em; animation: blinker 1s linear infinite; }"; 
//   html += "@keyframes blinker { 50% { opacity: 0; } }";
//   html += "</style></head><body>";
  
//   html += "<h1>🌍 TRẠM GIÁM SÁT CHẤT LƯỢNG KHÔNG KHÍ</h1>";
//   html += "<div class=\"card\"><h2>🌫️ Nồng độ CO</h2><p class=\"value\" style=\"color:#555\">" + String(co) + " <small>ppm</small></p></div>";
//   html += "<div class=\"card\"><h2>💨 Bụi PM2.5</h2><p class=\"value\" style=\"color:#555\">" + String(pm) + " <small>µg/m³</small></p></div>";

//   html += "<div><p>Trạng thái hệ thống:</p>";
//   if(status == 0) html += "<p class=\"status0\">✅ CHẤT LƯỢNG: TỐT</p>";
//   else if(status == 1) html += "<p class=\"status1\">⚠️ CHẤT LƯỢNG: KÉM</p>";
//   else html += "<p class=\"status2\">🚨 CẢNH BÁO: NGUY HIỂM!</p>";
//   html += "</div></body></html>";
  
//   return html;
// }

// void handleRoot() {
//   server.send(200, "text/html", getHTML(coValue, pmValue, currentStatus));
// }

// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);
  
//   pinMode(PIN_LED_GREEN, OUTPUT);
//   pinMode(PIN_LED_YELLOW, OUTPUT);
//   pinMode(PIN_LED_RED, OUTPUT);
  
//   // Khởi tạo luồng riêng cho LED (Core 0), giúp LED chớp đều dù WiFi hay Telegram bị lag
//   xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 0);

//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//     Serial.println(F("❌ Lỗi khởi tạo SSD1306"));
//     for(;;);
//   }
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0, 10);
//   display.println(F("Air Monitor IoT"));
//   display.println(F("Starting WiFi..."));
//   display.display();

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
//   server.on("/", handleRoot);
//   server.begin();

//   Blynk.config(BLYNK_AUTH_TOKEN);
//   secured_client.setInsecure(); // Bắt buộc cho ESP32 gọi API Telegram
  
//   display.clearDisplay();
//   display.setCursor(0, 10);
//   display.println(F("WiFi Connected!"));
//   display.display();
//   Serial.println("\n🌐 Đã kết nối WiFi. Sẵn sàng hoạt động!");
  
//   if (!isTelegramEnabled()) {
//     Serial.println("⚠️ CHÚ Ý: Bạn chưa điền BOT_TOKEN. Tính năng Telegram tạm thời bị vô hiệu hóa để tránh lag hệ thống.");
//   }
  
//   delay(1000);
// }

// // ---------------- VÒNG LẶP CHÍNH ----------------
// void loop() {
//   Blynk.run();
//   server.handleClient();
  
//   // 2. Kiểm tra tin nhắn Telegram (Chỉ chạy khi đã điền Token thật)
//   if (isTelegramEnabled() && millis() - lastBotCheck > botCheckInterval) {
//     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//     while (numNewMessages) {
//       handleNewMessages(numNewMessages);
//       numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//     }
//     lastBotCheck = millis();
//   }
  
//   // 3. Đọc cảm biến và cập nhật trạng thái
//   if (millis() - lastUpdate > updateInterval) {
//     lastUpdate = millis();
    
//     int rawCO = analogRead(PIN_CO_SENSOR);
//     coValue = map(rawCO, 0, 4095, 0, 2000); 
    
//     int rawPM = analogRead(PIN_PM_SENSOR);
//     pmValue = map(rawPM, 0, 4095, 0, 150);  

//     // Bổ sung: In liên tục ra Terminal để theo dõi dễ dàng
//     Serial.printf("📊 Đang đọc... CO: %d ppm | PM2.5: %d ug/m3\n", coValue, pmValue);

//     Blynk.virtualWrite(V1, coValue);
//     Blynk.virtualWrite(V2, pmValue);

//     // Xác định trạng thái
//     int newStatus = 0;
//     if (coValue > 1000 || pmValue > 50) newStatus = 2;      // Nguy hiểm
//     else if (coValue > 400 || pmValue > 20) newStatus = 1;  // Xấu
//     else newStatus = 0;                                     // Tốt
    
//     // Cập nhật OLED
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setCursor(0, 0);
//     display.println(F("--- AIR QUALITY ---"));
//     display.setTextSize(2);
//     display.setCursor(0, 16);
//     display.printf("CO: %d\n", coValue);
//     display.printf("PM: %d\n", pmValue);
//     display.setTextSize(1);
//     display.setCursor(0, 50);
//     if (newStatus == 0) display.print(F("Status: GOOD (SAFE)"));
//     else if (newStatus == 1) display.print(F("Status: BAD (WARN)"));
//     else display.print(F("Status: DANGER!"));
//     display.display();

//     // 4. KIỂM TRA SỰ THAY ĐỔI TRẠNG THÁI ĐỂ THÔNG BÁO
//     if (newStatus != currentStatus) {
//       currentStatus = newStatus;
      
//       String consoleMsg = "";
//       String teleMsg = "";

//       if (currentStatus == 0) {
//         consoleMsg = "✅ CHUYỂN TRẠNG THÁI: TỐT | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "✅ CHẤT LƯỢNG KHÔNG KHÍ ĐÃ TỐT LÊN!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
//         Blynk.virtualWrite(V3, "An toàn");
//       } 
//       else if (currentStatus == 1) {
//         consoleMsg = "⚠️ CHUYỂN TRẠNG THÁI: KÉM | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "⚠️ CHÚ Ý: CHẤT LƯỢNG KHÔNG KHÍ KÉM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
//         Blynk.virtualWrite(V3, "Cảnh báo");
//       } 
//       else if (currentStatus == 2) {
//         consoleMsg = "🚨 CHUYỂN TRẠNG THÁI: NGUY HIỂM | CO: " + String(coValue) + " | PM: " + String(pmValue);
//         teleMsg = "🚨 BÁO ĐỘNG ĐỎ: KHÔNG KHÍ NGUY HIỂM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³\n🏃‍♂️ Vui lòng có biện pháp xử lý ngay!";
//         Blynk.virtualWrite(V3, "NGUY HIỂM");
//       }

//       // In ra Terminal (VS Code / Arduino IDE)
//       Serial.println("\n==============================================");
//       Serial.println(consoleMsg);
//       Serial.println("==============================================");
      
//       // Gửi Telegram (Kiểm tra Token trước khi gửi)
//       if (isTelegramEnabled()) {
//         bot.sendMessage(CHAT_ID, teleMsg, "");
//       }
//     }
//   }
// }





// VER 3333333333333 -----------------------

// ---------------- CẤU HÌNH CHÂN (PINS) ----------------
#define PIN_CO_SENSOR 39  // Chân VN (pot1)
#define PIN_PM_SENSOR 36  // Chân VP (pot2)
#define PIN_LED_GREEN 4   // Đèn báo Tốt (led1)
#define PIN_LED_YELLOW 5  // Đèn báo Xấu (led2)
#define PIN_LED_RED 2     // Đèn báo Nguy hiểm (led3)

// Khởi tạo màn hình OLED SSD1306 (Kích thước 128x64)
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

// Biến điều khiển chế độ đọc cảm biến
bool useSimulatedData = false; 

// Biến cho LED nhấp nháy (FreeRTOS)
bool ledState = false;

// Biến cho Telegram Bot
unsigned long lastBotCheck = 0;
const int botCheckInterval = 2000; // Kiểm tra mỗi 2 giây

// Hàm kiểm tra xem đã điền Telegram Token chưa
bool isTelegramEnabled() {
  return String(BOT_TOKEN) != "ĐIỀN_BOT_TOKEN_VÀO_ĐÂY";
}

// ---------------- TASK ĐA LUỒNG CHO LED NHẤP NHÁY ----------------
void blinkTask(void * pvParameters) {
  for (;;) {
    int blinkInterval = 1000; // Mặc định chớp 1s (Tốt)
    if (currentStatus == 1) blinkInterval = 700;       // Kém: chớp 0.7s
    else if (currentStatus == 2) blinkInterval = 500;  // Nguy hiểm: chớp 0.5s

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
  }
}

// ---------------- HÀM XỬ LÝ LỆNH TELEGRAM ----------------
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    
    Serial.println("💬 Nhận lệnh Telegram: " + text);

    if (text == "/start") {
      String welcome = "👋 Xin chào " + from_name + "!\n";
      welcome += "🤖 Tôi là Bot Giám sát Không Khí.\n\n";
      welcome += "📋 CÁC LỆNH ĐIỀU KHIỂN:\n";
      welcome += "/status : Xem thông số hiện tại\n";
      welcome += "/pot_good : Set thông số TỐT\n";
      welcome += "/pot_bad : Set thông số CẢNH BÁO\n";
      welcome += "/pot_danger : Set thông số NGUY HIỂM\n";
      welcome += "/pot_real : Đọc lại từ mạch thật (Slider)";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/status") {
      String statMsg = "📊 THÔNG SỐ HIỆN TẠI:\n";
      statMsg += "🌫️ Nồng độ CO: " + String(coValue) + " ppm\n";
      statMsg += "💨 Bụi PM2.5: " + String(pmValue) + " µg/m³\n";
      
      if (currentStatus == 0) statMsg += "✅ Trạng thái: TỐT (An toàn)";
      else if (currentStatus == 1) statMsg += "⚠️ Trạng thái: KÉM (Cảnh báo)";
      else if (currentStatus == 2) statMsg += "🚨 Trạng thái: NGUY HIỂM!";
      bot.sendMessage(chat_id, statMsg, "");
    }
    else if (text == "/pot_good") {
      useSimulatedData = true;
      coValue = random(10, 350); // Mức Tốt: CO < 400
      pmValue = random(5, 18);   // Mức Tốt: PM < 20
      bot.sendMessage(chat_id, "🔧 Đã chỉnh giả lập mức: TỐT (Xanh)", "");
      Serial.println(">>> Đã kích hoạt Mode: /pot_good");
    }
    else if (text == "/pot_bad") {
      useSimulatedData = true;
      coValue = random(450, 900); // Mức Xấu: 400 < CO < 1000
      pmValue = random(25, 45);   // Mức Xấu: 20 < PM < 50
      bot.sendMessage(chat_id, "🔧 Đã chỉnh giả lập mức: KÉM (Vàng)", "");
      Serial.println(">>> Đã kích hoạt Mode: /pot_bad");
    }
    else if (text == "/pot_danger") {
      useSimulatedData = true;
      coValue = random(1200, 1900); // Nguy hiểm: CO > 1000
      pmValue = random(70, 140);    // Nguy hiểm: PM > 50
      bot.sendMessage(chat_id, "🔧 Đã chỉnh giả lập mức: NGUY HIỂM (Đỏ)", "");
      Serial.println(">>> Đã kích hoạt Mode: /pot_danger");
    }
    else if (text == "/pot_real") {
      useSimulatedData = false;
      bot.sendMessage(chat_id, "🔧 Đã quay lại đọc cảm biến từ phần cứng!", "");
      Serial.println(">>> Đã quay lại Mode: /pot_real");
    }
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
  html += "<div class=\"card\"><h2>🌫️ Nồng độ CO</h2><p class=\"value\" style=\"color:#555\">" + String(co) + " <small>ppm</small></p></div>";
  html += "<div class=\"card\"><h2>💨 Bụi PM2.5</h2><p class=\"value\" style=\"color:#555\">" + String(pm) + " <small>µg/m³</small></p></div>";

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
  Serial.begin(115200);
  delay(1000); // Cho Serial khởi động hoàn toàn
  Serial.println("\n\n--- HỆ THỐNG ĐANG KHỞI ĐỘNG ---");
  
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  
  xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 0);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("❌ Lỗi khởi tạo SSD1306"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(F("Air Monitor IoT"));
  display.println(F("Starting WiFi..."));
  display.display();

  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\n🌐 Đã kết nối WiFi. IP: " + WiFi.localIP().toString());
  
  server.on("/", handleRoot);
  server.begin();

  Blynk.config(BLYNK_AUTH_TOKEN);
  secured_client.setInsecure(); 
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(F("WiFi Connected!"));
  display.display();
  
  if (!isTelegramEnabled()) {
    Serial.println("⚠️ CHÚ Ý: Bạn chưa điền BOT_TOKEN.");
  }
}

// ---------------- VÒNG LẶP CHÍNH ----------------
void loop() {
  Blynk.run();
  server.handleClient();
  
  // 1. Kiểm tra tin nhắn Telegram
  if (isTelegramEnabled() && millis() - lastBotCheck > botCheckInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotCheck = millis();
  }
  
  // 2. Đọc cảm biến và cập nhật trạng thái
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    
    // Nếu KHÔNG sử dụng lệnh giả lập thì mới đọc từ phần cứng (Slider)
    if (!useSimulatedData) {
      int rawCO = analogRead(PIN_CO_SENSOR);
      coValue = map(rawCO, 0, 4095, 0, 2000); 
      
      int rawPM = analogRead(PIN_PM_SENSOR);
      pmValue = map(rawPM, 0, 4095, 0, 150);  
    }

    // In thông số ra VS Code / Wokwi Terminal đồng bộ
    Serial.printf("📊 Thông số hiện tại -> CO: %d ppm | PM2.5: %d ug/m3 | Mode: %s\n", 
                   coValue, pmValue, useSimulatedData ? "TELEGRAM" : "SLIDER");

    Blynk.virtualWrite(V1, coValue);
    Blynk.virtualWrite(V2, pmValue);

    // Xác định trạng thái
    int newStatus = 0;
    if (coValue > 1000 || pmValue > 50) newStatus = 2;      // Nguy hiểm
    else if (coValue > 400 || pmValue > 20) newStatus = 1;  // Xấu
    else newStatus = 0;                                     // Tốt
    
    // Cập nhật OLED
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
    if (newStatus == 0) display.print(F("Status: GOOD (SAFE)"));
    else if (newStatus == 1) display.print(F("Status: BAD (WARN)"));
    else display.print(F("Status: DANGER!"));
    display.display();

    // KIỂM TRA SỰ THAY ĐỔI TRẠNG THÁI ĐỂ THÔNG BÁO TỚI TELEGRAM VÀ TERMINAL
    if (newStatus != currentStatus) {
      currentStatus = newStatus;
      
      String consoleMsg = "";
      String teleMsg = "";

      if (currentStatus == 0) {
        consoleMsg = "✅ [CHUYỂN TRẠNG THÁI]: TỐT | CO: " + String(coValue) + " | PM: " + String(pmValue);
        teleMsg = "✅ CHẤT LƯỢNG KHÔNG KHÍ ĐÃ TỐT LÊN!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
        Blynk.virtualWrite(V3, "An toàn");
      } 
      else if (currentStatus == 1) {
        consoleMsg = "⚠️ [CHUYỂN TRẠNG THÁI]: KÉM | CO: " + String(coValue) + " | PM: " + String(pmValue);
        teleMsg = "⚠️ CHÚ Ý: CHẤT LƯỢNG KHÔNG KHÍ KÉM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³";
        Blynk.virtualWrite(V3, "Cảnh báo");
      } 
      else if (currentStatus == 2) {
        consoleMsg = "🚨 [CHUYỂN TRẠNG THÁI]: NGUY HIỂM | CO: " + String(coValue) + " | PM: " + String(pmValue);
        teleMsg = "🚨 BÁO ĐỘNG ĐỎ: KHÔNG KHÍ NGUY HIỂM!\n🌫️ CO: " + String(coValue) + " ppm\n💨 PM2.5: " + String(pmValue) + " µg/m³\n🏃‍♂️ Vui lòng có biện pháp xử lý ngay!";
        Blynk.virtualWrite(V3, "NGUY HIỂM");
      }

      // In dải phân cách và tin nhắn ra Terminal
      Serial.println("\n==============================================");
      Serial.println(consoleMsg);
      Serial.println("==============================================\n");
      
      // Gửi Telegram
      if (isTelegramEnabled()) {
        bot.sendMessage(CHAT_ID, teleMsg, "");
      }
    }
  }
}