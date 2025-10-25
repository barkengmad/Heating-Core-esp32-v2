#include "mqtt.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "pins.h"
#include "config_secret.h"

static AsyncMqttClient mqtt;
// simple reconnect backoff
static unsigned long lastMqttAttemptMs = 0;

static const char* DEVICE_ID = "heating-core-s2";
static const char* STATUS_TOPIC = "heating-core-s2/status";

// MQTT broker config (adjust as needed)
static const char* MQTT_HOST_ADDR = MQTT_HOST;
static const uint16_t MQTT_PORT_NUM = MQTT_PORT;

static void connectToMqtt() {
  mqtt.connect();
}

static void connectToWifi() {
  // WiFi already handled in main
}

static String topicOf(const String &path) {
  return path;
}

static void publishRetained(const String &topic, const String &payload) {
  mqtt.publish(topic.c_str(), 1, true, payload.c_str());
}

static void publishAvailability(bool online) {
  publishRetained(STATUS_TOPIC, online ? "online" : "offline");
}

static void subscribeCommands() {
  // Switch commands
  const char* cmds[] = {
    "heating-core-s2/switch/kitchen_ufh/set",
    "heating-core-s2/switch/utility_ufh/set",
    "heating-core-s2/switch/rad_basement/set",
    "heating-core-s2/switch/rad_lounge/set",
    "heating-core-s2/switch/rad_front_bed/set",
    "heating-core-s2/switch/rad_back_bed/set",
    "heating-core-s2/switch/rad_office/set",
    "heating-core-s2/switch/rad_master_bed/set",
    "heating-core-s2/switch/bathroom_rad/set",
    "heating-core-s2/switch/bathroom_towel/set",
    "heating-core-s2/switch/ensuite_towel/set",
    // Optional manual overrides (keep boiler/transformer; rads_main_valve and ufh_mech are AUTO)
    "heating-core-s2/switch/boiler/set",
    "heating-core-s2/switch/transformer_24v/set",
    // Setpoint
    "heating-core-s2/number/tank2_setpoint/set"
  };
  for (auto t : cmds) mqtt.subscribe(t, 1);
}

static void publishDiscoveryForSensor(const char* name, const char* unique, const char* state_topic) {
  StaticJsonDocument<512> doc;
  char buf[512];
  doc["name"] = name;
  doc["uniq_id"] = unique;
  doc["stat_t"] = state_topic;
  doc["unit_of_meas"] = "°C";
  doc["dev"]["ids"][0] = DEVICE_ID;
  doc["dev"]["name"] = "Heating Core";
  doc["dev"]["mf"] = "Custom";
  doc["dev"]["mdl"] = "ESP32-S2";
  doc["avty_t"] = STATUS_TOPIC;
  size_t n = serializeJson(doc, buf, sizeof(buf));
  String topic = String("homeassistant/sensor/") + DEVICE_ID + "/" + unique + "/config";
  mqtt.publish(topic.c_str(), 1, true, buf, n);
}

static void publishDiscoveryForSwitch(const char* name, const char* unique, const char* cmd, const char* stat) {
  StaticJsonDocument<512> doc;
  char buf[512];
  doc["name"] = name;
  doc["uniq_id"] = unique;
  doc["cmd_t"] = cmd;
  doc["stat_t"] = stat;
  doc["dev"]["ids"][0] = DEVICE_ID;
  doc["dev"]["name"] = "Heating Core";
  doc["dev"]["mf"] = "Custom";
  doc["dev"]["mdl"] = "ESP32-S2";
  doc["avty_t"] = STATUS_TOPIC;
  size_t n = serializeJson(doc, buf, sizeof(buf));
  String topic = String("homeassistant/switch/") + DEVICE_ID + "/" + unique + "/config";
  mqtt.publish(topic.c_str(), 1, true, buf, n);
}

