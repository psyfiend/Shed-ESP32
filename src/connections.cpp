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
  occupancy_cmp["name"] = "Shed Occupancy";
  occupancy_cmp["p"] = "binary_sensor";
  occupancy_cmp["dev_cla"] = "occupancy";
  occupancy_cmp["uniq_id"] = "shed_esp32_pir_occupancy";
  occupancy_cmp["object_id"] = "shed_occupancy"; // <-- ADDED
  occupancy_cmp["stat_t"] = MQTT_TOPIC_OCCUPANCY_STATE;  // shed/monitor/occupancy/state
  occupancy_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  occupancy_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  occupancy_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;
  occupancy_cmp["pl_on"] = "on";
  occupancy_cmp["pl_off"] = "off";

  // Physical PIR Motion Sensor
  JsonObject motion_cmp = cmps_doc["shed_monitor_motion"].to<JsonObject>();
  motion_cmp["name"] = "Shed Motion";
  motion_cmp["p"] = "binary_sensor";
  motion_cmp["dev_cla"] = "motion";
  motion_cmp["uniq_id"] = "shed_esp32_pir_motion";
  motion_cmp["object_id"] = "shed_motion"; // <-- ADDED
  motion_cmp["stat_t"] = MQTT_TOPIC_MOTION_STATE;  // shed/monitor/motion/state
  motion_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  motion_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  motion_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;
  motion_cmp["pl_on"] = "on";
  motion_cmp["pl_off"] = "off";

  // Light (relay) entity
  JsonObject light_cmp = cmps_doc["shed_monitor_light"].to<JsonObject>();
  light_cmp["name"] = "Shed Light";
  light_cmp["p"] = "light";
  light_cmp["uniq_id"] = "shed_esp32_light";
  light_cmp["object_id"] = "shed_light"; // <-- ADDED
  light_cmp["~"] = MQTT_TOPIC_LIGHT_BASE; //shed/monitor/light
  light_cmp["stat_t"] = "~/state";        //shed/monitor/light/state
  light_cmp["cmd_t"] = "~/switch";        //shed/monitor/light/switch
  light_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  light_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  light_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // Motion Timer (for the light)
  JsonObject motion_timer_cmp = cmps_doc["shed_monitor_light_motion_timer"].to<JsonObject>();
  motion_timer_cmp["name"] = "Shed Motion Timer";
  motion_timer_cmp["p"] = "number";
  motion_timer_cmp["min"] = 10;
  motion_timer_cmp["max"] = 3600;
  motion_timer_cmp["unit_of_meas"] = "s";  // seconds
  motion_timer_cmp["uniq_id"] = "shed_esp32_light_motion_timer";
  motion_timer_cmp["object_id"] = "shed_light_motion_timer"; // <-- ADDED
  motion_timer_cmp["~"] = MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE; // shed/monitor/light/motion_timer
  motion_timer_cmp["stat_t"] = "~/state";                     // shed/monitor/light/motion_timer/state
  motion_timer_cmp["cmd_t"] = "~/set";                        // shed/monitor/light/motion_timer/set
  motion_timer_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  motion_timer_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  motion_timer_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // Manual override Timer (for the light)
  JsonObject manual_timer_cmp = cmps_doc["shed_monitor_light_override_timer"].to<JsonObject>();
  manual_timer_cmp["name"] = "Shed Override Timer";
  manual_timer_cmp["p"] = "number";
  manual_timer_cmp["min"] = 10;
  manual_timer_cmp["max"] = 3600;
  manual_timer_cmp["unit_of_meas"] = "s";  // seconds
  manual_timer_cmp["uniq_id"] = "shed_esp32_light_override_timer";
  manual_timer_cmp["object_id"] = "shed_light_override_timer"; // <-- ADDED
  manual_timer_cmp["~"] = MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE; // shed/monitor/light/manual_timer
  manual_timer_cmp["stat_t"] = "~/state";                     // shed/monitor/light/manual_timer/state
  manual_timer_cmp["cmd_t"] = "~/set";                        // shed/monitor/light/manual_timer/set
  manual_timer_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;       // shed/monitor/availability
  manual_timer_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  manual_timer_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Voltage
  JsonObject power_ch1_v_cmp = cmps_doc["shed_monitor_power_ch1_voltage"].to<JsonObject>();
  power_ch1_v_cmp["name"] = "Solar Panel Voltage";
  power_ch1_v_cmp["p"] = "sensor";
  power_ch1_v_cmp["dev_cla"] = "voltage";
  power_ch1_v_cmp["unit_of_meas"] = "V";
  power_ch1_v_cmp["stat_cla"] = "measurement";
  power_ch1_v_cmp["val_tpl"] = "{{ value_json.bus_voltage }}";
  power_ch1_v_cmp["uniq_id"] = "shed_esp32_power_ch1_voltage";
  power_ch1_v_cmp["object_id"] = "shed_solar_panel_voltage"; // <-- ADDED
  power_ch1_v_cmp["ic"] = "mdi:flash"; // icon
  power_ch1_v_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_v_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Current
  JsonObject power_ch1_a_cmp = cmps_doc["shed_monitor_power_ch1_current"].to<JsonObject>();
  power_ch1_a_cmp["name"] = "Solar Panel Current";
  power_ch1_a_cmp["p"] = "sensor";
  power_ch1_a_cmp["dev_cla"] = "current";
  power_ch1_a_cmp["unit_of_meas"] = "mA";
  power_ch1_a_cmp["stat_cla"] = "measurement";
  power_ch1_a_cmp["val_tpl"] = "{{ value_json.current }}";
  power_ch1_a_cmp["uniq_id"] = "shed_esp32_power_ch1_current";
  power_ch1_a_cmp["object_id"] = "shed_solar_panel_current"; // <-- ADDED
  power_ch1_a_cmp["ic"] = "mdi:current-dc"; // icon
  power_ch1_a_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_a_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 1 - Solar Panel Power
  JsonObject power_ch1_p_cmp = cmps_doc["shed_monitor_power_ch1_power"].to<JsonObject>();
  power_ch1_p_cmp["name"] = "Solar Panel Power";
  power_ch1_p_cmp["p"] = "sensor";
  power_ch1_p_cmp["dev_cla"] = "power";
  power_ch1_p_cmp["unit_of_meas"] = "mW";
  power_ch1_p_cmp["stat_cla"] = "measurement";
  power_ch1_p_cmp["val_tpl"] = "{{ value_json.power }}";
  power_ch1_p_cmp["uniq_id"] = "shed_esp32_power_ch1_power";
  power_ch1_p_cmp["object_id"] = "shed_solar_panel_power"; // <-- ADDED
  power_ch1_p_cmp["ic"] = "mdi:solar-power-variant"; // icon
  power_ch1_p_cmp["stat_t"] = MQTT_TOPIC_POWER_CH1_STATE;  // shed/monitor/power/ch1
  power_ch1_p_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch1_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch1_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

    // INA226 Channel 2 - Battery Voltage
  JsonObject power_ch2_v_cmp = cmps_doc["shed_monitor_power_ch2_voltage"].to<JsonObject>();
  power_ch2_v_cmp["name"] = "Battery Voltage";
  power_ch2_v_cmp["p"] = "sensor";
  power_ch2_v_cmp["dev_cla"] = "voltage";
  power_ch2_v_cmp["unit_of_meas"] = "V";
  power_ch2_v_cmp["stat_cla"] = "measurement";
  power_ch2_v_cmp["val_tpl"] = "{{ value_json.bus_voltage }}";
  power_ch2_v_cmp["uniq_id"] = "shed_esp32_power_ch2_voltage";
  power_ch2_v_cmp["object_id"] = "shed_battery_voltage"; // <-- ADDED
  power_ch2_v_cmp["ic"] = "mdi:flash"; // icon
  power_ch2_v_cmp["stat_t"] = MQTT_TOPIC_POWER_CH2_STATE;  // shed/monitor/power/ch2
  power_ch2_v_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch2_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch2_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 2 - Battery Current
  JsonObject power_ch2_a_cmp = cmps_doc["shed_monitor_power_ch2_current"].to<JsonObject>();
  power_ch2_a_cmp["name"] = "Battery Current";
  power_ch2_a_cmp["p"] = "sensor";
  power_ch2_a_cmp["dev_cla"] = "current";
  power_ch2_a_cmp["unit_of_meas"] = "mA";
  power_ch2_a_cmp["stat_cla"] = "measurement";
  power_ch2_a_cmp["val_tpl"] = "{{ value_json.current }}";
  power_ch2_a_cmp["uniq_id"] = "shed_esp32_power_ch2_current";
  power_ch2_a_cmp["object_id"] = "shed_battery_current"; // <-- ADDED
  power_ch2_a_cmp["ic"] = "mdi:current-dc"; // icon
  power_ch2_a_cmp["stat_t"] = MQTT_TOPIC_POWER_CH2_STATE;  // shed/monitor/power/ch2
  power_ch2_a_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch2_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch2_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 2 - Battery Power
  JsonObject power_ch2_p_cmp = cmps_doc["shed_monitor_power_ch2_power"].to<JsonObject>();
  power_ch2_p_cmp["name"] = "Battery Power";
  power_ch2_p_cmp["p"] = "sensor";
  power_ch2_p_cmp["dev_cla"] = "power";
  power_ch2_p_cmp["unit_of_meas"] = "mW";
  power_ch2_p_cmp["stat_cla"] = "measurement";
  power_ch2_p_cmp["val_tpl"] = "{{ value_json.power }}";
  power_ch2_p_cmp["uniq_id"] = "shed_esp32_power_ch2_power";
  power_ch2_p_cmp["object_id"] = "shed_battery_power"; // <-- ADDED
  power_ch2_p_cmp["ic"] = "mdi:battery"; // icon
  power_ch2_p_cmp["stat_t"] = MQTT_TOPIC_POWER_CH2_STATE;  // shed/monitor/power/ch2
  power_ch2_p_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch2_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch2_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 3 - Load Voltage
  JsonObject power_ch3_v_cmp = cmps_doc["shed_monitor_power_ch3_voltage"].to<JsonObject>();
  power_ch3_v_cmp["name"] = "Load Voltage";
  power_ch3_v_cmp["p"] = "sensor";
  power_ch3_v_cmp["dev_cla"] = "voltage";
  power_ch3_v_cmp["unit_of_meas"] = "V";
  power_ch3_v_cmp["stat_cla"] = "measurement";
  power_ch3_v_cmp["val_tpl"] = "{{ value_json.bus_voltage }}";
  power_ch3_v_cmp["uniq_id"] = "shed_esp32_power_ch3_voltage";
  power_ch3_v_cmp["object_id"] = "shed_load_voltage"; // <-- ADDED
  power_ch3_v_cmp["ic"] = "mdi:flash"; // icon
  power_ch3_v_cmp["stat_t"] = MQTT_TOPIC_POWER_CH3_STATE;  // shed/monitor/power/ch3
  power_ch3_v_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch3_v_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch3_v_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 3 - Load Current
  JsonObject power_ch3_a_cmp = cmps_doc["shed_monitor_power_ch3_current"].to<JsonObject>();
  power_ch3_a_cmp["name"] = "Load Current";
  power_ch3_a_cmp["p"] = "sensor";
  power_ch3_a_cmp["dev_cla"] = "current";
  power_ch3_a_cmp["unit_of_meas"] = "mA";
  power_ch3_a_cmp["stat_cla"] = "measurement";
  power_ch3_a_cmp["val_tpl"] = "{{ value_json.current }}";
  power_ch3_a_cmp["uniq_id"] = "shed_esp32_power_ch3_current";
  power_ch3_a_cmp["object_id"] = "shed_load_current"; // <-- ADDED
  power_ch3_a_cmp["ic"] = "mdi:current-dc"; // icon
  power_ch3_a_cmp["stat_t"] = MQTT_TOPIC_POWER_CH3_STATE;  // shed/monitor/power/ch3
  power_ch3_a_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch3_a_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch3_a_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // INA226 Channel 3 - Load Power
  JsonObject power_ch3_p_cmp = cmps_doc["shed_monitor_power_ch3_power"].to<JsonObject>();
  power_ch3_p_cmp["name"] = "Load Power";
  power_ch3_p_cmp["p"] = "sensor";
  power_ch3_p_cmp["dev_cla"] = "power";
  power_ch3_p_cmp["unit_of_meas"] = "mW";
  power_ch3_p_cmp["stat_cla"] = "measurement";
  power_ch3_p_cmp["val_tpl"] = "{{ value_json.power }}";
  power_ch3_p_cmp["uniq_id"] = "shed_esp32_power_ch3_power";
  power_ch3_p_cmp["object_id"] = "shed_load_power"; // <-- ADDED
  power_ch3_p_cmp["ic"] = "mdi:power-plug"; // icon
  power_ch3_p_cmp["stat_t"] = MQTT_TOPIC_POWER_CH3_STATE;  // shed/monitor/power/ch3
  power_ch3_p_cmp["avty_t"] = MQTT_TOPIC_AVAILABILITY;
  power_ch3_p_cmp["pl_avail"] = MQTT_PAYLOAD_ONLINE;
  power_ch3_p_cmp["pl_not_avail"] = MQTT_PAYLOAD_OFFLINE;

  // Print the total size of the JSON payload
  size_t jsonSize = measureJson(discovery_doc);
  Serial.println("------------------------------");
  Serial.print("Calculated Discovery Payload Size: ");
  Serial.println(jsonSize);
  Serial.println("------------------------------");

  if (PUBLISH_DISCOVERY) {  // True: publish the discovery payload
    Serial.println("Publishing MQTT Discovery Payload...");
    char buffer[6288];
    serializeJson(discovery_doc, buffer);
    client.publish(discovery_topic, buffer, true);
  } else {  // False: skip publishing, just print to serial
    Serial.println("------------------------------");
    Serial.println("------------------------------");
    Serial.println("------------------------------");
    Serial.println("PUBLISH_DISCOVERY is set to false. Skipping MQTT Discovery publish.");
    Serial.println("Note payload size and update main.cpp and connections.cpp accordingly.");
    Serial.println("------------------------------");
    Serial.println("------------------------------");
    Serial.println("------------------------------");
    return;
  }

  Serial.println("--- Single Discovery Payload ---");
  serializeJsonPretty(discovery_doc, Serial);
  Serial.println();
  Serial.print("Publishing to topic: ");
  Serial.println(discovery_topic);

}

void reconnect() {
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32-XIAOC6-ShedMonitor";

  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE)) {
    Serial.println("connected!");
    Serial.println("------------------------------");
    
    // Birth message and initial states
    client.publish(MQTT_TOPIC_AVAILABILITY, MQTT_PAYLOAD_ONLINE, true);
    
    // Publish the default timers
    Serial.println("------------------------------");
    String motion_payload = String(MOTION_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE, motion_payload.c_str(), true);
    String manual_payload = String(MANUAL_TIMER_DURATION / 1000);
    client.publish(MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE, manual_payload.c_str(), true);
    Serial.println("Published initial timer states.");

    // Subscribe to the command topics, apply retained values if broker is online
    Serial.println("------------------------------");
    client.subscribe(MQTT_TOPIC_LIGHT_COMMAND);
    client.subscribe(MQTT_TOPIC_LIGHT_MOTION_TIMER_SET);
    client.subscribe(MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET);
    Serial.print("Subscribed to command topics.");

    // Publish the discovery message
    mqtt_discovery();

  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

