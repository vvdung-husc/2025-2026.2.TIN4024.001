#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 4  // D2
#define I2C_SCL 5  // D1

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== I2C SCANNER ===");

  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.println("Scanning I2C addresses...");
  byte count = 0;

  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      if (i < 16) Serial.print("0");
      Serial.print(i, HEX);
      Serial.println();
      count++;
    }
  }

  Serial.print("Total devices: ");
  Serial.println(count);
  Serial.println("=== END SCAN ===");
}

void loop() {
  delay(10000);
}
