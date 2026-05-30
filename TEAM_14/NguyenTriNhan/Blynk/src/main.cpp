#define BLYNK_TEMPLATE_ID "TMPL6EfFsiDNq"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "5QVyoURCThPoECJHYXge7pSCnPohhrAB"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi mặc định của Wokwi (Không có mật khẩu)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Cấu hình chân linh kiện (Khớp với mạch bạn vừa vẽ)
#define DHTPIN 15
#define DHTTYPE DHT22
#define LED_PIN 2
#define BUTTON_PIN 4
#define CLK 18
#define DIO 19

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// Các biến lưu trạng thái
int countdownValue = 0;
bool isCounting = false;
int lastButtonState = HIGH;

// Hàm 1: Nhận lệnh bật/tắt công tắc từ Blynk (Chân V1)
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    digitalWrite(LED_PIN, HIGH); // Bật đèn
    countdownValue = 60;         // Bắt đầu đếm ngược 60 giây
    isCounting = true;
  }
  else
  {
    digitalWrite(LED_PIN, LOW); // Tắt đèn
    isCounting = false;
    countdownValue = 0;
    display.showNumberDec(0);  // Hiện số 0 trên mạch
    Blynk.virtualWrite(V4, 0); // Gửi số 0 lên màn hình Blynk
  }
}

// Hàm 2: Đọc cảm biến DHT22 gửi lên Blynk
void sendSensor()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h))
  {
    Blynk.virtualWrite(V2, t); // Gửi Nhiệt độ vào V2
    Blynk.virtualWrite(V3, h); // Gửi Độ ẩm vào V3
  }
}

// Hàm 3: Xử lý đếm ngược thời gian
void handleTimer()
{
  if (isCounting && countdownValue > 0)
  {
    countdownValue--;
    display.showNumberDec(countdownValue);  // Hiển thị số lên màn hình TM1637
    Blynk.virtualWrite(V4, countdownValue); // Gửi số thời gian lên chân V4 Blynk

    // Nếu đếm về 0 thì tắt đèn
    if (countdownValue == 0)
    {
      isCounting = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, 0); // Tự động gạt tắt công tắc trên app Blynk
    }
  }
}

// Hàm 4: Kiểm tra nút nhấn cứng trên mạch
void checkButton()
{
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH)
  {
    // Nếu nút được nhấn, đổi trạng thái
    isCounting = !isCounting;
    if (isCounting)
    {
      digitalWrite(LED_PIN, HIGH);
      countdownValue = 60;
      Blynk.virtualWrite(V1, 1); // Cập nhật nút trên app thành Bật
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
      countdownValue = 0;
      Blynk.virtualWrite(V1, 0); // Cập nhật nút trên app thành Tắt
      display.showNumberDec(0);
      Blynk.virtualWrite(V4, 0);
    }
  }
  lastButtonState = buttonState;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f); // Bật sáng tối đa LED 7 đoạn
  display.showNumberDec(0);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Cài đặt chu kỳ chạy cho các hàm
  timer.setInterval(2000L, sendSensor);  // 2 giây đo nhiệt độ 1 lần
  timer.setInterval(1000L, handleTimer); // 1 giây đếm ngược 1 lần
  timer.setInterval(200L, checkButton);  // 0.2 giây kiểm tra nút nhấn 1 lần
}

void loop()
{
  Blynk.run();
  timer.run();
}
