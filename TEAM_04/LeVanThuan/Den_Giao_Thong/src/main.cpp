#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define LED_RED    4
#define LED_YELLOW 2
#define LED_GREEN  15

// ====== KHAI BÁO HÀM ======
void redPhase(int seconds);
void yellowPhase(int seconds, String note);
void greenPhase(int seconds);
void setLight(bool red, bool yellow, bool green);
void countdown(String label, int seconds, String extra = "");

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 20);
  display.print("Traffic Light");
  display.display(); 

  delay(2000);
}


void loop() {
  redPhase(10);
  yellowPhase(3, "CHUAN BI XANH");
  greenPhase(10);
  yellowPhase(3, "CHUAN BI DO");
}

// ====== PHA ĐỎ ======
void redPhase(int seconds) {
  setLight(HIGH, LOW, LOW);
  countdown("DEN DO", seconds);
}

// ====== PHA VÀNG ======
void yellowPhase(int seconds, String note) {
  setLight(LOW, HIGH, LOW);
  countdown("DEN VANG", seconds, note);
}

// ====== PHA XANH ======
void greenPhase(int seconds) {
  setLight(LOW, LOW, HIGH);
  countdown("DEN XANH", seconds);
}

// ====== BẬT ĐÈN ======
void setLight(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

// ====== HIỂN THỊ OLED + ĐẾM NGƯỢC ======
void countdown(String label, int seconds, String extra) {
  for (int i = seconds; i > 0; i--) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("DEN GIAO THONG");

    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print(label);

    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print("Con: ");
    display.print(i);
    display.print(" s");

    if (extra != "") {
      display.setCursor(0, 55);
      display.print(extra);
    }

    display.display();
    delay(1000);
  }
}
