/*
THÔNG TIN NHÓM 3:
1.Trần Hữu Bảo Anh
2.Lương Gia Mẫn
3.Nguyễn Trung Quân
4.Phan Văn Tôn Bảo
5.Cao Khả Tài
*/

#include "main.h"
#include "ultils.h"

// ===== Hardware objects (định nghĩa tại đây) =====
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

// ===== Shared state =====
float h = NAN, t = NAN;
const char *statusText = "----";
int activeLED = -1;
bool ledOn = false;

unsigned long lastDhtMs = 0;
unsigned long lastBlinkMs = 0;

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  dht.begin();

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  setAllLEDsLow(LED_RED, LED_YELLOW, LED_GREEN);
  drawOLED(display, t, h, statusText);
}

void loop()
{
  unsigned long now = millis();

  // ===== 1) Đọc DHT theo chu kỳ (non-blocking) =====
  if (now - lastDhtMs >= DHT_INTERVAL_MS)
  {
    lastDhtMs = now;

    float newH = dht.readHumidity();
    float newT = dht.readTemperature();

    if (isnan(newH) || isnan(newT))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      statusText = "DHT ERR";
      activeLED = -1; // hoặc giữ LED cũ nếu bạn muốn
    }
    else
    {
      h = newH;
      t = newT;

      updateStatusFromTemp(t, statusText, activeLED, LED_GREEN, LED_YELLOW, LED_RED);

      Serial.print("T=");
      Serial.print(t);
      Serial.print("C  ");
      Serial.print("H=");
      Serial.print(h);
      Serial.println("%");
    }

    drawOLED(display, t, h, statusText);
  }

  // ===== 2) Nhấp nháy LED theo chu kỳ (non-blocking) =====
  if (now - lastBlinkMs >= BLINK_INTERVAL_MS)
  {
    lastBlinkMs = now;

    setAllLEDsLow(LED_RED, LED_YELLOW, LED_GREEN);

    if (activeLED != -1)
    {
      ledOn = !ledOn;
      digitalWrite(activeLED, ledOn ? HIGH : LOW);
    }
    else
    {
      ledOn = false;
    }
  }
}
