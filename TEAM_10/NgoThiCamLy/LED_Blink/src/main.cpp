#include <Arduino.h>

// ================= PIN MAP =================
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// ================= CONFIG =================
#define BLINK_INTERVAL 500   
#define BLINK_COUNT   7     

// ================= STATE =================
enum TrafficState {
  RED,
  YELLOW,
  GREEN
};

TrafficState currentState = RED;

// ================= TIMER & COUNTER =================
unsigned long lastBlinkTime = 0;
bool ledState = false;
int blinkCounter = 0;

// ====================================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.println("Start -> RED");
}

// ====================================================
void loop() {
  unsigned long now = millis();

  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = now;
    ledState = !ledState;

    // Tắt hết trước
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);

    // Bật LED theo trạng thái hiện tại
    switch (currentState) {
      case RED:
        digitalWrite(LED_RED, ledState);
        break;
      case YELLOW:
        digitalWrite(LED_YELLOW, ledState);
        break;
      case GREEN:
        digitalWrite(LED_GREEN, ledState);
        break;
    }

    // Đếm số lần nhấp nháy (chỉ đếm khi vừa TẮT xong)
    if (!ledState) {
      blinkCounter++;

      if (blinkCounter >= BLINK_COUNT) {
        blinkCounter = 0;

        // Chuyển trạng thái
        switch (currentState) {
          case RED:
            currentState = YELLOW;
            Serial.println("Change -> YELLOW");
            break;
          case YELLOW:
            currentState = GREEN;
            Serial.println("Change -> GREEN");
            break;
          case GREEN:
            currentState = RED;
            Serial.println("Change -> RED");
            break;
        }
      }
    }
  }
}


