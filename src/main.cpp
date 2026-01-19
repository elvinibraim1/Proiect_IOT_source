#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <LiquidCrystal.h>

/* ================= CONFIG ================= */

#define WIFI_SSID     "DIGI-HT7A"
#define WIFI_PASSWORD "sxFxK9gVk4"

#define MQTT_HOST     "test.mosquitto.org"
#define MQTT_PORT     1883
#define BASE_TOPIC    "devices/esp32"

#define PIR_PIN            16
#define REARM_DELAY_MS     5000

/* ================= LCD PINOUT (paralel 4-bit) ================= */
#define LCD_RS 23
#define LCD_E  18
#define LCD_D4 26
#define LCD_D5 17
#define LCD_D6 25
#define LCD_D7 5

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/* ================= GLOBALS ================= */

WiFiClient   net;
PubSubClient mqtt(net);
Preferences  prefs;

unsigned long pirLockedUntil = 0;
unsigned long lastHeartbeat = 0;

int lastPirState = LOW;
unsigned long pirCounter = 0;

/* ================= WIFI ================= */

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi OK, IP: %s\n", WiFi.localIP().toString().c_str());
}

/* ================= MQTT ================= */

void connectMQTT() {
  String clientId = "esp32-pir-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  String willTopic = String(BASE_TOPIC) + "/status";

  Serial.print("MQTT connecting");
  while (!mqtt.connect(clientId.c_str(),
                       willTopic.c_str(),
                       0, true, "offline")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT connected");
  mqtt.publish(willTopic.c_str(), "online", true);
}

/* ================= LCD ================= */

void lcdShowCounter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Counter:");
  lcd.setCursor(0, 1);
  lcd.print(pirCounter);
}

/* ================= FLASH ================= */

void saveCounterToFlash() {
  prefs.putULong("counter", pirCounter);
}

/* ================= PIR ================= */

void publishPir(int state) {
  String topic = String(BASE_TOPIC) + "/pir/raw";
  String payload = String(state) + ",count=" + String(pirCounter);
  mqtt.publish(topic.c_str(), payload.c_str(), true);
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIR_PIN, INPUT);

  /* ---------- FLASH ---------- */
  prefs.begin("pir", false);

  // Reset counter la fiecare restart (cerinta ta)
  pirCounter = 0;
  prefs.putULong("counter", pirCounter);

  /* ---------- LCD ---------- */
  lcd.begin(16, 2);      // initializare LCD 16x2
  lcdShowCounter();      // afiseaza 0

  /* ---------- WiFi + MQTT ---------- */
  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  connectMQTT();

  Serial.println("Sistem pornit. Counter resetat la 0.");
}

/* ================= LOOP ================= */

void loop() {
  connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();

  /* ---------- PIR LOGIC ---------- */
  int pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH && lastPirState == LOW) {
    if (now >= pirLockedUntil) {
      pirCounter++;
      pirLockedUntil = now + REARM_DELAY_MS;

      saveCounterToFlash();
      publishPir(pirState);
      lcdShowCounter();

      Serial.printf("PIR activat. Counter=%lu\n", pirCounter);
    }
  }

  lastPirState = pirState;

  /* ---------- HEARTBEAT ---------- */
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    String topic = String(BASE_TOPIC) + "/heartbeat";
    String payload = String("{\"uptime_ms\":") + now + "}";
    mqtt.publish(topic.c_str(), payload.c_str());
  }

  delay(10);
}
