#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>                    // Thêm thư viện DHT

#define BLYNK_TEMPLATE_ID "TMPL64YL8fJrk"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "S9-UuqRP6ItPoUGPZYbtSWknol03FF-0"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23
#define pinBLED  21
#define CLK      18
#define DIO      19
#define DHTPIN   16                 // SDA của DHT22 nối chân 16
#define DHTTYPE  DHT22

DHT dht(DHTPIN, DHTTYPE);           // Khởi tạo DHT

TM1637Display display(CLK, DIO);

ulong currentMiliseconds = 0;
bool blueButtonON = true;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  dht.begin();                      // Khởi động DHT22
  
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("WiFi connected");
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  
  Serial.println("== START ==");
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  
  uptimeBlynk();
  updateBlueButton();
  readDHT();                        // Đọc cảm biến mỗi vòng lặp
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  
  if (blueButtonON) {
    Serial.println("Blue Light ON");
  } else {
    Serial.println("Blue Light OFF");
    display.clear();
  }
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  
  if (blueButtonON) {
    display.showNumberDec(value);   // Hiển thị uptime khi đèn ON
  }
}

void readDHT() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;  // Đọc mỗi 2 giây
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V2, t);     // Nhiệt độ → V2
  Blynk.virtualWrite(V3, h);     // Độ ẩm → V3
  
  Serial.printf("Temp: %.1f °C | Hum: %.1f %%\n", t, h);
  
  if (blueButtonON) {
    display.showNumberDec(t, true);  // Hiển thị nhiệt độ trên TM1637 (có dấu chấm)
  }
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    display.clear();
  }
}