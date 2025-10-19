#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

void mqttSetup();
void mqttLoop();
void mqttPublishDiscovery();
void mqttPublishAllStates(bool includeTemps);
void mqttPublishTemps();
void mqttPublishSwitchStates();
void mqttPublishSetpoint();

#endif



