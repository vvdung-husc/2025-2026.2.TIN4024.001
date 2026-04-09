#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- THÔNG TIN CẤU HÌNH ---
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "Wokwi-GUEST"; // Đổi thành WiFi nhà khi nạp vào máy thật
char pass[] = ""; 

// Telegram
#define BOTtoken "YOUR_TELEGRAM_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"

// MQTT Broker
const char* mqtt_server = "broker.emqx.io";
const char* topic_sub = "nha/thongminh/den";
const char* topic_pub = "nha/thongminh/trangthai";

// Phần cứng
const int relayPin = 2;
const int btnPin = 0;
bool relayState = LOW;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiClientSecure clientSecure;
UniversalTelegramBot bot(BOTtoken, clientSecure);

unsigned long lastTimeBotRan;
int botRequestDelay = 1000; // 1 giây kiểm tra Telegram 1 lần

// --- HÀM CẬP NHẬT TRẠNG THÁI CHUNG ---
void updateSwitch(bool state, String source) {
  relayState = state;
  digitalWrite(relayPin, relayState);
  
  // Đồng bộ lên Blynk
  Blynk.virtualWrite(V1, relayState);
  
  // Đồng bộ lên MQTT
  String msg = relayState ? "ON" : "OFF";
  mqttClient.publish(topic_pub, msg.c_str());

  Serial.println("Trạng thái: " + msg + " (Nguồn: " + source + ")");
}

// Xử lý lệnh từ Blynk
BLYNK_WRITE(V1) {
  updateSwitch(param.asInt(), "Blynk App");
}

// Xử lý lệnh từ MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  if (message == "ON") updateSwitch(HIGH, "MQTT");
  else if (message == "OFF") updateSwitch(LOW, "MQTT");
}

// Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/on") updateSwitch(HIGH, "Telegram");
    else if (text == "/off") updateSwitch(LOW, "Telegram");
    else if (text == "/status") bot.sendMessage(chat_id, relayState ? "Đèn đang BẬT" : "Đèn đang TẮT", "");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);
  
  Blynk.begin(auth, ssid, pass);
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
  
  clientSecure.setInsecure(); // Cho phép Telegram chạy mà không cần chứng chỉ phức tạp
}

void loop() {
  Blynk.run();
  
  if (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32_SmartSwitch")) mqttClient.subscribe(topic_sub);
  }
  mqttClient.loop();

  // Nút nhấn vật lý
  if (digitalRead(btnPin) == LOW) {
    delay(200);
    updateSwitch(!relayState, "Nút nhấn tay");
    while(digitalRead(btnPin) == LOW);
  }

  // Telegram polling
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}