#ifndef PINS_H
#define PINS_H

// Pin Definitions (ESP32-S2)
// DS18B20 1-Wire DATA
#define TEMP_SENSOR_PIN 13

// Existing outputs
#define BOILER_RELAY_PIN 1
#define UFH_VALVE_PUMP_PIN 2
#define RADS_VALVE_PIN 3 // Rads main zone valve
#define KITCHEN_UFH_VALVE_PIN 4
#define UTILITY_UFH_VALVE_PIN 5

// New outputs
#define TRANSFORMER_24V_ENABLE_PIN 6
#define BATHROOM_RAD_24V_PIN 7
#define BATHROOM_TOWEL_24V_PIN 8
#define IMMERSION_RELAY_PIN 9
#define ENSUITE_TOWEL_24V_PIN 18
#define BASEMENT_RAD_ZONE_PIN 10
#define LOUNGE_RAD_ZONE_PIN 11
#define FRONT_BEDROOM_RAD_ZONE_PIN 12
#define BACK_BEDROOM_RAD_ZONE_PIN 15
#define OFFICE_RAD_ZONE_PIN 16
#define MASTER_BEDROOM_RAD_ZONE_PIN 17

// Optional: define if there is a dedicated rads pump output
// #define RADS_PUMP_PIN <pin_number>


// Function to initialize pins
void setupPins();

#endif
