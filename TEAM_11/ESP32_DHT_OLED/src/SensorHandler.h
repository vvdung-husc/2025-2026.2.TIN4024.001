#include <DHT.h>
#define DHTPIN 16
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000; // 2 giây theo datasheet

void setupSensors() {
  dht.begin();
}

bool handleSensorReading(float &h, float &t) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    h = dht.readHumidity();
    t = dht.readTemperature();
    
    if (isnan(h) || isnan(t)) {
      Serial.println("Loi cam bien!");
      return false;
    }
    return true; // trả về true khi có dữ liệu mới
  }
  return false;
}
