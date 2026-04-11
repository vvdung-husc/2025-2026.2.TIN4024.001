/*
 * ============================================================
 * File       : src/main.cpp
 * Dự án      : Hệ thống Cảnh Báo Trộm
 *              (ESP32 + PIR + Telegram + Email)
 * Tác giả    : Phạm Thanh Hiếu
 * Ngày tạo   : 2024
 * Mô tả      : Hệ thống phát hiện chuyển động qua cảm biến PIR,
 *              cảnh báo tại chỗ bằng LED + Buzzer, gửi thông báo
 *              qua Telegram Bot và Email tự động.
 *
 * Sơ đồ kết nối:
 *   PIR OUT  --> GPIO 13
 *   LED Đỏ   --> GPIO 26 (qua điện trở 220Ω)
 *   Buzzer   --> GPIO 27
 * ============================================================
 */

// ============================================================
// PHẦN 1: KHAI BÁO THƯ VIỆN
// ============================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Thư viện Telegram Bot
#include <ArduinoJson.h>            // Thư viện xử lý JSON
#include <ESP_Mail_Client.h>        // Thư viện gửi Email qua SMTP


// ============================================================
// PHẦN 2: CẤU HÌNH WI-FI
// ============================================================
#define WIFI_SSID       "Wokwi-GUEST"   // Tên mạng Wi-Fi (Wokwi Simulator)
#define WIFI_PASSWORD   ""               // Mật khẩu (để trống với Wokwi-GUEST)


// ============================================================
// PHẦN 3: CẤU HÌNH TELEGRAM BOT
// ============================================================
// Hướng dẫn lấy Token:
//   1. Mở Telegram, tìm @BotFather
//   2. Gõ /newbot và làm theo hướng dẫn
//   3. BotFather sẽ cấp cho bạn một TOKEN
//
// Hướng dẫn lấy Chat ID:
//   1. Tìm @userinfobot trên Telegram
//   2. Gửi /start -> bot sẽ trả về Chat ID của bạn
// ============================================================
#define BOT_TOKEN       "8223812502:AAEBDywGhyGpKZ-sDmg0y94NDRHcHPtvbGo"   // << THAY THẾ TOKEN CỦA BẠN
#define CHAT_ID         "1900826323"     // << THAY THẾ CHAT ID CỦA BẠN


// ============================================================
// PHẦN 4: CẤU HÌNH EMAIL (SMTP)
// ============================================================
// Ví dụ sử dụng Gmail:
//   - Bật "App Password" trong tài khoản Google
//   - Dùng App Password thay cho mật khẩu thường
//   - Bật 2-Factor Authentication trước
// ============================================================
#define EMAIL_ENABLED       true                    // true = bật gửi email, false = tắt

#define SMTP_HOST           "smtp.gmail.com"        // Server SMTP (Gmail)
#define SMTP_PORT           465                     // Cổng SMTP SSL

#define SENDER_EMAIL        "your_email@gmail.com"  // << Email người gửi
#define SENDER_PASSWORD     "your_app_password"     // << App Password Gmail
#define SENDER_NAME         "He Thong Canh Bao Trom" // Tên hiển thị người gửi

#define RECIPIENT_EMAIL     "recipient@gmail.com"   // << Email người nhận


// ============================================================
// PHẦN 5: ĐỊNH NGHĨA CHÂN GPIO
// ============================================================
#define PIN_PIR     13    // Chân tín hiệu đầu ra của cảm biến PIR HC-SR501
#define PIN_LED     26    // Chân điều khiển LED đỏ cảnh báo
#define PIN_BUZZER  27    // Chân điều khiển còi Buzzer


// ============================================================
// PHẦN 6: HẰNG SỐ CẤU HÌNH HỆ THỐNG
// ============================================================
#define ALERT_DURATION_MS   10000   // Thời gian giữ cảnh báo: 10 giây
#define DEBOUNCE_DELAY_MS   15000   // Thời gian chống spam tin nhắn: 15 giây
#define WIFI_TIMEOUT_MS     30000   // Thời gian chờ kết nối Wi-Fi tối đa: 30 giây


// ============================================================
// PHẦN 7: BIẾN TRẠNG THÁI HỆ THỐNG (Non-blocking)
// ============================================================
// Trạng thái cảnh báo hiện tại
bool isAlerting = false;

