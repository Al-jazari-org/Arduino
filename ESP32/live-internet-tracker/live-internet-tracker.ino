#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============ ⚙️ إعدادات الشاشة (لا تغيّرها عادة) ============
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ============ ⚙️ الإعدادات (بدّلها هنا فقط) ============

const char* ssid     = "wokwi-GUEST";
const char* password = "";

// 🔢 رقم التجربة: بدّل الرقم باش تختار وش تحب تشوف
// 1 = الطقس | 2 = عداد المشتركين | 3 = رواد الفضاء
int experimentNumber = 3;

// إحداثيات المدينة (للتجربة رقم 1)
float latitude  = 36.75;
float longitude = 3.06;
String cityName = "Algiers";   // بالإنجليزية أفضل لأن الشاشة لا تدعم الحروف العربية

// عداد المشتركين (للتجربة رقم 2)
long subscriberCount = 100;
long increaseBy = 5;

const unsigned long updateInterval = 10000;

// ============ لا تحتاج تغيير أي شيء تحت هذا السطر ============

unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("❌ فشل تشغيل شاشة OLED");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  showMessage("Connecting WiFi...");

  connectWiFi();
}

void loop() {
  if (millis() - lastUpdate > updateInterval) {
    runExperiment();
    lastUpdate = millis();
  }
}

void connectWiFi() {
  Serial.print("جاري الاتصال بالواي فاي");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ تم الاتصال بالواي فاي!");
  showMessage("WiFi Connected!");
  delay(1000);
}

// دالة تساعد على طباعة رسالة بسيطة على الشاشة
void showMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}

void runExperiment() {
  switch (experimentNumber) {
    case 1: getWeather(); break;
    case 2: updateSubscribers(); break;
    case 3: getAstronautsInSpace(); break;
    default: showMessage("Invalid experiment number");
  }
}

// التجربة 1: الطقس
void getWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 2) +
               "&longitude=" + String(longitude, 2) + "&current_weather=true";
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    float temp = doc["current_weather"]["temperature"];

    Serial.println("🌍 Weather in " + cityName + ": " + String(temp) + " C");

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Weather - " + cityName);
    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(temp);
    display.println(" C");
    display.display();
  } else {
    showMessage("Weather error!");
  }
  http.end();
}

// التجربة 2: عداد المشتركين
void updateSubscribers() {
  subscriberCount += increaseBy;
  Serial.println("📺 Subscribers: " + String(subscriberCount));

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Subscribers Counter");
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print(subscriberCount);
  display.display();
}

// التجربة 3: عدد رواد الفضاء الحاليين
void getAstronautsInSpace() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://api.open-notify.org/astros.json");
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);
    int number = doc["number"];

Serial.println("🚀 Astronauts in space: " + String(number));
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Astronauts in Space");
    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(number);
    display.display();
  } else {
    showMessage("Astronauts error!");
  }
  http.end();
}
