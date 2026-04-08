#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6HaxfHF1Z"
#define BLYNK_TEMPLATE_NAME "ESP32 BLYNK MAX30100"
#define BLYNK_AUTH_TOKEN "7EndaDXZDnv93hW74qHGuQ3IeUu5vQ1b"

#include <WiFi.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include "MAX30100_PulseOximeter.h"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

void sendDemoDataToBlynk() {
    // Random nhịp tim từ 45 đến 125 BPM để có thể kích hoạt ngưỡng <50 và >120
    float bpm = random(45, 126); 
    // Random SpO2 từ 92 đến 100% để có thể kích hoạt ngưỡng <95%
    float spo2 = random(92, 101); 
    Serial.print("Nhịp tim (Demo): ");
    Serial.print(bpm);
    Serial.print(" BPM | SpO2 (Demo): ");
    Serial.print(spo2);
    Serial.println(" %");
    Blynk.virtualWrite(V1, bpm);
    Blynk.virtualWrite(V2, spo2);
    Blynk.virtualWrite(V3, "Đang đo...");
    // Xử lý cảnh báo nhịp tim
    if (bpm > 120) {
        Blynk.logEvent("heart_rate_alert", "Cảnh báo: Nhịp tim quá nhanh!");
        Blynk.virtualWrite(V4, 1);
    } else if (bpm < 50) {
        Blynk.logEvent("heart_rate_alert", "Cảnh báo: Nhịp tim quá chậm!");
        Blynk.virtualWrite(V4, 1);
    } else {
        Blynk.virtualWrite(V4, 0);
    }
    // Xử lý cảnh báo SpO2
    if (spo2 < 95) {
        Blynk.logEvent("spo2_alert", "Cảnh báo: SpO2 thấp bất thường!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Đang khởi động ESP32 và kết nối Wokwi WiFi...");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Kết nối Blynk thành công! Bắt đầu gửi dữ liệu giả lập...");
    Blynk.virtualWrite(V3, "Hệ thống sẵn sàng");
    timer.setInterval(2000L, sendDemoDataToBlynk);
}

void loop() {
    Blynk.run();
    timer.run();
}