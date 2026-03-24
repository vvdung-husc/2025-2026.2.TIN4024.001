/*
THÔNG TIN NHÓM 6
1. Bùi Hữu Quốc - Telegram : Duy Pham
2. Lê Văn Tài - Telegram : Tài Lee
3. Hồ Tiến Bảo
4. Phạm Thanh Hiếu
*/

//===== BLYNK =====
#define BLYNK_TEMPLATE_ID "TMPL6qJNhA1AH"
#define BLYNK_TEMPLATE_NAME "TELEGRAM BLYNK"
#define BLYNK_AUTH_TOKEN "djyHz7_pY8ST0I3LuNkAoMW4ZFDCpIP8"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <TM1637Display.h>

//===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//===== TELEGRAM =====
#define BOT_TOKEN "8675633144:AAGFXaqphd2NC_rb9tX-333EdRveWTbenec"
#define CHAT_ID "-5229130370"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//===== PIN =====
#define DHTPIN D4
#define DHTTYPE DHT22
#define LED_PIN D1
#define BUTTON_PIN D3
#define PIR_PIN D5
#define GAS_PIN A0
#define CLK D6
#define DIO D7

//===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
TM1637Display sevseg(CLK, DIO);

//===== DATA =====
float temp = 0;
float hum = 0;
int gasValue = 0;
bool ledState = false;

unsigned long lastButtonPress = 0;
BlynkTimer timer;

//==================================
void updateDisplays()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println("IoT Team 6");

  display.print("T: ");
  display.print(temp, 1);
  display.print("  H: ");
  display.println(hum, 1);

  display.print("Gas: ");
  display.println(gasValue);

  display.print("PIR: ");
  display.print(digitalRead(PIR_PIN));
  display.print("  LED: ");
  display.println(ledState ? "ON" : "OFF");

  display.print("Up: ");
  display.print(millis() / 1000);
  display.println(" s");

  display.display();

  sevseg.showNumberDec((int)temp);
}

//==================================
void sendBlynk()
{
  Blynk.virtualWrite(V5, temp);
  Blynk.virtualWrite(V7, hum);
  Blynk.virtualWrite(V8, gasValue);
  Blynk.virtualWrite(V11, millis() / 1000);
}

//==================================
BLYNK_WRITE(V0)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

//==================================
void handleTelegram()
{
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numNewMessages; i++)
  {
    String text = bot.messages[i].text;

    if (text == "/start")
    {
      bot.sendMessage(CHAT_ID, "IoT Team 6\n/led_on\n/led_off\n/led_status\n/get_weather", "");
    }
    else if (text == "/led_on")
    {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V0, 1);
      bot.sendMessage(CHAT_ID, "LED ON", "");
    }
    else if (text == "/led_off")
    {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V0, 0);
      bot.sendMessage(CHAT_ID, "LED OFF", "");
    }
    else if (text == "/led_status")
    {
      bot.sendMessage(CHAT_ID, ledState ? "LED ON" : "LED OFF", "");
    }
    else if (text == "/get_weather")
    {
      String msg = "Temp: " + String(temp) + "C\nHum: " + String(hum) + "%";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

//==================================
void readSensor()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h))
  {
    temp = t;
    hum = h;
  }

  int rawGas = analogRead(A0);
  gasValue = map(rawGas, 0, 1023, 0, 100);
}

//==================================
void checkButton()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (millis() - lastButtonPress > 300)
    {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      Blynk.virtualWrite(V0, ledState);
      lastButtonPress = millis();
    }
  }
}

//==================================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  // I2C OLED
  Wire.begin(D2, D1);

  // TEST OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED 0x3C FAIL");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
    {
      Serial.println("OLED 0x3D FAIL");
      while (1); // dừng nếu không nhận
    }
  }

  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("OLED OK");
  display.display();
  delay(2000);

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Telegram
  client.setInsecure();
  client.setTimeout(1500);

  // Sensor
  dht.begin();

  // 7 segment
  sevseg.setBrightness(7);

  // Timer
  timer.setInterval(2000L, updateDisplays);
  timer.setInterval(3000L, readSensor);
  timer.setInterval(3000L, sendBlynk);
  timer.setInterval(10000L, handleTelegram);

  Serial.println("READY!");
}

//==================================
void loop()
{
  Blynk.run();
  timer.run();
  checkButton();
}