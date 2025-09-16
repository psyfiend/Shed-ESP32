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

enum LightsSubMode { LIVE_STATUS, SUB_SCREEN };
enum PowerSubMode { LIVE_POWER, POWER_SUBSCREEN };

// --- Public Functions ---

// Call this in setup() to initialize the screen
void setup_display();

// These are the high-level functions that main.cpp will call
void draw_lights_screen(LightsSubMode subMode, bool p_lightIsOn, unsigned long p_lastMotionTime, unsigned long p_lightOnTime, unsigned long p_relayOnDuration);
void draw_power_all_screen();
void draw_power_ch_screen(int channel, PowerSubMode subMode);


#endif // DISPLAY_MANAGER_H