#define BLYNK_TEMPLATE_ID "TMPL6Z4Abl1Pq"
#define BLYNK_TEMPLATE_NAME "TraficDHT"
#define BLYNK_AUTH_TOKEN "iaxm2L6Ss35AFsfRhnppPtEr-JD9RJ_Q"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN PIN (Khớp 100% với diagram.json của bạn) ---
#define PIN_DHT 16      
#define PIN_LED 21      
#define PIN_BTN 23      
#define CLK_TM1637 18   
#define DIO_TM1637 19   

DHT dht(PIN_DHT, DHT22);
TM1637Display display(CLK_TM1637, DIO_TM1637);
BlynkTimer timer;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

int countdown = 11;
bool isAuto = true; // Đặt thành true để giây tự chạy ngay lập tức

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
      digitalWrite(PIN_LED, HIGH);
    } else {
      countdown = 11; // Reset vòng lặp
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
  
  // HIỂN THỊ TÊN ĐÀO VĂN LỢI TRÊN TERMINAL
  Serial.println("================================");
  Serial.println("STUDENT: ĐÀO VĂN LỢI"); 
  Serial.println("STATUS: RUNNING SIMULATION");
  Serial.println("================================");

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);
  
  // Khởi tạo WiFi
  WiFi.begin(ssid, pass);

  // Dùng config thay cho begin để tránh lỗi treo DNS
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, trafficTimer);
}

void loop() {
  // Chỉ chạy Blynk.run khi đã có kết nối để tránh lag code
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  timer.run();

  // Nút nhấn vật lý
  static bool lastBtnState = HIGH;
  bool btnState = digitalRead(PIN_BTN);
  if (btnState == LOW && lastBtnState == HIGH) {
    isAuto = !isAuto;
    delay(200); 
  }
  lastBtnState = btnState;
}