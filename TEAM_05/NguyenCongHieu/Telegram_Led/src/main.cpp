#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "lwip/dns.h"
#include "secrets.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const int ledPin = 23;
const int pirPin = 27;

bool ledState = LOW;
int botRequestDelay = 2000;
unsigned long lastTimeBotRan;

int pirState = LOW;
int lastPirState = LOW;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Bạn không có quyền điều khiển bot này!", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
      welcome += "Gửi /led_on bật sáng đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_on") {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED bật sáng", "");
    }
    else if (text == "/led_off") {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED is OFF", "");
    }
    else if (text == "/get_state") {
      if (ledState == HIGH) {
        bot.sendMessage(chat_id, "LED is ON", "");
      } else {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, ledState);

  Serial.print("Connecting to WiFi");

  // 1. Cài DNS chính (8.8.8.8) và DNS phụ (8.8.4.4) của Google
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, primaryDNS, secondaryDNS);

  // 2. Thêm tham số 6 (kênh WiFi) giúp giả lập Wokwi kết nối mượt hơn
  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 3. Override DNS trực tiếp tầng LwIP (bypass Wokwi DNS override)
  ip_addr_t dnsServer;
  IP4_ADDR(&dnsServer.u_addr.ip4, 8, 8, 8, 8);
  dnsServer.type = IPADDR_TYPE_V4;
  dns_setserver(0, &dnsServer);
  Serial.print("DNS override: 8.8.8.8 -> ");
  Serial.println(WiFi.dnsIP());

  // 4. Bỏ qua xác thực chứng chỉ SSL
  secured_client.setInsecure();
  secured_client.setTimeout(15);

  // 5. Chờ DNS ổn định
  delay(1500);
}

void loop() {
  // Kiểm tra tin nhắn Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  // Kiểm tra cảm biến PIR
  pirState = digitalRead(pirPin);
  if (pirState == HIGH && lastPirState == LOW) {
    Serial.println("Phát hiện chuyển động!");
    bot.sendMessage(CHAT_ID, "CẢNH BÁO: Phát hiện có chuyển động!", "");
    lastPirState = HIGH;
  } else if (pirState == LOW && lastPirState == HIGH) {
    lastPirState = LOW;
  }
}