#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>

// Include our configuration and utility files
#include "pin_config.h"
#include "connections.h"
#include "utilities.h"

// Initialize the display using the standard constructor
Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Global state variables
int pirState = LOW;
int lastPirState = LOW;
unsigned long pirHighTime = 0;
unsigned long lastChangeTime = 0;
unsigned long previousStateDuration = 0;
unsigned long totalTriggeredTime = 0;
int triggerCount = 0;

// Function to handle display updates
void drawDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 0);

  // Main status: ON or OFF
  if (digitalRead(RELAY_PIN) == HIGH) {
    display.print("ON");
  } else {
    display.print("OFF");
  }

  // Draw the progress bar
  int barLength = map(min(millis() - lastChangeTime, (unsigned long)PIR_DELAY_MS), 0, PIR_DELAY_MS, 0, 128);
  display.drawRect(0, 15, barLength, 5, SH110X_WHITE);

  // Display the timing information
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("Current:");
  display.setCursor(50, 25);
  display.print(formatDuration(millis() - lastChangeTime));

  display.setCursor(0, 35);
  display.print("Prev:");
  display.setCursor(50, 35);
  display.print(formatDuration(previousStateDuration));

  display.setCursor(0, 45);
  display.print("Triggers:");
  display.setCursor(50, 45);
  display.print(triggerCount);

  display.setCursor(0, 55);
  display.print("Total On:");
  display.setCursor(50, 55);
  display.print(formatDuration(totalTriggeredTime));
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting up...");

  // Set pin modes
  pinMode(PIR_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off on boot
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initialize OLED display
  Wire.begin(); // Use default I2C pins (SDA=4, SCL=5)
  if(!display.begin(OLED_I2C_ADDRESS, true)) { // Address from config, true = don't reset
    Serial.println(F("SH1107 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(100);

  // Setup Wi-Fi and MQTT
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);

  Serial.println("Setup complete.");
}

void loop() {
  // MQTT loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read PIR sensor
  pirState = digitalRead(PIR_PIN);

  // Check for state change
  if (pirState != lastPirState) {
    lastChangeTime = millis();
    previousStateDuration = millis() - lastChangeTime; // This line is incorrect
    lastPirState = pirState;
  }
  
  // Timer and relay control logic
  if (pirState == HIGH) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // This is where a software timer would go
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  // Update display
  drawDisplay();
}

