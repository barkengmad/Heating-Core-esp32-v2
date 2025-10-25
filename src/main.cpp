// main.cpp

//Network Related Libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <time.h>
// Temp Sensor Related Libraries
#include <OneWire.h>
#include <DallasTemperature.h>

#include "web_pages.h"
#include "globals.h"
#include "pins.h"
#include "control.h"
#include "mqtt.h"
#include "config_secret.h"

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// HTML page for firmware update
const char* serverIndex = 
  "<form method='POST' action='/update' enctype='multipart/form-data'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>";


// Initialize OneWire and DallasTemperature for DS18B20
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
#define tempSensorRes 9 // set the resolution to 9 bit (Can be 9 to 12 bits .. lower is faster)
DeviceAddress tempTank1ID = { 0x28, 0xFF, 0x1E, 0xAC, 0xB3, 0x17, 0x01, 0xA9 };
DeviceAddress tempTank2ID = { 0x28, 0xFF, 0xD0, 0xAA, 0xB3, 0x17, 0x01, 0x20 };
DeviceAddress tempTank3ID = { 0x28, 0xFF, 0x26, 0xC5, 0xB3, 0x17, 0x01, 0x10 };
DeviceAddress tempTank4ID = { 0x28, 0xFF, 0x85, 0xD8, 0xB3, 0x17, 0x01, 0xC8 };
DeviceAddress tempTank5ID = { 0x28, 0xFF, 0xA5, 0xC0, 0xB3, 0x17, 0x01, 0x14 };
DeviceAddress tempRadFlowID = { 0x28, 0xFF, 0xFD, 0x21, 0x61, 0x16, 0x04, 0xD4 };
DeviceAddress tempRadReturnID = { 0x28, 0xFF, 0x72, 0xAE, 0xB3, 0x17, 0x01, 0x9E };
DeviceAddress tempUnderfloorFlowID = { 0x28, 0xFF, 0xF6, 0xAC, 0xB3, 0x17, 0x01, 0x4D };
DeviceAddress tempUnderfloorFlowMixID = { 0x28, 0xFF, 0x62, 0x1F, 0xC1, 0x17, 0x05, 0x3D }; // Shorted, so need to double check if working correcty
DeviceAddress tempUnderfloorReturn1ID = { 0x28, 0xFF, 0x90, 0xAC, 0xB3, 0x17, 0x01, 0xC0 };
DeviceAddress tempUnderfloorReturn2ID = { 0x28, 0xFF, 0x33, 0xAC, 0xB3, 0x17, 0x01, 0xBD };
DeviceAddress tempUnderfloorReturn3ID = { 0x28, 0xFF, 0x9C, 0xA9, 0xB3, 0x17, 0x01, 0x60 };
DeviceAddress tempStoveFlowID = { 0x28, 0xFF, 0x8D, 0x8C, 0x60, 0x16, 0x03, 0xA5 };
DeviceAddress tempStoveReturnID = { 0x28, 0xFF, 0x1E, 0x1E, 0xC1, 0x17, 0x05, 0xFE };
/* Sensors that are currently not connected
  DeviceAddress tempTankSolarID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
  DeviceAddress tempRadFlowMixID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
  DeviceAddress tempHotWaterID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
  DeviceAddress tempColdWaterID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
  DeviceAddress tempGasFlowID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
  DeviceAddress tempGasReturnID = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00 };
*/




// legacy schedule arrays removed



unsigned long previousMillis = 0;
const long interval = 1000;  // Interval at which to read temperature (1000 milliseconds = 1 second)


void getTemperatures() {
  sensors.requestTemperatures(); // Command all devices on bus to read temperature
  Temps::Tank1 = sensors.getTempC(tempTank1ID);
  Temps::Tank2 = sensors.getTempC(tempTank2ID);
  Temps::Tank3 = sensors.getTempC(tempTank3ID);
  Temps::Tank4 = sensors.getTempC(tempTank4ID);
  Temps::Tank5 = sensors.getTempC(tempTank5ID);
  Temps::RadFlow = sensors.getTempC(tempRadFlowID);
  Temps::RadReturn = sensors.getTempC(tempRadReturnID);
  Temps::UnderfloorFlow = sensors.getTempC(tempUnderfloorFlowID);
  Temps::UnderfloorFlowMix = sensors.getTempC(tempUnderfloorFlowMixID);
  Temps::UnderfloorReturn1 = sensors.getTempC(tempUnderfloorReturn1ID);
  Temps::UnderfloorReturn2 = sensors.getTempC(tempUnderfloorReturn2ID);
  Temps::UnderfloorReturn3 = sensors.getTempC(tempUnderfloorReturn3ID);
  Temps::StoveFlow = sensors.getTempC(tempStoveFlowID);
  Temps::StoveReturn = sensors.getTempC(tempStoveReturnID);

}

