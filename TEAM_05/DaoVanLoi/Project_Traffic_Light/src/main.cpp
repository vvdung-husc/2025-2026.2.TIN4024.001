#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm theo diagram.json của Lợi
#define CLK 18
#define DIO 19
#define BTN_PIN 23
#define LDR_PIN 13
#define LED_RED 27
#define LED_YEL 26
#define LED_GRE 25
#define LED_STREET 21 // Đèn xanh dương (led4)

TM1637Display display(CLK, DIO);

bool showDisplay = true;
int countdown = 0;
unsigned long previousMillis = 0;
int currentMode = 0; // 0: Đỏ, 1: Vàng, 2: Xanh

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YEL, OUTPUT);
  pinMode(LED_GRE, OUTPUT);
  pinMode(LED_STREET, OUTPUT);
  
  display.setBrightness(0x0f);
  Serial.println("--- HE THONG DAO VAN LOI READY ---");
}

void loop() {
  // 1. Xử lý nút nhấn để bật/tắt màn hình
  if (digitalRead(BTN_PIN) == LOW) {
    delay(200); // Chống dội
    showDisplay = !showDisplay;
    if (!showDisplay) display.clear();
    Serial.print("Display Status: ");
    Serial.println(showDisplay ? "ON" : "OFF");
  }

  // 2. Xử lý LDR - Cảm biến ánh sáng điều khiển LED_STREET
  int ldrValue = analogRead(LDR_PIN);
  if (ldrValue > 2000) { // Trời tối
    digitalWrite(LED_STREET, HIGH);
  } else {
    digitalWrite(LED_STREET, LOW);
  }

  // 3. Logic Đèn giao thông và Đếm ngược
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    if (countdown <= 0) {
      currentMode = (currentMode + 1) % 3;
      if (currentMode == 0) { countdown = 5; Serial.println("MODE: RED"); }
      else if (currentMode == 1) { countdown = 3; Serial.println("MODE: YELLOW"); }
      else { countdown = 7; Serial.println("MODE: GREEN"); }
    }

    // Cập nhật đèn
    digitalWrite(LED_RED, currentMode == 0);
    digitalWrite(LED_YEL, currentMode == 1);
    digitalWrite(LED_GRE, currentMode == 2);

    // Hiển thị lên TM1637
    if (showDisplay) {
      display.showNumberDec(countdown);
    }
    countdown--;
  }
}