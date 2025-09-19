#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "connections.h"
#include "config.h" 

extern PubSubClient client;

// These are defined in main.cpp, but our callback needs to control them.
extern bool lightManualOverride;
extern unsigned long lastMotionTime;
extern unsigned long MOTION_TIMER_DURATION;
extern unsigned long MANUAL_TIMER_DURATION;

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

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload to a printable string
  payload[length] = '\0'; // Add a null terminator
  String message = (char*)payload;

  Serial.println("--- MQTT Message Received ---");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);
  Serial.println("-----------------------------");

  // ---- Route messages based on topic ----
    if (String(topic) == MQTT_TOPIC_LIGHT_COMMAND) {
    handle_lights_command(message);
    } else if (String(topic) == MQTT_TOPIC_LIGHT_MOTION_TIMER_SET) {
    handle_motion_timer_command(message);
    } else if (String(topic) == MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET) {
    handle_manual_timer_command(message);
  }
}


void mqtt_discovery() {
  StaticJsonDocument<2048> discovery_doc;
  const char* discovery_topic = "homeassistant/device/shed_esp32_c6_01/config";

  JsonObject device_doc = discovery_doc.createNestedObject("device");
  device_doc["name"] = "Shed Monitor System";
  device_doc["identifiers"] = DEVICE_ID;
  device_doc["manufacturer"] = "Seeed Studio";
  device_doc["model"] = "XIAO ESP32-C6";
  device_doc["suggested_area"] = "Shed";
  
  JsonObject origin_doc = discovery_doc.createNestedObject("o");
  origin_doc["name"] = "Shed Monitor (o)";
  origin_doc["sw"] = "0.1";
  origin_doc["url"] = "https://switz.org";

  JsonObject cmps_doc = discovery_doc.createNestedObject("cmps");// JsonArray cmps_array = discovery_doc.createNestedArray("cmps");

  // Software-based Occupancy Sensor (timer-based)
  JsonObject occupancy_cmp = cmps_doc.createNestedObject("shed_monitor_occupancy");
  occupancy_cmp["name"] = "Shed Occupancy";
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
  motion_cmp["name"] = "Shed Motion";
  motion_cmp["state_topic"] = MQTT_TOPIC_MOTION_STATUS;
  motion_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  motion_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  motion_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  motion_cmp["platform"] = "binary_sensor";
  motion_cmp["device_class"] = "motion";
  motion_cmp["payload_on"] = "on";
  motion_cmp["payload_off"] = "off";
  motion_cmp["unique_id"] = "shed_esp32_pir_motion";

  // Light (relay) entity
  JsonObject light_cmp = cmps_doc.createNestedObject("shed_monitor_light");
  light_cmp["name"] = "Shed Light";
  light_cmp["platform"] = "light";
  light_cmp["state_topic"] = MQTT_TOPIC_LIGHT_STATE;
  light_cmp["command_topic"] = MQTT_TOPIC_LIGHT_COMMAND;
  light_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  light_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  light_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  light_cmp["unique_id"] = "shed_esp32_light";

  // Motion Timer (for the light)
  JsonObject motion_timer_cmp = cmps_doc.createNestedObject("shed_monitor_light_motion_timer");
  motion_timer_cmp["name"] = "Shed Light Motion Timer";
  motion_timer_cmp["platform"] = "number";
  motion_timer_cmp["state_topic"] = MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE;
  motion_timer_cmp["command_topic"] = MQTT_TOPIC_LIGHT_MOTION_TIMER_SET;
  motion_timer_cmp["min"] = 10;
  motion_timer_cmp["max"] = 3600;
  motion_timer_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  motion_timer_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  motion_timer_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  motion_timer_cmp["unit_of_measurement"] = "s";  // seconds
  motion_timer_cmp["unique_id"] = "shed_esp32_light_motion_timer";

  // Manual override Timer (for the light)
  JsonObject manual_timer_cmp = cmps_doc.createNestedObject("shed_monitor_light_override_timer");
  manual_timer_cmp["name"] = "Shed Light Override Timer";
  manual_timer_cmp["platform"] = "number";
  manual_timer_cmp["state_topic"] = MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE;
  manual_timer_cmp["command_topic"] = MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET;
  manual_timer_cmp["min"] = 10;
  manual_timer_cmp["max"] = 3600;
  manual_timer_cmp["availability_topic"] = MQTT_TOPIC_AVAILABILITY;
  manual_timer_cmp["payload_available"] = MQTT_PAYLOAD_ONLINE;
  manual_timer_cmp["payload_not_available"] = MQTT_PAYLOAD_OFFLINE;
  manual_timer_cmp["unit_of_measurement"] = "s";  // seconds
  manual_timer_cmp["unique_id"] = "shed_esp32_light_override_timer";
 

  Serial.println("--- Single Discovery Payload ---");
  serializeJsonPretty(discovery_doc, Serial);
  Serial.println();
  Serial.print("Publishing to topic: ");
  Serial.println(discovery_topic);
  
  char buffer[2048];
  serializeJson(discovery_doc, buffer);
  client.publish(discovery_topic, buffer, true);
  Serial.println("------------------------------");
}

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32-XIAOC6-ShedMonitor";

  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    Serial.println("connected");
    
    // Birth message and initial states
    client.publish(MQTT_TOPIC_AVAILABILITY, MQTT_PAYLOAD_ONLINE, true);
    
    // Publish the default timers
    String motion_payload = String(MOTION_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE, motion_payload.c_str(), true);
    String manual_payload = String(MANUAL_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE, manual_payload.c_str(), true);
    Serial.println("Published initial timer states.");

    // Subscribe to the command topics, apply retained values if broker is online
    client.subscribe(MQTT_TOPIC_LIGHT_COMMAND);
    client.subscribe(MQTT_TOPIC_LIGHT_MOTION_TIMER_SET);
    client.subscribe(MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET);
    Serial.print("Subscribed to: ");
    Serial.println();
    Serial.println(MQTT_TOPIC_LIGHT_COMMAND);
    Serial.println(MQTT_TOPIC_LIGHT_MOTION_TIMER_SET);
    Serial.println(MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET);

    // Publish the discovery message
    mqtt_discovery();

  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

