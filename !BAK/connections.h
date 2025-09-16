#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <PubSubClient.h>

// --- Device Identifiers ---
// These are used for MQTT Discovery and should be unique to your device
extern const char* device_id;
extern const char* device_name;

// --- Wi-Fi Credentials ---
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// --- MQTT Broker Settings ---
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_TOPIC_STATUS;
extern const char* MQTT_TOPIC_TRIGGERS;
extern const char* MQTT_TOPIC_DEVICE_DISCOVERY;

extern PubSubClient client;

void setup_wifi();
void reconnect();
void mqtt_discovery();

#endif