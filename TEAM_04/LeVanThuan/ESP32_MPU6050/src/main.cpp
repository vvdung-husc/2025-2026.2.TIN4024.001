#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

// WIFI + TELEGRAM
#include <WiFi.h>
#include <HTTPClient.h>

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
String botToken = "8791087879:AAEIkG9nYjvSM0__4QdFcHjs5VWLur_A9M4";
String chatID   = "-5252683483";

// ================= HARDWARE =================
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LED_PIN = 2;
const int BUZZER_PIN = 4;
const int BUZZER_CHANNEL = 0;

// ================= THRESHOLD =================
float baseline = 0.0;

const float NGUONG_YEU  = 0.8;
const float NGUONG_VUA  = 2.0;
const float NGUONG_MANH = 4.0;

const int SO_MAU = 20;

// ================= CONTROL =================
String trangThaiCu = ""; // lưu trạng thái trước
unsigned long lanCapNhatLCD = 0;

// ================= ENCODE =================
String urlEncode(String msg) {
  String encoded = "";
  char c;
  char code0;
  char code1;

  for (int i = 0; i < msg.length(); i++) {
    c = msg.charAt(i);

    if (isalnum(c)) {
      encoded += c;
    } else {
      encoded += '%';
      code0 = (c >> 4) & 0xF;
      code1 = c & 0xF;
      encoded += (code0 > 9) ? char(code0 + 'A' - 10) : char(code0 + '0');
      encoded += (code1 > 9) ? char(code1 + 'A' - 10) : char(code1 + '0');
    }
  }
  return encoded;
}

// ================= TELEGRAM =================
void guiTelegram(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://api.telegram.org/bot" + botToken +
                 "/sendMessage?chat_id=" + chatID +
                 "&text=" + urlEncode(message);

    http.begin(url);
    int httpCode = http.GET();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      Serial.println("✅ Gui Telegram thanh cong");
    } else {
      Serial.println("❌ Loi gui Telegram");
    }

    http.end();
  }
}

// ================= MPU =================
float tinhDoRung() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float tong = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  return fabs(tong - baseline);
}

float locTrungBinh() {
  float tong = 0;
  for (int i = 0; i < SO_MAU; i++) {
    tong += tinhDoRung();
    delay(5);
  }
  return tong / SO_MAU;
}

// ================= CALIB =================
void hieuChuanCamBien() {
  lcd.clear();
  lcd.print("Hieu chuan...");

  float tong = 0;

  for (int i = 0; i < 100; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float doLon = sqrt(
      a.acceleration.x * a.acceleration.x +
      a.acceleration.y * a.acceleration.y +
      a.acceleration.z * a.acceleration.z
    );

    tong += doLon;
    delay(20);
  }

  baseline = tong / 100.0;

  lcd.clear();
  lcd.print("Baseline:");
  lcd.setCursor(0, 1);
  lcd.print(baseline, 2);

  delay(1500);
}

// ================= ALERT =================
void tatCanhBao() {
  digitalWrite(LED_PIN, LOW);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void canhBaoVua() {
  digitalWrite(LED_PIN, HIGH);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void canhBaoManh() {
  digitalWrite(LED_PIN, HIGH);
  ledcWriteTone(BUZZER_CHANNEL, 2000);
}

// ================= LCD =================
void hienThiLCD(float doRung, String muc) {
  lcd.setCursor(0, 0);
  lcd.print("Do rung:        ");
  lcd.setCursor(8, 0);
  lcd.print(doRung, 2);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(muc);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  // WIFI
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi OK");

  guiTelegram("He thong ESP32 da ket noi thanh cong");

  // MPU
  if (!mpu.begin()) {
    lcd.print("MPU ERROR!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);
  hieuChuanCamBien();
}

// ================= LOOP =================
void loop() {
  float doRung = locTrungBinh();
  String muc = "ON DINH";

  if (doRung >= NGUONG_MANH) {
    muc = "RUNG MANH";
    canhBaoManh();
  }
  else if (doRung >= NGUONG_VUA) {
    muc = "RUNG VUA";
    canhBaoVua();
  }
  else if (doRung >= NGUONG_YEU) {
    muc = "RUNG NHE";
    tatCanhBao();
  }
  else {
    muc = "ON DINH";
    tatCanhBao();
  }

  // 🔥 CHỈ GỬI KHI TRẠNG THÁI THAY ĐỔI
  if (muc != trangThaiCu) {
    String message = "📡 He thong giam sat rung ESP32\n";
    message += "Trang thai: " + muc + "\n";
    message += "Do rung: " + String(doRung, 2) + " m/s^2";

    if (muc == "RUNG MANH") {
      message += "\n🚨 NGUY HIEM !!!";
    }

    guiTelegram(message);
    trangThaiCu = muc;
  }

  // LCD
  if (millis() - lanCapNhatLCD > 500) {
    hienThiLCD(doRung, muc);
    lanCapNhatLCD = millis();
  }

  delay(100);
}