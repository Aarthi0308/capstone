
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include "secrets.h"

const int DHT_PIN = 15;
const unsigned long READ_INTERVAL = 5000;
const unsigned long WIFI_RETRY = 5000;
const unsigned long MQTT_RETRY = 3000;
const unsigned long SEND_INTERVAL = 250;

const int BUFFER_SIZE = 20;
String queueData[BUFFER_SIZE];
int head = 0;
int tail = 0;
int count = 0;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
DHTesp dht;

unsigned long lastRead = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;
unsigned long lastSend = 0;

void enqueue(String message) {
  if (count == BUFFER_SIZE) {
    head = (head + 1) % BUFFER_SIZE;
    count--;
    Serial.println("Queue full: oldest reading discarded");
  }

  queueData[tail] = message;
  tail = (tail + 1) % BUFFER_SIZE;
  count++;
}

void maintainWiFi(unsigned long now) {
  if (WiFi.status() == WL_CONNECTED) return;

  if (now - lastWiFiAttempt >= WIFI_RETRY) {
    lastWiFiAttempt = now;
    Serial.println("Attempting Wi-Fi connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void maintainMQTT(unsigned long now) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;

  if (now - lastMQTTAttempt >= MQTT_RETRY) {
    lastMQTTAttempt = now;

    String clientId = "ESP32-" +
      String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.println("Attempting MQTT connection...");

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT connection failed: ");
      Serial.println(mqtt.state());
    }
  }
}

void readSensor(unsigned long now) {
  if (now - lastRead < READ_INTERVAL) return;
  lastRead = now;

  TempAndHumidity reading = dht.getTempAndHumidity();

  if (isnan(reading.temperature) || isnan(reading.humidity)) {
    Serial.println("Sensor reading failed");
    return;
  }

  String message = "{\"temperature\":" +
      String(reading.temperature, 1) +
      ",\"humidity\":" +
      String(reading.humidity, 1) +
      ",\"uptime_ms\":" +
      String(now) + "}";

  Serial.println("Reading: " + message);
  enqueue(message);
}

void sendQueued(unsigned long now) {
  if (!mqtt.connected() || count == 0) return;
  if (now - lastSend < SEND_INTERVAL) return;

  lastSend = now;

  if (mqtt.publish(MQTT_TOPIC, queueData[head].c_str())) {
    Serial.println("Published: " + queueData[head]);
    head = (head + 1) % BUFFER_SIZE;
    count--;
  } else {
    Serial.println("Publish failed; reading remains queued");
  }
}

void setup() {
  Serial.begin(115200);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(256);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("IoT monitoring device started");
}

void loop() {
  unsigned long now = millis();

  maintainWiFi(now);
  maintainMQTT(now);

  if (mqtt.connected()) {
    mqtt.loop();
  }

  readSensor(now);
  sendQueued(now);
}
