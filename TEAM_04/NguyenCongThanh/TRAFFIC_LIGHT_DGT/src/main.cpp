#include <TM1637Display.h>
#include <Arduino.h>

// --- 1. CẤU HÌNH CHÂN (PINOUT) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21

#define CLK 19
#define DIO 18

#define SENSOR_PIN  13  // Chân đọc Analog cảm biến ánh sáng (THAY ĐỔI TỪ 34 SANG 13)
#define BUTTON_PIN  23  // Chân nút nhấn

// Ngưỡng ánh sáng để phân biệt Ngày/Đêm (0-4095)
// Trên Wokwi: Kéo thanh trượt sang phải là Tối (Giá trị cao), sang trái là Sáng (Giá trị thấp)
#define LIGHT_THRESHOLD 2000 

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

// Biến trạng thái màn hình
bool displayOn = true; // true = Hiển thị, false = Tắt màn hình

// Biến cho debounce nút nhấn
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200; // 200ms

void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân đèn là Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Cấu hình chân cảm biến là Input
  pinMode(SENSOR_PIN, INPUT);
  
  // Cấu hình chân nút nhấn với INPUT_PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Cài đặt độ sáng màn hình (0-7)
  display.setBrightness(0x0f);
  
  Serial.println("HE THONG DEN GIAO THONG KET HOP CAM BIEN KHOI DONG...");
  Serial.println("Pinout: LED_RED=27, LED_YELLOW=26, LED_GREEN=25, LED_BLUE=21");
  Serial.println("        SENSOR=13, BUTTON=23, CLK=19, DIO=18");
}

// Hàm tắt hết tất cả đèn
void tatHetDen() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
}

// Hàm kiểm tra xem có phải ban đêm không
bool laBanDem() {
  int sensorValue = analogRead(SENSOR_PIN);
  Serial.print("Gia tri cam bien anh sang: ");
  Serial.println(sensorValue);
  return (sensorValue > LIGHT_THRESHOLD); // Nếu giá trị > 2000 là Đêm
}

// Hàm kiểm tra nút nhấn và toggle trạng thái màn hình
void kiemTraButton() {
  // Đọc trạng thái nút (LOW = đang nhấn vì dùng INPUT_PULLUP)
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long currentTime = millis();
    
    // Debounce: Chỉ xử lý nếu đã qua thời gian debounce
    if (currentTime - lastButtonPress > debounceDelay) {
      lastButtonPress = currentTime;
      
      // Toggle trạng thái màn hình
      displayOn = !displayOn;
      
      if (displayOn) {
        Serial.println("*** MAN HINH BAT ***");
        display.setBrightness(0x0f); // Bật sáng lại
      } else {
        Serial.println("*** MAN HINH TAT ***");
        display.clear(); // Xóa màn hình
      }
      
      // Chờ người dùng thả nút
      while(digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// Hàm xử lý đèn sáng liên tục và đếm ngược (CHẾ ĐỘ BAN NGÀY)
// pin: Chân đèn cần sáng
// soLan: Số lần đếm (Ví dụ: 7)
// soDemBatDau: Số bắt đầu hiển thị trên màn hình (Ví dụ: 6)
void chayCheDo(int pin, int soLan, int soDemBatDau) {
  
  // Bật đèn sáng liên tục
  digitalWrite(pin, HIGH);
  
  for (int i = 0; i < soLan; i++) {
    // 1. Kiểm tra nút nhấn để toggle màn hình
    kiemTraButton();
    
    // 2. Kiểm tra LDR ngay lập tức
    // Nếu đột ngột chuyển sang đêm -> Tắt đèn và thoát ngay
    if (laBanDem()) {
      digitalWrite(pin, LOW);
      return; 
    }
    
    // 3. Hiển thị số đếm ngược (CHỈ KHI displayOn = true)
    int soHienThi = soDemBatDau - i;
    
    if (displayOn) {
      display.showNumberDec(soHienThi);
    }
    
    Serial.print("Den: "); 
    Serial.print(pin); 
    Serial.print(" | Dem nguoc: "); 
    Serial.print(soHienThi);
    Serial.print(" | Man hinh: ");
    Serial.println(displayOn ? "BAT" : "TAT");
    
    // 4. Chờ 1 giây (đèn vẫn sáng liên tục)
    delay(1000); 
  }
  
  // Tắt đèn sau khi hết thời gian
  digitalWrite(pin, LOW);
}

void loop() {
  // --- KIỂM TRA CHẾ ĐỘ ---
  if (laBanDem()) {
    // === CHẾ ĐỘ BAN ĐÊM ===
    Serial.println("--- CHE DO BAN DEM (Night Mode) ---");
    
    // Kiểm tra nút nhấn
    kiemTraButton();
    
    // Tắt đèn đỏ và xanh
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    
    // Hiển thị trên màn hình (CHỈ KHI displayOn = true)
    if (displayOn) {
      display.showNumberDec(0); // Hiển thị số 0
    }
    
    // Nhấp nháy đèn vàng
    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
    
  } else {
    // === CHẾ ĐỘ BAN NGÀY ===
    Serial.println("--- CHE DO BAN NGAY (Day Mode) ---");
    
    // Đảm bảo các đèn khác tắt trước khi vào pha mới
    tatHetDen();
    
    // 1. ĐÈN XANH: Nháy 7 lần, Đếm 6 -> 0
    Serial.println("-> PHA XANH");
    chayCheDo(LED_GREEN, 7, 6);
    if (laBanDem()) return; // Nếu đang chạy mà chuyển đêm thì reset loop
    
    // 2. ĐÈN VÀNG: Nháy 3 lần, Đếm 2 -> 0
    Serial.println("-> PHA VANG");
    chayCheDo(LED_YELLOW, 3, 2);
    if (laBanDem()) return;
    
    // 3. ĐÈN ĐỎ: Nháy 5 lần, Đếm 4 -> 0
    Serial.println("-> PHA DO");
    chayCheDo(LED_RED, 5, 4);
    if (laBanDem()) return;
  }
}
