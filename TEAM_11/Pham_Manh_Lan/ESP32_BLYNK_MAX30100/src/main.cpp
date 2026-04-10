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
    float bpm = random(45, 126); 
    float spo2 = random(92, 101); 

    Serial.print("Nhịp tim: ");
    Serial.print(bpm);
    Serial.print(" BPM | SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    Blynk.virtualWrite(V1, bpm);
    Blynk.virtualWrite(V2, spo2);
    Blynk.virtualWrite(V3, "Đang đo...");

    // LED cảnh báo
    if (bpm > 120 || bpm < 50 || spo2 < 95) {
        Blynk.virtualWrite(V4, 1); // bật LED
    } else {
        Blynk.virtualWrite(V4, 0); // tắt LED
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