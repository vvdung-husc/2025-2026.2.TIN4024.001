#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// 1. CẤU HÌNH MẠNG & THÔNG TIN KẾT NỐI
// ==========================================
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

#define BOT_TOKEN "8608717322:AAFs-BQsixyuwxh5Kppng-vHJ3zpIFTa6wQ"
#define CHAT_ID "-1003901034505"
const char* API_URL = "http://smarthome-truong.free.beeceptor.com";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ==========================================
// 2. KHAI BÁO CÁC CHÂN LINH KIỆN (GPIO)
// ==========================================
#define PIR_PIN 27       
#define SWITCH_PIN 14    
#define RELAY_PIN 4      
#define BUZZER_PIN 26    
#define LED_PIN 25       
#define SERVO_PIN 18     

// Cấu hình màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Cấu hình Keypad 4x4
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 15, 2}; 
byte colPins[COLS] = {0, 16, 17, 5};  
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==========================================
// 3. BIẾN TRẠNG THÁI & HẸN GIỜ
// ==========================================
enum SystemState { ARMED, DISARMED, LOCKDOWN };
SystemState currentState = ARMED;

const String MASTER_PIN = "1234";
String entered_pin = "";
int fail_count = 0;

unsigned long lastUpdateMillis = 0;       
unsigned long lastTrackMillis = 0;        
unsigned long lastBlinkMillis = 0;        
unsigned long lastOledMillis = 0;         
const unsigned long MSG_COOLDOWN = 10000; 

bool blinkState = false;

// ==========================================
// 4. CÁC HÀM XỬ LÝ CHỨC NĂNG
// ==========================================
void setServoAngle(int angle) {
  int dutyCycle = map(angle, 0, 180, 1638, 8192);
  ledcWrite(2, dutyCycle);
}

void updateOLED(String statusMsg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("--- SMART HOME ---");
  display.println("");
  display.setTextSize(2);
  display.println(statusMsg);
  
  if (currentState == ARMED) {
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("PIN: ");
    for(int i=0; i<entered_pin.length(); i++) display.print("*");
  }
  display.display();
}

// BẢN VÁ LỖI TIMING: Truyền trực tiếp trạng thái door và pir vào hàm thay vì đọc lại
void sendLogToAPI(String eventType, String location, String riskLevel, String doorState, int pirState) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client; 
    HTTPClient http;
    
    http.begin(client, API_URL); 
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["device_id"] = "ESP32_SMARTHOME_01";
    doc["event_type"] = eventType;
    doc["location"] = location;
    
    // Sử dụng trực tiếp tham số truyền vào
    doc["door_status"] = doorState;
    doc["pir_status"] = pirState;
    doc["risk_level"] = riskLevel;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    Serial.print("[API] HTTP Response Code: ");
    Serial.println(httpResponseCode); 
    http.end();
  }
}

// Hàm tiện ích để đọc trạng thái tức thời rồi gửi API
void logCurrentStateToAPI(String eventType, String location, String riskLevel) {
  String currentDoor = digitalRead(SWITCH_PIN) ? "OPEN" : "CLOSED";
  int currentPir = digitalRead(PIR_PIN);
  sendLogToAPI(eventType, location, riskLevel, currentDoor, currentPir);
}

// Hàm xử lý Telegram
void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;
      String chat_id = String(bot.messages[i].chat_id);
      
      if (chat_id != CHAT_ID) continue; 

      if (text == "/disarm") {
        currentState = DISARMED;
        setServoAngle(90); 
        digitalWrite(RELAY_PIN, LOW); 
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        bot.sendMessage(CHAT_ID, "✅ Đã giải trừ báo động từ xa qua Telegram!", "");
        logCurrentStateToAPI("REMOTE_DISARM", "System", "LOW");
        updateOLED("DISARMED");
      }
      else if (text == "/arm") {
        currentState = ARMED;
        setServoAngle(0); 
        entered_pin = "";
        bot.sendMessage(CHAT_ID, "🛡️ Hệ thống đã bật chế độ BẢO VỆ.", "");
        logCurrentStateToAPI("REMOTE_ARM", "System", "LOW");
        updateOLED("ARMED");
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ==========================================
// 5. SETUP THIẾT BỊ
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); 
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  ledcSetup(2, 50, 16);
  ledcAttachPin(SERVO_PIN, 2);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Lỗi OLED");
  }
  display.setTextColor(WHITE);
  updateOLED("CONNECTING...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  // Ưu tiên gửi log API trước khi bot.sendMessage khóa luồng
  logCurrentStateToAPI("SYSTEM_START", "Main_Door", "LOW");
  bot.sendMessage(CHAT_ID, "🔄 Hệ thống SmartHome đã khởi động. Đang ở chế độ ARMED.", "");
  
  setServoAngle(0); 
  digitalWrite(RELAY_PIN, LOW); 
  updateOLED("ARMED");
}

