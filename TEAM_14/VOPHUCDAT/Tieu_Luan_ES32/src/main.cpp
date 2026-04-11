#define BLYNK_TEMPLATE_ID "TMPL6EsjlYA1y"
#define BLYNK_TEMPLATE_NAME "ESP32StepMotor"
#define BLYNK_AUTH_TOKEN "j3ChhLz-WwVkuPY6adR-FpTo7srKk1vs"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <AccelStepper.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ====== WiFi ======
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ====== Stepper ======
#define STEP_PIN 4
#define DIR_PIN 2
#define EN_PIN 5

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// ====== Control ======
int motorState = 0; // 0=STOP, 1=FWD, -1=BWD
int lastState = 99;

const int FIXED_SPEED = 500; // tốc độ cố định

// ====== Telegram ======
#define BOTtoken "8661218943:AAEbl_Qf1grCVWdOPgM9JbX9RolizR0WFtk"
#define CHAT_ID "6409998737"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);

const int BOT_DELAY = 1000; // polling interval (ms)

// Forward declaration
void handleNewMessages(int numNewMessages);

// ====== FreeRTOS task: Telegram chạy trên Core 0 ======
void telegramTask(void *pvParameters)
{
  unsigned long lastRan = 0;
  for (;;)
  {
    if (millis() - lastRan > BOT_DELAY)
    {
      int n = bot.getUpdates(bot.last_message_received + 1);
      while (n)
      {
        handleNewMessages(n);
        n = bot.getUpdates(bot.last_message_received + 1);
      }
      lastRan = millis();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // nhường CPU tránh watchdog
  }
}

// ====== Helper: trạng thái dạng chuỗi ======
String stateString()
{
  if (motorState == 1)
    return "FORWARD";
  if (motorState == -1)
    return "BACKWARD";
  return "STOP";
}

// ====== Blynk Buttons ======
BLYNK_WRITE(V0)
{ // STOP
  if (param.asInt())
    motorState = 0;
}

BLYNK_WRITE(V1)
{ // FORWARD
  if (param.asInt())
    motorState = 1;
}

BLYNK_WRITE(V2)
{ // BACKWARD
  if (param.asInt())
    motorState = -1;
}

// ====== Gửi trạng thái lên Blynk ======
void sendBlynkStatus()
{
  if (motorState != lastState)
  {
    lastState = motorState;
    Blynk.virtualWrite(V3, stateString());
  }
}

// ====== Reconnect sync ======
BLYNK_CONNECTED()
{
  lastState = 99; // force update lại
}

// ====== Xử lý tin nhắn Telegram ======
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    String name = bot.messages[i].from_name;
    Serial.println("Telegram: " + text);

    if (text == "/start")
    {
      String welcome = "Xin chào, " + name + "!\n";
      welcome += "Các lệnh điều khiển motor:\n";
      welcome += "/forward - Chạy thuận\n";
      welcome += "/backward - Chạy nghịch\n";
      welcome += "/stop - Dừng\n";
      welcome += "/status - Trạng thái hiện tại";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/forward")
    {
      motorState = 1;
      bot.sendMessage(chat_id, "✅ Motor: FORWARD", "");
    }
    else if (text == "/backward")
    {
      motorState = -1;
      bot.sendMessage(chat_id, "✅ Motor: BACKWARD", "");
    }
    else if (text == "/stop")
    {
      motorState = 0;
      bot.sendMessage(chat_id, "✅ Motor: STOP", "");
    }
    else if (text == "/status")
    {
      bot.sendMessage(chat_id, "Trạng thái hiện tại: " + stateString(), "");
    }
    else
    {
      bot.sendMessage(chat_id, "Lệnh không hợp lệ. Gõ /start để xem hướng dẫn.", "");
    }
  }
}

// ====== SETUP ======
void setup()
{
  Serial.begin(115200);

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);

  stepper.setMaxSpeed(1000);
  stepper.setSpeed(0);

  // Blynk tự kết nối WiFi
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Cấu hình TLS cho Telegram sau khi WiFi đã sẵn sàng
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Chạy Telegram polling trên Core 0 (Arduino loop mặc định ở Core 1)
  xTaskCreatePinnedToCore(
      telegramTask,   // hàm task
      "TelegramTask", // tên
      8192,           // stack size (bytes) – TLS cần nhiều stack
      NULL,           // parameter
      1,              // priority
      NULL,           // task handle
      0               // Core 0
  );
  Serial.println("Telegram bot ready.");
}

// ====== LOOP ======
void loop()
{
  Blynk.run();

  // ---- Điều khiển stepper ----
  if (motorState == 1)
  {
    digitalWrite(EN_PIN, LOW);
    stepper.setSpeed(FIXED_SPEED);
    stepper.runSpeed();
  }
  else if (motorState == -1)
  {
    digitalWrite(EN_PIN, LOW);
    stepper.setSpeed(-FIXED_SPEED);
    stepper.runSpeed();
  }
  else
  {
    stepper.setSpeed(0);
    digitalWrite(EN_PIN, HIGH); // disable driver
  }

  sendBlynkStatus();
}