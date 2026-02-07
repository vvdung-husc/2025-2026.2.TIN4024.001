#include "ultils.h"

void updateStatusFromTemp(float tempC, const char *&statusText, int &activeLED,
                          int ledGreen, int ledYellow, int ledRed)
{
    if (tempC < 13)
    {
        statusText = "TOO COLD";
        activeLED = ledGreen;
    }
    else if (tempC < 20)
    {
        statusText = "COLD";
        activeLED = ledGreen;
    }
    else if (tempC < 25)
    {
        statusText = "COOL";
        activeLED = ledYellow;
    }
    else if (tempC < 30)
    {
        statusText = "WARM";
        activeLED = ledYellow;
    }
    else if (tempC < 35)
    {
        statusText = "HOT";
        activeLED = ledRed;
    }
    else
    {
        statusText = "TOO HOT";
        activeLED = ledRed;
    }
}

void drawOLED(Adafruit_SSD1306 &display, float t, float h, const char *statusText)
{
    display.clearDisplay();
    display.setTextColor(WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Temp: ");
    if (isnan(t))
        display.print("--");
    else
        display.print(t, 1);
    display.println(" C");

    display.print("Humi: ");
    if (isnan(h))
        display.print("--");
    else
        display.print(h, 1);
    display.println(" %");

    display.drawFastHLine(0, 25, 128, WHITE);

    display.setTextSize(2);
    display.setCursor(0, 35);
    display.println(statusText);

    display.display();
}

void setAllLEDsLow(int ledRed, int ledYellow, int ledGreen)
{
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, LOW);
}
