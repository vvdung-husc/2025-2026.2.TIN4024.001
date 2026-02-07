/*
THÔNG TIN NHÓM 9
1. Nguyễn Hoàng Huyền Trân
2. Hoàng Anh Minh
3. Hồ Văn Tấn Phát
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>


#define DHTPIN 16      
#define DHTTYPE DHT22  


#define LED_GREEN  15  
#define LED_YELLOW 2  
#define LED_RED    4   


#define OLED_SDA 13    
#define OLED_SCL 12    
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

 
  
  Wire.begin(OLED_SDA, OLED_SCL);

 
  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khong tim thay OLED!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
}


void resetLeds() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
}

void loop() {
  
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Loi doc DHT22!");
    return;
  }

  
  String status = "";
  int blinkLed = -1;

  if (t < 13) {
    status = "TOO COLD";
    blinkLed = LED_GREEN;
  } else if (t >= 13 && t < 20) {
    status = "COLD";
    blinkLed = LED_GREEN;
  } else if (t >= 20 && t < 25) {
    status = "COOL";
    blinkLed = LED_YELLOW;
  } else if (t >= 25 && t < 30) {
    status = "WARM";
    blinkLed = LED_YELLOW;
  } else if (t >= 30 && t < 35) {
    status = "HOT";
    blinkLed = LED_RED;
  } else { 
    status = "TOO HOT";
    blinkLed = LED_RED;
  }

 
  display.clearDisplay(); 

  
  display.setTextSize(1);      
  display.setCursor(0, 0);     
  display.print("Temperature: "); 
  display.print(status);       

 
  display.setTextSize(2);      
  display.setCursor(0, 12);   
  display.print(t);
  display.cp437(true);        
  display.write(167);          
  display.print("C");

 
  display.setTextSize(1);      
  display.setCursor(0, 35);    
  display.print("Humidity:");

  
  display.setTextSize(2);      
  display.setCursor(0, 47);    
  display.print(h);
  display.print(" %");        

  display.display();


  resetLeds(); 
  if (blinkLed != -1) {
    digitalWrite(blinkLed, HIGH); 
    delay(500);
    digitalWrite(blinkLed, LOW);  
    delay(500);
  }
}