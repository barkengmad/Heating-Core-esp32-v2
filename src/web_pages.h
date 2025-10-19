// web_pages.h
#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "globals.h"  // Include the header with the global variable

namespace WebPages {
  String getMainPageHtml();
  String getConfigPageHtml();
  void handleRadiatorSchedule(AsyncWebServerRequest *request);
  void handleUnderfloorSchedule(AsyncWebServerRequest *request);
  void handleSetThreshold(AsyncWebServerRequest *request);
  void handleSetNightSetpoint(AsyncWebServerRequest *request);
  void handleSetNightWindow(AsyncWebServerRequest *request);
  void handleImmersionSchedule(AsyncWebServerRequest *request);
  void handleImmersionThreshold(AsyncWebServerRequest *request);
  int getHourFromParam(AsyncWebServerRequest *request, const String &paramName);
  int getMinuteFromParam(AsyncWebServerRequest *request, const String &paramName);
  // Add other functions
}





#endif
