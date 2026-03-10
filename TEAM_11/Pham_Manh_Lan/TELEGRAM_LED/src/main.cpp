// Thêm biến này (global)
unsigned long lastTimeBotRan = 0;
const unsigned long BOT_MTBS = 1000;  // Check Telegram mỗi 1 giây (nhỏ hơn = nhanh hơn, nhưng tốn CPU)
#include <Arduino.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "8585335448:AAHH4HjWNbcSVrauA47sDf-y8DvDWuoP_e0"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-4265908169" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
bool motionDetected = false;

//Định dạng chuỗi %s,%d,...
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}



void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== IoT Telegram Motion Detection - Starting ===");

  pinMode(motionSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.printf("Connecting to WiFi: %s ", ssid);

  // FIX DNS: Dùng IP tĩnh để bypass DHCP DNS (10.13.37.1 rác)
  IPAddress localIP(10, 13, 37, 55);       // Chọn IP chưa dùng (tránh trùng .2)
  IPAddress gateway(10, 13, 37, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);         // Google DNS
  IPAddress secondaryDNS(1, 1, 1, 1);       // Cloudflare DNS – thường fix tốt hơn

  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("\n[ERROR] WiFi.config failed!");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Timeout dài hơn, in dấu chấm mỗi 500ms
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 30000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[CRITICAL] WiFi connect FAILED after 30s! Restarting...");
    ESP.restart();  // Tự reset nếu fail
  }

  Serial.println("\nWiFi connected SUCCESS!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("DNS primary: %s\n", WiFi.dnsIP(0).toString().c_str());
  Serial.printf("DNS secondary: %s\n", WiFi.dnsIP(1).toString().c_str());

  client.setInsecure();  // Bắt buộc cho HTTPS Telegram trên Wokwi

  Serial.println("Waiting for network full stable (Wokwi needs this)...");
  delay(10000);  // Tăng lên 10 giây – nhiều case fix nhờ delay dài

  // Test DNS resolve ngay lập tức (debug siêu quan trọng)
  IPAddress resolved;
  if (WiFi.hostByName("api.telegram.org", resolved)) {
    Serial.printf("DNS TEST SUCCESS: api.telegram.org -> %s\n", resolved.toString().c_str());
  } else {
    Serial.println("[DNS TEST FAIL] Still cannot resolve api.telegram.org!");
  }

  // Gửi test message với info đầy đủ
  String testMsg = "Device ONLINE!\n"
                   "IP: " + WiFi.localIP().toString() + "\n"
                   "DNS: " + WiFi.dnsIP(0).toString() + "\n"
                   "Uptime: " + String(millis()/1000) + "s";

  Serial.println("Sending test message to Telegram...");
  if (bot.sendMessage(GROUP_ID, testMsg, "")) {
    Serial.println("→ Test message SENT OK!");
  } else {
    Serial.println("→ Send FAILED – check token/GROUP_ID or DNS still broken");
  }

  Serial.println("=== Setup done – ready for motion detection ===");
}


void loop() {
  static uint count_ = 0;

  if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    motionDetected = false;
  }
}