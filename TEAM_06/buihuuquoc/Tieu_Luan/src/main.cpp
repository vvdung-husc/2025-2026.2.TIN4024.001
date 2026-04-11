#include <DHT.h>

// ====== Cấu hình chân ======
#define DHTPIN 4 // DHT22 nối D4
#define DHTTYPE DHT22

#define LED_PIN 2    // LED nối D2
#define BUTTON_PIN 5 // Nút nối D5

// ====== Khởi tạo ======
DHT dht(DHTPIN, DHTTYPE);

bool ledState = false;
bool lastButtonState = HIGH;

float threshold = 30.0; // Ngưỡng nhiệt độ (độ C)

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  Serial.println("=== SYSTEM START ===");
}

void loop()
{
  // ===== Đọc nhiệt độ =====
  float temp = dht.readTemperature();

  if (isnan(temp))
  {
    Serial.println("Lỗi đọc DHT!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temp);
  Serial.println(" °C");

  // ===== Điều khiển tự động =====
  if (temp > threshold)
  {
    ledState = true;
  }
  else
  {
    ledState = false;
  }

  // ===== Đọc nút nhấn =====
  bool buttonState = digitalRead(BUTTON_PIN);

  // Nhấn nút (LOW vì dùng INPUT_PULLUP)
  if (buttonState == LOW && lastButtonState == HIGH)
  {
    ledState = !ledState; // đảo trạng thái
    Serial.println("Button pressed -> Toggle LED");
    delay(200); // chống dội
  }

  lastButtonState = buttonState;

  // ===== Xuất ra LED =====
  digitalWrite(LED_PIN, ledState);

  // ===== In trạng thái =====
  Serial.print("LED: ");
  Serial.println(ledState ? "ON" : "OFF");

  Serial.println("--------------------");

  delay(2000);
}