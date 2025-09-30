#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Pin definitions
extern const int PIR_PIN;
extern const int LED_PIN;
extern const int RELAY_PIN;
extern const int ENCODER_CLK_PIN;
extern const int ENCODER_DT_PIN;
extern const int ENCODER_SW_PIN;

// OLED display settings
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int OLED_RESET;
extern const int OLED_I2C_ADDRESS;

// --- Wi-Fi Credentials ---
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// --- MQTT Broker Settings ---
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* DEVICE_ID;

// --- MQTT Topics ---
extern const char* MQTT_BASE_TOPIC;
extern const char* MQTT_TOPIC_AVAILABILITY;
extern const char* MQTT_TOPIC_MOTION_STATE;    // For the physical PIR sensor
extern const char* MQTT_TOPIC_OCCUPANCY_STATE; // For the software timer
extern const char* MQTT_TOPIC_LIGHT_STATE;      // For the physical light state
extern const char* MQTT_TOPIC_LIGHT_COMMAND;    // For commands to control the light

// --- Base Topics for MQTT Discovery ---
extern const char* MQTT_TOPIC_LIGHT_BASE;       // shed/monitor/light
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_BASE;  // shed/monitor/light/motion_timer"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_BASE;  // shed/monitor/light/manual_timer"
extern const char* MQTT_TOPIC_POWER_CH1_STATE;     // shed/monitor/power/ch1
extern const char* MQTT_TOPIC_POWER_CH2_STATE;     // shed/monitor/power/ch2
extern const char* MQTT_TOPIC_POWER_CH3_STATE;     // shed/monitor/power/ch3

// --- Topics for functions to address
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE; // shed/monitor/light/motion_timer/state"
extern const char* MQTT_TOPIC_LIGHT_MOTION_TIMER_SET;   // shed/monitor/light/motion_timer/set"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE; // shed/monitor/light/manual_timer/state"
extern const char* MQTT_TOPIC_LIGHT_MANUAL_TIMER_SET;   // shed/monitor/light/manual_timer/set"

// --- MQTT Payloads ---
extern const char* MQTT_PAYLOAD_ONLINE;
extern const char* MQTT_PAYLOAD_OFFLINE;

// --- Power Monitor ---
extern const uint8_t INA226_CH1_ADDRESS;
extern const uint8_t INA226_CH2_ADDRESS;
extern const uint8_t INA226_CH3_ADDRESS;
extern const float INA226_CH1_SHUNT;
extern const float INA226_CH2_SHUNT;
extern const float INA226_CH3_SHUNT;

// --- Application Logic Constants ---
extern unsigned long MOTION_TIMER_DURATION;     // <-- RENAMED & CHANGED
extern unsigned long MANUAL_TIMER_DURATION;     // <-- ADDED
extern const int NUM_MODES;
extern const unsigned long INACTIVITY_TIMEOUT;
extern const int DISPLAY_UPDATE_INTERVAL;

#endif // CONFIG_H

