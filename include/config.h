#ifndef CONFIG_H
#define CONFIG_H

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

extern const int INA3221_I2C_ADDRESS;

// WiFi credentials
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// MQTT settings
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_TOPIC_STATE;
extern const char* MQTT_TOPIC_TRIGGERS;
extern const char* MQTT_DISCOVERY_TOPIC;
extern const char* DEVICE_ID;

// --- Application Logic Constants ---
extern const unsigned long RELAY_ON_DURATION; // <-- ADDED THIS
extern const int NUM_MODES;                   // <-- ADDED THIS

#endif

