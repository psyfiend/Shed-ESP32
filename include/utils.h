#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// --- Public Function Declarations ---

/**
 * @brief Formats a duration in milliseconds into a HH:MM:SS string.
 * @param milliseconds The duration to format.
 * @return A String object with the formatted time.
 */
String formatDuration(unsigned long milliseconds);

/**
 * @brief Determines the correct light timer duration based on the override state.
 * @param isManualOverride True if the light was turned on by HA, false otherwise.
 * @return The appropriate timer duration in milliseconds.
 */
unsigned long get_current_timer_duration(bool isManualOverride);

#endif // UTILS_H