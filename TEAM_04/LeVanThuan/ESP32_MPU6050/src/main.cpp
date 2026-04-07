#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>

Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LED_PIN = 2;
const int BUZZER_PIN = 4;
const int BUZZER_CHANNEL = 0;

float baseline = 0.0;

const float NGUONG_YEU  = 0.8;
const float NGUONG_VUA  = 2.0;
const float NGUONG_MANH = 4.0;

const int SO_MAU = 20;

// Thoi gian in Serial
const unsigned long TG_IN_ON_DINH = 3000;
const unsigned long TG_IN_RUNG    = 300;

// Thoi gian cap nhat LCD
const unsigned long TG_LCD_ON_DINH = 1500;
const unsigned long TG_LCD_RUNG    = 400;

unsigned long lanInCuoi = 0;
unsigned long lanCapNhatLCD = 0;

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

void hieuChuanCamBien() {
  Serial.println("Dang hieu chuan, giu cam bien dung yen...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dang hieu chuan");
  lcd.setCursor(0, 1);
  lcd.print("Giu yen cam bien");

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

  Serial.print("Baseline = ");
  Serial.println(baseline, 3);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Baseline:");
  lcd.setCursor(0, 1);
  lcd.print(baseline, 2);
  delay(1500);
}

void tatCanhBao() {
  digitalWrite(LED_PIN, LOW);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void canhBaoNhe() {
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

void hienThiLCD(float doRung, const String &muc) {
  lcd.setCursor(0, 0);
  lcd.print("Do rung:        ");
  lcd.setCursor(8, 0);
  lcd.print(doRung, 2);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(muc);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  lcd.init();
  lcd.backlight();

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteTone(BUZZER_CHANNEL, 0);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("He thong giam");
  lcd.setCursor(0, 1);
  lcd.print("sat do rung");
  delay(1500);

  if (!mpu.begin()) {
    Serial.println("Khong tim thay MPU6050!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Khong tim thay");
    lcd.setCursor(0, 1);
    lcd.print("MPU6050!");

    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);
  hieuChuanCamBien();

  Serial.println("He thong giam sat do rung bat dau...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("He thong san");
  lcd.setCursor(0, 1);
  lcd.print("sang hoat dong");
  delay(1500);
}

void loop() {
  float doRung = locTrungBinh();
  String muc = "ON DINH";
  bool dangRung = false;

  if (doRung >= NGUONG_MANH) {
    muc = "RUNG MANH";
    canhBaoManh();
    dangRung = true;
  }
  else if (doRung >= NGUONG_VUA) {
    muc = "RUNG VUA";
    canhBaoVua();
    dangRung = true;
  }
  else if (doRung >= NGUONG_YEU) {
    muc = "RUNG NHE";
    canhBaoNhe();
    dangRung = true;
  }
  else {
    muc = "ON DINH";
    tatCanhBao();
    dangRung = false;
  }

  unsigned long tgIn = dangRung ? TG_IN_RUNG : TG_IN_ON_DINH;
  unsigned long tgLCD = dangRung ? TG_LCD_RUNG : TG_LCD_ON_DINH;

  if (millis() - lanInCuoi >= tgIn) {
    if (dangRung) {
      Serial.print("[CANH BAO] Do rung: ");
      Serial.print(doRung, 3);
      Serial.print(" m/s^2 | ");
      Serial.println(muc);
    } else {
      Serial.print("[Trang thai] ");
      Serial.print(muc);
      Serial.print(" | Do rung: ");
      Serial.print(doRung, 3);
      Serial.println(" m/s^2");
    }
    lanInCuoi = millis();
  }

  if (millis() - lanCapNhatLCD >= tgLCD) {
    hienThiLCD(doRung, muc);
    lanCapNhatLCD = millis();
  }

  delay(100);
}