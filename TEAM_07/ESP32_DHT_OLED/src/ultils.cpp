#include "ultils.h"

// ===== INTERNAL =====
static unsigned long lastDHTRead = 0;
static unsigned long lastBlink = 0;
static bool ledState = false;

// ===== DHT TASK =====
void readDHTTask(unsigned long now) {
  if (now - lastDHTRead < DHT_INTERVAL) return;
  lastDHTRead = now;

  TempAndHumidity data = dht.getTempAndHumidity();
  temperature = data.temperature;
  humidity = data.humidity;

  if (temperature < 13) {
    statusText = "TOO COLD";
    activeLED = LED_GREEN;
  } else if (temperature < 20) {
    statusText = "COLD";
    activeLED = LED_GREEN;
  } else if (temperature < 25) {
    statusText = "COOL";
    activeLED = LED_YELLOW;
  } else if (temperature < 30) {
    statusText = "WARM";
    activeLED = LED_YELLOW;
  } else if (temperature < 35) {
    statusText = "HOT";
    activeLED = LED_RED;
  } else {
    statusText = "TOO HOT";
    activeLED = LED_RED;
  }

  Serial.printf(
    "Temp: %.1f | Humi: %.1f | %s\r\n",
    temperature, humidity, statusText.c_str()
  );
}

// ===== LED TASK =====
void ledBlinkTask(unsigned long now) {
  if (now - lastBlink < BLINK_INTERVAL) return;
  lastBlink = now;

  ledState = !ledState;

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  if (activeLED != -1 && ledState) {
    digitalWrite(activeLED, HIGH);
  }
}

// ===== OLED TASK =====
void oledTask() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temperature - HOT");

  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(temperature, 1);
  display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 54);
  display.print("Status: ");
  display.print(statusText);

  display.display();
}
