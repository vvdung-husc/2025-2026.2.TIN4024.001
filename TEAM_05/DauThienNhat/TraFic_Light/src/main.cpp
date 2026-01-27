#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân cắm
#define CLK 18
#define DIO 19
#define LDR_PIN 13
#define BUTTON_PIN 23  
#define LED_BLUE 21    

const int LED_RED = 27;    
const int LED_YELLOW = 26; 
const int LED_GREEN = 25;  

TM1637Display display(CLK, DIO);

// Biến trạng thái
bool isDisplayOn = true; 
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50ms để chống rung nút nhấn

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  Serial.begin(115200);
  
  // Trạng thái ban đầu
  digitalWrite(LED_BLUE, HIGH);
}

// Hàm kiểm tra nút nhấn cực nhạy
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      isDisplayOn = !isDisplayOn; // Đảo trạng thái Bật <-> Tắt
      
      // Điều khiển LED Xanh dương và Màn hình theo biến isDisplayOn
      digitalWrite(LED_BLUE, isDisplayOn ? HIGH : LOW);
      if (!isDisplayOn) {
        display.clear();
      }
      
      Serial.print("Trang thai he thong: ");
      Serial.println(isDisplayOn ? "DANG BAT" : "DANG TAT");
    }
  }
  lastButtonState = reading;
}

void runTrafficLight(int ledPin, int seconds, String colorName) {
  digitalWrite(ledPin, HIGH);
  
  // Vòng lặp đếm ngược thời gian đèn
  for (int i = seconds; i >= 0; i--) {
    
    // Chia 1 giây thành nhiều phần nhỏ để kiểm tra nút nhấn liên tục
    for (int ms = 0; ms < 1000; ms += 100) {
      checkButton();
      
      if (isDisplayOn) {
        // Hiệu ứng nhấp nháy màn hình khi đang Bật
        if (ms < 500) display.showNumberDec(i);
        else display.clear();
      } else {
        display.clear(); // Luôn tắt nếu isDisplayOn = false
      }
      
      delay(100); 
    }
    
    Serial.printf("%s: %d s | LDR: %d\n", colorName.c_str(), i, analogRead(LDR_PIN));
  }
  
  digitalWrite(ledPin, LOW);
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  bool isNight = (lightValue < 500);

  runTrafficLight(LED_GREEN, 7, "XANH");

  if (!isNight) {
    runTrafficLight(LED_YELLOW, 2, "VANG");
  }

  runTrafficLight(LED_RED, 8, "DO");
}