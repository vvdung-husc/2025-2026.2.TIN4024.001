#include "utils.h"
#include "main.h"

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150;

// ====== XỬ LÝ NÚT BLUE  ======
void handleBlueButton() {
    bool reading = digitalRead(BUTTON_BLUE);

    // phát hiện cạnh nhấn
    if (reading == LOW && lastButton == HIGH) {
        if (millis() - lastDebounceTime > debounceDelay) {
            blueEnabled = !blueEnabled;

            digitalWrite(LED_BLUE, blueEnabled ? HIGH : LOW);

            if (!blueEnabled) {
                display.clear();
            }

            lastDebounceTime = millis();
        }
    }

    lastButton = reading;
}

// ====== NHẤP NHÁY + ĐẾM NGƯỢC ======
void blinkWithCountdown(int ledPin, int seconds) {
    for (int i = seconds; i >= 0; i--) {

        if (blueEnabled) {
            display.showNumberDec(i, true);
        }

        digitalWrite(ledPin, HIGH);
        delay(500);
        handleBlueButton();   // kiểm tra nút trong lúc delay
        digitalWrite(ledPin, LOW);
        delay(500);
        handleBlueButton();   // kiểm tra lại
    }
}