static void publishDiscoveryForNumber(const char* name, const char* uniq, const char* cmd_t, const char* stat_t) {
  StaticJsonDocument<512> doc;
  char buf[512];
  doc["name"] = name;
  doc["uniq_id"] = uniq;
  doc["cmd_t"] = cmd_t;
  doc["stat_t"] = stat_t;
  doc["min"] = 30;
  doc["max"] = 80;
  doc["step"] = 0.5;
  doc["unit_of_meas"] = "°C";
  doc["dev"]["ids"][0] = DEVICE_ID;
  doc["dev"]["name"] = "Heating Core";
  doc["dev"]["mf"] = "Custom";
  doc["dev"]["mdl"] = "ESP32-S2";
  doc["avty_t"] = STATUS_TOPIC;
  size_t n = serializeJson(doc, buf, sizeof(buf));
  String topic = String("homeassistant/number/") + DEVICE_ID + "/" + uniq + "/config";
  mqtt.publish(topic.c_str(), 1, true, buf, n);
}

void mqttPublishDiscovery() {
  // Temperatures
  publishDiscoveryForSensor("Tank 1", "tank1", "heating-core-s2/sensor/tank1/state");
  publishDiscoveryForSensor("Tank 2", "tank2", "heating-core-s2/sensor/tank2/state");
  publishDiscoveryForSensor("Tank 3", "tank3", "heating-core-s2/sensor/tank3/state");
  publishDiscoveryForSensor("Tank 4", "tank4", "heating-core-s2/sensor/tank4/state");
  publishDiscoveryForSensor("Tank 5", "tank5", "heating-core-s2/sensor/tank5/state");
  publishDiscoveryForSensor("Rad Flow", "rad_flow", "heating-core-s2/sensor/rad_flow/state");
  publishDiscoveryForSensor("Rad Return", "rad_return", "heating-core-s2/sensor/rad_return/state");
  publishDiscoveryForSensor("UFH Flow", "ufh_flow", "heating-core-s2/sensor/ufh_flow/state");
  publishDiscoveryForSensor("UFH Mix", "ufh_mix", "heating-core-s2/sensor/ufh_mix/state");
  publishDiscoveryForSensor("UFH Return 1", "ufh_ret1", "heating-core-s2/sensor/ufh_ret1/state");
  publishDiscoveryForSensor("UFH Return 2", "ufh_ret2", "heating-core-s2/sensor/ufh_ret2/state");
  publishDiscoveryForSensor("UFH Return 3", "ufh_ret3", "heating-core-s2/sensor/ufh_ret3/state");

  // Switches (zones and valves)
  publishDiscoveryForSwitch("Kitchen UFH", "kitchen_ufh", "heating-core-s2/switch/kitchen_ufh/set", "heating-core-s2/switch/kitchen_ufh/state");
  publishDiscoveryForSwitch("Utility UFH", "utility_ufh", "heating-core-s2/switch/utility_ufh/set", "heating-core-s2/switch/utility_ufh/state");
  publishDiscoveryForSwitch("Basement Rad", "rad_basement", "heating-core-s2/switch/rad_basement/set", "heating-core-s2/switch/rad_basement/state");
  publishDiscoveryForSwitch("Lounge Rad", "rad_lounge", "heating-core-s2/switch/rad_lounge/set", "heating-core-s2/switch/rad_lounge/state");
  publishDiscoveryForSwitch("Front Bedroom Rad", "rad_front_bed", "heating-core-s2/switch/rad_front_bed/set", "heating-core-s2/switch/rad_front_bed/state");
  publishDiscoveryForSwitch("Back Bedroom Rad", "rad_back_bed", "heating-core-s2/switch/rad_back_bed/set", "heating-core-s2/switch/rad_back_bed/state");
  publishDiscoveryForSwitch("Office Rad", "rad_office", "heating-core-s2/switch/rad_office/set", "heating-core-s2/switch/rad_office/state");
  publishDiscoveryForSwitch("Master Bedroom Rad", "rad_master_bed", "heating-core-s2/switch/rad_master_bed/set", "heating-core-s2/switch/rad_master_bed/state");
  publishDiscoveryForSwitch("Bathroom Rad 24V", "bathroom_rad", "heating-core-s2/switch/bathroom_rad/set", "heating-core-s2/switch/bathroom_rad/state");
  publishDiscoveryForSwitch("Bathroom Towel 24V", "bathroom_towel", "heating-core-s2/switch/bathroom_towel/set", "heating-core-s2/switch/bathroom_towel/state");
  publishDiscoveryForSwitch("Ensuite Towel 24V", "ensuite_towel", "heating-core-s2/switch/ensuite_towel/set", "heating-core-s2/switch/ensuite_towel/state");
  // AUTO-only entities removed from command exposure
  publishDiscoveryForSwitch("Boiler", "boiler", "heating-core-s2/switch/boiler/set", "heating-core-s2/switch/boiler/state");
  publishDiscoveryForSwitch("24V Transformer", "transformer_24v", "heating-core-s2/switch/transformer_24v/set", "heating-core-s2/switch/transformer_24v/state");

  // Numbers: expose Day/Night setpoints (Tank2 legacy removed)
  publishDiscoveryForNumber(
    "Day Setpoint",
    "day_setpoint",
    "heating-core-s2/number/day_setpoint/set",
    "heating-core-s2/number/day_setpoint/state"
  );
  publishDiscoveryForNumber(
    "Night Setpoint",
    "night_setpoint",
    "heating-core-s2/number/night_setpoint/set",
    "heating-core-s2/number/night_setpoint/state"
  );
}

