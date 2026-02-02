#include <Arduino.h>
#include <TM1637Display.h>
#include <math.h>

// ================= PIN =================
#define PIN_RED     25
#define PIN_YELLOW  26
#define PIN_GREEN   27
#define PIN_BLUE    21
#define PIN_BTN     23
#define LDR_PIN     13

#define CLK 18
#define DIO 19

// ================= TIME (SECONDS) =================
#define TIME_RED     5
#define TIME_YELLOW  3
#define TIME_GREEN   7

// ================= LDR =================
#define DARK_ANALOG  2000   

// ================= NON-BLOCKING =================
bool IsReady(unsigned long &ulTimer, uint32_t ms)
{
    if (millis() - ulTimer < ms) return false;
    ulTimer = millis();
    return true;
}

// ================= LED BLINK =================
class LED_Blink
{
private:
    int _pin;
    bool _state;
    unsigned long _timer;

public:
    LED_Blink() : _pin(-1), _state(false), _timer(0) {}

    void setup(int pin)
    {
        _pin = pin;
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }

    void blink(uint32_t interval)
    {
        if (!IsReady(_timer, interval)) return;
        _state = !_state;
        digitalWrite(_pin, _state ? HIGH : LOW);
    }

    void off()
    {
        _state = false;
        digitalWrite(_pin, LOW);
    }
};

// ================= LDR =================
class LDR
{
private:
    int _pin;
    int _value;

public:
    LDR() : _pin(-1), _value(0) {}

    void setup(int pin)
    {
        _pin = pin;
        pinMode(_pin, INPUT);
    }

    int readAnalog()
    {
        static int prevValue = -1;

        _value = analogRead(_pin);

        float voltage = (float)_value * 3.3 / 4096.0;
        float resistance = 2000 * voltage / (1 - voltage / 3.3);
        float lux = pow(33e3 * pow(10, 0.7) / resistance, (1 / 0.7));

        // ===== PRINT ONLY WHEN CHANGED =====
        if (abs(_value - prevValue) > 50)
        {
            printf(
                "LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f lux\n",
                _value, voltage, resistance, lux
            );
            prevValue = _value;
        }

        return _value;
    }
};

// ================= TRAFFIC =================
class Trafic_Blink
{
private:
    int _pinRed, _pinYellow, _pinGreen;
    LED_Blink _blink;
    unsigned long _timerSecond;
    int _countdown;
    int _state;

public:
    enum { RED, YELLOW, GREEN };

    Trafic_Blink() : _timerSecond(0), _state(RED) {}

    void setup(int r, int y, int g)
    {
        _pinRed = r;
        _pinYellow = y;
        _pinGreen = g;

        pinMode(r, OUTPUT);
        pinMode(y, OUTPUT);
        pinMode(g, OUTPUT);

        changeState(RED);
    }

    void changeState(int newState)
    {
        _state = newState;

        digitalWrite(_pinRed, LOW);
        digitalWrite(_pinYellow, LOW);
        digitalWrite(_pinGreen, LOW);

        switch (_state)
        {
        case RED:
            _countdown = TIME_RED;
            _blink.setup(_pinRed);
            Serial.println("[RED] 5s");
            break;

        case YELLOW:
            _countdown = TIME_YELLOW;
            _blink.setup(_pinYellow);
            Serial.println("[YELLOW] 3s");
            break;

        case GREEN:
            _countdown = TIME_GREEN;
            _blink.setup(_pinGreen);
            Serial.println("[GREEN] 7s");
            break;
        }
    }

    void run(TM1637Display &display, bool showDisplay, bool isDark)
    {
        // ===== DARK MODE =====
        if (isDark)
        {
            _blink.setup(_pinYellow);
            _blink.blink(200);  
            display.clear();
            return;
        }

        // ===== NORMAL MODE =====
        _blink.blink(300);

        if (IsReady(_timerSecond, 1000))
        {
            _countdown--;

            if (showDisplay)
                display.showNumberDec(_countdown, true);
            else
                display.clear();

            if (_countdown <= 0)
            {
                if (_state == RED) changeState(GREEN);
                else if (_state == GREEN) changeState(YELLOW);
                else changeState(RED);
            }
        }
    }
};

// ================= GLOBAL =================
TM1637Display display(CLK, DIO);
Trafic_Blink traffic;
LDR ldr;

bool showCountdown = true;
bool lastBtnState = HIGH;

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);

    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_BLUE, OUTPUT);
    digitalWrite(PIN_BLUE, HIGH);

    display.setBrightness(7);
    display.clear();

    ldr.setup(LDR_PIN);
    traffic.setup(PIN_RED, PIN_YELLOW, PIN_GREEN);
}

// ================= LOOP =================
void loop()
{
    // ===== BUTTON =====
    bool btn = digitalRead(PIN_BTN);
    if (lastBtnState == HIGH && btn == LOW)
    {
        showCountdown = !showCountdown;
        digitalWrite(PIN_BLUE, showCountdown ? HIGH : LOW);

        Serial.println(showCountdown ? "DISPLAY ON" : "DISPLAY OFF");
        delay(200);
    }
    lastBtnState = btn;

    // ===== LDR =====
    int analogValue = ldr.readAnalog();
    bool isDark = analogValue > DARK_ANALOG;

    // ===== TRAFFIC =====
    traffic.run(display, showCountdown, isDark);
}