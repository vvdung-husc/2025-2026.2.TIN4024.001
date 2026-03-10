#include <Arduino.h>
#include <TM1637Display.h>

// ====== KHAI BÁO CHÂN ======
#define RED_LED     27
#define YELLOW_LED  26
#define GREEN_LED   25
#define BLUE_LED    12
#define BUTTON_PIN  14

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ====== THỜI GIAN (giây) ======
int timeRed = 10;
int timeGreen = 8;
int timeYellow = 3;

bool buttonPressed = false;

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7); // độ sáng 0-7
}

void loop() {

  // ====== ĐÈN ĐỎ ======
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  runCountdown(timeRed);

  // ====== ĐÈN XANH ======
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  runCountdown(timeGreen);

  // ====== ĐÈN VÀNG ======
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  runCountdown(timeYellow);

  digitalWrite(YELLOW_LED, LOW);
}

// ====== HÀM ĐẾM NGƯỢC ======
void runCountdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {

    display.showNumberDec(i, true);

    // nếu nhấn nút → bật LED xanh dương
    if (digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(BLUE_LED, HIGH);
    } else {
      digitalWrite(BLUE_LED, LOW);
    }

    delay(1000);
  }
}