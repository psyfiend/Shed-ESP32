#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "connections.h"
#include "config.h" 

extern PubSubClient client;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_discovery() {
  StaticJsonDocument<1024> discovery_doc;
  const char* discovery_topic = "homeassistant/device/shed_esp32_c6_01/config";

  JsonObject device_doc = discovery_doc.createNestedObject("device");
  device_doc["name"] = "Shed Monitor System";
  device_doc["identifiers"] = DEVICE_ID;    // shed_esp32_c6_01
  device_doc["manufacturer"] = "Seeed Studio";
  device_doc["model"] = "XIAO ESP32-C6";
  device_doc["suggested_area"] = "Shed";

  JsonObject origin_doc = discovery_doc.createNestedObject("o");
  origin_doc["name"] = "Shed Monitor (o)";
  origin_doc["sw"] = "0.1";
  origin_doc["url"] = "https://switz.org";

  JsonObject cmps_doc = discovery_doc.createNestedObject("cmps");

  // Software-based Occupancy Sensor (timer-based)
  JsonObject occupancy_cmp = cmps_doc.createNestedObject("shed_monitor_occupancy");
  occupancy_cmp["name"] = "Occupancy";
  occupancy_cmp["state_topic"] = MQTT_TOPIC_OCCUPANCY_STATUS;
  occupancy_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  occupancy_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  occupancy_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  occupancy_cmp["platform"] = "binary_sensor";
  occupancy_cmp["device_class"] = "occupancy";
  occupancy_cmp["payload_on"] = "on";
  occupancy_cmp["payload_off"] = "off";
  occupancy_cmp["unique_id"] = "shed_esp32_pir_occupancy";

  // Physical PIR Motion Sensor
  JsonObject motion_cmp = cmps_doc.createNestedObject("shed_monitor_motion");
  motion_cmp["name"] = "Motion";
  motion_cmp["state_topic"] = MQTT_TOPIC_MOTION_STATUS;
  motion_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  motion_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  motion_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  motion_cmp["platform"] = "binary_sensor";
  motion_cmp["device_class"] = "motion";
  motion_cmp["payload_on"] = "on";
  motion_cmp["payload_off"] = "off";
  motion_cmp["unique_id"] = "shed_esp32_pir_motion";

  // --- REMOVED THE TRIGGERS COMPONENT ---
  // JsonObject triggers_cmp = cmps_doc.createNestedObject("shed_pir_01_triggers");
  // ...

  discovery_doc["state_topic"] = "shed/monitor";

  Serial.println("--- Single Discovery Payload ---");
  serializeJsonPretty(discovery_doc, Serial);
  Serial.println();
  Serial.print("Publishing to topic: ");
  Serial.println(discovery_topic);
  
  char buffer[1024];
  serializeJson(discovery_doc, buffer);
  client.publish(discovery_topic, buffer, true);
  Serial.println("------------------------------");
}

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    Serial.println("connected");
    client.publish(MQTT_TOPIC_AVAILABILITY, MQTT_PAYLOAD_ONLINE, true);
    mqtt_discovery();
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

