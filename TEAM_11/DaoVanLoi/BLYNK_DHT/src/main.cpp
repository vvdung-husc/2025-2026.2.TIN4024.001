#define BLYNK_TEMPLATE_ID "TMPL64uSRDbGx"
#define BLYNK_TEMPLATE_NAME "Traffic DHT"
#define BLYNK_AUTH_TOKEN "jUB3r-q1gcgicGMynE8qeJDo4uPJgsn2"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN PIN (Khớp 100% với hình anh gửi) ---
#define PIN_DHT 16      
#define PIN_LED 21      
#define PIN_BTN 23      
#define CLK_TM1637 18   
#define DIO_TM1637 19   

DHT dht(PIN_DHT, DHT22);
TM1637Display display(CLK_TM1637, DIO_TM1637);
BlynkTimer timer;

// WiFi mặc định của Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

int countdown = 11;
bool isAuto = true; 

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    if (Blynk.connected()) {
      Blynk.virtualWrite(V1, t); 
      Blynk.virtualWrite(V2, h); 
    }
    Serial.printf("Nhiệt độ: %.1f°C | Độ ẩm: %.1f%%\n", t, h);
  }
}

void trafficTimer() {
  if (isAuto) { 
    display.showNumberDec(countdown);
    if (Blynk.connected()) {
        Blynk.virtualWrite(V0, countdown);
    }
    
    if (countdown > 0) {
      countdown--;
      digitalWrite(PIN_LED, HIGH); // Đèn đỏ sáng khi đếm ngược
    } else {
      countdown = 11; 
    }
  } else {
    display.clear();
    digitalWrite(PIN_LED, LOW);
  }
}

BLYNK_WRITE(V3) {
  isAuto = param.asInt();
}

void setup() {
  Serial.begin(115200);
  
  // THÔNG TIN SINH VIÊN
  Serial.println("================================");
  Serial.println("STUDENT: DAO VAN LOI"); 
  Serial.println("PROJECT: TRAFFIC LIGHT SIMULATION");
  Serial.println("================================");

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);
  
  // Kết nối Blynk qua Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, trafficTimer);
}

void loop() {
  Blynk.run();
  timer.run();

  // Nút nhấn vật lý để bật/tắt Auto
  static bool lastBtnState = HIGH;
  bool btnState = digitalRead(PIN_BTN);
  if (btnState == LOW && lastBtnState == HIGH) {
    isAuto = !isAuto;
    delay(200); 
  }
  lastBtnState = btnState;
}