// Thời điểm bắt đầu cảnh báo (dùng millis())
unsigned long alertStartTime = 0;

// Thời điểm gửi tin nhắn cuối cùng (chống spam)
unsigned long lastMessageTime = 0;

// Biến lưu trạng thái PIR chu kỳ trước (để phát hiện cạnh lên)
bool lastPirState = LOW;


// ============================================================
// PHẦN 8: KHỞI TẠO ĐỐI TƯỢNG
// ============================================================
// Client bảo mật SSL cho Telegram (cần để gọi API HTTPS)
WiFiClientSecure secureClient;

// Đối tượng Telegram Bot
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

// Đối tượng SMTP Session cho Email
SMTPSession smtp;


// ============================================================
// PHẦN 9: KHAI BÁO HÀM (Function Prototypes)
// ============================================================
void connectWifi();
void sendTelegramAlert();
void sendEmailAlert();
void activateAlert();
void deactivateAlert();
void printSystemStatus();


// ============================================================
// PHẦN 10: HÀM SETUP - Chạy một lần khi khởi động
// ============================================================
void setup() {
  // Khởi tạo Serial Monitor
  Serial.begin(115200);
  delay(500); // Chờ Serial ổn định
  Serial.println("\n\n============================================");
  Serial.println("  HE THONG CANH BAO TROM - Pham Thanh Hieu");
  Serial.println("============================================");
  Serial.println("[INIT] Dang khoi dong he thong...");

  // --- Cấu hình chân GPIO ---
  pinMode(PIN_PIR,    INPUT);    // PIR là đầu vào
  pinMode(PIN_LED,    OUTPUT);   // LED là đầu ra
  pinMode(PIN_BUZZER, OUTPUT);   // Buzzer là đầu ra

  // Đảm bảo LED và Buzzer tắt khi khởi động
  digitalWrite(PIN_LED,    LOW);
  digitalWrite(PIN_BUZZER, LOW);
  Serial.println("[INIT] GPIO da duoc cau hinh: PIR=13(IN), LED=26(OUT), Buzzer=27(OUT)");

  // --- Kết nối Wi-Fi ---
  connectWifi();

  // --- Cấu hình SSL cho Telegram ---
  // Quan trọng: Bỏ qua xác thực SSL certificate để đơn giản hóa
  // (Trong production nên dùng certificate thực)
  secureClient.setInsecure();
  Serial.println("[INIT] SSL Client da duoc cau hinh (insecure mode)");

  // --- Gửi thông báo hệ thống đã khởi động ---
  Serial.println("[INIT] Dang gui thong bao khoi dong len Telegram...");
  String startMsg = "✅ *HỆ THỐNG ĐÃ KHỞI ĐỘNG*\n";
  startMsg += "📡 Wi-Fi: Đã kết nối\n";
  startMsg += "🔍 Cảm biến PIR: Sẵn sàng\n";
  startMsg += "🛡️ Trạng thái: Đang giám sát...";
  bot.sendMessage(CHAT_ID, startMsg, "Markdown");

  Serial.println("[INIT] He thong san sang! Dang giam sat chuyển động...");
  Serial.println("============================================\n");
}


