#include <TM1637Display.h>
#include <Arduino.h>

// --- 1. CẤU HÌNH CHÂN (PINOUT) ---
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define CLK 19
#define DIO 18

#define SENSOR_PIN  34 // Chân đọc Analog cảm biến ánh sáng

// Ngưỡng ánh sáng để phân biệt Ngày/Đêm (0-4095)
// Trên Wokwi: Kéo thanh trượt sang phải là Tối (Giá trị cao), sang trái là Sáng (Giá trị thấp)
#define LIGHT_THRESHOLD 2000 

// Khởi tạo màn hình
TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(115200);

  // Cấu hình chân đèn là Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  // Cấu hình chân cảm biến là Input
  pinMode(SENSOR_PIN, INPUT);

  // Cài đặt độ sáng màn hình (0-7)
  display.setBrightness(0x0f);
  
  Serial.println("HE THONG DEN GIAO THONG KET HOP CAM BIEN KHOI DONG...");
}

// Hàm tắt hết tất cả đèn
void tatHetDen() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

// Hàm kiểm tra xem có phải ban đêm không
bool laBanDem() {
  int sensorValue = analogRead(SENSOR_PIN);
  return (sensorValue > LIGHT_THRESHOLD); // Nếu giá trị > 2000 là Đêm
}

// Hàm xử lý nhấp nháy đèn và đếm ngược
// pin: Chân đèn cần nháy
// soLan: Số lần nháy (Ví dụ: 7)
// soDemBatDau: Số bắt đầu hiển thị trên màn hình (Ví dụ: 6)
void chayCheDo(int pin, int soLan, int soDemBatDau) {
  
  for (int i = 0; i < soLan; i++) {
    // 1. Kiểm tra LDR ngay lập tức
    // Nếu đột ngột chuyển sang đêm -> Thoát ngay khỏi vòng lặp này để về loop chính
    if (laBanDem()) return; 

    // 2. Hiển thị số đếm ngược
    int soHienThi = soDemBatDau - i;
    display.showNumberDec(soHienThi);
    Serial.print("Den: "); Serial.print(pin); 
    Serial.print(" | Dem nguoc: "); Serial.println(soHienThi);

    // 3. Nhấp nháy đèn (Sáng 0.5s - Tắt 0.5s)
    digitalWrite(pin, HIGH);
    delay(500); 
    if (laBanDem()) { digitalWrite(pin, LOW); return; } // Check lại lần nữa cho nhạy

    digitalWrite(pin, LOW);
    delay(500);
  }
}

void loop() {
  // --- KIỂM TRA CHẾ ĐỘ ---
  if (laBanDem()) {
    // === CHẾ ĐỘ BAN ĐÊM ===
    Serial.println("--- CHE DO BAN DEM (Night Mode) ---");
    
    // Tắt đèn đỏ và xanh
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    
    // Màn hình hiện gạch ngang hoặc tắt
    uint8_t data[] = { 0x40, 0x40, 0x40, 0x40 }; // Ký tự gạch ngang (-)
    display.setSegments(data);

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