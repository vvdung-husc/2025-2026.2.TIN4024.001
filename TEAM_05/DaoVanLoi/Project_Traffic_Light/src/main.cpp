#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân dựa theo sơ đồ diagram.json mới nhất
#define CLK 18
#define DIO 19
#define BTN_PIN 23
#define LDR_PIN 13
#define LED_RED 27
#define LED_YEL 26
#define LED_GRE 25
#define LED_STREET 21

TM1637Display display(CLK, DIO);

bool showDisplay = true;
int countdown = 0;
unsigned long previousMillis = 0;
int currentMode = 0; // 0:Red, 1:Yellow, 2:Green

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
  // Cảm biến ánh sáng LDR điều khiển LED Xanh Dương (led4)
  int ldrValue = analogRead(LDR_PIN);
  if (ldrValue > 2500) { // Trời tối
    digitalWrite(LED_STREET, HIGH);
  } else {
    digitalWrite(LED_STREET, LOW);
  }

  // Nút nhấn điều khiển bật/tắt màn hình TM1637
  if (digitalRead(BTN_PIN) == LOW) {
    delay(200);
    showDisplay = !showDisplay;
    if (!showDisplay) display.clear();
  }

  // Đếm ngược đèn giao thông
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    
    if (countdown <= 0) {
      currentMode = (currentMode + 1) % 3;
      if (currentMode == 0) countdown = 5;
      else if (currentMode == 1) countdown = 3;
      else countdown = 7;
    }

    digitalWrite(LED_RED, currentMode == 0);
    digitalWrite(LED_YEL, currentMode == 1);
    digitalWrite(LED_GRE, currentMode == 2);

    if (showDisplay) {
      display.showNumberDec(countdown);
    }
    countdown--;
  }
}