// ============================================================
// PHẦN 11: HÀM LOOP - Chạy lặp liên tục
// ============================================================
void loop() {
  // Đọc trạng thái hiện tại của cảm biến PIR
  bool currentPirState = digitalRead(PIN_PIR);
  unsigned long currentTime = millis();

  // -------------------------------------------------------
  // TRƯỜNG HỢP 1: PIR vừa phát hiện chuyển động
  // (Phát hiện cạnh lên: LOW -> HIGH)
  // -------------------------------------------------------
  if (currentPirState == HIGH && lastPirState == LOW) {
    Serial.println("\n[PIR] !!! PHAT HIEN CHUYEN DONG !!!");

    // Kiểm tra thời gian chống spam (debounce)
    bool canSendMessage = (currentTime - lastMessageTime) >= DEBOUNCE_DELAY_MS;

    if (!isAlerting) {
      // Kích hoạt cảnh báo tại chỗ (LED + Buzzer)
      activateAlert();
      alertStartTime = currentTime;
    }

    // Gửi thông báo nếu chưa gửi gần đây (chống spam)
    if (canSendMessage) {
      Serial.println("[ALERT] Dieu kien gui tin: OK. Dang gui canh bao...");
      sendTelegramAlert();

      if (EMAIL_ENABLED) {
        sendEmailAlert();
      }

      lastMessageTime = currentTime;
    } else {
      unsigned long remainingCooldown = (DEBOUNCE_DELAY_MS - (currentTime - lastMessageTime)) / 1000;
      Serial.printf("[ALERT] Che do chong spam: Con %lu giay cho den lan gui tiep theo\n", remainingCooldown);
    }
  }

  // -------------------------------------------------------
  // TRƯỜNG HỢP 2: Đang trong trạng thái cảnh báo
  // Kiểm tra xem đã hết 10 giây chưa
  // -------------------------------------------------------
  if (isAlerting) {
    if (currentTime - alertStartTime >= ALERT_DURATION_MS) {
      Serial.println("[ALERT] Het thoi gian canh bao (10 giay). Dang tat canh bao...");
      deactivateAlert();
    }
  }

  // -------------------------------------------------------
  // TRƯỜNG HỢP 3: PIR không còn phát hiện chuyển động
  // -------------------------------------------------------
  if (currentPirState == LOW && lastPirState == HIGH) {
    Serial.println("[PIR] Khong con phat hien chuyen dong.");
  }

  // Cập nhật trạng thái PIR cho chu kỳ tiếp theo
  lastPirState = currentPirState;

  // In trạng thái định kỳ mỗi 5 giây (tuỳ chọn - debug)
  // printSystemStatus();

  // Delay ngắn để tránh CPU quá tải (non-blocking - vẫn xử lý được)
  delay(100);
}


// ============================================================
// PHẦN 12: CÁC HÀM CHỨC NĂNG
// ============================================================

/**
 * connectWifi()
 * Kết nối ESP32 với mạng Wi-Fi và chờ đến khi thành công
 * hoặc hết thời gian timeout.
 */
void connectWifi() {
  Serial.printf("[WIFI] Dang ket noi toi mang: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  int dotCount = 0;

  // Chờ kết nối thành công
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttempt >= WIFI_TIMEOUT_MS) {
      Serial.println("\n[WIFI] !!! KET NOI THAT BAI - HET TIMEOUT !!!");
      Serial.println("[WIFI] Vui long kiem tra SSID va Password, roi reset lai ESP32.");
      // Trong production: có thể deep sleep hoặc restart ở đây
      while (true) { delay(1000); } // Dừng hệ thống
    }

    delay(500);
    Serial.print(".");
    dotCount++;
    if (dotCount >= 40) {
      Serial.println();
      dotCount = 0;
    }
  }

  // Kết nối thành công
  Serial.println("\n[WIFI] Ket noi thanh cong!");
  Serial.printf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WIFI] Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
}


/**
 * sendTelegramAlert()
 * Gửi tin nhắn cảnh báo đến Telegram Bot.
 * Sử dụng thư viện UniversalTelegramBot.
 */
void sendTelegramAlert() {
  Serial.println("[TELEGRAM] Dang gui tin nhan canh bao...");

  // Nội dung tin nhắn cảnh báo
  String message = "🚨 *CẢNH BÁO KHẨN CẤP!*\n\n";
  message += "⚠️ *PHÁT HIỆN CÓ NGƯỜI ĐỘT NHẬP!*\n\n";
  message += "📍 Vị trí: Khu vực được giám sát\n";
  message += "🕐 Thời gian: Vừa phát hiện\n";
  message += "📡 Cảm biến: PIR HC-SR501 (GPIO 13)\n\n";
  message += "🔔 LED và Buzzer đang kích hoạt!\n";
  message += "_Hệ thống của Phạm Thanh Hiếu_";

  // Gửi tin nhắn với định dạng Markdown
  bool sent = bot.sendMessage(CHAT_ID, message, "Markdown");

  if (sent) {
    Serial.println("[TELEGRAM] Gui tin nhan THANH CONG!");
  } else {
    Serial.println("[TELEGRAM] Gui tin nhan THAT BAI! Kiem tra Token va Chat ID.");
  }
}


