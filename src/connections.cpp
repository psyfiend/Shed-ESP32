#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Include our configuration file
// #include "config.h"
#include "connections.h"

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "M&M Motors";
const char* WIFI_PASSWORD = "seamosss";

// --- Device Identifiers ---
// These are used for MQTT Discovery and should be unique to your device
const char* device_id = "shed_esp32_c6_01";
const char* device_name = "Shed ESP32-C6";

// --- MQTT Broker Settings ---
const char* MQTT_SERVER = "192.168.0.70";
const char* MQTT_USER = "mqtt_user";
const char* MQTT_PASSWORD = "mqtt_user3700";
const char* MQTT_TOPIC_STATUS = "shed/motion/status";
const char* MQTT_TOPIC_TRIGGERS = "shed/motion/triggers";
const char* MQTT_TOPIC_DEVICE_DISCOVERY = "homeassistant/device/shed_esp32_c6_01/config";

// Initialize the MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Function prototypes
void mqtt_discovery();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

// Function to connect to Wi-Fi
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

// Function to reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect with a client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Publish discovery message on successful connection
      mqtt_discovery();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Function to handle incoming MQTT messages (not yet implemented)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // This is where you would handle incoming commands from Home Assistant
  // For example, setting a new relay timer or other device settings
}

// Function to publish MQTT discovery configuration
void mqtt_discovery() {
  // Use a JsonDocument with a larger capacity to hold all discovery info
  StaticJsonDocument<512> discovery_doc;
  const char* discovery_topic = "homeassistant/device/shed_esp32_c6_01/config";

  // Create the device object
  JsonObject device_doc = discovery_doc.createNestedObject("device");
  JsonArray identifiers = device_doc.createNestedArray("identifiers");
  identifiers.add(device_id);
  device_doc["name"] = "Shed ESP32-C6";
  device_doc["manufacturer"] = "Seeed Studio";
  device_doc["model"] = "XIAO ESP32-C6";
  device_doc["suggested_area"] = "Shed";

  // Create the optional origin object
  JsonObject origin_doc = discovery_doc.createNestedObject("o");
  origin_doc["name"] = "Shed PIR Sensor (o)";
  origin_doc["sw"] = "0.1";
  origin_doc["url"] = "https://switz.org";

  // Create the cmps object for all components
  JsonObject cmps_doc = discovery_doc.createNestedObject("cmps");

  // Binary sensor (motion) component
  JsonObject motion_cmp = cmps_doc.createNestedObject("shed_pir_01_motion");
  motion_cmp["name"] = "Motion Status";
  motion_cmp["state_topic"] = MQTT_TOPIC_STATUS;
  motion_cmp["platform"] = "binary_sensor";
  motion_cmp["device_class"] = "motion";
  motion_cmp["payload_on"] = "on";
  motion_cmp["payload_off"] = "off";
  motion_cmp["unique_id"] = "shed_pir_01_motion";

  // Sensor (trigger count) component
  JsonObject triggers_cmp = cmps_doc.createNestedObject("shed_pir_01_triggers");
  triggers_cmp["name"] = "Trigger Count";
  triggers_cmp["state_topic"] = MQTT_TOPIC_TRIGGERS;
  triggers_cmp["platform"] = "sensor";
  triggers_cmp["unique_id"] = "shed_pir_01_triggers";

  discovery_doc["state_topic"] = "shed/motion";

  // Debugging output
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
