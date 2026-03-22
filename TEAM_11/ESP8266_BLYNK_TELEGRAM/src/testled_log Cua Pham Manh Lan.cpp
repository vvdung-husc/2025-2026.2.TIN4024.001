// đã làm vào ngày đi học nhưng bị người trong nhóm xóa (xác nhận có đi học, điểm danh)
// Có bằng chứng trên tin nhắn telegram (nội dung test với chat và ảnh chụp khi test thành công)


// #include <Arduino.h>
// #include <WiFi.h> // Sử dụng thư viện WiFi của ESP32
// #include <WiFiClientSecure.h>
// #include <UniversalTelegramBot.h>
// #include <ArduinoJson.h>

// const char* ssid = "Wokwi-GUEST";
// const char* password = "";

// #define BOTtoken "8482351137:AAG3HGrN3ofuROEq9GzZgOMKYjh4nTKgft8"
// #define GROUP_ID "-5204265134"

// // Khai báo chân cho ESP32
// const int ledPin = 23;
// const int motionSensor = 27;

// // ================= TELEGRAM =================
// WiFiClientSecure client;
// UniversalTelegramBot bot(BOTtoken, client);

// bool motionDetected = false;

// unsigned long lastTimeBotRan = 0;
// const unsigned long botRequestDelay = 1000;

// // Sử dụng IRAM_ATTR cho ESP32
// void IRAM_ATTR detectsMovement() {
//   motionDetected = true;
// }

// void handleNewMessages(int numNewMessages) {
//   for (int i = 0; i < numNewMessages; i++) {
//     String chat_id = bot.messages[i].chat_id;
//     String text = bot.messages[i].text;
//     String from_name = bot.messages[i].from_name;

//     if (text == "/start") {
//       String welcome = "Xin chào " + from_name + "\n";
//       welcome += "Các lệnh điều khiển:\n";
//       welcome += "/led_on : Bật LED\n";
//       welcome += "/led_off : Tắt LED\n";
//       welcome += "/led_status : Trạng thái LED";

//       bot.sendMessage(chat_id, welcome, "");
//     }

//     if (text == "/led_on") {
//       digitalWrite(ledPin, HIGH);
//       bot.sendMessage(chat_id, "LED đã bật", "");
//     }

//     if (text == "/led_off") {
//       digitalWrite(ledPin, LOW);
//       bot.sendMessage(chat_id, "LED đã tắt", "");
//     }

//     if (text == "/led_status") {
//       if (digitalRead(ledPin)) {
//         bot.sendMessage(chat_id, "LED đang bật", "");
//       } else {
//         bot.sendMessage(chat_id, "LED đang tắt", "");
//       }
//     }
//   }
// }

// void setup() {
//   Serial.begin(115200);

//   pinMode(ledPin, OUTPUT);
//   digitalWrite(ledPin, LOW);

//   pinMode(motionSensor, INPUT_PULLUP);

//   attachInterrupt(
//     digitalPinToInterrupt(motionSensor),
//     detectsMovement,
//     RISING
//   );

//   Serial.print("Connecting WiFi ");
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("\nWiFi Connected");

//   // Yêu cầu cho ESP32 khi dùng WiFiClientSecure
//   client.setInsecure();

//   bot.sendMessage(
//     GROUP_ID,
//     "ESP32 IoT System Started",
//     ""
//   );
// }

// void loop() {
//   if (motionDetected) {
//     Serial.println("Motion detected");

//     bot.sendMessage(
//       GROUP_ID,
//       "Cảnh báo: Phát hiện chuyển động!",
//       ""
//     );

//     motionDetected = false;
//   }

//   if (millis() > lastTimeBotRan + botRequestDelay) {
//     int numNewMessages = bot.getUpdates(
//       bot.last_message_received + 1
//     );

//     while (numNewMessages) {
//       handleNewMessages(numNewMessages);

//       numNewMessages = bot.getUpdates(
//         bot.last_message_received + 1
//       );
//     }

//     lastTimeBotRan = millis();
//   }
// }    

// =================================================================
// {
//   "version": 1,
//   "author": "Assistant",
//   "editor": "wokwi",
//   "parts": [
//     { "type": "board-esp32-devkit-v1", "id": "esp", "top": 0, "left": 0, "attrs": {} },
//     {
//       "type": "wokwi-pir-motion-sensor",
//       "id": "pir1",
//       "top": -120,
//       "left": -150,
//       "attrs": {}
//     },
//     {
//       "type": "wokwi-led",
//       "id": "led1",
//       "top": -80,
//       "left": 150,
//       "attrs": { "color": "red" }
//     },
//     {
//       "type": "wokwi-resistor",
//       "id": "r1",
//       "top": -30,
//       "left": 150,
//       "attrs": { "value": "220" }
//     }
//   ],
//   "connections": [
//     [ "esp:3V3", "pir1:VCC", "red", [ "v0" ] ],
//     [ "esp:GND", "pir1:GND", "black", [ "v0" ] ],
//     [ "esp:D27", "pir1:OUT", "yellow", [ "v0" ] ],
//     [ "esp:D23", "r1:1", "green", [ "v0" ] ],
//     [ "r1:2", "led1:A", "green", [ "v0" ] ],
//     [ "esp:GND", "led1:C", "black", [ "v0" ] ]
//   ],
//   "dependencies": {}
// }