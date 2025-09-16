#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "display_manager.h"
#include "config.h"
#include "power_monitor.h" // Include this to get power data

// --- Private Objects and Variables ---
Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Forward Declarations for private functions ---
String formatDuration(unsigned long milliseconds);
void draw_lights_live_screen(bool p_lightIsOn, unsigned long p_lastMotionTime, unsigned long p_lightOnTime, unsigned long p_relayOnDuration);
void draw_lights_sub_screen();
void draw_power_ch_live_screen(int channel);
void draw_power_sub_screen(int channel);


// --- Public Function Implementations ---

void setup_display() {
  if (!display.begin(OLED_I2C_ADDRESS, true)) {
    Serial.println(F("SH1107 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

void draw_lights_screen(LightsSubMode subMode, bool p_lightIsOn, unsigned long p_lastMotionTime, unsigned long p_lightOnTime, unsigned long p_relayOnDuration) {
  switch (subMode) {
    case LIVE_STATUS:
      draw_lights_live_screen(p_lightIsOn, p_lastMotionTime, p_lightOnTime, p_relayOnDuration);
      break;
    case SUB_SCREEN:
      draw_lights_sub_screen();
      break;
  }
}

void draw_power_ch_screen(int channel, PowerSubMode subMode) {
  switch (subMode) {
    case LIVE_POWER:
      draw_power_ch_live_screen(channel);
      break;
    case POWER_SUBSCREEN:
      draw_power_sub_screen(channel);
      break;
  }
}

// --- Private Drawing Functions ---

void draw_power_all_screen() {
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
    int channel = i + 1;
    int yPos = 38 + (i * 30);
    display.setCursor(8, yPos);
    display.print(labels[i]);
    display.setCursor(10, yPos + 12);
    display.print(get_bus_voltage(channel), 2);
    display.print("V");
    display.setCursor(70, yPos + 12);
    display.print(get_current(channel), 2);
    display.print("A");
    if (i < 2) {
        display.drawLine(5, yPos + 25, SCREEN_WIDTH - 5, yPos + 25, SH110X_WHITE);
    }
  }

  display.display();
}

void draw_power_ch_live_screen(int channel) {
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
    display.print(get_bus_voltage(channel), 2);
    display.print(" V");

    display.setCursor(10, 65);
    display.print("Current:");
    display.setCursor(70, 65);
    display.print(get_current(channel), 2);
    display.print(" A");
    
    display.setCursor(10, 85);
    display.print("Power:");
    display.setCursor(70, 85);
    display.print(get_power(channel), 2);
    display.print(" W");
    
    display.display();
}

void draw_lights_live_screen(bool p_lightIsOn, unsigned long p_lastMotionTime, unsigned long p_lightOnTime, unsigned long p_relayOnDuration) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SH110X_WHITE);
  
  display.setTextSize(3);
  String stateText = p_lightIsOn ? "ON" : "OFF";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(stateText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 8);
  display.println(stateText);
  
  int barWidth = 0;
  if(p_lightIsOn) {
      unsigned long timeSinceMotion = millis() - p_lastMotionTime;
      if (timeSinceMotion < p_relayOnDuration) {
          barWidth = map(timeSinceMotion, 0, p_relayOnDuration, 0, SCREEN_WIDTH - 20);
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
  display.print(formatDuration(p_lightIsOn ? millis() - p_lightOnTime : 0));
  
  display.setTextSize(1);
  display.setCursor(10, 100);
  display.print("Time Left");
  
  unsigned long timeRemaining = 0;
  if (p_lightIsOn) {
    unsigned long timeSinceMotion = millis() - p_lastMotionTime;
    if (timeSinceMotion < p_relayOnDuration) {
      timeRemaining = p_relayOnDuration - timeSinceMotion;
    }
  }
  display.setTextSize(2);
  display.setCursor(10, 110);
  display.print(formatDuration(timeRemaining));

  display.display();
}

void draw_lights_sub_screen() {
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

void draw_power_sub_screen(int channel) {
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

// --- Helper Functions ---
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
