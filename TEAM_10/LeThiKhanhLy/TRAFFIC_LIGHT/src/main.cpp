#include <Arduino.h>
#include <TM1637Display.h>

// ====== LED GIAO THÔNG ======
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

// ====== LED & BUTTON XANH DƯƠNG ======
#define LED_BLUE    21
#define BUTTON_BLUE 23

// ====== TM1637 ======
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ====== BIẾN TRẠNG THÁI ======
bool countdownEnabled = true;
bool lastButton = HIGH;

// ====== XỬ LÝ NÚT (ĐƠN GIẢN – CHẮC CHẮN ĂN) ======
void handleButton() {
  bool current = digitalRead(BUTTON_BLUE);

  // phát hiện cạnh nhấn
  if (lastButton == HIGH && current == LOW) {
    countdownEnabled = !countdownEnabled;

    digitalWrite(LED_BLUE, countdownEnabled ? HIGH : LOW);

    if (!countdownEnabled) {
      display.clear();
    }

    delay(200); // debounce đơn giản
  }

  lastButton = current;
}

// ====== NHẤP NHÁY + ĐẾM NGƯỢC ======
void blinkWithCountdown(int ledPin, int seconds) {
  for (int i = seconds; i >= 0; i--) {

    if (countdownEnabled) {
      display.showNumberDec(i, true);
    }

    digitalWrite(ledPin, HIGH);
    for (int t = 0; t < 50; t++) {
      handleButton();
      delay(10);
    }

    digitalWrite(ledPin, LOW);
    for (int t = 0; t < 50; t++) {
      handleButton();
      delay(10);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_BLUE, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  digitalWrite(LED_BLUE, HIGH);

  Serial.println("SYSTEM READY");
}

void loop() {
  blinkWithCountdown(LED_RED, 5);
  blinkWithCountdown(LED_YELLOW, 3);
  blinkWithCountdown(LED_GREEN, 7);
}
