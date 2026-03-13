#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

#define DHTPIN D3
#define DHTTYPE DHT22

#define LED_PIN LED_BUILTIN

#define GAS_PIN A0



DHT dht(DHTPIN, DHTTYPE);

// OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

float temperature = 0;
float humidity = 0;
int gasValue = 0;

unsigned long previousLedMillis = 0;
unsigned long previousDhtMillis = 0;

bool ledState = false;



// ===== Hàm khởi tạo =====
void setupSystem()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(D2, D1);

  u8g2.begin();

  dht.begin();

  Serial.println("System started");
}



// ===== Hàm đọc DHT =====
void readDHT()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C  ");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

}
//Hàm đọc khí gas
void readGas()
{
  gasValue = analogRead(GAS_PIN);

  Serial.print("Gas value: ");
  Serial.println(gasValue);

  // Serial.print("Analog A0 = ");
  // Serial.println(analogRead(A0));
}



// ===== Hàm nhấp nháy LED =====
void blinkLED()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousLedMillis >= 1000)
  {
    previousLedMillis = currentMillis;

    ledState = !ledState;

    digitalWrite(LED_PIN, ledState);
  }
}



// ===== Hàm hiển thị OLED =====
void displayOLED()
{
  // u8g2.clearBuffer();

  // u8g2.setFont(u8g2_font_ncenB08_tr);

  // u8g2.drawStr(0,10,"Ho Van Dien");

  // char tempStr[20];
  // sprintf(tempStr,"Temp: %.1f C",temperature);
  // u8g2.drawStr(0,30,tempStr);

  // char humStr[20];
  // sprintf(humStr,"Hum : %.1f %%",humidity);
  // u8g2.drawStr(0,50,humStr);

  // u8g2.sendBuffer();
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0,10,"Ho Van Dien");

  char tempStr[20];
  sprintf(tempStr,"Temp: %.1f C",temperature);
  u8g2.drawStr(0,25,tempStr);

  char humStr[20];
  sprintf(humStr,"Hum : %.1f %%",humidity);
  u8g2.drawStr(0,40,humStr);

  char gasStr[20];
  sprintf(gasStr,"Gas : %d",gasValue);
  u8g2.drawStr(0,55,gasStr);

  u8g2.sendBuffer();
}



void setup()
{
  setupSystem();
}



void loop()
{
  blinkLED();

  unsigned long currentMillis = millis();

  // đọc DHT mỗi 2 giây
  if (currentMillis - previousDhtMillis >= 2000)
  {
    previousDhtMillis = currentMillis;

    readDHT();
    readGas();
    displayOLED();
  }
  yield();
}