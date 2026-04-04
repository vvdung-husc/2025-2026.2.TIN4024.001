#define BLYNK_TEMPLATE_ID "TMPL6I7bT220N"
#define BLYNK_TEMPLATE_NAME "ESP32 RFID DOOR LOCK"
#define BLYNK_AUTH_TOKEN "8Jofuy6VoKDI82xgKsjPuJVWh9bzSlcN"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

#define SERVO_PIN 13
Servo doorLock;

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_GREEN 12
#define LED_RED 14
#define BUZZER_PIN 27

String validCardUID = "01 02 03 04"; 
bool isLocked = true;
unsigned long unlockTime = 0;

BlynkTimer timer;

void unlockDoor();
void lockDoor();
void invalidCardAlert();

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    h = random(40, 80);
    t = random(20, 35);
  }
  
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    unlockDoor();
    unlockTime = millis();
  } else {
    lockDoor();
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();
  
  doorLock.attach(SERVO_PIN);
  doorLock.write(0); 
  
  dht.begin();
  
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
  
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uidString = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidString += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uidString += String(rfid.uid.uidByte[i], HEX);
    }
    uidString.trim();
    uidString.toUpperCase();
    
    if (uidString == validCardUID) {
      unlockDoor();
      unlockTime = millis();
    } else {
      Serial.println("Invalid Card");
      invalidCardAlert();
    }
    rfid.PICC_HaltA();
  }

  if (!isLocked && millis() - unlockTime > 5000) {
    lockDoor();
  }
}

void unlockDoor() {
  doorLock.write(90);
  isLocked = false;
  
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  
  tone(BUZZER_PIN, 1000, 200);
  
  Blynk.virtualWrite(V0, 1);
  Blynk.virtualWrite(V3, "Unlocked");
  Serial.println("Door Unlocked");
}

void lockDoor() {
  doorLock.write(0);
  isLocked = true;
  
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V3, "Locked");
  Serial.println("Door Locked");
}

void invalidCardAlert() {
  for(int i=0; i<3; i++) {
    digitalWrite(LED_RED, LOW);
    tone(BUZZER_PIN, 500, 100);
    delay(150);
    digitalWrite(LED_RED, HIGH);
    delay(150);
  }
}