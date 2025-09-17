#include "display_manager.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- Private Objects and Helper Functions ---

// The display object is now private to this module
static Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// This helper function is also private
static String formatDuration(unsigned long milliseconds) {
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


// --- Private Drawing Functions ---
// These are now static, meaning they can only be called from within this file.
// They now accept a 'data' struct to get the information they need to draw.

static void draw_power_all_screen(const DisplayData& data) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
  
  display.setTextSize(2);
  String title = "POWER";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 8);
  display.println(title);
  
  display.drawLine(5, 28, SCREEN_WIDTH - 5, 28, SH110X_WHITE);
  
  const char* labels[] = {"Solar Panel", "Battery", "Output"};
  
  display.setTextSize(1);
  for (int i = 0; i < 3; i++) {
    int yPos = 38 + (i * 30);
    display.setCursor(8, yPos);
    display.print(labels[i]);
    display.setCursor(10, yPos + 12);
    display.print(data.busVoltage[i], 2);
    display.print("V");
    display.setCursor(70, yPos + 12);
    display.print(data.current[i], 2);
    display.print("A");
    if (i < 2) {
        display.drawLine(5, yPos + 25, SCREEN_WIDTH - 5, yPos + 25, SH110X_WHITE);
    }
  }

  display.display();
}

static void draw_power_ch_live_screen(int channel, const DisplayData& data) {
    const char* titles[] = {"", "PANEL", "BATTERY", "OUTPUT"};
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
    
    display.setTextSize(2);
    String title = titles[channel];
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 8);
    display.println(title);
    
    display.drawLine(5, 28, SCREEN_WIDTH - 5, 28, SH110X_WHITE);

    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print("Voltage:");
    display.setCursor(70, 45);
    display.print(data.busVoltage[channel-1], 2);
    display.print(" V");

    display.setCursor(10, 65);
    display.print("Current:");
    display.setCursor(70, 65);
    display.print(data.current[channel-1], 2);
    display.print(" A");
    
    display.setCursor(10, 85);
    display.print("Power:");
    display.setCursor(70, 85);
    display.print(data.power[channel-1], 2);
    display.print(" W");
    
    display.display();
}

static void draw_lights_live_screen(const DisplayData& data) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
  
  display.setTextSize(3);
  String stateText = data.lightIsOn ? "ON" : "OFF";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(stateText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 8);
  display.println(stateText);
  
  int barWidth = 0;
  if(data.lightIsOn) {
      unsigned long timeSinceMotion = millis() - data.lastMotionTime;
      if (timeSinceMotion < RELAY_ON_DURATION) {
          barWidth = map(timeSinceMotion, 0, RELAY_ON_DURATION, 0, SCREEN_WIDTH - 20);
      }
  } else {
    barWidth = 0;
  }
  barWidth = (SCREEN_WIDTH - 20) - barWidth;

  int barY = 40;
  display.drawRoundRect(10, barY, SCREEN_WIDTH - 20, 8, 2, SH110X_WHITE);
  display.fillRoundRect(10, barY, barWidth, 8, 2, SH110X_WHITE);
  
  display.drawLine(5, 60, SCREEN_WIDTH - 5, 60, SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(10, 70);
  display.print("Time On");

  display.setTextSize(2);
  display.setCursor(10, 80);
  display.print(formatDuration(data.lightIsOn ? millis() - data.lightOnTime : 0));
  
  display.setTextSize(1);
  display.setCursor(10, 100);
  display.print("Time Left");
  
  unsigned long timeRemaining = 0;
  if (data.lightIsOn) {
    unsigned long timeSinceMotion = millis() - data.lastMotionTime;
    if (timeSinceMotion < RELAY_ON_DURATION) {
      timeRemaining = RELAY_ON_DURATION - timeSinceMotion;
    }
  }
  display.setTextSize(2);
  display.setCursor(10, 110);
  display.print(formatDuration(timeRemaining));

  display.display();
}

static void draw_lights_sub_screen() {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(22, 55);
    display.println("Lights");
    display.setCursor(10, 75);
    display.println("Subscreen");
    display.display();
}

static void draw_power_sub_screen(int channel) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(10, 55);
    display.print("CH ");
    display.print(channel);
    display.print(" Sub");
    display.display();
}


// --- Public Functions ---

void setup_display() {
  if (!display.begin(OLED_I2C_ADDRESS, true)) {
    Serial.println(F("SH1107 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
}


// --- THIS IS THE NEW DISPATCHER FUNCTION ---
// Its job is to look at the current state and call the correct private drawing function.
void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data) {
  switch (mode) {
    case LIGHTS_MODE:
      if (lightsSub == LIVE_STATUS) {
        draw_lights_live_screen(data);
      } else {
        draw_lights_sub_screen();
      }
      break;
    case POWER_MODE_ALL:
      draw_power_all_screen(data);
      break;
    case POWER_MODE_CH1:
      if (powerSub == LIVE_POWER) {
        draw_power_ch_live_screen(1, data);
      } else {
        draw_power_sub_screen(1);
      }
      break;
    case POWER_MODE_CH2:
      if (powerSub == LIVE_POWER) {
        draw_power_ch_live_screen(2, data);
      } else {
        draw_power_sub_screen(2);
      }
      break;
    case POWER_MODE_CH3:
      if (powerSub == LIVE_POWER) {
        draw_power_ch_live_screen(3, data);
      } else {
        draw_power_sub_screen(3);
      }
      break;
    default:
      draw_power_all_screen(data);
      break;
  }
}

