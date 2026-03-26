
#include "ultils.h"

// ===== CLASS LED_Blink - Điều khiển LED đơn giản =====

LED_Blink::LED_Blink()
{
    _pin = -1;
    _state = false;
    _previousMillis = 0;
}

void LED_Blink::setup(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

void LED_Blink::blink(unsigned long interval)
{
    // Kiểm tra thời gian, nếu chưa đủ interval thì return
    if (!IsReady(_previousMillis, interval))
        return;
    
    // Đảo trạng thái LED (ON <-> OFF)
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
}

// ===== CLASS Trafic_Blink - Điều khiển đèn giao thông =====

Trafic_Blink::Trafic_Blink()
{
    _ledStatus = false;
    _previousMillis = 0;
}

void Trafic_Blink::setupPin(int pinRed, int pinYellow, int pinGreen)
{
    // Lưu thứ tự LED: [0]=GREEN, [1]=YELLOW, [2]=RED
    _LEDs[0] = pinGreen;
    _LEDs[1] = pinYellow;
    _LEDs[2] = pinRed;
    
    // Cấu hình các chân là OUTPUT
    pinMode(pinRed, OUTPUT);
    pinMode(pinYellow, OUTPUT);
    pinMode(pinGreen, OUTPUT);

    // Bắt đầu với đèn XANH (index 0)
    _idxLED = 0;
    
    // Khởi tạo: Bật đèn XANH, tắt VÀNG và ĐỎ
    digitalWrite(pinGreen, HIGH);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinRed, LOW);
}

void Trafic_Blink::setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait)
{
    // Chuyển từ giây sang milliseconds và lưu vào mảng tương ứng
    _waitTime[0] = greenWait * 1000;    // Thời gian đèn XANH
    _waitTime[1] = yellowWait * 1000;   // Thời gian đèn VÀNG
    _waitTime[2] = redWait * 1000;      // Thời gian đèn ĐỎ
}

/*
 * Hàm điều khiển nhấp nháy đèn giao thông với logic tự động
 * 
 * @param interval: Khoảng thời gian nhấp nháy (milliseconds) - mặc định 500ms
 * @param isDark: Trạng thái môi trường (true = tối, false = sáng)
 * @return: true nếu đèn chuyển màu, false nếu đang nhấp nháy
 */
