// This is the "menu" for our connections module.
// Any file that includes this can use the functions declared below.

#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <PubSubClient.h>

// "extern" tells the compiler that this object exists, but is defined
// in a different file (in our case, connections.cpp).
extern PubSubClient client;

// This is the public list of functions available from this module.
void setup_wifi();
void reconnect();
void mqtt_discovery();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

// New function declarations for handling specific MQTT commands
void handle_lights_command(String message);
void handle_motion_timer_command(String message);
void handle_manual_timer_command(String message);

#endif // CONNECTIONS_H