/**
 * sendEmailAlert()
 * Gửi email cảnh báo qua giao thức SMTP.
 * Sử dụng thư viện ESP Mail Client.
 */
void sendEmailAlert() {
  Serial.println("[EMAIL] Dang chuan bi gui email canh bao...");

  // --- Cấu hình Session SMTP ---
  Session_Config config;
  config.server.host_name  = SMTP_HOST;
  config.server.port       = SMTP_PORT;
  config.login.email       = SENDER_EMAIL;
  config.login.password    = SENDER_PASSWORD;
  config.login.user_domain = "";

  // --- Tạo nội dung Email ---
  SMTP_Message message;
  message.sender.name  = SENDER_NAME;
  message.sender.email = SENDER_EMAIL;
  message.subject      = "🚨 CANH BAO TROM - Phat hien chuyen dong!";
  message.addRecipient("Chu nha", RECIPIENT_EMAIL);

  // Nội dung HTML của email
  String htmlBody = R"(
  <html>
  <body style="font-family: Arial, sans-serif; background: #f4f4f4; padding: 20px;">
    <div style="max-width:600px; margin:auto; background:#fff; border-radius:10px;
                padding:30px; border-left: 5px solid #ff0000;">
      <h1 style="color:#ff0000; text-align:center;">
        🚨 CẢNH BÁO TRỘM!
      </h1>
      <hr>
      <h2>⚠️ PHÁT HIỆN CÓ NGƯỜI ĐỘT NHẬP!</h2>
      <p><b>📡 Cảm biến:</b> PIR HC-SR501 (GPIO 13)</p>
      <p><b>📍 Khu vực:</b> Vùng được giám sát</p>
      <p><b>🔔 Trạng thái:</b> LED và Buzzer đang kích hoạt</p>
      <hr>
      <p style="color:#888; font-size:12px;">
        Email tự động từ Hệ thống Cảnh báo Trộm<br>
        Tác giả: Phạm Thanh Hiếu | ESP32 + PIR Sensor
      </p>
    </div>
  </body>
  </html>
  )";
  message.html.content = htmlBody;
  message.html.charSet = "utf-8";
  message.text.charSet = "utf-8";

  // --- Kết nối và gửi Email ---
  if (!smtp.connect(&config)) {
    Serial.printf("[EMAIL] Ket noi SMTP that bai: %s\n", smtp.errorReason().c_str());
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.printf("[EMAIL] Gui email that bai: %s\n", smtp.errorReason().c_str());
  } else {
    Serial.println("[EMAIL] Gui email THANH CONG!");
  }

  smtp.closeSession();
}


/**
 * activateAlert()
 * Bật đèn LED đỏ và còi Buzzer để cảnh báo tại chỗ.
 */
void activateAlert() {
  isAlerting = true;
  digitalWrite(PIN_LED,    HIGH);   // Bật LED đỏ
  digitalWrite(PIN_BUZZER, HIGH);   // Bật Buzzer
  Serial.println("[ALERT] >>> BAT LED DO + BUZZER (Canh bao 10 giay) <<<");
}


/**
 * deactivateAlert()
 * Tắt đèn LED đỏ và còi Buzzer sau 10 giây.
 */
void deactivateAlert() {
  isAlerting = false;
  digitalWrite(PIN_LED,    LOW);    // Tắt LED đỏ
  digitalWrite(PIN_BUZZER, LOW);    // Tắt Buzzer
  Serial.println("[ALERT] >>> TAT LED + BUZZER. Ve trang thai cho <<<\n");
}


/**
 * printSystemStatus()
 * In trạng thái hệ thống ra Serial Monitor (hàm debug tuỳ chọn).
 */
void printSystemStatus() {
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 5000) {
    Serial.println("--- [STATUS] ---");
    Serial.printf("  WiFi: %s | IP: %s\n",
                  WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED",
                  WiFi.localIP().toString().c_str());
    Serial.printf("  PIR State: %s\n", lastPirState ? "DETECTED" : "CLEAR");
    Serial.printf("  Alerting: %s\n", isAlerting ? "YES" : "NO");
    Serial.printf("  Uptime: %lu giay\n", millis() / 1000);
    Serial.println("----------------");
    lastPrintTime = millis();
  }
}