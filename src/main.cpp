#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Include our modularized files
#include "config.h"
#include "connections.h"
#include "encoder.h"
#include "power_monitor.h"
#include "display_manager.h"
#include "utils.h"

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- State Tracking Variables ---
DisplayMode currentMode = POWER_MODE_ALL;
LightsSubMode currentLightsSubMode = LIVE_STATUS;
PowerSubMode currentPowerSubMode = LIVE_POWER;

int lightsMenuSelection = 0;
const int LIGHTS_MENU_ITEM_COUNT = 4;

// --- Temporary variables for editing timers ---
unsigned long tempMotionTimerDuration;
unsigned long tempManualTimerDuration;

bool lightIsOn = false;
bool lightManualOverride = false;
unsigned long lastMotionTime = 0;
unsigned long lightOnTime = 0;
int lastEncoderValue = 0;
int pirState = LOW;
int lastPirState = LOW;
unsigned long previousStateDuration = 0;
unsigned long totalTriggeredTime = 0;

// --- Non-Blocking Timers ---
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastMqttReconnectAttempt = 0;
unsigned long lastUserActivityTime = 0;

// --- Forward Declarations ---
void handle_lights_mode();
void handle_lights_command(String message);
void handle_motion_timer_command(String message);
void handle_manual_timer_command(String message);


void setup() {
  Serial.begin(115200);

  setup_display();
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  setup_encoder();
  setup_wifi();
  setup_power_monitor();
  
  client.setServer(MQTT_SERVER, 1883);
  client.setBufferSize(2048);
  client.setCallback(mqtt_callback);

  lastUserActivityTime = millis();
  lastEncoderValue = get_encoder_value();
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastMqttReconnectAttempt > 5000) {
      lastMqttReconnectAttempt = now;
      reconnect();
    }
  } else {
    client.loop();
  }
  
  loop_encoder();

  if (millis() - lastUserActivityTime > INACTIVITY_TIMEOUT) {
    currentMode = POWER_MODE_ALL;
    currentLightsSubMode = LIVE_STATUS;
    currentPowerSubMode = LIVE_POWER;
  }

  int currentEncoderValue = get_encoder_value();
  if (currentEncoderValue != lastEncoderValue) {
    lastUserActivityTime = millis();
    int encoderChange = currentEncoderValue - lastEncoderValue;

    switch (currentLightsSubMode) {
      case LIGHTS_MENU:
        lightsMenuSelection += (encoderChange > 0) ? 1 : -1;
        if (lightsMenuSelection < 0) lightsMenuSelection = LIGHTS_MENU_ITEM_COUNT - 1;
        if (lightsMenuSelection >= LIGHTS_MENU_ITEM_COUNT) lightsMenuSelection = 0;
        break;

      case EDIT_MOTION_TIMER:
        // --- UPDATED: Safe timer adjustment to prevent underflow ---
        if (encoderChange < 0) { // Decrementing
          if (tempMotionTimerDuration > 30000) tempMotionTimerDuration -= 30000;
          else tempMotionTimerDuration = 10000;
        } else { // Incrementing
          if (tempMotionTimerDuration < 3600000) tempMotionTimerDuration += 30000;
          else tempMotionTimerDuration = 3600000;
        }
        break;
        
      case EDIT_MANUAL_TIMER:
        // --- UPDATED: Safe timer adjustment to prevent underflow ---
        if (encoderChange < 0) { // Decrementing
          if (tempManualTimerDuration > 30000) tempManualTimerDuration -= 30000;
          else tempManualTimerDuration = 10000;
        } else { // Incrementing
          if (tempManualTimerDuration < 3600000) tempManualTimerDuration += 30000;
          else tempManualTimerDuration = 3600000;
        }
        break;
        
      default: // Includes LIVE_STATUS
        if (currentMode != LIGHTS_MODE) {
          int modeIndex = (int)currentMode;
          if (encoderChange > 0) modeIndex++;
          else modeIndex--;
          if (modeIndex < 0) modeIndex = NUM_MODES - 1;
          if (modeIndex >= NUM_MODES) modeIndex = 0;
          currentMode = (DisplayMode)modeIndex;
        }
        break;
    }
    lastEncoderValue = currentEncoderValue;
  }
  
  if (button_was_clicked()) {
    lastUserActivityTime = millis();
    switch (currentMode) {
      case LIGHTS_MODE:
        switch (currentLightsSubMode) {
          case LIVE_STATUS:
            currentLightsSubMode = LIGHTS_MENU;
            lightsMenuSelection = 0;
            break;
          case LIGHTS_MENU:
            switch (lightsMenuSelection) {
              case 0: // Turn light on/off
                // --- UPDATED: Toggles based on actual light state ---
                if (lightIsOn) {
                  handle_lights_command("OFF");
                } else {
                  handle_lights_command("ON");
                }
                break;
              case 1:
                tempMotionTimerDuration = MOTION_TIMER_DURATION;
                currentLightsSubMode = EDIT_MOTION_TIMER;
                break;
              case 2:
                tempManualTimerDuration = MANUAL_TIMER_DURATION;
                currentLightsSubMode = EDIT_MANUAL_TIMER;
                break;
              case 3:
                currentLightsSubMode = LIVE_STATUS;
                break;
            }
            break;
          case EDIT_MOTION_TIMER:
            handle_motion_timer_command(String(tempMotionTimerDuration / 1000));
            currentLightsSubMode = LIGHTS_MENU;
            break;
          case EDIT_MANUAL_TIMER:
            handle_manual_timer_command(String(tempManualTimerDuration / 1000));
            currentLightsSubMode = LIGHTS_MENU;
            break;
        }
        break;
      case POWER_MODE_ALL:
        break;
      case POWER_MODE_CH1:
      case POWER_MODE_CH2:
      case POWER_MODE_CH3:
        currentPowerSubMode = (currentPowerSubMode == LIVE_POWER) ? POWER_SUBSCREEN : LIVE_POWER;
        break;
    }
  }

  handle_lights_mode();
  loop_power_monitor();

  if (millis() - lastDisplayUpdateTime > DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdateTime = millis();
    
    DisplayData data;
    data.lightIsOn = lightIsOn;
    data.lightManualOverride = lightManualOverride;
    data.lightOnTime = lightOnTime;
    data.lastMotionTime = lastMotionTime;
    data.lightsMenuSelection = lightsMenuSelection;
    data.tempMotionTimerDuration = tempMotionTimerDuration;
    data.tempManualTimerDuration = tempManualTimerDuration;
    for(int i=0; i<3; i++) {
      data.busVoltage[i] = get_bus_voltage(i+1);
      data.current[i] = get_current(i+1);
      data.power[i] = get_power(i+1);
    }
    
    update_display(currentMode, currentLightsSubMode, currentPowerSubMode, data);
  }
}

