#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN ---
#define PIN_LED_RED    27 
#define PIN_LED_YELLOW 26 
#define PIN_LED_GREEN  25 

#define CLK 18            
#define DIO 19            
#define PIN_BUTTON 23     

// --- KHỞI TẠO ĐỐI TƯỢNG ---
TM1637Display display(CLK, DIO);

// Thời gian cho từng đèn (miligiây)
const unsigned long RED_TIME    = 5000;
const unsigned long GREEN_TIME  = 7000;
const unsigned long YELLOW_TIME = 3000;

enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

unsigned long stateStartTime = 0;
unsigned long duration = RED_TIME;
bool isPedestrianMode = false;

// Biến toàn cục để theo dõi việc hiển thị, tránh in trùng
int lastDisplayedSecond = -1; 

const char* GetStateName(TrafficState state) {
    switch (state) {
        case RED: return "RED";
        case GREEN: return "GREEN";
        case YELLOW: return "YELLOW";
        default: return "UNKNOWN";
    }
}

void Init_Traffic_System() {
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    display.setBrightness(0x0f); 
    display.clear();
}

void SetTrafficLight(TrafficState state) {
    // Tắt đèn cũ
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    switch (state) {
        case RED:
            digitalWrite(PIN_LED_RED, HIGH);
            duration = RED_TIME;
            break;
        case GREEN:
            digitalWrite(PIN_LED_GREEN, HIGH);
            duration = GREEN_TIME;
            break;
        case YELLOW:
            digitalWrite(PIN_LED_YELLOW, HIGH);
            duration = YELLOW_TIME;
            break;
    }
    stateStartTime = millis(); 
    currentState = state;
    
    // Reset biến hiển thị khi chuyển trạng thái mới
    lastDisplayedSecond = -1;

    // IN DÒNG TIÊU ĐỀ (Ví dụ: LED [RED] ON => 5 Seconds)
    Serial.printf("LED [%s] ON => %lu Seconds\r\n", GetStateName(state), duration / 1000);
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. XỬ LÝ NÚT BẤM
    if (digitalRead(PIN_BUTTON) == LOW && currentState == GREEN && !isPedestrianMode) {
        unsigned long elapsedTime = currentMillis - stateStartTime;
        if (duration - elapsedTime > 2000) {
            Serial.println("!!! PEDESTRIAN BUTTON PRESSED !!!");
            stateStartTime = currentMillis - (duration - 2000); 
            isPedestrianMode = true; 
        }
    }

    // 2. TÍNH TOÁN THỜI GIAN
    unsigned long elapsedTime = currentMillis - stateStartTime;
    
    // Chuyển trạng thái khi hết giờ
    if (elapsedTime >= duration) {
        switch (currentState) {
            case RED:    SetTrafficLight(GREEN); isPedestrianMode = false; break;
            case GREEN:  SetTrafficLight(YELLOW); break;
            case YELLOW: SetTrafficLight(RED); break;
        }
        return; 
    }

    // 3. HIỂN THỊ LOGIC (Đã sửa theo yêu cầu của bạn)
    int totalSeconds = duration / 1000;              // Tổng thời gian (Ví dụ: 5)
    int remainingSeconds = (duration - elapsedTime) / 1000; // Thời gian còn lại (Ví dụ: 5, 4, 3...)
    
    if (remainingSeconds != lastDisplayedSecond) {
        // Màn hình vẫn hiện số giây thực tế (5, 4, 3...)
        display.showNumberDec(remainingSeconds); 
        
        // --- CHỈ IN LOG KHI THỜI GIAN CÒN LẠI NHỎ HƠN TỔNG THỜI GIAN ---
        // (Tức là bỏ qua giây đầu tiên vì đã in ở dòng Tiêu đề rồi)
        if (remainingSeconds < totalSeconds && remainingSeconds > 0) {
            Serial.printf(" [%s] => Second: %d\r\n", GetStateName(currentState), remainingSeconds);
        }
        
        lastDisplayedSecond = remainingSeconds;
    }
}

void setup() {
    Serial.begin(115200);
    Init_Traffic_System();
    SetTrafficLight(RED); 
}