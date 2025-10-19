// config.h
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials - DO NOT COMMIT TO GIT
// Copy this file to config_secret.h and update with your actual credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// MQTT broker settings - Update these in config_secret.h
const char* MQTT_HOST = "YOUR_MQTT_BROKER_IP";
const uint16_t MQTT_PORT = 1883;

#endif
