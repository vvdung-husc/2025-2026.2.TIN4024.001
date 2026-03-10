#include <Arduino.h>
#include <TM1637Display.h>

#define BLYNK_TEMPLATE_ID "TMPL6VZS0Pcds"
#define BLYNK_TEMPLATE_NAME "BlinkDHT"
#define BLYNK_AUTH_TOKEN "1mNeSBntjstbek95KfoAivAB-dnki-xD"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ================= WIFI =================
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ================= PIN =================
#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

// ================= OBJECTS =================
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// ================= GLOBAL =================
ulong currentMiliseconds = 0;
bool blueButtonON = true;

// ================= TIMER FUNCTION =================
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
// Function Prototypes
void uptimeBlynk();
void updateBlueButton();
void readDHT();
// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==>");
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  currentMiliseconds = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHT();
}

// ================= BUTTON =================
void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;

  if (v == LOW) return;

  blueButtonON = !blueButtonON;

  if (blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }

  Blynk.virtualWrite(V1, blueButtonON);
}

// ================= UPTIME =================
void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  ulong value = millis() / 1000;

  Blynk.virtualWrite(V0, value);

  if (blueButtonON){
    display.showNumberDec(value);
  }
}

// ================= READ DHT =================
void readDHT(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // đọc mỗi 2s

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

// ================= BLYNK WRITE =================
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();

  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}