#include <Arduino.h>
#include <TM1637Display.h>
#include <math.h>
#include <stdarg.h>

// ================= PIN =================
#define PIN_LED_RED     18
#define PIN_LED_YELLOW  5
#define PIN_LED_GREEN   17
#define PIN_LED_BLUE    12
#define PIN_BUTTON_BLUE 13

#define PIN_LDR 34
#define PIN_CLK 22
#define PIN_DIO 23

// ================= DISPLAY =================
TM1637Display display(PIN_CLK, PIN_DIO);

// ================= TIMER =================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond){
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// ================= STRING FORMAT =================
String StringFormat(const char *fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char *buff = (char *)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// ================= LED CLASS =================
class LED{
public:
  LED(){ _pin=-1; _state=false; _previousMillis=0; }

  void setup(int pin){
    _pin = pin;
    pinMode(_pin, OUTPUT);
  }

  void blink(unsigned long interval=500){
    if(!IsReady(_previousMillis, interval)) return;
    _state=!_state;
    digitalWrite(_pin,_state);
  }

  void set(bool bON){
    digitalWrite(_pin,bON?HIGH:LOW);
  }

private:
  int _pin;
  bool _state;
  unsigned long _previousMillis;
};

// ================= LDR =================
class LDR{
public:
  static int DAY_THRESHOLD;

  LDR(){ _pin=-1; _value=0; }

  void setup(int pin){
    _pin=pin;
    pinMode(_pin,INPUT);
  }

  int getValue(){
    _value=analogRead(_pin);
    return _value;
  }

private:
  int _pin;
  int _value;
};
int LDR::DAY_THRESHOLD = 2000;

// ================= TRAFFIC CLASS =================
class Trafic_Blink{
public:
  Trafic_Blink(){
    _previousMillis=0;
    _ledStatus=false;
  }

  void setupPin(int red,int yellow,int green,int blue,int button){
    _LEDs[0]=green;
    _LEDs[1]=yellow;
    _LEDs[2]=red;

    pinMode(red,OUTPUT);
    pinMode(yellow,OUTPUT);
    pinMode(green,OUTPUT);

    ledBlue.setup(blue);

    _pinButton=button;
    pinMode(_pinButton,INPUT);

    _idxLED=0;
  }

  void setupWaitTime(uint32_t redWait,uint32_t yellowWait,uint32_t greenWait){
    _waitTime[0]=greenWait*1000;
    _waitTime[1]=yellowWait*1000;
    _waitTime[2]=redWait*1000;
  }

  bool isButtonON(){
    static ulong t=0;
    static bool state=false;
    if(!IsReady(t,10)) return state;
    state = digitalRead(_pinButton)==HIGH;
    return state;
  }

  const char* ledString(int pin){
    if(pin==_LEDs[2]) return "RED";
    if(pin==_LEDs[1]) return "YELLOW";
    if(pin==_LEDs[0]) return "GREEN";
    return "UNK";
  }

  void run(LDR& ldr, TM1637Display& display){
    static uint32_t count=_waitTime[_idxLED];
    static bool prevDark=false;
    static bool prevBtn=false;

    bool btn=isButtonON();

    if(!IsReady(_previousMillis,500)) return;

    if(prevBtn!=btn){
      if(!btn) display.clear();
      ledBlue.set(btn);
      prevBtn=btn;
    }

    bool isDark = ldr.getValue()>LDR::DAY_THRESHOLD;

    // ===== NIGHT MODE =====
    if(isDark){
      if(prevDark!=isDark){
        prevDark=isDark;
        digitalWrite(_LEDs[0],LOW);
        digitalWrite(_LEDs[2],LOW);
        display.clear();
        Serial.println("🌙 NIGHT MODE");
      }
      _ledStatus=!_ledStatus;
      digitalWrite(_LEDs[1],_ledStatus);
      return;
    }

    // ===== DAY MODE =====
    if(prevDark!=isDark && prevDark){
      Serial.println("☀ DAY MODE");
      prevDark=isDark;
      _idxLED=0;
      count=_waitTime[_idxLED];
    }

    if(count==_waitTime[_idxLED]){
      _secondCount=(count/1000)-1;
      _ledStatus=true;

      for(int i=0;i<3;i++){
        digitalWrite(_LEDs[i], i==_idxLED);
      }
    }
    else{
      _ledStatus=!_ledStatus;
      digitalWrite(_LEDs[_idxLED],_ledStatus);
    }

    if(_ledStatus){
      if(btn) display.showNumberDec(_secondCount);
      Serial.printf("LED %s : %d s\n",ledString(_LEDs[_idxLED]),_secondCount);
      _secondCount--;
    }

    count-=500;
    if(count>0) return;

    _idxLED=(_idxLED+1)%3;
    count=_waitTime[_idxLED];
  }

private:
  bool _ledStatus;
  unsigned long _previousMillis;
  int _LEDs[3];
  int _idxLED;
  uint32_t _waitTime[3];
  int _secondCount;
  int _pinButton;
  LED ledBlue;
};

// ================= GLOBAL =================
Trafic_Blink traficLight;
LDR ldrSensor;

// ================= SETUP =================
void setup(){
  Serial.begin(115200);
  Serial.println("🚦 TRAFFIC SYSTEM START");

  ldrSensor.setup(PIN_LDR);

  traficLight.setupPin(
    PIN_LED_RED,
    PIN_LED_YELLOW,
    PIN_LED_GREEN,
    PIN_LED_BLUE,
    PIN_BUTTON_BLUE
  );

  traficLight.setupWaitTime(5,3,7);

  display.setBrightness(0x0a);
  display.clear();
}

// ================= LOOP =================
void loop(){
  traficLight.run(ldrSensor, display);
}
