#include "pin_config.h"

// --- Pin Definitions ---
const int PIR_PIN = 16;
const int RELAY_PIN = 17; // Moved from pin 4 to 6 to resolve conflict
// We can remove SDA/SCL pins as we will use board defaults
// const int OLED_SDA_PIN = 4; // This was conflicting with the relay
// const int OLED_SCL_PIN = 5;
const int OLED_I2C_ADDRESS = 0x3C;

// --- OLED Screen Settings ---
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;

// --- Other Settings ---
const int BUTTON_PIN = 3; // Example: Rotary encoder switch pin