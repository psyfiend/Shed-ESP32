#include <Arduino.h>
#include <Wire.h> 
#include <INA226.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "connections.h"
#include "power_monitor.h"
#include "config.h"

// --- DECLARED AS POINTERS ---
INA226 *ina_ch1;
// INA226 *ina_ch2;
INA226 *ina_ch3;

// Variables to hold the latest sensor readings for all 3 channels
float busVoltage[3] = {0.0, 0.0, 0.0};
float current[3] = {0.0, 0.0, 0.0};
float power[3] = {0.0, 0.0, 0.0};

// Non-blocking timer for sensor reads
unsigned long lastSensorReadTime = 0;
const int SENSOR_READ_INTERVAL = 250; // Read sensors every 250ms

void setup_power_monitor() {
  Serial.println("Initializing INA226 Sensor...");

  // --- Initialize the I2C bus FIRST ---
  Wire.begin();
  
  // Initialize and calibrate our first sensor
  ina_ch1 = new INA226();
  ina_ch1->begin(INA226_CH1_ADDRESS);
  ina_ch1->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  ina_ch1->calibrate(INA226_CH1_SHUNT, 10);
  Serial.println("INA226 Channel 1 (Solar Panel) Initialized.");

//  ina_ch2 = new INA226();
//  ina_ch2->begin(INA226_CH2_ADDRESS);
//  ina_ch2->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
//  ina_ch2->calibrate(INA226_CH2_SHUNT, 10);
//  Serial.println("INA226 Channel 2 (Battery) Initialized.");

  ina_ch3 = new INA226();
  ina_ch3->begin(INA226_CH3_ADDRESS);
  ina_ch3->configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  ina_ch3->calibrate(INA226_CH3_SHUNT, 10);
  Serial.println("INA226 Channel 3 (Load) Initialized.");
}

void loop_power_monitor() {
  if (millis() - lastSensorReadTime > SENSOR_READ_INTERVAL) {
    lastSensorReadTime = millis();
    
    // --- Read from Channel 1 ---
    busVoltage[0] = ina_ch1->readBusVoltage();
    current[0] = ina_ch1->readShuntCurrent() * 1000; // Convert Amps to Milliamps
    power[0] = ina_ch1->readBusPower() * 1000;       // Convert Watts to Milliwatts ---

    JsonDocument powerCh1Payload;
    powerCh1Payload["bus_voltage"] = busVoltage[0];
    powerCh1Payload["current"] = current[0];
    powerCh1Payload["power"] = power[0];
    char buffer1[128];
    serializeJson(powerCh1Payload, buffer1);
    client.publish(MQTT_TOPIC_POWER_CH1_STATE, buffer1, true);

    // --- Read from Channel 2 ---
//    busVoltage[1] = ina_ch2->readBusVoltage();
//    current[1] = ina_ch2->readShuntCurrent() * 1000; // Convert Amps to Milliamps
//    power[1] = ina_ch2->readBusPower() * 1000;       // Convert Watts to Milliwatts ---

//    JsonDocument powerCh2Payload;
//    powerCh2Payload["bus_voltage"] = busVoltage[1];
//    powerCh2Payload["current"] = current[1];
//    powerCh2Payload["power"] = power[1];
//    char buffer2[128];
//    serializeJson(powerCh2Payload, buffer2);
//    client.publish(MQTT_TOPIC_POWER_CH2_STATE, buffer2, true);

    // --- Read from Channel 3 ---
    busVoltage[2] = ina_ch3->readBusVoltage();
    current[2] = ina_ch3->readShuntCurrent() * 1000; // Convert Amps to Milliamps
    power[2] = ina_ch3->readBusPower() * 1000;       // Convert Watts to Milliwatts ---

    JsonDocument powerCh3Payload;
    powerCh3Payload["bus_voltage"] = busVoltage[2];
    powerCh3Payload["current"] = current[2];
    powerCh3Payload["power"] = power[2];
    char buffer3[128];
    serializeJson(powerCh3Payload, buffer3);
    client.publish(MQTT_TOPIC_POWER_CH3_STATE, buffer3, true);
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

