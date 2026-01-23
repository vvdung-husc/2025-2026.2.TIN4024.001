#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- CẬP NHẬT CHÂN THEO HÌNH MỚI CỦA BẠN ---
#define LED_RED     15   // Đèn Đỏ nối chân 15
#define LED_YELLOW  2    // Đèn Vàng nối chân 2
#define LED_GREEN   16   // Đèn Xanh nối chân 16

// Địa chỉ màn hình LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

 

  void tatHetDen() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void chayPhaDen(int chanDen, String tenMau, int thoiGian) {
  tatHetDen();
  digitalWrite(chanDen, HIGH);
  Serial.println("Bat den: " + tenMau);
  
  for (int i = thoiGian; i > 0; i--) {
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.print("DEN: ");
    lcd.print(tenMau);
    
    lcd.setCursor(0, 1);
    lcd.print("CON LAI: ");
    lcd.print(i);
    lcd.print("s");
    
    Serial.println(String(tenMau) + ": " + String(i) + "s");
    delay(1000);
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);  // Đợi Serial ready
  
  Serial.println("\n\n=== TRAFFIC LIGHT SYSTEM ===");
  Serial.println("Initializing Wire...");
  Wire.begin(21, 22);  // SDA=21, SCL=22
  
  delay(500);
  
  // Quét địa chỉ I2C
  Serial.println("\nScanning I2C addresses...");
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Found I2C device at: 0x");
      if(address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      nDevices++;
    }
  }
  
  Serial.print("Total devices found: ");
  Serial.println(nDevices);
  
  // Cấu hình chân
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Tắt hết đèn lúc đầu
  tatHetDen();

  // Khởi động màn hình LCD
  Serial.println("\nInitializing LCD with address 0x3F...");
  lcd.init();
  delay(1000);
  lcd.backlight();
  delay(500);
  
  // Hiện chữ chào mừng
  Serial.println("Writing to LCD...");
  lcd.setCursor(0, 0);
  lcd.print("HE THONG DEN");
  lcd.setCursor(0, 1);
  lcd.print("GIAO THONG");
  
  Serial.println("Setup complete!");
  delay(2000);
  lcd.clear();
}





void loop() {
  // 1. Đèn Xanh: 10 giây
  chayPhaDen(LED_GREEN, "XANH", 10);

  // 2. Đèn Vàng: 3 giây
  chayPhaDen(LED_YELLOW, "VANG", 3);

  // 3. Đèn Đỏ: 7 giây
  chayPhaDen(LED_RED, "DO", 7);
}