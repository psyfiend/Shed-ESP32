#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Include our modularized files
#include "config.h"
#include "connections.h"
#include "encoder.h"
#include "power_monitor.h"
#include "display_manager.h"
#include "utils.h" // <-- ADDED: Include our new utilities

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- State Tracking Variables ---
DisplayMode currentMode = POWER_MODE_ALL;
LightsSubMode currentLightsSubMode = LIVE_STATUS;
PowerSubMode currentPowerSubMode = LIVE_POWER;

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
  
  // Configure the MQTT client
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
    if (currentLightsSubMode == LIVE_STATUS && currentPowerSubMode == LIVE_POWER) {
       int modeIndex = (int)currentMode;
       if (currentEncoderValue > lastEncoderValue) modeIndex++;
       else modeIndex--;
       if (modeIndex < 0) modeIndex = NUM_MODES - 1;
       if (modeIndex >= NUM_MODES) modeIndex = 0;
       currentMode = (DisplayMode)modeIndex;
    }
    lastEncoderValue = currentEncoderValue;
  }
  
  if (button_was_clicked()) {
    lastUserActivityTime = millis();
    switch (currentMode) {
      case LIGHTS_MODE:
        currentLightsSubMode = (currentLightsSubMode == LIVE_STATUS) ? SUB_SCREEN : LIVE_STATUS;
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
      lastMotionTime = millis(); // Start the timer manually
      Serial.println("Manual override ON");
    } else if (message == "OFF") {
      unsigned long currentTimerDuration = lightManualOverride ? MANUAL_TIMER_DURATION : MOTION_TIMER_DURATION;
      lightManualOverride = false;
      // Force the timer to expire by setting lastMotionTime to the past
      lastMotionTime = millis() - currentTimerDuration - 1; 
      Serial.println("Manual override OFF");
    }
}

void handle_motion_timer_command(String message) {
    unsigned long newDuration = message.toInt() * 1000; // Convert seconds to milliseconds
    if (newDuration >= 10 * 1000 && newDuration <= 3600 * 1000) { // Valid range: 10s to 3600s
        MOTION_TIMER_DURATION = newDuration;
        Serial.print("Motion timer updated to ");
        Serial.print(newDuration / 1000);
        Serial.println(" seconds.");
    } else {
        Serial.println("Invalid motion timer value received.");
    }
}

void handle_manual_timer_command(String message) {
    unsigned long newDuration = message.toInt() * 1000; // Convert seconds to milliseconds
    if (newDuration >= 10 * 1000 && newDuration <= 3600 * 1000) { // Valid range: 10s to 3600s
        MANUAL_TIMER_DURATION = newDuration;
        Serial.print("Manual timer updated to ");
        Serial.print(newDuration / 1000);
        Serial.println(" seconds.");
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
  
  // Use the new helper function to get the correct timer duration
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
    client.publish(MQTT_TOPIC_LIGHT_STATE, "ON"); // Publish light state
  } else if (!relayShouldBeOn && lightIsOn) {
    lightIsOn = false;
    Serial.println("Timer expired. Turning relay OFF.");
    digitalWrite(RELAY_PIN, LOW);
    totalTriggeredTime += (millis() - lightOnTime);
    previousStateDuration = millis() - lightOnTime;
    client.publish(MQTT_TOPIC_OCCUPANCY_STATUS, "off");
    client.publish(MQTT_TOPIC_LIGHT_STATE, "OFF"); // Publish light state

    if (lightManualOverride) {
      lightManualOverride = false;
      Serial.println("Manual override timer expired. Returning to auto mode.");
    }
  }
}

