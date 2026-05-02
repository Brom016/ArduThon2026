/*
 * SmartRoom IoT — ESP32 Firmware (Wokwi)
 * ESP32 → Firebase RTDB → Web Dashboard
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin configuration
#define DHT_PIN      4
#define DHT_TYPE     DHT22
#define LDR_PIN      34
#define POT_PIN      35
#define BUZZER_PIN   26
#define LED_RED      27
#define LED_GREEN    25
#define LED_FAN      14

// OLED
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// Sensor
DHT dht(DHT_PIN, DHT_TYPE);

// WiFi (Wokwi default)
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// Firebase endpoint
const char* FIREBASE_URL =
  "https://smartroom-982da-default-rtdb.asia-southeast1.firebasedatabase.app/smartroom.json";

// Data structures
struct SensorData {
  float temp;
  float humidity;
  int   lightLux;
  int   co2PPM;
};

struct ScoreData {
  int    temp;
  int    humidity;
  int    light;
  int    co2;
  int    total;
  String status;
  String color;
};

SensorData sensors = {28.0, 60.0, 400, 450};
ScoreData  scores  = {0, 0, 0, 0, 0, "Loading", "green"};

// Timing
unsigned long lastSensorRead   = 0;
unsigned long lastBuzzerToggle = 0;
bool buzzerState = false;
int  buzzerMode  = 0; // 0=off, 1=intermittent, 2=continuous

// --- Scoring ---
int scoreTemperature(float t) {
  if (t >= 18 && t <= 26) return 25;
  if (t <= 30) return 15;
  if (t <= 35) return 5;
  return 0;
}

int scoreHumidity(float h) {
  if (h >= 40 && h <= 60) return 25;
  if (h <= 70) return 15;
  if (h <= 80) return 5;
  return 0;
}

int scoreLight(int lux) {
  if (lux >= 300 && lux <= 700) return 25;
  if (lux <= 900) return 15;
  if (lux >= 100) return 10;
  return 0;
}

int scoreCO2(int ppm) {
  if (ppm < 400) return 25;
  if (ppm < 600) return 15;
  if (ppm < 800) return 5;
  return 0;
}

void determineStatus(int total, ScoreData& s) {
  if      (total >= 75) { s.status = "COMFORTABLE";   s.color = "green";  }
  else if (total >= 50) { s.status = "ACCEPTABLE";    s.color = "yellow"; }
  else if (total >= 25) { s.status = "UNCOMFORTABLE"; s.color = "orange"; }
  else                  { s.status = "CRITICAL";      s.color = "red";    }
}

// --- Actuator control ---
void controlActuators(ScoreData& s) {
  if (s.total >= 75) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_FAN, LOW);
    buzzerMode = 0;

  } else if (s.total >= 50) {
    digitalWrite(LED_FAN, HIGH);
    buzzerMode = 0;

  } else if (s.total >= 25) {
    digitalWrite(LED_FAN, HIGH);
    buzzerMode = 1;

  } else {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_FAN, HIGH);
    buzzerMode = 2;
  }
}

void updateBuzzer() {
  unsigned long now = millis();

  if (buzzerMode == 0) {
    noTone(BUZZER_PIN);
    buzzerState = false;

  } else if (buzzerMode == 1) {
    if (!buzzerState && now - lastBuzzerToggle > 3000) {
      tone(BUZZER_PIN, 1000);
      buzzerState = true;
      lastBuzzerToggle = now;
    } else if (buzzerState && now - lastBuzzerToggle > 300) {
      noTone(BUZZER_PIN);
      buzzerState = false;
      lastBuzzerToggle = now;
    }

  } else if (buzzerMode == 2) {
    if (!buzzerState) {
      tone(BUZZER_PIN, 2000);
      buzzerState = true;
    }
  }
}

// --- OLED display ---
void updateOLED() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  oled.setCursor(0, 0);
  oled.print("Score:");
  oled.print(scores.total);

  oled.setCursor(0, 12);
  oled.printf("T:%.1f H:%d", sensors.temp, (int)sensors.humidity);

  oled.setCursor(0, 22);
  oled.printf("L:%d CO2:%d", sensors.lightLux, sensors.co2PPM);

  oled.setTextSize(2);
  oled.setCursor(0, 46);
  oled.print(scores.status.substring(0, 8));

  oled.display();
}

// --- Firebase ---
void pushToFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, FIREBASE_URL)) return;

  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"temperature\":"  + String(sensors.temp, 1)     + ",";
  json += "\"humidity\":"     + String(sensors.humidity, 1) + ",";
  json += "\"lightLux\":"     + String(sensors.lightLux)    + ",";
  json += "\"co2PPM\":"       + String(sensors.co2PPM)      + ",";
  json += "\"totalScore\":"   + String(scores.total)        + ",";
  json += "\"status\":\""     + scores.status               + "\"";
  json += "}";

  http.PUT(json);
  http.end();
}

// --- WiFi ---
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_FAN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(LDR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

  dht.begin();
  Wire.begin(21, 22);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  connectWiFi();
}

// --- Main loop ---
void loop() {
  updateBuzzer();

  if (millis() - lastSensorRead >= 2000) {
    lastSensorRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) sensors.temp = t;
    if (!isnan(h)) sensors.humidity = h;

    sensors.lightLux = map(analogRead(LDR_PIN), 0, 4095, 0, 1000);
    sensors.co2PPM   = map(analogRead(POT_PIN), 0, 4095, 200, 1200);

    scores.temp     = scoreTemperature(sensors.temp);
    scores.humidity = scoreHumidity(sensors.humidity);
    scores.light    = scoreLight(sensors.lightLux);
    scores.co2      = scoreCO2(sensors.co2PPM);
    scores.total    = scores.temp + scores.humidity + scores.light + scores.co2;

    determineStatus(scores.total, scores);
    controlActuators(scores);

    updateOLED();
    pushToFirebase();
  }
}