bool Trafic_Blink::blink(unsigned long interval, bool isDark)
{
    // Biến static giữ giá trị giữa các lần gọi hàm
    static uint32_t count = 0;           // Đếm ngược thời gian (milliseconds)
    static int secondCount = 0;          // Đếm ngược giây để hiển thị
    static bool prevDark = false;        // Lưu trạng thái tối/sáng trước đó
    static bool initialized = false;     // Cờ đánh dấu đã khởi tạo

    // ===== KHỞI TẠO LẦN ĐẦU =====
    if (!initialized)
    {
        initialized = true;
        count = _waitTime[_idxLED];      // Lấy thời gian đèn hiện tại
        secondCount = count / 1000;       // Chuyển sang giây
        _ledStatus = true;
        
        // Bật đèn đầu tiên (GREEN) không in ra Serial
        for (size_t i = 0; i < 3; i++)
        {
            if (i == _idxLED)
                digitalWrite(_LEDs[i], HIGH);
            else
                digitalWrite(_LEDs[i], LOW);
        }
    }

    // Kiểm tra thời gian, nếu chưa đủ interval thì return
    if (!IsReady(_previousMillis, interval)) return false;

    // ===== CHẾ ĐỘ BAN ĐÊM (DARK MODE) =====
    if (isDark)
    {
        // Phát hiện chuyển từ SÁNG → TỐI
        if (prevDark != isDark)
        {
            prevDark = isDark;
            digitalWrite(_LEDs[0], LOW);     // Tắt đèn XANH
            digitalWrite(_LEDs[2], LOW);     // Tắt đèn ĐỎ
            
            // *** IN RA THÔNG BÁO: CHUYỂN SANG CHẾ ĐỘ BAN ĐÊM ***
            printf("IT IS DARK!!!!\n");
        }

        // Chỉ nhấp nháy đèn VÀNG trong chế độ ban đêm
        _ledStatus = !_ledStatus;
        digitalWrite(_LEDs[1], _ledStatus ? HIGH : LOW);
        return false;  // Không chuyển đèn trong chế độ tối
    }

    // ===== PHÁT HIỆN CHUYỂN TỪ TỐI → SÁNG =====
    if (prevDark != isDark && prevDark == true)
    {
        // *** IN RA THÔNG BÁO: CHUYỂN SANG CHẾ ĐỘ BAN NGÀY ***
        printf("YEAH!!! IT IS DAY!!!!\n");
        
        prevDark = isDark;
        _ledStatus = false;
        _idxLED = 0;                      // Reset về đèn XANH
        count = _waitTime[_idxLED];       // Lấy lại thời gian đèn XANH
    }

    // ===== CHẾ ĐỘ BAN NGÀY - BẮT ĐẦU CHU KỲ ĐÈN MỚI =====
    if (count == _waitTime[_idxLED])
    {
        secondCount = count / 1000;

        _ledStatus = true;
        
        // Bật đèn hiện tại, tắt các đèn khác
        for (size_t i = 0; i < 3; i++)
        {
            if (i == _idxLED)
            {
                digitalWrite(_LEDs[i], HIGH);
                // In ra thông tin đèn vừa bật và thời gian
                printf("LED [%-6s] ON => %d Seconds\n", ledString(_LEDs[i]), count / 1000);
            }
            else
                digitalWrite(_LEDs[i], LOW);
        }
    }
    // ===== NHẤP NHÁY ĐÈN HIỆN TẠI =====
    else
    {
        _ledStatus = !_ledStatus;
        digitalWrite(_LEDs[_idxLED], _ledStatus ? HIGH : LOW);
    }

    // ===== IN RA COUNTDOWN (khi đèn đang sáng) =====
    if (_ledStatus)
    {
        printf(" [%s] => Second: %d\n", ledString(_LEDs[_idxLED]), secondCount);
        --secondCount;  // Giảm số giây
    }

    // ===== ĐẾM NGƯỢC THỜI GIAN =====
    count -= 500;  // Giảm 500ms mỗi lần
    if (count > 0)
        return false;  // Chưa hết thời gian, tiếp tục ở đèn hiện tại

    // ===== CHUYỂN SANG ĐÈN TIẾP THEO =====
    _idxLED = (_idxLED + 1) % 3;        // 0→1→2→0→1→2...
    count = _waitTime[_idxLED];          // Lấy thời gian đèn mới
    return true;                         // Trả về true: đã chuyển đèn
}

const char *Trafic_Blink::ledString(int pin)
{
    if (pin == _LEDs[2])
        return "RED";
    else if (pin == _LEDs[1])
        return "YELLOW";
    else if (pin == _LEDs[0])
        return "GREEN";
    else
        return "UNKNOWN";
}

//----- LDR -----
LDR::LDR()
{
    _pin = -1;
    _value = 0;
    _vcc5Volt = true;
}
void LDR::setup(int pin, bool vcc5Volt)
{
    _pin = pin;
    _vcc5Volt = vcc5Volt;
    pinMode(_pin, INPUT);
}
int LDR::getValue()
{
    _value = analogRead(_pin);
    return _value;
}

float LDR::readLux(int *analogValue)
{
    static float prevLux = -1.0;
    float voltage, resistance, lux;

    getValue();

    if (analogValue != nullptr)
    {
        *analogValue = _value;
    }

    if (_vcc5Volt)
    {
        // VCC = 5V
        voltage = (float)_value * 5.0 / 4096.0;
        resistance = 2000 * voltage / (1 - voltage / 5.0);
        lux = pow(50 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
        return lux;
    }
    else
    {
        // VCC = 3.3V
        voltage = (float)_value * 3.3 / 4096.0;
        resistance = 2000 * voltage / (1 - voltage / 3.3);
        lux = pow(33 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
    }

    if (lux != prevLux)
    {
        prevLux = lux;
        if (prevLux >= 1.0)
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.0f] lux\n", _value, voltage, resistance, lux, prevLux);
        else
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.1f] lux\n", _value, voltage, resistance, lux, prevLux);
    }

    return lux;
}

//----- Functions -----
// Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
    if (millis() - ulTimer < millisecond)
        return false;
    ulTimer = millis();
    return true;
}

// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...)
{
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
