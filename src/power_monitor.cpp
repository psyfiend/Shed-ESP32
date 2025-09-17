#include <Arduino.h>
#include <Wire.h> 
#include <INA226.h>
#include "power_monitor.h"
#include "config.h"

// --- DECLARED AS POINTERS ---
INA226 *ina_ch1;
// INA226 *ina_ch2;
// INA226 *ina_ch3;

// Variables to hold the latest sensor readings for all 3 channels
float busVoltage[3] = {0.0, 0.0, 0.0};
float current[3] = {0.0, 0.0, 0.0};
float power[3] = {0.0, 0.0, 0.0};

// Non-blocking timer for sensor reads
unsigned long lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL = 500; // Read sensors every 500ms

void setup_power_monitor() {
  Serial.println("Initializing INA226 Sensor...");

  // --- Initialize the I2C bus FIRST ---
  Wire.begin();
  
  // --- INSTANTIATED IN SETUP using the default constructor ---
  ina_ch1 = new INA226();

  // Initialize and calibrate our first sensor
  ina_ch1->begin(INA226_CH1_ADDRESS);
  ina_ch1->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  ina_ch1->calibrate(INA226_CH1_SHUNT, 10);

  Serial.println("INA226 Channel 1 Initialized.");
}

void loop_power_monitor() {
  if (millis() - lastSensorReadTime > SENSOR_READ_INTERVAL) {
    lastSensorReadTime = millis();
    
    busVoltage[0] = ina_ch1->readBusVoltage();
    current[0] = ina_ch1->readShuntCurrent();
    power[0] = ina_ch1->readBusPower();
  }
}

// --- Data Getter Functions ---
float get_bus_voltage(int channel) {
  if (channel >= 1 && channel <= 3) return busVoltage[channel - 1];
  return 0.0;
}

float get_current(int channel) {
  if (channel >= 1 && channel <= 3) return current[channel - 1];
  return 0.0;
}

float get_power(int channel) {
  if (channel >= 1 && channel <= 3) return power[channel - 1];
  return 0.0;
}

