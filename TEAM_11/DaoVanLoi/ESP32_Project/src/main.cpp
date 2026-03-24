#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// --- THÔNG TIN SINH VIÊN: ĐÀO VĂN LỢI - K46 ---

#define LED_BOARD 2    // Chân D4 (GPIO2)
#define DHTPIN 0       // Chân D3 (GPIO0) - Theo sơ đồ mạch thực tế
#define PIRPIN 4       // Chân D2 (GPIO4) - Theo sơ đồ mạch thực tế
#define GASPIN A0      // Chân A0
#define DHTTYPE DHT22  

// Khởi tạo màn hình OLED SH1106 (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastBlink = 0;
unsigned long lastUpdate = 0;
bool ledStatus = false; 

void setup() {
  // Khởi tạo Serial cho Terminal VS Code với tốc độ 115200
  Serial.begin(115200); 
  delay(500);

  pinMode(LED_BOARD, OUTPUT);
  pinMode(PIRPIN, INPUT); 
  
  dht.begin();
  u8g2.begin();

  Serial.println("\n==========================================");
  Serial.println("  HE THONG GIAM SAT - DAO VAN LOI K46   ");
  Serial.println("==========================================");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Nhấp nháy LED Board (Chớp sáng cực nhanh để dễ quan sát)
  if (currentMillis - lastBlink >= 500) {
    lastBlink = currentMillis;
    ledStatus = !ledStatus;
    // Logic Active Low: true -> LED Sáng, false -> LED Tắt
    digitalWrite(LED_BOARD, ledStatus ? LOW : HIGH); 
  }

  // 2. Cập nhật dữ liệu (Mỗi 2 giây nhảy một lần)
  if (currentMillis - lastUpdate >= 2000) {
    lastUpdate = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int gasVal = analogRead(GASPIN);          
    int motion = digitalRead(PIRPIN);       

    // --- HIỂN THỊ TERMINAL VS CODE ---
    Serial.println("==========================================");
    Serial.printf("DAO VAN LOI | Nhiet do: %.1f*C | Do am: %.1f%%\n", t, h);
    Serial.printf("GAS: %d | PIR: %s | LED: %s\n", 
                  gasVal, (motion == HIGH) ? "DETECTED!" : "Clear", ledStatus ? "ON" : "OFF");

    // --- HIỂN THỊ OLED ---
    u8g2.clearBuffer();          
    u8g2.setFont(u8g2_font_6x12_tf); 
    
    // Dòng 1: Tên chính chủ
    u8g2.drawStr(0, 10, "DAO VAN LOI - K46");
    
    // Dòng 2: Nhiệt độ
    u8g2.setCursor(0, 24);
    u8g2.print("Nhiet do: "); 
    if (isnan(t)) u8g2.print("--"); else { u8g2.print(t); u8g2.print(" C"); }
    
    // Dòng 3: Độ ẩm
    u8g2.setCursor(0, 34);
    u8g2.print("Do am: "); 
    if (isnan(h)) u8g2.print("--"); else { u8g2.print(h); u8g2.print(" %"); }
    
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