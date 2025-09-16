#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

void setup_power_monitor();
void loop_power_monitor();

// --- Data Getter Functions ---
float get_bus_voltage(int channel);
float get_shunt_voltage(int channel);
float get_current(int channel);
float get_power(int channel);

#endif