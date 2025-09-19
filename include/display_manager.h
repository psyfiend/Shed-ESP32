#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

// --- UI State Enums ---
// By placing these here, both main.cpp and display_manager.cpp can see them.
enum DisplayMode {
  POWER_MODE_ALL,
  POWER_MODE_CH1,
  POWER_MODE_CH2,
  POWER_MODE_CH3,
  LIGHTS_MODE
};

enum LightsSubMode { 
  LIVE_STATUS, 
  LIGHTS_MENU, 
  EDIT_MOTION_TIMER, 
  EDIT_MANUAL_TIMER 
};
enum PowerSubMode { LIVE_POWER, POWER_SUBSCREEN };

// --- Display Data Structure ---
// This struct packages up all the data the display might need,
// making it easy to pass from the main logic to the display manager.
struct DisplayData {
  bool lightIsOn;
  bool lightManualOverride;
  unsigned long lastMotionTime;
  unsigned long lightOnTime;
  float busVoltage[3];
  float current[3];
  float power[3];
  int lightsMenuSelection;
  unsigned long tempMotionTimerDuration; // <-- ADDED
  unsigned long tempManualTimerDuration; // <-- ADDED
};


// --- Public Functions ---

// Call this in setup() to initialize the screen
void setup_display();

// This is the single, high-level function that main.cpp will call to handle all drawing.
void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data);


#endif // DISPLAY_MANAGER_H

