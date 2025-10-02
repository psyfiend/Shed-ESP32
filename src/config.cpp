#include "config.h"
#include <Arduino.h> // For LED_BUILTIN

// Pin definitions
const int PIR_PIN = 16;
const int LED_PIN = LED_BUILTIN;
const int RELAY_PIN = 17; // D7 is GPIO17

// --- Rotary Encoder Pins ---
const int ENCODER_CLK_PIN = 1; 
const int ENCODER_DT_PIN = 2; 
const int ENCODER_SW_PIN = 21;  

// OLED display settings
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;
const int OLED_RESET = -1;
const int OLED_I2C_ADDRESS = 0x3C;

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "M&M Motors";
const char* WIFI_PASSWORD = "seamosss";

// --- MQTT Broker Settings ---
const char* MQTT_SERVER = "192.168.0.70";
const char* MQTT_USER = "mqtt_user";
const char* MQTT_PASSWORD = "mqtt_user3700";
const char* DEVICE_ID = "shed_esp32_c6_01";

// --- MQTT Topics ---
const char* MQTT_BASE_TOPIC = "shed/monitor";
const char* MQTT_TOPIC_AVAILABILITY = "shed/monitor/availability";
const char* MQTT_TOPIC_MOTION_STATE = "shed/monitor/motion/state";       // Physical PIR sensor state
const char* MQTT_TOPIC_OCCUPANCY_STATE = "shed/monitor/occupancy/state"; // Software-based occupancy state (based on timer)
const char* MQTT_TOPIC_LIGHT_STATE = "shed/monitor/light/state";         // Physical light state
const char* MQTT_TOPIC_LIGHT_COMMAND = "shed/monitor/light/switch";      // Command to control the light state

// --- Base Topics for MQTT Discovery ---
const char* MQTT_TOPIC_LIGHT_BASE = "shed/monitor/light";
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE = "shed/monitor/light/motion_timer";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE = "shed/monitor/light/manual_timer";
const char* MQTT_TOPIC_POWER_CH1_STATE = "shed/monitor/power/ch1";
const char* MQTT_TOPIC_POWER_CH2_STATE = "shed/monitor/power/ch2";
const char* MQTT_TOPIC_POWER_CH3_STATE = "shed/monitor/power/ch3";

// --- Topics for functions to address
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE = "shed/monitor/light/motion_timer/state";
const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_SET = "shed/monitor/light/motion_timer/set";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE = "shed/monitor/light/manual_timer/state";
const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET = "shed/monitor/light/manual_timer/set";

// --- MQTT Payloads ---
const char* MQTT_PAYLOAD_ONLINE = "online";
const char* MQTT_PAYLOAD_OFFLINE = "offline";

// --- Power Monitor ---
const uint8_t INA226_CH1_ADDRESS = 0x40; // Solar Panel
const uint8_t INA226_CH2_ADDRESS = 0x41; // Battery 
const uint8_t INA226_CH3_ADDRESS = 0x44; // Output 
const float INA226_CH1_SHUNT = 0.01;
const float INA226_CH2_SHUNT = 0.01;
const float INA226_CH3_SHUNT = 0.01;

// --- Application Logic Constants ---
unsigned long MOTION_TIMER_DURATION = 10000;      // 10 seconds
unsigned long MANUAL_TIMER_DURATION = 300000;     // 5 minutes (300,000 ms)
const int NUM_MODES = 5;
const unsigned long INACTIVITY_TIMEOUT = 30000;
const int DISPLAY_UPDATE_INTERVAL = 100;
const bool PUBLISH_DISCOVERY = true; // Set to 'false' to prevent publishing

