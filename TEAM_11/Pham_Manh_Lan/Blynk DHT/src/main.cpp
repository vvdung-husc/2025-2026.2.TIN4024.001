#define BLYNK_TEMPLATE_ID "TMPL6ul-5vSqB"
#define BLYNK_TEMPLATE_NAME "Blynk LED"
#define BLYNK_AUTH_TOKEN "x1fMNdGMTVcWPK5Wy-diX7Jod7KPKAe9"
#define BLYNK_PRINT Serial

// 2. THƯ VIỆN
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

// 3. CẤU HÌNH WIFI
// -> Nếu bạn dùng Wokwi trên VS Code, bỏ comment 2 dòng dưới:
char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; 
// -> Nếu dùng ESP32 thật cắm cáp, điền WiFi nhà bạn vào 2 dòn/g dưới:
// char ssid[] = "CNTT-MMT"; 
// char pass[] = "13572468";            

// 4. KHAI BÁO CHÂN & BIẾN
#define btnBLED  23 
#define pinBLED  21 
#define CLK      18 
#define DIO      19 
#define DHTPIN   4  
#define DHTTYPE  DHT22 

unsigned long currentMiliseconds = 0; 
bool blueButtonON = true;     

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void updateUptimeDisplay();
void updateDHTSensor();

// 5. SETUP
void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin(); 
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  
  Serial.println("Đang kết nối WiFi và Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("== ĐÃ KẾT NỐI THÀNH CÔNG ==");

  Blynk.virtualWrite(V1, blueButtonON ? 1 : 0);
}

// 6. LOOP
void loop() {  
  Blynk.run(); 

  currentMiliseconds = millis(); 
  updateUptimeDisplay(); 
  updateBlueButton();    
  updateDHTSensor();     
}

// 7. CÁC HÀM XỬ LÝ
bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;
  
  if (!IsReady(lastTime, 50)) return; 
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == HIGH) return;

  blueButtonON = !blueButtonON; 

  if (blueButtonON) {
    digitalWrite(pinBLED, HIGH);
  } else {
    digitalWrite(pinBLED, LOW);    
    display.clear(); 
  }    
  
  Blynk.virtualWrite(V1, blueButtonON ? 1 : 0);
}

void updateUptimeDisplay() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  
  unsigned long value = lastTime / 1000; 
  if (blueButtonON) display.showNumberDec(value);
  
  Blynk.virtualWrite(V0, value);
}

void updateDHTSensor() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; 

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) return;

  Serial.print("Nhiệt độ: "); Serial.print(t); Serial.print("°C | Độ ẩm: "); Serial.print(h); Serial.println("%");

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
}

// 8. NHẬN LỆNH TỪ BLYNK APP ĐIỀU KHIỂN LED
BLYNK_WRITE(V1) { 
  int pinValue = param.asInt(); 
  blueButtonON = (pinValue == 1);
  
  if (blueButtonON) {
    digitalWrite(pinBLED, HIGH);
  } else {
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}