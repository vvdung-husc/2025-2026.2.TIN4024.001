/* Thông tin nhóm 12
1. Phan Thanh Trưởng - 22T1020490
2. Lê Khắc Thức - 21t1020733
*/
#include "main.h"
#include "ultils.h"

// Khởi tạo các đối tượng toàn cục
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHTesp dht;

// Định nghĩa các biến dữ liệu
float temperature = 0;
float humidity = 0;
String statusText = "STARTING...";
int activeLED = -1;

void systemInit() {
  Serial.begin(115200);

  // Thiết lập chân LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Thiết lập I2C và OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  display.clearDisplay();
  display.display();

  // Thiết lập cảm biến
  dht.setup(DHT_PIN, DHTesp::DHT22);
  
  Serial.println("System Initialized!");
}

void systemLoop() {
  unsigned long now = millis();

  // Chạy các tác vụ độc lập
  readDHTTask(now);
  ledBlinkTask(now);
  oledTask(now);
}

void setup() {
  systemInit();
}

void loop() {
  systemLoop();
}