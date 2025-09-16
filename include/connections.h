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

#endif // CONNECTIONS_H

