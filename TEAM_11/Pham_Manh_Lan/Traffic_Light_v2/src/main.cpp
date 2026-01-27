#include <Arduino.h>
#include <TM1637Display.h>

/* ========= CHÂN THEO diagram.json (ĐÃ SỬA LED BLUE) ========= */
#define LED_RED     14
#define LED_YELLOW  27
#define LED_GREEN   26

#define LED_BLUE    21
#define BTN_PIN     23

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

/* ========= BIẾN ========= */
bool paused = false;
bool lastBtnState = HIGH;

/* ========= BUTTON ========= */
void checkButton() {
  bool btnState = digitalRead(BTN_PIN);

  if (lastBtnState == HIGH && btnState == LOW) {
    paused = !paused;
    Serial.println(paused ? "PAUSE" : "RESUME");
    delay(200);
  }
  lastBtnState = btnState;
}

/* ========= DELAY CÓ CHECK ========= */
void smartDelay(int ms) {
  for (int i = 0; i < ms / 10; i++) {
    checkButton();
    if (paused) return;
    delay(10);
  }
}

/* ========= BLINK ========= */
void blinkLed(int ledPin, int seconds) {
  for (int i = seconds; i > 0; i--) {

    while (paused) {
      checkButton();
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH); // báo pause
      delay(50);
    }

    digitalWrite(LED_BLUE, LOW);
    display.showNumberDec(i, true);

    digitalWrite(ledPin, HIGH);
    smartDelay(500);
    digitalWrite(ledPin, LOW);
    smartDelay(500);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();

  Serial.println("System start");
}

void loop() {

  /* ===== ĐÈN XANH ===== */
  blinkLed(LED_GREEN, 7);
  Serial.println("Den xanh da nhay 7s");

  /* ===== ĐÈN VÀNG ===== */
  blinkLed(LED_YELLOW, 3);
  Serial.println("Den vang da nhay 3s");

  /* ===== ĐÈN ĐỎ ===== */
  blinkLed(LED_RED, 5);
  Serial.println("Den do da nhay 5s");
}
