#define BLYNK_TEMPLATE_ID "TMPL6ntzB6RM6"
#define BLYNK_TEMPLATE_NAME "DoNhipTim"
#define BLYNK_AUTH_TOKEN "PsSJsB1uTBBnE42PXiFb81gJpEWjOQkb"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 👉 thêm cái này
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define PULSE_PIN 34
int threshold = 2000;
bool beatDetected = false;

unsigned long lastBeatTime = 0;
int BPM = 0;

unsigned long lastDisplayTime = 0;

void setup() {
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("START");
  display.display();
  delay(1000);
}

void loop() {
  Blynk.run();

  int signal = analogRead(PULSE_PIN);

  // phát hiện nhịp
  if (signal > threshold && !beatDetected) {
    beatDetected = true;

    unsigned long now = millis();
    unsigned long delta = now - lastBeatTime;
    lastBeatTime = now;

    if (delta > 0) {
      BPM = 60000 / delta;
    }
  }

  if (signal < threshold) {
    beatDetected = false;
  }

  // gửi lên Blynk
  Blynk.virtualWrite(V0, BPM);
  Blynk.virtualWrite(V1, signal);

  // OLED
  if (millis() - lastDisplayTime > 500) {
    lastDisplayTime = millis();

    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Signal: ");
    display.println(signal);

    display.setTextSize(2);
    display.setCursor(0, 30);

    if (BPM > 0) {
      display.print("BPM: ");
      display.println(BPM);
    } else {
      display.println("Dang do...");
    }

    display.display();
  }
}