void mqttPublishTemps() {
  publishRetained("heating-core-s2/sensor/tank1/state", String(Temps::Tank1));
  publishRetained("heating-core-s2/sensor/tank2/state", String(Temps::Tank2));
  publishRetained("heating-core-s2/sensor/tank3/state", String(Temps::Tank3));
  publishRetained("heating-core-s2/sensor/tank4/state", String(Temps::Tank4));
  publishRetained("heating-core-s2/sensor/tank5/state", String(Temps::Tank5));
  publishRetained("heating-core-s2/sensor/rad_flow/state", String(Temps::RadFlow));
  publishRetained("heating-core-s2/sensor/rad_return/state", String(Temps::RadReturn));
  publishRetained("heating-core-s2/sensor/ufh_flow/state", String(Temps::UnderfloorFlow));
  publishRetained("heating-core-s2/sensor/ufh_mix/state", String(Temps::UnderfloorFlowMix));
  publishRetained("heating-core-s2/sensor/ufh_ret1/state", String(Temps::UnderfloorReturn1));
  publishRetained("heating-core-s2/sensor/ufh_ret2/state", String(Temps::UnderfloorReturn2));
  publishRetained("heating-core-s2/sensor/ufh_ret3/state", String(Temps::UnderfloorReturn3));
}

static void publishSwitch(const char* id, bool on) {
  publishRetained(String("heating-core-s2/switch/") + id + "/state", on ? "ON" : "OFF");
}

void mqttPublishSwitchStates() {
  publishSwitch("kitchen_ufh", ufhKitchenOn);
  publishSwitch("utility_ufh", ufhUtilityOn);
  publishSwitch("rad_basement", radBasementOn);
  publishSwitch("rad_lounge", radLoungeOn);
  publishSwitch("rad_front_bed", radFrontBedroomOn);
  publishSwitch("rad_back_bed", radBackBedroomOn);
  publishSwitch("rad_office", radOfficeOn);
  publishSwitch("rad_master_bed", radMasterBedroomOn);
  publishSwitch("bathroom_rad", bathRadOn);
  publishSwitch("bathroom_towel", bathTowelOn);
  publishSwitch("ensuite_towel", ensuiteTowelOn);
  // Keep publishing state-only topics if you want read-only entities via templates, else remove
  publishSwitch("boiler", digitalRead(BOILER_RELAY_PIN)==LOW);
  publishSwitch("transformer_24v", transformer24VOn);
  publishSwitch("immersion", immersionOn);
}

