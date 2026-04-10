#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define IR_A_PIN 18 // Thay bằng Quang trở A
#define IR_B_PIN 19 // Thay bằng Quang trở B
#define LED_PIN 27
#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

// Mức logic của mô-đun quang trở trên Wokwi: Tối (bị che) = HIGH, Sáng = LOW
#define SENSOR_BLOCKED HIGH 
#define SENSOR_CLEAR LOW  

volatile int peopleCount = 0;
int totalIn = 0;
int totalOut = 0;
#define MAX_PEOPLE 30

enum State { IDLE, WAIT_B, WAIT_A, WAIT_CLEAR };
State currentState = IDLE;

unsigned long stateTime = 0;
unsigned long lastDebounce = 0;
const int DEBOUNCE_MS = 50;
const int TIMEOUT_MS = 5000;

void blinkLed(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        if (i < times - 1) delay(150);
    }
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // Tiêu đề
    display.setTextSize(1);
    display.setCursor(8, 0);
    display.println("= PEOPLE COUNTER =");
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    // Hiển thị số người
    display.setCursor(10, 15);
    display.println("So nguoi hien tai:");
    
    display.setTextSize(2);
    char countStr[10];
    sprintf(countStr, "%d", peopleCount);
    display.setCursor(55, 30);
    display.println(countStr);

    // Thống kê
    display.setTextSize(1);
    display.drawLine(0, 51, 127, 51, SSD1306_WHITE);
    display.setCursor(5, 54);
    char statStr[30];
    sprintf(statStr, "IN: %d   OUT: %d", totalIn, totalOut);
    display.println(statStr);

    // Cảnh báo
    if (peopleCount >= MAX_PEOPLE) {
        display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(10, 2);
        display.println("!! DANG QUA TAI !!");
        display.setTextColor(SSD1306_WHITE);
    }
    display.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(IR_A_PIN, INPUT);
    pinMode(IR_B_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Khong tim thay OLED!"));
        while (true);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("Khoi dong he thong...");
    display.display();
    delay(1000);
    
    updateDisplay();
    Serial.println("He thong san sang!");
}

void loop() {
    unsigned long now = millis();
    if (now - lastDebounce < DEBOUNCE_MS) return;
    lastDebounce = now;

    bool sA = digitalRead(IR_A_PIN) == SENSOR_BLOCKED;
    bool sB = digitalRead(IR_B_PIN) == SENSOR_BLOCKED;

    // Chống kẹt trạng thái
    if (currentState != IDLE && (now - stateTime > TIMEOUT_MS)) {
        Serial.println("Qua thoi gian, huy bo luot di.");
        currentState = IDLE;
    }

    switch (currentState) {
        case IDLE:
            if (sA && !sB) {
                currentState = WAIT_B;
                stateTime = now;
                Serial.println("[FSM] A bi che -> Cho B (Dang Vao)");
            } else if (sB && !sA) {
                currentState = WAIT_A;
                stateTime = now;
                Serial.println("[FSM] B bi che -> Cho A (Dang Ra)");
            }
            break;

        case WAIT_B:
            if (sB) {
                peopleCount++;
                totalIn++;
                Serial.printf("=> CO NGUOI VAO. Tong vao: %d, Hien tai: %d\n", totalIn, peopleCount);
                blinkLed(2);
                updateDisplay();
                currentState = WAIT_CLEAR;
            } else if (!sA && !sB) {
                currentState = IDLE;
            }
            break;

        case WAIT_A:
            if (sA) {
                if (peopleCount > 0) peopleCount--;
                totalOut++;
                Serial.printf("=> CO NGUOI RA. Tong ra: %d, Hien tai: %d\n", totalOut, peopleCount);
                blinkLed(1);
                updateDisplay();
                currentState = WAIT_CLEAR;
            } else if (!sA && !sB) {
                currentState = IDLE;
            }
            break;

        case WAIT_CLEAR:
            if (!sA && !sB) {
                currentState = IDLE;
                Serial.println("[FSM] Da qua han, tro ve IDLE");
            }
            break;
    }
}