#include <Arduino.h>

// Include our configuration file
// #include "config.h"

// --- Other Settings ---
const int PIR_DELAY_MS = 2000; // Example: 2-second delay after motion stops
const int longPressThreshold = 1500; // 1.5 seconds

// Helper function to format milliseconds into HH:MM:SS string
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