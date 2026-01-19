#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <LiquidCrystal.h>
#include <time.h>

/* ================= MODE ================= */

//#define USER_MODE
#define TEST_MODE

/* ================= CONFIG ================= */

#define WIFI_SSID      "DIGI-HT7A"
#define WIFI_PASSWORD  "sxFxK9gVk4"

#define MQTT_HOST      "test.mosquitto.org"
#define MQTT_PORT      1883
#define BASE_TOPIC     "devices/esp32"

#define PIR_PIN        16

/* --------- NTP --------- */
#define NTP_SERVER     "pool.ntp.org"
#define GMT_OFFSET_SEC 7200
#define DAYLIGHT_SEC   3600

/* --------- LCD --------- */
#define LCD_RS 23
#define LCD_E  18
#define LCD_D4 26
#define LCD_D5 17
#define LCD_D6 25
#define LCD_D7 5

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/* ================= TIMING ================= */

#ifdef TEST_MODE
  #define REARM_DELAY_MS   6000     // 6 sec
#endif

#ifdef USER_MODE
  #define DOUBLE_TRIGGER_MS 30000    // max 30 sec intre 2 miscari
  #define LOCK_TIME_MS     300000    // 5 minute
#endif

/* ================= GLOBALS ================= */

WiFiClient   net;
PubSubClient mqtt(net);
Preferences  prefs;

unsigned long visitCounter = 0;

/* ---- PIR logic ---- */
unsigned long firstTriggerMs = 0;
unsigned long lastTriggerMs  = 0;
bool waitingSecondTrigger    = false;
bool lockActive              = false;
unsigned long lockUntilMs    = 0;

/* ---- time ---- */
time_t visitStartTime;

/* ---- heartbeat ---- */
unsigned long lastHeartbeat = 0;

/* ================= WIFI ================= */

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

/* ================= MQTT ================= */

void connectMQTT() {
  String clientId = "esp32-litter-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  String willTopic = String(BASE_TOPIC) + "/status";

  while (!mqtt.connect(clientId.c_str(),
                       willTopic.c_str(),
                       0, true, "offline")) {
    delay(1000);
  }

  mqtt.publish(willTopic.c_str(), "online", true);
}

/* ================= TIME ================= */

String formatTime(time_t t) {
  struct tm ti;
  localtime_r(&t, &ti);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
  return String(buf);
}

/* ================= LCD ================= */

void lcdShowCounter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vizite:");
  lcd.setCursor(0, 1);
  lcd.print(visitCounter);
}

/* ================= MQTT PUBLISH ================= */

void publishVisit(unsigned long durationMs) {
  String topic = String(BASE_TOPIC) + "/visit";

  String payload = "{";
  payload += "\"start\":\"" + formatTime(visitStartTime) + "\",";
  payload += "\"duration_sec\":" + String(durationMs / 1000) + ",";
  payload += "\"total\":" + String(visitCounter);
  payload += "}";

  mqtt.publish(topic.c_str(), payload.c_str(), true);
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIR_PIN, INPUT);

  prefs.begin("litter", false);

  /* ---- RESET COUNTER LA 0 ---- */
  visitCounter = 0;
  prefs.putULong("visits", visitCounter);

  lcd.begin(16, 2);
  lcdShowCounter();

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  connectMQTT();

  configTime(GMT_OFFSET_SEC, DAYLIGHT_SEC, NTP_SERVER);

  Serial.println("Sistem pornit, counter resetat");
}

/* ================= LOOP ================= */

void loop() {
  connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  int pirState = digitalRead(PIR_PIN);

  /* --------- LOCK USER MODE --------- */
#ifdef USER_MODE
  if (lockActive && now >= lockUntilMs) {
    lockActive = false;
    waitingSecondTrigger = false;
    Serial.println("LOCK dezactivat");
  }
#endif

  if (pirState == HIGH) {

#ifdef TEST_MODE
    static unsigned long lastVisitMs = 0;

    if (now - lastVisitMs >= REARM_DELAY_MS) {
      time(&visitStartTime);
      visitCounter++;
      lastVisitMs = now;

      lcdShowCounter();
      publishVisit(0);

      Serial.println("TEST MODE: vizita inregistrata");
    }
#endif


#ifdef USER_MODE
    if (lockActive) return;

    if (!waitingSecondTrigger) {
      waitingSecondTrigger = true;
      firstTriggerMs = now;
      time(&visitStartTime);
      Serial.println("USER MODE: prima miscare");
    } else {
      if (now - firstTriggerMs <= DOUBLE_TRIGGER_MS) {
        unsigned long duration = now - firstTriggerMs;
        visitCounter++;

        lcdShowCounter();
        publishVisit(duration);

        lockActive = true;
        lockUntilMs = now + LOCK_TIME_MS;
        waitingSecondTrigger = false;

        Serial.println("USER MODE: vizita valida, LOCK 5 min");
      } else {
        firstTriggerMs = now;
        time(&visitStartTime);
        Serial.println("USER MODE: restart fereastra");
      }
    }
#endif
  }

 /* ---------- HEARTBEAT ---------- */
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    String topic = String(BASE_TOPIC) + "/heartbeat";
    String payload = String("{\"uptime_ms\":") + now + "}";
    mqtt.publish(topic.c_str(), payload.c_str());
  }

  delay(10);
}
