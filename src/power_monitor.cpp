#include <Arduino.h>
#include <Adafruit_INA3221.h>
#include "power_monitor.h"
#include "config.h"

// Create an instance of the INA3221 sensor object using the Adafruit library
Adafruit_INA3221 ina3221 = Adafruit_INA3221();

// Variables to hold the latest sensor readings
float busVoltage[3];
float shuntVoltage[3];
float current[3];
float power[3];

// Non-blocking timer for sensor reads
unsigned long lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL = 1000; // Read sensors once per second

void setup_power_monitor() {
  Serial.println("Initializing INA3221...");
  if (!ina3221.begin(INA3221_I2C_ADDRESS, &Wire)) { // can use other I2C addresses or buses
    Serial.println("Failed to find INA3221 chip");
    while (1);
  }

  // --- Set the correct shunt resistor values ---
  // As you correctly identified, the channels are 0, 1, and 2.
  for (uint8_t i = 0; i < 3; i++) {
    ina3221.setShuntResistance(i, 0.10);
  }
  
  Serial.println("INA3221 Initialized and Configured.");
}

void loop_power_monitor() {
  // Read the sensor on a non-blocking timer
  if (millis() - lastSensorReadTime > SENSOR_READ_INTERVAL) {
    lastSensorReadTime = millis();
    for (int i = 0; i < 3; i++) {
      // Use the correct 0-indexed channel and function names
      busVoltage[i] = ina3221.getBusVoltage(i);
      shuntVoltage[i] = ina3221.getShuntVoltage(i); // This returns Volts
      current[i] = ina3221.getCurrentAmps(i);
      power[i] = busVoltage[i] * current[i];
    }
  }
}

// --- Data Getter Functions ---
// These functions allow other parts of the code to safely get the latest data
float get_bus_voltage(int channel) {
  if (channel >= 1 && channel <= 3) {
    return busVoltage[channel - 1];
  }
  return 0.0;
}

float get_shunt_voltage(int channel) {
  if (channel >= 1 && channel <= 3) {
    return shuntVoltage[channel - 1];
  }
  return 0.0;
}

float get_current(int channel) {
  if (channel >= 1 && channel <= 3) {
    return current[channel - 1];
  }
  return 0.0;
}

float get_power(int channel) {
  if (channel >= 1 && channel <= 3) {
    return power[channel - 1];
  }
  return 0.0;
}

