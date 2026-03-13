#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// --- THÔNG TIN SINH VIÊN: ĐÀO VĂN LỢI ---

#define LED_BOARD 2    // Chân D4 (GPIO2)
#define DHTPIN 0       // Chân D3 (GPIO0)
#define PIRPIN 4       // Chân D2 (GPIO4)
#define GASPIN A0      // Chân A0
#define DHTTYPE DHT22  

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastBlink = 0;
unsigned long lastUpdate = 0;
bool ledStatus = false; 

void setup() {
  Serial.begin(115200); 
  delay(500);

  pinMode(LED_BOARD, OUTPUT);
  pinMode(PIRPIN, INPUT); 
  
  dht.begin();
  u8g2.begin();

  Serial.println("\n--- HE THONG KHOI DONG: DAO VAN LOI ---");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Nhấp nháy LED Board (Chớp sáng cực nhanh để dễ quan sát)
  if (currentMillis - lastBlink >= 500) {
    lastBlink = currentMillis;
    ledStatus = !ledStatus;
    digitalWrite(LED_BOARD, ledStatus ? LOW : HIGH); 
  }

  // 2. Cập nhật dữ liệu (Mỗi 2 giây)
  if (currentMillis - lastUpdate >= 2000) {
    lastUpdate = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int gasVal = analogRead(GASPIN);          
    int motion = digitalRead(PIRPIN);       

    // --- HIỂN THỊ TERMINAL ---
    Serial.println("==========================================");
    Serial.printf("DAO VAN LOI | Temp: %.1f*C | Humi: %.1f%%\n", t, h);
    Serial.printf("GAS: %d | PIR: %s | LED: %s\n", 
                  gasVal, (motion == HIGH) ? "DETECTED!" : "Clear", ledStatus ? "ON" : "OFF");

    // --- HIỂN THỊ OLED ---
    u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_6x12_tf); 
    
    // Dòng 1: Tên
    u8g2.drawStr(0, 10, "DAO VAN LOI - K46");
    
    // Dòng 2: Nhiệt độ
    u8g2.setCursor(0, 24);
    u8g2.print("Nhiet do: "); u8g2.print(t); u8g2.print(" C");
    
    // Dòng 3: Độ ẩm
    u8g2.setCursor(0, 34);
    u8g2.print("Do am: "); u8g2.print(h); u8g2.print(" %");
    
    // Dòng 4: Khí Gas
    u8g2.setCursor(0, 44);
    u8g2.print("GAS : "); u8g2.print(gasVal);
    
    // Dòng 5: Chuyển động
    u8g2.setCursor(0, 54);
    u8g2.print("PIR : "); 
    u8g2.print((motion == HIGH) ? "DETECTED!" : "Safe");
    
    // Dòng 6: Trạng thái đèn (Dưới cùng)
    u8g2.setCursor(0, 64);
    u8g2.print("LED Status: "); 
    u8g2.print(ledStatus ? "ON" : "OFF");
    
    u8g2.sendBuffer();
  }
}