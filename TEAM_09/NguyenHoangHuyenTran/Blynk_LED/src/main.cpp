#include <Arduino.h>
#include <TM1637Display.h>
// 1. [MỚI] THÊM THƯ VIỆN CHO CẢM BIẾN DHT
#include "DHT.h" 

#define BLYNK_TEMPLATE_ID "TMPL6ASlZf_Q8"
#define BLYNK_TEMPLATE_NAME "esp32"
#define BLYNK_AUTH_TOKEN "Piseg1_V3_zEs-xC3oDMOAK8AtK7dRjg"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi


#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xxanh

#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637

// 2. [MỚI] ĐỊNH NGHĨA CHÂN VÀ LOẠI CẢM BIẾN
#define DHTPIN 16     // Chân SDA của DHT22 nối vào chân 16 của ESP32 (khớp với diagram)
#define DHTTYPE DHT22   // Loại cảm biến là DHT22

//Biến toàn cục
ulong currentMiliseconds = 0; //Thời gian hiện tại - miliseconds 
bool blueButtonON = true;     //Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637

//Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);

// 3. [MỚI] KHỞI TẠO ĐỐI TƯỢNG DHT
DHT dht(DHTPIN, DHTTYPE); 

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void sendDHTData(); // 4. [MỚI] KHAI BÁO HÀM ĐỌC VÀ GỬI DỮ LIỆU DHT

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  
  // 5. [MỚI] BẮT ĐẦU CHẠY CẢM BIẾN DHT
  dht.begin(); 

  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  //Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  sendDHTData(); // 6. [MỚI] CHẠY HÀM DHT TRONG VÒNG LẶP
}

// put function definitions here:
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}
void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  
  // 7. [TINH CHỈNH] ĐỔI THÀNH HIGH ĐỂ CHẠM LÀ ĐÈN BẬT/TẮT NGAY
  if (v == HIGH) return; 

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
  if (blueButtonON){
    display.showNumberDec(value);
  }
}

// 8. [MỚI] VIẾT ĐỊNH NGHĨA HÀM ĐỌC VÀ GỬI DỮ LIỆU DHT
void sendDHTData() {
  static ulong lastTime = 0;
  // Chỉ đọc cảm biến 2 giây một lần để tránh làm nóng cảm biến và nghẽn mạng
  if (!IsReady(lastTime, 2000)) return; 

  // Đọc độ ẩm (%)
  float h = dht.readHumidity();
  // Đọc nhiệt độ (°C)
  float t = dht.readTemperature(); 

  // Kiểm tra nếu đọc thất bại (NAN - Not A Number)
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // In ra Serial để kiểm tra
  Serial.print("Humidity: "); Serial.print(h); Serial.print("% ");
  Serial.print("Temperature: "); Serial.print(t); Serial.println("°C");

  // Gửi giá trị lên Blynk.
  // Công chúa nhớ tạo Datastream V2 cho Nhiệt độ và V3 cho Độ ẩm trên Web Blynk nhé!
  Blynk.virtualWrite(V2, t); // Chân ảo V2 cho Nhiệt độ
  Blynk.virtualWrite(V3, h); // Chân ảo V3 cho Độ ẩm
}

//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) { //virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  blueButtonON = param.asInt();  // Lấy giá trị từ ứng dụng Blynk
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}