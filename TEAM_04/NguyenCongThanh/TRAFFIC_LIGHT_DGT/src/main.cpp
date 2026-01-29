// #include <Arduino.h>

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
#include <Arduino.h>
#include <TM1637Display.h>

// ===== CẤU HÌNH CHÂN =====
const uint8_t PIN_LED_RED = 14;
const uint8_t PIN_LED_YELLOW = 27;
const uint8_t PIN_LED_GREEN = 26;
const uint8_t PIN_LED_BLUE = 21;
const uint8_t PIN_BUTTON = 23;
const uint8_t PIN_LDR = 13;
const uint8_t PIN_CLK = 18;
const uint8_t PIN_DIO = 19;

// ===== THỜI GIAN (giây) =====
const int TIME_RED = 5;
const int TIME_GREEN = 7;
const int TIME_YELLOW = 3;

const int LDR_THRESHOLD = 1500;
const int DEBOUNCE_TIME = 50;

// ===== KHỞI TẠO =====
TM1637Display display(PIN_CLK, PIN_DIO);

// ===== BIẾN TRẠNG THÁI =====
bool showCountdown = true;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ===== KHAI BÁO HÀM TRƯỚC =====
void checkButton();
void runRedLight();
void runGreenLight();
void runYellowLight();
void runNightMode();

// ===== HÀM CHÍNH =====
void setup() {
  Serial.begin(115200);
  
  // Cấu hình chân
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  // Khởi tạo màn hình
  display.setBrightness(7);
  display.clear();
  
  // LED xanh dương báo trạng thái countdown
  digitalWrite(PIN_LED_BLUE, HIGH);
  
  // Tắt hết đèn giao thông
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  
  Serial.println("=================================");
  Serial.println("   ĐÈN GIAO THÔNG THÔNG MINH");
  Serial.println("=================================");
  Serial.println("Thứ tự: ĐỎ(5s) → XANH(7s) → VÀNG(3s)");
  Serial.println("Nhấn nút: BẬT/TẮT đếm ngược");
  Serial.println("=================================\n");
}

void loop() {
  checkButton();
  
  int lightLevel = analogRead(PIN_LDR);
  
  if (lightLevel >= LDR_THRESHOLD) {
    // CHẾ ĐỘ NGÀY
    Serial.println("☀️ CHẾ ĐỘ NGÀY");
    runRedLight();
    runGreenLight();
    runYellowLight();
  } else {
    // CHẾ ĐỘ ĐÊM
    Serial.println("🌙 CHẾ ĐỘ ĐÊM - Đèn vàng nhấp nháy");
    runNightMode();
  }
}

// ===== XỬ LÝ NÚT NHẤN =====
void checkButton() {
  bool reading = digitalRead(PIN_BUTTON);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading == LOW && lastButtonState == HIGH) {
      // Nhấn nút → Đổi trạng thái
      showCountdown = !showCountdown;
      digitalWrite(PIN_LED_BLUE, showCountdown ? HIGH : LOW);
      
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━");
      Serial.print("🔵 Đếm ngược: ");
      Serial.println(showCountdown ? "BẬT ✓" : "TẮT ✗");
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━\n");
    }
    lastButtonState = reading;
  }
}

// ===== ĐÈN ĐỎ =====
void runRedLight() {
  Serial.println("🔴 ĐÈN ĐỎ - DỪNG LẠI");
  
  // BẬT đèn đỏ, TẮT đèn khác
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  
  // Đếm ngược từ 5 → 1
  for (int i = TIME_RED; i >= 1; i--) {
    checkButton();
    
    Serial.print("  ⏱️  ");
    Serial.println(i);
    
    // Hiển thị số
    if (showCountdown) {
      display.showNumberDec(i, false);
    } else {
      display.clear();
    }
    
    delay(1000); // 1 giây
  }
  
  digitalWrite(PIN_LED_RED, LOW);
  Serial.println();
}

// ===== ĐÈN XANH =====
void runGreenLight() {
  Serial.println("🟢 ĐÈN XANH - ĐI");
  
  // BẬT đèn xanh, TẮT đèn khác
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  
  // Đếm ngược từ 7 → 1
  for (int i = TIME_GREEN; i >= 1; i--) {
    checkButton();
    
    Serial.print("  ⏱️  ");
    Serial.println(i);
    
    if (showCountdown) {
      display.showNumberDec(i, false);
    } else {
      display.clear();
    }
    
    delay(1000);
  }
  
  digitalWrite(PIN_LED_GREEN, LOW);
  Serial.println();
}

// ===== ĐÈN VÀNG =====
void runYellowLight() {
  Serial.println("🟡 ĐÈN VÀNG - CHỜ");
  
  // BẬT đèn vàng, TẮT đèn khác
  digitalWrite(PIN_LED_YELLOW, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  
  // Đếm ngược từ 3 → 1
  for (int i = TIME_YELLOW; i >= 1; i--) {
    checkButton();
    
    Serial.print("  ⏱️  ");
    Serial.println(i);
    
    if (showCountdown) {
      display.showNumberDec(i, false);
    } else {
      display.clear();
    }
    
    delay(1000);
  }
  
  digitalWrite(PIN_LED_YELLOW, LOW);
  Serial.println();
}

// ===== CHẾ ĐỘ ĐÊM =====
void runNightMode() {
  // Tắt đèn đỏ và xanh
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  
  // Nhấp nháy đèn vàng
  while (analogRead(PIN_LDR) < LDR_THRESHOLD) {
    checkButton();
    
    // Hiển thị 0 hoặc tắt màn hình
    if (showCountdown) {
      display.showNumberDec(0, false);
    } else {
      display.clear();
    }
    
    digitalWrite(PIN_LED_YELLOW, HIGH);
    delay(500);
    
    digitalWrite(PIN_LED_YELLOW, LOW);
    delay(500);
  }
}