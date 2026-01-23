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
  
  // Xóa dòng 1 trước
  lcd.setCursor(0, 0);
  lcd.print("                ");  // 16 space để xóa toàn bộ
  
  // Hiển thị tên màu đèn lần đầu (không nhảy)
  lcd.setCursor(0, 0);
  lcd.print("DEN: ");
  lcd.print(tenMau);
  
  for (int i = thoiGian; i > 0; i--) {
    // Chỉ cập nhật số giây ở dòng thứ 2
    lcd.setCursor(0, 1);
    lcd.print("CON LAI: ");
    lcd.print(i);
    lcd.print("s  ");  // Thêm space để xóa chữ cũ
    
    Serial.println(String(tenMau) + ": " + String(i) + "s");
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== TRAFFIC LIGHT SYSTEM ===");
  
  // Khởi tạo I2C với SDA=21, SCL=22
  Wire.begin(21, 22);
  delay(500);
  
  // Cấu hình chân LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Tắt hết đèn lúc đầu
  tatHetDen();

  // Khởi động màn hình LCD
  Serial.println("Initializing LCD...");
  lcd.init();
  delay(500);
  lcd.backlight();
  delay(500);
  
  // Hiện chữ chào mừng
  lcd.setCursor(0, 0);
  lcd.print("HE THONG DEN");
  lcd.setCursor(0, 1);
  lcd.print("GIAO THONG");
  
  Serial.println("System ready!");
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