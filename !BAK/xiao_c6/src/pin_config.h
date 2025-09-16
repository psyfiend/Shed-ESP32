
#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

// --- Pin Definitions ---
extern const int PIR_PIN;
extern const int RELAY_PIN;
// No longer need to export these as we're using defaults
// extern const int OLED_SDA_PIN;
// extern const int OLED_SCL_PIN;
extern const int OLED_I2C_ADDRESS;

// --- OLED Screen Settings ---
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

extern const int BUTTON_PIN; // Example: Rotary encoder switch pin


#endif

