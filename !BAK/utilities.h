#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

// --- Other Settings ---
extern const int PIR_DELAY_MS;                        // Software wait time in milliseconds
extern const int longPressThreshold;     // Long press threshold in milliseconds

String formatDuration(unsigned long duration);

#endif