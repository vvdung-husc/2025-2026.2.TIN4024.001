#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- CẤU HÌNH CẢM BIẾN DHT ---
// Thử chân D3 trước. Nếu nạp xong vẫn lỗi, hãy đổi số 3 thành số 4 (D4) nhé!
#define DHTPIN 2      
#define DHTTYPE DHT22   // Cảm biến màu trắng là DHT22

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  u8g2.begin();
  u8g2.enableUTF8Print(); 
  
  // Hiện thông báo đang khởi động lên OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, 30);
  u8g2.print("Khoi dong DHT...");
  u8g2.sendBuffer();

  Serial.println("Dang cho DHT22 lam nong...");
  
  // QUAN TRỌNG: Cho DHT22 thời gian 2 giây để "làm nóng" trước khi đọc
  delay(2000); 
  dht.begin();
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);  
  delay(100);                      
  digitalWrite(LED_BUILTIN, HIGH); 

  // Đọc dữ liệu
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  u8g2.clearBuffer();                  
  u8g2.setFont(u8g2_font_ncenB10_tr);  

  if (isnan(h) || isnan(t)) {
    // Đã sửa lại text báo lỗi thành DHT22
    u8g2.setCursor(10, 30);
    u8g2.print("Loi DHT22 !"); 
    Serial.println("Lỗi: Không đọc được dữ liệu từ DHT22! Hãy thử đổi DHTPIN sang D4.");
  } else {
    // In ra màn hình OLED
    u8g2.setCursor(0, 25);
    u8g2.print("Nhiet do: ");
    u8g2.print(t, 1); 
    u8g2.print(" C");

    u8g2.setCursor(0, 50);
    u8g2.print("Do am: ");
    u8g2.print(h, 1);
    u8g2.print(" %");
    
    // In ra Terminal để kiểm tra kép
    Serial.printf("Nhiet do: %.1f C, Do am: %.1f %%\n", t, h);
  }
  
  u8g2.sendBuffer(); 
  delay(2000); // Đợi 2 giây cho lần đọc tiếp theo
}