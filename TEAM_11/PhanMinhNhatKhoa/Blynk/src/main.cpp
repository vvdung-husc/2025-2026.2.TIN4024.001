#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h> // Thêm thư viện DHT

/* Điền thông tin Blynk của bạn vào đây */
#define BLYNK_TEMPLATE_ID "TMPL6yrROMUTd"
#define BLYNK_TEMPLATE_NAME "He Thong Giam Sat"
#define BLYNK_AUTH_TOKEN "OatJPynbJCRUlXD4nyATNRlrYr1vJ_FY"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";             

#define btnBLED  23
#define pinBLED  21
#define CLK 18  
#define DIO 19  

// --- Cấu hình cảm biến DHT22 ---
#define DHTPIN 4        // Chân kết nối DHT22
#define DHTTYPE DHT22   // Loại cảm biến
DHT dht(DHTPIN, DHTTYPE);
// -------------------------------

ulong currentMiliseconds = 0; 
bool blueButtonON = true;     

TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateDHT22(); // Hàm mới để đọc nhiệt độ/độ ẩm

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin(); // Khởi động DHT22
  
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("\nWiFi connected");
  
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); 
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run(); 

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  updateDHT22(); // Chạy hàm cập nhật cảm biến
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  
  if (blueButtonON){
    display.showNumberDec(value);
  }
}

// Hàm mới: Đọc và gửi dữ liệu DHT22 lên Blynk mỗi 2 giây
void updateDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // Đọc 2 giây/lần

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi nếu không đọc được
  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi: Không thể đọc dữ liệu từ DHT22!");
    return;
  }

  // Gửi lên Blynk qua V2 và V3
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
  
  Serial.print("Nhiệt độ: "); Serial.print(t);
  Serial.print(" °C - Độ ẩm: "); Serial.print(h); Serial.println(" %");
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();  
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}