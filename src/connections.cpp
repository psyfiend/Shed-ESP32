#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "connections.h"

// Define the global client object
WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_discovery() {
  StaticJsonDocument<512> discovery_doc;
  const char* discovery_topic = MQTT_DISCOVERY_TOPIC;

  JsonObject device_doc = discovery_doc.createNestedObject("device");
  device_doc["name"] = "Shed ESP32";
  device_doc["identifiers"] = DEVICE_ID;
  device_doc["manufacturer"] = "Seeed Studio";
  device_doc["model"] = "XIAO ESP32-C6";
  device_doc["suggested_area"] = "Shed";

  JsonObject origin_doc = discovery_doc.createNestedObject("o");
  origin_doc["name"] = "Shed PIR Sensor (o)";
  origin_doc["sw"] = "0.1";
  origin_doc["url"] = "https://switz.org";

  JsonObject cmps_doc = discovery_doc.createNestedObject("cmps");

  JsonObject motion_cmp = cmps_doc.createNestedObject("shed_pir_01_motion");
  motion_cmp["name"] = "Motion Status";
  motion_cmp["state_topic"] = MQTT_TOPIC_STATE;
  motion_cmp["platform"] = "binary_sensor";
  motion_cmp["device_class"] = "motion";
  motion_cmp["payload_on"] = "on";
  motion_cmp["payload_off"] = "off";
  motion_cmp["unique_id"] = "shed_pir_01_motion";

  JsonObject triggers_cmp = cmps_doc.createNestedObject("shed_pir_01_triggers");
  triggers_cmp["name"] = "Trigger Count";
  triggers_cmp["state_topic"] = MQTT_TOPIC_TRIGGERS;
  triggers_cmp["platform"] = "sensor";
  triggers_cmp["unique_id"] = "shed_pir_01_triggers";

  discovery_doc["state_topic"] = "shed/motion";

  Serial.println("--- Single Discovery Payload ---");
  serializeJsonPretty(discovery_doc, Serial);
  Serial.println();
  Serial.print("Publishing to topic: ");
  Serial.println(discovery_topic);
  
  char buffer[512];
  serializeJson(discovery_doc, buffer);
  client.publish(discovery_topic, buffer, true);
  Serial.println("------------------------------");
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) { // Try for 10 seconds
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed.");
  }
}

void reconnect() {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      mqtt_discovery();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
}