// ==========================================
// 6. VÒNG LẶP CHÍNH (STATE MACHINE)
// ==========================================
void loop() {
  if (millis() - lastUpdateMillis > 5000) {
    handleTelegramMessages();
    lastUpdateMillis = millis();
  }

  if (millis() - lastOledMillis > 200) {
    if(currentState == ARMED) updateOLED("ARMED");
    lastOledMillis = millis();
  }

  switch (currentState) {
    case ARMED: {
      char key = keypad.getKey();
      if (key) {
        if (key == '*' || key == '#') {
          entered_pin = ""; 
          Serial.println("\n[XÓA] Đã reset mật khẩu.");
        } else {
          entered_pin += key;
          tone(BUZZER_PIN, 2000, 50); 
          Serial.print("Đang nhập PIN: "); Serial.println(entered_pin);
        }

        if (entered_pin.length() == 4) {
          if (entered_pin == MASTER_PIN) {
            currentState = DISARMED;
            setServoAngle(90); 
            fail_count = 0;
            entered_pin = "";
            Serial.println("=> MẬT KHẨU ĐÚNG! Mở cửa...");
            
            logCurrentStateToAPI("AUTH_SUCCESS", "Main_Door", "LOW");
            bot.sendMessage(CHAT_ID, "🟢 CHỦ NHÀ ĐÃ VỀ: Mở cửa thành công.", "");
            
            updateOLED("DISARMED");
          } else {
            fail_count++;
            entered_pin = "";
            Serial.println("=> SAI MẬT KHẨU!");
            updateOLED("WRONG PIN");
            tone(BUZZER_PIN, 500, 500); 
            delay(1000); 
            
            if (fail_count >= 3) {
              Serial.println("=> BÁO ĐỘNG DÒ MẬT KHẨU!");
              logCurrentStateToAPI("BRUTE_FORCE_ATTACK", "Main_Door", "HIGH");
              bot.sendMessage(CHAT_ID, "🚨 CẢNH BÁO DÒ MẬT KHẨU: Có kẻ đang cố gắng can thiệp khóa cửa chính!", "");
              fail_count = 0; 
            }
          }
        }
      }

      // ĐỌC TRẠNG THÁI TỨC THỜI
      int currentPir = digitalRead(PIR_PIN);
      int currentDoor = digitalRead(SWITCH_PIN);

      if (currentDoor == HIGH || currentPir == HIGH) {
        currentState = LOCKDOWN;
        setServoAngle(0);              
        digitalWrite(RELAY_PIN, HIGH); 
        Serial.println("=> ĐỘT NHẬP! Chuyển sang chế độ LOCKDOWN.");
        
        String dStatus = (currentDoor == HIGH) ? "OPEN" : "CLOSED";
        
        // GỬI API NGAY LẬP TỨC để bắt đúng khoảnh khắc PIR = 1 hoặc Cửa OPEN
        sendLogToAPI("INTRUSION_DETECTED", "Living_Room", "CRITICAL", dStatus, currentPir);
        
        // Sau đó mới gửi Telegram
        bot.sendMessage(CHAT_ID, "🔴 BÁO ĐỘNG ĐỎ: Phát hiện đột nhập trái phép! Đã khóa cửa và phong tỏa thiết bị!", "");
        updateOLED("LOCKDOWN!");
      }
      break;
    }

    case DISARMED: {
      char key = keypad.getKey();
      if (key == '#') {
        currentState = ARMED;
        setServoAngle(0);
        entered_pin = "";
        Serial.println("=> BẬT LẠI BẢO VỆ.");
        
        logCurrentStateToAPI("LOCAL_ARM", "Main_Door", "LOW");
        bot.sendMessage(CHAT_ID, "🛡️ Đã bật lại chế độ bảo vệ từ bàn phím.", "");
        updateOLED("ARMED");
      }
      break;
    }

    case LOCKDOWN: {
      if (millis() - lastBlinkMillis > 300) {
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
        if(blinkState) tone(BUZZER_PIN, 1500); else noTone(BUZZER_PIN);
        lastBlinkMillis = millis();
      }

      // Tracking PIR liên tục
      if (digitalRead(PIR_PIN) == HIGH) {
        if (millis() - lastTrackMillis > MSG_COOLDOWN) {
          logCurrentStateToAPI("TRACKING_UPDATE", "Living_Room", "HIGH");
          bot.sendMessage(CHAT_ID, "🚶 CAMERA (PIR): Vẫn tiếp tục phát hiện kẻ gian đang di chuyển trong nhà!", "");
          lastTrackMillis = millis();
        }
      }
      break;
    }
  }
}