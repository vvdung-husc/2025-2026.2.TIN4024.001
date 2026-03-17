#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>

// Chân kết nối theo sơ đồ bạn gửi
#define DHTPIN 0          // Chân D3 (GPIO0)
#define DHTTYPE DHT11     
#define LED_BOARD 2       // LED trên board nhấp nháy
#define RELAY_QUAT 12     // Relay 2 (D6)

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

unsigned long previousMillis = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(115200);
  
  // Thiết lập chân D3 là Input Pullup để hỗ trợ đọc DHT11 tốt hơn
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(LED_BOARD, OUTPUT);
  pinMode(RELAY_QUAT, OUTPUT);
  
  dht.begin();
  u8g2.begin();
  
  digitalWrite(RELAY_QUAT, LOW);
  Serial.println("He thong bat dau...");
}

void loop() {
  // 1. Đọc dữ liệu (Thử đọc lại nếu lỗi)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 2. Hiệu ứng LED nhấp nháy mỗi 500ms
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_BOARD, ledState);
  }

  // 3. Hiển thị lên màn hình
  u8g2.clearBuffer();
  
  // Dòng 1: Tên của bạn (Việt hóa không dấu để rõ nét)
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "P.M. NHAT KHOA"); 
  u8g2.drawHLine(0, 18, 128);

  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // Kiểm tra nếu cảm biến chưa sẵn sàng
  if (isnan(h) || isnan(t) || t < 1.0) {
    u8g2.drawStr(0, 40, "DANG KET NOI...");
    u8g2.drawStr(0, 55, "KIEM TRA DHT11");
    // Debug ra máy tính để xem lỗi gì
    Serial.println("Loi: Khong doc duoc DHT11!");
  } else {
    // Hiển thị Nhiệt độ
    u8g2.setCursor(0, 40);
    u8g2.print("Nhiet do: "); 
    u8g2.print((int)t); 
    u8g2.print(" C");

    // Hiển thị Độ ẩm
    u8g2.setCursor(0, 55);
    u8g2.print("Do am: "); 
    u8g2.print((int)h); 
    u8g2.print(" %");

    // Điều khiển Relay quạt
    if(t > 32) digitalWrite(RELAY_QUAT, HIGH);
    else digitalWrite(RELAY_QUAT, LOW);
  }

  u8g2.sendBuffer();
  delay(100); 
}