#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>

// ── Cấu hình Wi-Fi ──────────────────────────────────────
#define WIFI_SSID  "Wokwi-GUEST"
#define WIFI_PASS  ""

// ── OLED ─────────────────────────────────────────────────
#define SCREEN_W  128
#define SCREEN_H   64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ── NTP ───────────────────────────────────────────────────
#define NTP_SERVER   "pool.ntp.org"
#define GMT_OFFSET   25200   // UTC+7 (Ho Chi Minh City)
#define DST_OFFSET   0

// ── Lịch thông báo ───────────────────────────────────────
// Thêm/xoá giờ phút tuỳ ý (HH, MM)
struct Schedule { int hour; int minute; const char* msg; };
Schedule schedules[] = {
  {  7,  0, "Chao buoi sang!" },
  { 12,  0, "Den gio an trua" },
  { 18,  0, "Tan ca roi!"     },
  { 22,  0, "Di ngu thoi :)"  },
};
const int NUM_SCHED = sizeof(schedules) / sizeof(schedules[0]);
int lastNotifMinute = -1;   // tránh thông báo trùng

// ── Tên thứ & tháng ──────────────────────────────────────
const char* DOW[] = {"CN","T2","T3","T4","T5","T6","T7"};
const char* MON[] = {"","Jan","Feb","Mar","Apr","May","Jun",
                      "Jul","Aug","Sep","Oct","Nov","Dec"};

// ─────────────────────────────────────────────────────────
void connectWiFi() {
  Serial.printf("Ket noi Wi-Fi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("Connecting WiFi...");
  display.display();

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 40) {
    delay(500); Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? "\nWiFi OK!" : "\nWiFi FAIL");
}

void syncNTP() {
  Serial.println("Dong bo NTP...");
  configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);

  struct tm t;
  int tries = 0;
  while (!getLocalTime(&t) && tries++ < 20) delay(500);

  if (tries < 20)
    Serial.printf("Thoi gian: %02d:%02d:%02d %02d/%02d/%04d\n",
      t.tm_hour, t.tm_min, t.tm_sec,
      t.tm_mday, t.tm_mon+1, t.tm_year+1900);
  else
    Serial.println("NTP that bai!");
}

// ─────────────────────────────────────────────────────────
void drawClock(struct tm &t) {
  display.clearDisplay();

  // ── Đồng hồ lớn (HH:MM:SS) ───────────────────────────
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  char timeBuf[9];
  sprintf(timeBuf, "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  int x = (SCREEN_W - strlen(timeBuf) * 12) / 2;
  display.setCursor(x, 2);
  display.print(timeBuf);

  // ── Đường kẻ ngang ────────────────────────────────────
  display.drawFastHLine(0, 22, SCREEN_W, SSD1306_WHITE);

  // ── Ngày / Thứ ────────────────────────────────────────
  display.setTextSize(1);
  char dateBuf[20];
  sprintf(dateBuf, "%s %02d/%02d/%04d",
    DOW[t.tm_wday],
    t.tm_mday, t.tm_mon+1, t.tm_year+1900);
  x = (SCREEN_W - strlen(dateBuf) * 6) / 2;
  display.setCursor(x, 26);
  display.print(dateBuf);

  // ── Thanh tiến trình giây ─────────────────────────────
  display.drawRect(0, 37, SCREEN_W, 6, SSD1306_WHITE);
  int barW = map(t.tm_sec, 0, 59, 0, SCREEN_W - 2);
  display.fillRect(1, 38, barW, 4, SSD1306_WHITE);

  // ── Trạng thái kết nối ────────────────────────────────
  display.setCursor(0, 48);
  display.print(WiFi.status() == WL_CONNECTED ? "WiFi: OK" : "WiFi: --");

  // ── Sync interval (mỗi giờ) ───────────────────────────
  display.setCursor(70, 48);
  display.print("NTP:OK");

  display.setCursor(0, 57);
  display.printf("UTC+7 | %s", NTP_SERVER);

  display.display();
}

void checkSchedule(struct tm &t) {
  int nowMin = t.tm_hour * 60 + t.tm_min;
  if (nowMin == lastNotifMinute) return;   // đã thông báo phút này rồi

  for (int i = 0; i < NUM_SCHED; i++) {
    if (t.tm_hour == schedules[i].hour && t.tm_min == schedules[i].minute) {
      lastNotifMinute = nowMin;
      Serial.printf("[THONG BAO %02d:%02d] %s\n",
        schedules[i].hour, schedules[i].minute, schedules[i].msg);

      // Hiển thị thông báo trên OLED 3 giây
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(">> THONG BAO <<");
      display.drawFastHLine(0, 10, SCREEN_W, SSD1306_WHITE);
      display.setCursor(0, 18);
      display.setTextSize(1);
      display.print(schedules[i].msg);
      display.setCursor(0, 40);
      display.printf("%02d:%02d", t.tm_hour, t.tm_min);
      display.display();
      delay(3000);
      break;
    }
  }
}

// ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA=21, SCL=22

  // Khởi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED khong tim thay!");
    while (true);
  }
  display.clearDisplay();
  display.display();

  connectWiFi();
  syncNTP();
}

unsigned long lastSync = 0;
const unsigned long SYNC_INTERVAL = 3600000UL;  // 1 tiếng

void loop() {
  struct tm t;
  if (!getLocalTime(&t)) { delay(1000); return; }

  drawClock(t);
  checkSchedule(t);

  // Re-sync NTP mỗi giờ
  if (millis() - lastSync > SYNC_INTERVAL) {
    syncNTP();
    lastSync = millis();
  }

  delay(1000);
}