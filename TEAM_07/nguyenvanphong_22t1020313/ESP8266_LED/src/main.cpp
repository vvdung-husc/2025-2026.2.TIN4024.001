#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>

// ======================
// OLED SH1106
// ======================
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ======================
// DHT SENSOR
// ======================
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======================
#define MQ2 A0
#define LED LED_BUILTIN

// PIR SENSOR
#define PIR D7

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Khoi dong ESP8266...");

  pinMode(LED, OUTPUT);
  pinMode(PIR, INPUT_PULLUP);   // ổn định tín hiệu PIR

  dht.begin();
  delay(5000);

  u8g2.begin();

  Serial.println("Dang khoi dong PIR...");
  delay(30000);   // PIR cần ~30s ổn định

  Serial.println("He thong san sang!");
}

void loop() {

  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(300);

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  delay(100);

  temp = dht.readTemperature();
  hum  = dht.readHumidity();

  bool dht_error = false;

  if (isnan(temp) || isnan(hum)) {
    Serial.println("LOI: Khong doc duoc cam bien DHT!");
    dht_error = true;
  }

  int gas = analogRead(MQ2);

  // đọc PIR
  int motion = digitalRead(PIR);

  // OLED
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,12,"Nguyen Van Phong");

  if(dht_error){
    u8g2.drawStr(0,28,"DHT ERROR!");
  }else{

    char tempStr[20];
    char humStr[20];

    sprintf(tempStr,"Temp: %.1f C",temp);
    sprintf(humStr,"Hum : %.1f %%",hum);

    u8g2.drawStr(0,28,tempStr);
    u8g2.drawStr(0,42,humStr);
  }

  char gasStr[20];
  sprintf(gasStr,"Gas : %d",gas);
  u8g2.drawStr(0,56,gasStr);

  // hiển thị chuyển động
  if(motion == HIGH){
    u8g2.drawStr(80,56,"MOVE");
  } else {
    u8g2.drawStr(80,56,"SAFE");
  }

  u8g2.sendBuffer();

  // SERIAL
  Serial.println("------ SENSOR DATA ------");

  if(!dht_error){
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" C");

    Serial.print("Hum : ");
    Serial.print(hum);
    Serial.println(" %");
  }

  Serial.print("Gas : ");
  Serial.println(gas);

  Serial.print("Motion: ");
  Serial.println(motion);

  Serial.println("-------------------------");

  delay(3000);
}