void mqttPublishSetpoint() {
  publishRetained("heating-core-s2/number/day_setpoint/state", String(daySetpointC));
  publishRetained("heating-core-s2/number/night_setpoint/state", String(nightSetpointC));
}

void mqttPublishAllStates(bool includeTemps) {
  mqttPublishSetpoint();
  mqttPublishSwitchStates();
  if (includeTemps) mqttPublishTemps();
}

// Message handler
static void handleMessage(char* topic, char* payload, AsyncMqttClientMessageProperties props, size_t len, size_t, size_t) {
  String t(topic);
  String msg = String(payload).substring(0, len);
  bool turnOn = (msg == "ON" || msg == "on" || msg == "1");

  // Ignore retained commands on "/set" topics to prevent overwriting local UI state on reconnect
  if (props.retain && t.endsWith("/set")) {
    return;
  }

  auto applySwitch = [&](const char* key, bool &var) {
    if (t == String("heating-core-s2/switch/") + key + "/set") {
      var = turnOn;
      mqttPublishSwitchStates();
    }
  };

  applySwitch("kitchen_ufh", ufhKitchenOn);
  applySwitch("utility_ufh", ufhUtilityOn);
  applySwitch("rad_basement", radBasementOn);
  applySwitch("rad_lounge", radLoungeOn);
  applySwitch("rad_front_bed", radFrontBedroomOn);
  applySwitch("rad_back_bed", radBackBedroomOn);
  applySwitch("rad_office", radOfficeOn);
  applySwitch("rad_master_bed", radMasterBedroomOn);
  applySwitch("bathroom_rad", bathRadOn);
  applySwitch("bathroom_towel", bathTowelOn);
  applySwitch("ensuite_towel", ensuiteTowelOn);

  // rads_main_valve and ufh_mech are AUTO-only; ignore direct set requests
  if (t == "heating-core-s2/switch/boiler/set") {
    digitalWrite(BOILER_RELAY_PIN, turnOn ? LOW : HIGH);
  }
  if (t == "heating-core-s2/switch/transformer_24v/set") {
    transformer24VOn = turnOn;
  }
  if (t == "heating-core-s2/switch/immersion/set") {
    immersionOn = turnOn;
    digitalWrite(IMMERSION_RELAY_PIN, immersionOn ? HIGH : LOW);
  }
  if (t == "heating-core-s2/number/day_setpoint/set") {
    daySetpointC = msg.toFloat();
    saveSetpoints();
    mqttPublishSetpoint();
  }
  if (t == "heating-core-s2/number/night_setpoint/set") {
    nightSetpointC = msg.toFloat();
    saveSetpoints();
    mqttPublishSetpoint();
  }
}

void mqttSetup() {
  mqtt.onConnect([](bool sessionPresent){
    publishAvailability(true);
    subscribeCommands();
    mqttPublishDiscovery();
    mqttPublishAllStates(true);
  });
  mqtt.onDisconnect([](AsyncMqttClientDisconnectReason){
    publishAvailability(false);
    // reconnect handled by library or timer
  });
  mqtt.onMessage(handleMessage);
  mqtt.setServer(MQTT_HOST_ADDR, MQTT_PORT_NUM);
  mqtt.setWill(STATUS_TOPIC, 1, true, "offline");
  connectToMqtt();
}

void mqttLoop() {
  if (WiFi.isConnected() && !mqtt.connected()) {
    unsigned long now = millis();
    if (now - lastMqttAttemptMs > 5000) {
      lastMqttAttemptMs = now;
      mqtt.connect();
    }
  }
}


