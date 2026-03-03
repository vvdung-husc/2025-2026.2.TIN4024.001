#include "ultils.h"

static unsigned long lastDHTRead = 0;
static unsigned long lastBlink = 0;
static unsigned long lastOLED = 0;
static bool ledState = false;

void readDHTTask(unsigned long now) {
  if (now - lastDHTRead < DHT_INTERVAL) return;
  lastDHTRead = now;

  TempAndHumidity data = dht.getTempAndHumidity();
  temperature = data.temperature;
  humidity = data.humidity;

  // Logic ngưỡng nhiệt độ theo yêu cầu ảnh 1
  if (temperature < 13) {
    statusText = "TOO COLD"; activeLED = LED_GREEN;
  } else if (temperature < 20) {
    statusText = "COLD";     activeLED = LED_GREEN;
  } else if (temperature < 25) {
    statusText = "COOL";     activeLED = LED_YELLOW;
  } else if (temperature < 30) {
    statusText = "WARM";     activeLED = LED_YELLOW;
  } else if (temperature < 35) {
    statusText = "HOT";      activeLED = LED_RED;
  } else {
    statusText = "TOO HOT";  activeLED = LED_RED;
  }

  // Sửa lỗi terminal hiển thị ngang: Dùng println để ngắt dòng chuẩn
  Serial.print("Temp: ");
  Serial.print(temperature, 1);
  Serial.print(" | Hum: ");
  Serial.print(humidity, 1);
  Serial.print(" | Status: ");
  Serial.println(statusText); 
}

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

void oledTask(unsigned long now) {
  if (now - lastOLED < OLED_INTERVAL) return;
  lastOLED = now;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Status: "); display.println(statusText);

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(temperature, 1); display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Humidity: "); display.print(humidity, 1); display.println(" %");
  display.display();
}