#include "utils.h"
#include "config.h" // Needed for the timer duration constants

String formatDuration(unsigned long milliseconds) {
    unsigned long totalSeconds = milliseconds / 1000;
    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours = (totalSeconds / 3600);
    String formattedString = "";
    if (hours < 10) formattedString += "0";
    formattedString += String(hours) + ":";
    if (minutes < 10) formattedString += "0";
    formattedString += String(minutes) + ":";
    if (seconds < 10) formattedString += "0";
    formattedString += String(seconds);
    return formattedString;
}

unsigned long get_current_timer_duration(bool isManualOverride) {
  // This function is now the single source of truth for timer logic.
  return isManualOverride ? MANUAL_TIMER_DURATION : MOTION_TIMER_DURATION;
}