#include "config.h"
#include <Arduino.h> // For LED_BUILTIN

// Pin definitions
const int PIR_PIN = 16;
const int LED_PIN = LED_BUILTIN;
const int RELAY_PIN = 17; // D7 is GPIO17

// --- Rotary Encoder Pins ---
// IMPORTANT: Please update these pin numbers to match your wiring.
const int ENCODER_CLK_PIN = 1; // CLK pin of the encoder
const int ENCODER_DT_PIN = 2; // DT pin of the encoder
const int ENCODER_SW_PIN = 21;  // SW (button) pin of the encoder

// OLED display settings
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;
const int OLED_RESET = -1;
const int OLED_I2C_ADDRESS = 0x3C;

// INA3221 I2C address
const int INA3221_I2C_ADDRESS = 0x40;

// --- Wi-Fi Credentials ---
const char* WIFI_SSID = "M&M Motors";
const char* WIFI_PASSWORD = "seamosss";

// --- MQTT Broker Settings ---
const char* MQTT_SERVER = "192.168.0.70";
const char* MQTT_USER = "mqtt_user";
const char* MQTT_PASSWORD = "mqtt_user3700";
const char* MQTT_TOPIC_STATE = "shed/motion/status";
const char* MQTT_TOPIC_TRIGGERS = "shed/motion/triggers";
const char* MQTT_DISCOVERY_TOPIC = "homeassistant/device/shed_esp32_c6_01/config";
const char* DEVICE_ID = "shed_esp32_c6_01";

// --- Application Logic Constants ---
const unsigned long RELAY_ON_DURATION = 10000; // 10 seconds
const int NUM_MODES = 5;
