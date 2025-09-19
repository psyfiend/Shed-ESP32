#include "display_manager.h"
#include "config.h"
#include "utils.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- Private Objects ---
static Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// --- Forward declarations for new private drawing functions ---
static void draw_lights_menu_screen(const DisplayData& data);
static void draw_edit_timer_screen(const DisplayData& data, bool isMotionTimer);


// --- Private Drawing Functions ---

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
    display.print(data.current[i], 0);
    display.print("mA");
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
    display.print(data.current[channel-1], 0);
    display.print(" mA");
    
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
  if (data.lightManualOverride) {
      stateText = "MANUAL";
  }
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(stateText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 8);
  display.println(stateText);
  
  unsigned long currentTimerDuration = get_current_timer_duration(data.lightManualOverride);

  int barWidth = 0;
  if(data.lightIsOn) {
      unsigned long timeSinceMotion = millis() - data.lastMotionTime;
      if (timeSinceMotion < currentTimerDuration) {
          barWidth = map(timeSinceMotion, 0, currentTimerDuration, 0, SCREEN_WIDTH - 20);
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
    if (timeSinceMotion < currentTimerDuration) {
      timeRemaining = currentTimerDuration - timeSinceMotion;
    }
  }
  display.setTextSize(2);
  display.setCursor(10, 110);
  display.print(formatDuration(timeRemaining));

  display.display();
}

static void draw_lights_menu_screen(const DisplayData& data) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);

    display.setTextSize(2);
    String title = "LIGHTS";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 8);
    display.println(title);
    display.drawLine(5, 28, SCREEN_WIDTH - 5, 28, SH110X_WHITE);

    const char* menuItems[] = {"", "Motion", "Manual", "Back"};
    // --- UPDATED: Dynamic text now reflects actual light state ---
    menuItems[0] = (data.lightIsOn) ? "Turn Off" : "Turn On";

    display.setTextSize(2);
    for (int i = 0; i < 4; i++) {
        int yPos = 40 + (i * 22);
        if (i == data.lightsMenuSelection) {
            display.fillRect(5, yPos - 2, SCREEN_WIDTH - 10, 20, SH110X_WHITE);
            display.setTextColor(SH110X_BLACK);
            display.setCursor(10, yPos);
            display.println(menuItems[i]);
            display.setTextColor(SH110X_WHITE);
        } else {
            display.setCursor(10, yPos);
            display.println(menuItems[i]);
        }
    }
    display.display();
}

// --- UPDATED: Cosmetic changes for the edit screen ---
static void draw_edit_timer_screen(const DisplayData& data, bool isMotionTimer) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);

    display.setTextSize(2);
    String title = isMotionTimer ? "MOTION" : "MANUAL"; // Shorter title
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 8);
    display.println(title);
    display.drawLine(5, 28, SCREEN_WIDTH - 5, 28, SH110X_WHITE);
    
    // Display the timer value being edited with a smaller font
    display.setTextSize(2); // Smaller font size
    unsigned long durationToDisplay = isMotionTimer ? data.tempMotionTimerDuration : data.tempManualTimerDuration;
    String timeText = formatDuration(durationToDisplay);
    display.getTextBounds(timeText, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 65); // Repositioned
    display.println(timeText);

    // Display instructional text
    display.setTextSize(1);
    String instruction = "Turn to adjust. Press to Save.";
    display.getTextBounds(instruction, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 110);
    display.println(instruction);

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


void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data) {
  switch (mode) {
    case LIGHTS_MODE:
      switch (lightsSub) {
        case LIVE_STATUS:
          draw_lights_live_screen(data);
          break;
        case LIGHTS_MENU:
          draw_lights_menu_screen(data);
          break;
        case EDIT_MOTION_TIMER:
          draw_edit_timer_screen(data, true);
          break;
        case EDIT_MANUAL_TIMER:
          draw_edit_timer_screen(data, false);
          break;
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