// Function to extract the hour from a time input
int getHourFromParam(AsyncWebServerRequest *request, const String& paramName) {
  // Check if the request has the specified parameter (time input)
  if (request->hasParam(paramName, true)) {
    // Retrieve the value of the parameter (time in HH:MM format)
    String time = request->getParam(paramName, true)->value();
    // Extract the hour part (first two characters) and convert it to an integer
    return time.substring(0, 2).toInt();
  }
  // Return 0 if the parameter is not found or in a wrong format
  return 0;
}

// Function to extract the minutes from a time input
int getMinuteFromParam(AsyncWebServerRequest *request, const String& paramName) {
  // Check if the request has the specified parameter (time input)
  if (request->hasParam(paramName, true)) {
    // Retrieve the value of the parameter (time in HH:MM format)
    String time = request->getParam(paramName, true)->value();
    // Extract the minute part (characters from index 3 to 4) and convert it to an integer
    return time.substring(3, 5).toInt();
  }
  // Return 0 if the parameter is not found or in a wrong format
  return 0;
}


void setup() {
  // Set up the serial interface
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Heating System Core v2 booting...");

  // Preferences
  loadPreferences();

  // Set custom hostname
  WiFi.setHostname("Heating");

  // Connect to WiFi (STA) and block until connected, printing a dot each second
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("DNS0: "); Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS1: "); Serial.println(WiFi.dnsIP(1));

  // Small delay then sync time (revert to previous working method) with fallbacks
  delay(750);
  // Apply UK TZ (GMT/BST)
  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
  tzset();

  auto tryNtp = [&](const char* s1, const char* s2, const char* s3, uint32_t timeoutMs) -> bool {
    Serial.print("Connecting to NTP "); Serial.print(s1); Serial.print(", "); Serial.print(s2);
    Serial.println("");
    configTime(0, 3600, s1, s2, s3);
    struct tm timeinfo;
    unsigned long start = millis();
    unsigned long lastDot = start;
    bool synced = getLocalTime(&timeinfo);
    while (!synced && (millis() - start) < timeoutMs) {
      if (millis() - lastDot >= 1000UL) { Serial.print("."); lastDot += 1000UL; }
      delay(25);
      synced = getLocalTime(&timeinfo);
    }
    Serial.println("");
    if (synced) {
      char buf[40];
      strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.print("Time synced: "); Serial.println(buf);
      return true;
    }
    Serial.println("Time sync failed (NTP)");
    return false;
  };

  // Prefer direct NIST IPs first (fastest, no DNS), then named servers as fallback
  // Use DNS hosts: primary + backup
  tryNtp("time.nist.gov", "pool.ntp.org", "", 30000);


// Start the MDNS responder
if (!MDNS.begin("heating")) { // So now will be hosted at heating.local
  Serial.println("Error setting up MDNS responder!");
} else {
  Serial.println("MDNS responder started for heating.local");
}

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", WebPages::getMainPageHtml());
});

// Handle Tank Threshold form submission
server.on("/setThreshold", HTTP_POST, WebPages::handleSetThreshold);
server.on("/setNightSetpoint", HTTP_POST, WebPages::handleSetNightSetpoint);
// removed standalone night window route (captured in day/night forms)

// Config page
server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/html", WebPages::getConfigPageHtml());
});

// Toggle API
server.on("/api/toggle", HTTP_POST, WebPages::handleToggleOutput);
server.on("/api/toggle", HTTP_GET, WebPages::handleToggleOutput);


// Firmware page
server.on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/html", WebPages::getFirmwarePageHtml());
});

// Backward-compatible raw update form
server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/html", WebPages::getFirmwarePageHtml());
});

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", Update.hasError() ? "FAIL" : "Updated!");
    ESP.restart();
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if (!index) {
      Serial.printf("Update Start: %s\n", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
        Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    if (final) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });


  // Start server
  server.begin();

  setupPins(); // Initialize pins

    // Start the DS18B20 sensor
  sensors.begin();

  // MQTT setup
  mqttSetup();
}

void loop() {
  
  // Daily refresh of NTP/timezone at an obscure time (03:17 local time)
    static int lastSyncYday = -1; // day-of-year last synced
    {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 17) {
          if (lastSyncYday != timeinfo.tm_yday) {
            setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
            tzset();
            configTime(0, 3600, "time.nist.gov", "pool.ntp.org");
            lastSyncYday = timeinfo.tm_yday;
          }
        }
      }
    }


  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    getTemperatures();  // Get temperatures
    controlImmersion();
    controlBoiler();
    controlRads();
    controlUFH();
    mqttPublishTemps();
    mqttPublishSwitchStates();
    mqttPublishSetpoint();
  }
}