void handle_lights_command(String message) {
    if (message == "ON") {
      lightManualOverride = true;
      lastMotionTime = millis();
      Serial.println("Manual override ON");
    } else if (message == "OFF") {
      lightManualOverride = false;
      unsigned long currentTimerDuration = get_current_timer_duration(true);
      lastMotionTime = millis() - currentTimerDuration - 1; 
      Serial.println("Manual override OFF");
    }
}

void handle_motion_timer_command(String message) {
    unsigned long newDurationSec = message.toInt();
    if (newDurationSec >= 10 && newDurationSec <= 3600) {
        MOTION_TIMER_DURATION = newDurationSec * 1000;
        Serial.print("Motion timer updated to ");
        Serial.print(newDurationSec);
        Serial.println(" seconds.");
        client.publish(MQTT_TOPIC_LIGHT_MOTION_TIMER_STATE, message.c_str(), true);
    } else {
        Serial.println("Invalid motion timer value received.");
    }
}

void handle_manual_timer_command(String message) {
    unsigned long newDurationSec = message.toInt();
    if (newDurationSec >= 10 && newDurationSec <= 3600) {
        MANUAL_TIMER_DURATION = newDurationSec * 1000;
        Serial.print("Manual timer updated to ");
        Serial.print(newDurationSec);
        Serial.println(" seconds.");
        client.publish(MQTT_TOPIC_LIGHT_MANUAL_TIMER_STATE, message.c_str(), true);
    } else {
        Serial.println("Invalid manual timer value received.");
    }
}

void handle_lights_mode() {
  pirState = digitalRead(PIR_PIN);
  digitalWrite(LED_PIN, pirState);

  if (pirState != lastPirState) {
    if (pirState == HIGH) {
      client.publish(MQTT_TOPIC_MOTION_STATUS, "on");
    } else {
      client.publish(MQTT_TOPIC_MOTION_STATUS, "off");
    }
    lastPirState = pirState;
  }

  if (!lightManualOverride) {
    if (pirState == HIGH) {
      lastMotionTime = millis();
      lastUserActivityTime = millis();
    }
  }
  
  unsigned long currentTimerDuration = get_current_timer_duration(lightManualOverride);
  bool relayShouldBeOn = (millis() - lastMotionTime < currentTimerDuration);
  
  if (relayShouldBeOn && !lightIsOn) {
    lightIsOn = true;
    if (lightManualOverride) {
        Serial.println("Manual ON: Turning relay ON.");
    } else {
        Serial.println("Occupancy detected! Turning relay ON.");
    }
    digitalWrite(RELAY_PIN, HIGH);
    lightOnTime = millis();
    client.publish(MQTT_TOPIC_OCCUPANCY_STATUS, "on");
    client.publish(MQTT_TOPIC_LIGHT_STATE, "ON");
  } else if (!relayShouldBeOn && lightIsOn) {
    lightIsOn = false;
    Serial.println("Timer expired. Turning relay OFF.");
    digitalWrite(RELAY_PIN, LOW);
    totalTriggeredTime += (millis() - lightOnTime);
    previousStateDuration = millis() - lightOnTime;
    client.publish(MQTT_TOPIC_OCCUPANCY_STATUS, "off");
    client.publish(MQTT_TOPIC_LIGHT_STATE, "OFF");

    if (lightManualOverride) {
      lightManualOverride = false;
      Serial.println("Manual override timer expired. Returning to auto mode.");
    }
  }
}

