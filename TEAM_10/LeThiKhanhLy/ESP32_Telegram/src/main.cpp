#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>


// Thay đổi mạng WiFi của bạn
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Khai báo Bot Telegram
#define BOTtoken "8653397522:AAF-N3Zb133P_hO5i8kMQ_eFyUzB9kTo5fE"  // Thay thế thành Bot token của bạn

#define CHAT_ID "8636449732"  //Thay thế thành ID người dùng của bạn

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Kiểm tra có tin nhắn mới hay không liên tục sau mỗi 1 giây
int bot_delay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 23;
bool ledState = LOW;

// Thực thi lệnh khi có tin nhắn mới
void handleNewMessages(int numNewMessages) {
  Serial.println("Handling New Message");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat ID của yêu cầu
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // In ra tin nhắn nhận được
    String user_text = bot.messages[i].text;
    Serial.println(user_text);

    String your_name = bot.messages[i].from_name;

    if (user_text == "/start") {
      String welcome = "Welcome, " + your_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "Send /led2_on to turn GPIO2 ON \n";
      welcome += "Send /led2_off to turn GPIO2 OFF \n";
      welcome += "Send /get_state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (user_text == "/led2_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (user_text == "/led2_off") {
      bot.sendMessage(chat_id, "LED state is set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (user_text == "/get_state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  client.setInsecure(); // QUAN TRỌNG
  client.setTimeout(15000);  
  pinMode(ledPin, OUTPUT);
  
}

void loop() {
  if (millis() > lastTimeBotRan + bot_delay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("Got Response!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}