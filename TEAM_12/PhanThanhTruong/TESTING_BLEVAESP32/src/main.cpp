#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 
#include <HTTPClient.h> // Thư viện dùng để gọi API ThingSpeak

// --- CẤU HÌNH WIFI & TELEGRAM ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define BOT_TOKEN "8630518571:AAF3dp1Gyonj1fnnvOz17jg9VTFdaRjmAco"
#define CHAT_ID "-1003733920308"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- CẤU HÌNH THINGSPEAK API ---
const char* serverThingSpeak = "http://api.thingspeak.com/update";
String apiKeyThingSpeak = "FG2BE9VD46S830XR"; // Mã Write API Key lấy từ ảnh của bạn
unsigned long lastThingSpeakTime = 0;
const unsigned long THINGSPEAK_INTERVAL = 15000; // Gửi dữ liệu 15 giây/lần

// --- KHAI BÁO CÁC CHÂN LINH KIỆN ---
#define DHTPIN 15
#define DHTTYPE DHT22
#define LED_PIN 2
#define RELAY_PIN 4      
#define BUZZER_PIN 5     
#define SERVO_PIN 18     
#define POT_PIN 34       

DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- BIẾN TRẠNG THÁI HỆ THỐNG & ĐỊNH VỊ ---
const float DISTANCE_THRESHOLD = 3.0; 
const unsigned long TIMEOUT = 10000;  
unsigned long lastPresenceTime = 0;   
bool isRoomActive = false;            

// Các biến phục vụ tính năng Live Tracking (Telegram)
float lastReportedDistance = -1.0;     
unsigned long lastMessageTime = 0;     
const unsigned long MSG_COOLDOWN = 8000; 
const float MOVEMENT_SENSITIVITY = 0.5;  

// --- HÀM ĐIỀU KHIỂN SERVO ---
void setServoAngle(int angle) {
  int dutyCycle = map(angle, 0, 180, 1638, 8192);
  ledcWrite(2, dutyCycle);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  ledcSetup(2, 50, 16);
  ledcAttachPin(SERVO_PIN, 2);

  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Lỗi màn hình OLED");
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  setServoAngle(0); 
}

void loop() {
  int potVal = analogRead(POT_PIN);
  float distance = (potVal / 4095.0) * 10.0; 
  float temp = dht.readTemperature();

  // --------------------------------------------------------
  // 1. LOGIC HIỆN DIỆN & LIVE TRACKING (TELEGRAM & ĐIỀU KHIỂN)
  // --------------------------------------------------------
  if (distance <= DISTANCE_THRESHOLD) {
    lastPresenceTime = millis();
    
    // Vừa mới bước vào phòng
    if (!isRoomActive) {
      isRoomActive = true;
      lastReportedDistance = distance;
      lastMessageTime = millis();
      
      Serial.println(">> CÓ NGƯỜI: Mở rèm, bật đèn, bật điều hòa!");
      digitalWrite(LED_PIN, HIGH);     
      digitalWrite(RELAY_PIN, HIGH);   
      setServoAngle(90);               
      tone(BUZZER_PIN, 1500, 200);     
      
      String msg = "🟢 *CÓ NGƯỜI ĐANG Ở TRONG PHÒNG*\n";
      msg += "📱 *Thiết bị quét:* Khách/Admin (BLE ID: A1:B2...)\n";
      msg += "📍 *Vị trí hiện tại:* " + String(distance, 1) + " mét\n";
      msg += "🌡 *Nhiệt độ:* " + String(temp, 1) + " °C\n";
      msg += "⚙️ *Hệ thống đã tự động BẬT điện & MỞ rèm.*";
      bot.sendMessage(CHAT_ID, msg, "Markdown");
      
    } else {
      // Đang di chuyển trong phòng
      if (fabs(distance - lastReportedDistance) >= MOVEMENT_SENSITIVITY && (millis() - lastMessageTime > MSG_COOLDOWN)) {
        lastReportedDistance = distance;
        lastMessageTime = millis();
        
        Serial.println(">> CẬP NHẬT DI CHUYỂN: " + String(distance, 1) + "m");
        
        String msg = "🚶 *CẬP NHẬT DI CHUYỂN*\n";
        msg += "📍 *Vị trí mới:* Cách tâm điểm " + String(distance, 1) + " mét\n";
        msg += "🌡 *Nhiệt độ quanh thiết bị:* " + String(temp, 1) + " °C";
        bot.sendMessage(CHAT_ID, msg, "Markdown");
      }
    }
  } else {
    // Rời khỏi phòng (Vượt quá 3m) - Kích hoạt bộ đếm thời gian ân hạn
    if (isRoomActive && (millis() - lastPresenceTime > TIMEOUT)) {
      isRoomActive = false;
      lastReportedDistance = -1.0; 
      
      Serial.println(">> PHÒNG TRỐNG: Đóng rèm, tắt điện bảo vệ!");
      digitalWrite(LED_PIN, LOW);      
      digitalWrite(RELAY_PIN, LOW);    
      setServoAngle(0);                
      tone(BUZZER_PIN, 800, 1000);     
      
      String msg = "🔴 *PHÒNG HIỆN TẠI ĐANG TRỐNG !*\n";
      msg += "⚠️ *Cảnh báo:* Thiết bị đã ra khỏi vùng định vị (>3m)\n";
      msg += "📍 *Khoảng cách đo cuối:* " + String(distance, 1) + " mét\n";
      msg += "⚙️ *Đã TẮT toàn bộ thiết bị & ĐÓNG rèm.*";
      bot.sendMessage(CHAT_ID, msg, "Markdown");
    }
  }

  // --------------------------------------------------------
  // 2. LUỒNG THINGSPEAK API: VẼ BIỂU ĐỒ DI CHUYỂN
  // --------------------------------------------------------
  // Chỉ gửi dữ liệu khi có người trong phòng để tránh rác biểu đồ
  if (isRoomActive && (millis() - lastThingSpeakTime > THINGSPEAK_INTERVAL)) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Đóng gói URL chứa dữ liệu
      String serverPath = String(serverThingSpeak) + "?api_key=" + apiKeyThingSpeak + "&field1=" + String(distance, 2) + "&field2=" + String(temp, 1);
      
      http.begin(serverPath);
      int httpResponseCode = http.GET(); 
      
      if (httpResponseCode == 200) {
        Serial.print(">> Đã đẩy tọa độ lên ThingSpeak thành công! (Khoảng cách: ");
        Serial.print(distance);
        Serial.println("m)");
      } else {
        Serial.print(">> Lỗi gọi API ThingSpeak. HTTP Code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    lastThingSpeakTime = millis(); // Reset lại bộ đếm 15 giây
  }

  // --------------------------------------------------------
  // 3. CẬP NHẬT MÀN HÌNH OLED
  // --------------------------------------------------------
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("--- SMART ROOM ---");
  display.print("BLE Dist: "); display.print(distance, 1); display.println(" m");
  
  display.print("Status  : ");
  if (isRoomActive) {
    if (distance > DISTANCE_THRESHOLD) {
      int timeLeft = (TIMEOUT - (millis() - lastPresenceTime)) / 1000;
      display.print("Wait "); display.print(timeLeft); display.println("s");
    } else {
      display.println("USER PRESENT");
    }
  } else {
    display.println("EMPTY (AWAY)");
  }
  
  display.print("Temp    : "); display.print(temp, 1); display.println(" C");
  display.display();
  
  delay(100); 
}