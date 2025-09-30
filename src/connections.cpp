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

  JsonDocument discovery_doc;
  const char* discovery_topic = "homeassistant/device/shed_esp32_c6_01/config";

  JsonObject device_doc = discovery_doc["device"].to<JsonObject>();
  device_doc["name"] = "Shed Monitor System";
  device_doc["ids"] = DEVICE_ID;
  device_doc["mf"] = "Seeed Studio";
  device_doc["mdl"] = "XIAO ESP32-C6";
  device_doc["suggested_area"] = "Shed";
  
  JsonObject origin_doc = discovery_doc["o"].to<JsonObject>();
  origin_doc["name"] = "Shed Monitor (o)";
  origin_doc["sw"] = "0.1";
  origin_doc["url"] = "https://switz.org";

  JsonObject cmps_doc = discovery_doc["cmps"].to<JsonObject>();

  // Software-based Occupancy Sensor (timer-based)
  JsonObject occupancy_cmp = cmps_doc["shed_monitor_occupancy"].to<JsonObject>();
  occupancy_cmp["name"] = "Occupancy Sensor";
  occupancy_cmp["p"] = "binary_sensor";
  occupancy_cmp["dev_cla"] = "occupancy";
  occupancy_cmp["uniq_id"] = "shed_esp32_pir_occupancy";
  occupancy_cmp["stat_t"] = MQTT_TOPIC_OCCUPANCY_STATE;  // shed/monitor/occupancy/state
  occupancy_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  occupancy_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  occupancy_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;
  occupancy_cmp["pl_on"] = "on";
  occupancy_cmp["pl_off"] = "off";

  // Physical PIR Motion Sensor
  JsonObject motion_cmp = cmps_doc["shed_monitor_motion"].to<JsonObject>();
  motion_cmp["name"] = "Motion Sensor";
  motion_cmp["p"] = "binary_sensor";
  motion_cmp["dev_cla"] = "motion";
  motion_cmp["uniq_id"] = "shed_esp32_pir_motion";
  motion_cmp["stat_t"] = MQTT_TOPIC_MOTION_STATE;  // shed/monitor/motion/state
  motion_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  motion_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  motion_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;
  motion_cmp["pl_on"] = "on";
  motion_cmp["pl_off"] = "off";

  // Light (relay) entity
  JsonObject light_cmp = cmps_doc["shed_monitor_light"].to<JsonObject>();
  light_cmp["name"] = "Overhead Light";
  light_cmp["p"] = "light";
  light_cmp["uniq_id"] = "shed_esp32_light";
  light_cmp["~"] = MQTT_TOPIC_LIGHT_BASE; //shed/monitor/light
  light_cmp["stat_t"] = "~/state";        //shed/monitor/light/state
  light_cmp["cmd_t"] = "~/switch";        //shed/monitor/light/switch
  light_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  light_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  light_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // Motion Timer (for the light)
  JsonObject motion_timer_cmp = cmps_doc["shed_monitor_light_motion_timer"].to<JsonObject>();
  motion_timer_cmp["name"] = "Motion Timer";
  motion_timer_cmp["p"] = "number";
  motion_timer_cmp["min"] = 10;
  motion_timer_cmp["max"] = 3600;
  motion_timer_cmp["unit_of_meas"] = "s";  // seconds
  motion_timer_cmp["uniq_id"] = "shed_esp32_light_motion_timer";
  motion_timer_cmp["~"] = MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE; // shed/monitor/light/motion_timer
  motion_timer_cmp["stat_t"] = "~/state";                     // shed/monitor/light/motion_timer/state
  motion_timer_cmp["cmd_t"] = "~/set";                        // shed/monitor/light/motion_timer/set
  motion_timer_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  motion_timer_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  motion_timer_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // Manual override Timer (for the light)
  JsonObject manual_timer_cmp = cmps_doc["shed_monitor_light_override_timer"].to<JsonObject>();
  manual_timer_cmp["name"] = "Override Timer";
  manual_timer_cmp["p"] = "number";
  manual_timer_cmp["min"] = 10;
  manual_timer_cmp["max"] = 3600;
  manual_timer_cmp["unit_of_meas"] = "s";  // seconds
  manual_timer_cmp["uniq_id"] = "shed_esp32_light_override_timer";
  manual_timer_cmp["~"] = MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE; // shed/monitor/light/manual_timer
  manual_timer_cmp["stat_t"] = "~/state";                     // shed/monitor/light/manual_timer/state
  manual_timer_cmp["cmd_t"] = "~/set";                        // shed/monitor/light/manual_timer/set
  manual_timer_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;   // shed/monitor/availability
  manual_timer_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  manual_timer_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Voltage
  JsonObject power_ch1_v_cmp = cmps_doc["shed_monitor_power_ch1_voltage"].to<JsonObject>();
  power_ch1_v_cmp["name"] = "Channel 1 - Voltage";
  power_ch1_v_cmp["p"] = "sensor";
  power_ch1_v_cmp["dev_cla"] = "voltage";
  power_ch1_v_cmp["unit_of_meas"] = "V";
  power_ch1_v_cmp["stat_cla"] = "measurement";
  power_ch1_v_cmp["val_tpl"] = "{{ value_json.bus_voltage }}";
  power_ch1_v_cmp["uniq_id"] = "shed_esp32_power_ch1_voltage";
  power_ch1_v_cmp["i"] = "mdi:sine-wave"; // icon
  power_ch1_v_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_v_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Current
  JsonObject power_ch1_a_cmp = cmps_doc["shed_monitor_power_ch1_current"].to<JsonObject>();
  power_ch1_a_cmp["name"] = "Channel 1 - Current";
  power_ch1_a_cmp["p"] = "sensor";
  power_ch1_a_cmp["dev_cla"] = "current";
  power_ch1_a_cmp["unit_of_meas"] = "mA";
  power_ch1_a_cmp["stat_cla"] = "measurement";
  power_ch1_a_cmp["val_tpl"] = "{{ value_json.current }}";
  power_ch1_a_cmp["uniq_id"] = "shed_esp32_power_ch1_current";
  power_ch1_a_cmp["i"] = "mdi:current-dc"; // icon
  power_ch1_a_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_a_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Power
  JsonObject power_ch1_p_cmp = cmps_doc["shed_monitor_power_ch1_power"].to<JsonObject>();
  power_ch1_p_cmp["name"] = "Channel 1 - Power";
  power_ch1_p_cmp["p"] = "sensor";
  power_ch1_p_cmp["dev_cla"] = "power";
  power_ch1_p_cmp["unit_of_meas"] = "mW";
  power_ch1_p_cmp["stat_cla"] = "measurement";
  power_ch1_p_cmp["val_tpl"] = "{{ value_json.power }}";
  power_ch1_p_cmp["uniq_id"] = "shed_esp32_power_ch1_power";
  power_ch1_p_cmp["i"] = "mdi:solar-power-variant"; // icon
  power_ch1_p_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_p_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;


  // Print the total size of the JSON payload
  size_t jsonSize = measureJson(discovery_doc);
  Serial.println("------------------------------");
  Serial.print("Calculated Discovery Payload Size: ");
  Serial.println(jsonSize);
  Serial.println("------------------------------");


  Serial.println("--- Single Discovery Payload ---");
  serializeJsonPretty(discovery_doc, Serial);
  Serial.println();
  Serial.print("Publishing to topic: ");
  Serial.println(discovery_topic);
  
  char buffer[3072];
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

