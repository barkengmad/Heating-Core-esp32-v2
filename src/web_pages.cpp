// web_pages.cpp
#include "web_pages.h"
#include "pins.h"

namespace WebPages {
    // Function to return the main page HTML
String getMainPageHtml() {
     using namespace Temps;

    String htmlContent = "<!doctype html><html><head><meta charset='UTF-8'><title>Heating System Core v2</title>";
    htmlContent += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    htmlContent += "<style>:root{--bg:#0f172a;--card:#111827;--text:#f8fafc;--muted:#94a3b8;--primary:#3b82f6;--line:#1f2937}body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,\"Helvetica Neue\",Arial;background:var(--bg);color:var(--text)}.container{max-width:820px;margin:0 auto;padding:16px}.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;margin-bottom:12px}.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap;margin-bottom:10px}.label{color:var(--muted);min-width:120px}.btn{appearance:none;border:none;background:var(--primary);color:#fff;padding:10px 14px;border-radius:8px;cursor:pointer}.temps{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:8px}.grid{display:grid;grid-template-columns:1fr;gap:8px}.grid2{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:8px}.temp{border:1px solid var(--line);border-radius:8px;padding:8px 10px;color:var(--text)}.input{background:#0b1220;border:1px solid var(--line);border-radius:8px;padding:8px 10px;color:var(--text);width:140px}</style></head><body><div class='container'>";

    // Title and brief description
    htmlContent += "<div class='card'><h1 style='margin:0'>Heating System Core v2</h1><p style='color:var(--muted);margin:6px 0 0'>Monitor and control tanks, radiators, and UFH. Adjust setpoints and the night window below.</p></div>";

    // Header with current date/time rendered on page load
    time_t now; time(&now); struct tm timeinfo; localtime_r(&now, &timeinfo);
    char datetimeBuf[64]; strftime(datetimeBuf, sizeof(datetimeBuf), "%a %Y-%m-%d %H:%M", &timeinfo);
    htmlContent += String("<div class='card'><div class='row'><div><strong>") + String(datetimeBuf) + "</strong></div></div></div>";

    htmlContent += "<div class='card'><h2>Setpoints</h2>";
    htmlContent += "<form action='/setThreshold' method='post'><div class='row'><div class='label'>Day Setpoint (°C)</div><input class='input' type='text' name='threshold' value='" + String(daySetpointC, 1) + "' inputmode='decimal' pattern='[0-9]*[\\.,]?[0-9]+'></div><div class='row'><div class='label'>Day Source</div><label><input type='radio' name='day_src' value='boiler' " + String(daySourceBoiler?"checked":"") + "> Boiler</label><label style='margin-left:12px'><input type='radio' name='day_src' value='immersion' " + String(!daySourceBoiler?"checked":"") + "> Immersion</label></div><div class='row'><div class='label'>Day Window</div><input type='time' name='d_start' value='" + String(dayStartHour<10?"0":"") + String(dayStartHour) + ":" + String(dayStartMinute<10?"0":"") + String(dayStartMinute) + "'> <span>to</span> <input type='time' name='d_end' value='" + String(dayEndHour<10?"0":"") + String(dayEndHour) + ":" + String(dayEndMinute<10?"0":"") + String(dayEndMinute) + "'></div><div class='row'><button class='btn' type='submit'>Save Day</button></div></form>";
    htmlContent += "<form action='/setNightSetpoint' method='post'><div class='row'><div class='label'>Night Setpoint (°C)</div><input class='input' type='text' name='night' value='" + String(nightSetpointC, 1) + "' inputmode='decimal' pattern='[0-9]*[\\.,]?[0-9]+'></div><div class='row'><div class='label'>Night Source</div><label><input type='radio' name='night_src' value='boiler' " + String(nightSourceBoiler?"checked":"") + "> Boiler</label><label style='margin-left:12px'><input type='radio' name='night_src' value='immersion' " + String(!nightSourceBoiler?"checked":"") + "> Immersion</label></div><div class='row'><div class='label'>Night Window</div><input type='time' name='n_start' value='" + String(nightStartHour<10?"0":"") + String(nightStartHour) + ":" + String(nightStartMinute<10?"0":"") + String(nightStartMinute) + "'> <span>to</span> <input type='time' name='n_end' value='" + String(nightEndHour<10?"0":"") + String(nightEndHour) + ":" + String(nightEndMinute<10?"0":"") + String(nightEndMinute) + "'></div><div class='row'><button class='btn' type='submit'>Save Night</button></div></form>";
    // Removed Night Window Enabled checkbox; superseded by source selection
    htmlContent += "</div>";

    auto colorFor = [&](float c) -> String {
        float t = (c - 10.0f) / (60.0f - 10.0f);
        if (t < 0) t = 0; if (t > 1) t = 1;
        int r = (int)(t * 255);
        int b = (int)((1.0f - t) * 255);
        int g = 24; // keep dark theme subtle
        char buf[16];
        sprintf(buf, "#%02x%02x%02x", r, g, b);
        return String(buf);
    };

    htmlContent += "<div class='card'><h2>Temperatures</h2>";
    htmlContent += "<div class='grid'>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(Tank1) + "'>Tank 1: " + String(Tank1) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(Tank2) + "'>Tank 2: " + String(Tank2) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(Tank3) + "'>Tank 3: " + String(Tank3) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(Tank4) + "'>Tank 4: " + String(Tank4) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(Tank5) + "'>Tank 5: " + String(Tank5) + "&deg;C</div>";
    htmlContent += "</div>";
    htmlContent += "<div class='grid2'>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(RadFlow) + "'>Rad Flow: " + String(RadFlow) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(RadReturn) + "'>Rad Return: " + String(RadReturn) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(UnderfloorFlow) + "'>UF Flow: " + String(UnderfloorFlow) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(UnderfloorFlowMix) + "'>UF Mix: " + String(UnderfloorFlowMix) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(UnderfloorReturn1) + "'>UF Ret 1: " + String(UnderfloorReturn1) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(UnderfloorReturn2) + "'>UF Ret 2: " + String(UnderfloorReturn2) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(UnderfloorReturn3) + "'>UF Ret 3: " + String(UnderfloorReturn3) + "&deg;C</div>";
    htmlContent += "</div></div>";

    htmlContent += "<div class='card'><a href='/update' class='btn'>Firmware Update</a> <a href='/config' class='btn' style='margin-left:8px'>Config</a></div>";
    htmlContent += "</div></body></html>";
    return htmlContent;
}


String getAddressString(const DeviceAddress addr) {
    char buf[24];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
    return String(buf);
}

static String nameForAddress(const DeviceAddress addr) {
    // Compare against known IDs and return friendly names
    auto eq = [](const DeviceAddress a, const DeviceAddress b){
        for (int i=0;i<8;i++) if (a[i]!=b[i]) return false; return true;
    };
    if (eq(addr, tempTank1ID)) return "Tank 1";
    if (eq(addr, tempTank2ID)) return "Tank 2";
    if (eq(addr, tempTank3ID)) return "Tank 3";
    if (eq(addr, tempTank4ID)) return "Tank 4";
    if (eq(addr, tempTank5ID)) return "Tank 5";
    if (eq(addr, tempRadFlowID)) return "Rad Flow";
    if (eq(addr, tempRadReturnID)) return "Rad Return";
    if (eq(addr, tempUnderfloorFlowID)) return "UFH Flow";
    if (eq(addr, tempUnderfloorFlowMixID)) return "UFH Mix";
    if (eq(addr, tempUnderfloorReturn1ID)) return "UFH Return 1";
    if (eq(addr, tempUnderfloorReturn2ID)) return "UFH Return 2";
    if (eq(addr, tempUnderfloorReturn3ID)) return "UFH Return 3";
    if (eq(addr, tempStoveFlowID)) return "Stove Flow";
    if (eq(addr, tempStoveReturnID)) return "Stove Return";
    return "Unassigned";
}

String getConfigPageHtml() {
    String html = "<!doctype html><html><head><meta charset='UTF-8'><title>Config - Heating System Core v2</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>:root{--bg:#0f172a;--card:#111827;--text:#f8fafc;--muted:#94a3b8;--primary:#3b82f6;--line:#1f2937}body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,\"Helvetica Neue\",Arial;background:var(--bg);color:var(--text)}.container{max-width:980px;margin:0 auto;padding:16px}.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;margin-bottom:12px}.btn{appearance:none;border:none;background:var(--primary);color:#fff;padding:10px 14px;border-radius:8px;cursor:pointer}.table{width:100%;border-collapse:collapse}.table th,.table td{border:1px solid var(--line);padding:8px;text-align:left}.muted{color:var(--muted)}</style></head><body><div class='container'>";
    html += "<div class='card'><h1 style='margin:0'>Configuration</h1><p class='muted' style='margin:6px 0 0'>Outputs, states and temperature sensors detected on the 1-Wire bus.</p><div style='margin-top:10px'><a class='btn' href='/'>Back</a></div></div>";

    // Outputs and states
    html += "<div class='card'><h2>Outputs and States</h2><table class='table'><thead><tr><th>Name</th><th>State</th></tr></thead><tbody>";
    auto row = [&](const char* name, bool state){ html += String("<tr><td>") + name + "</td><td>" + (state?"ON":"OFF") + "</td></tr>"; };
    row("UFH Pump", ufhPumpOn);
    row("Rads Pump", radsPumpOn);
    row("Rads Main Valve", radsMainValveOn);
    row("24V Transformer", transformer24VOn);
    row("Immersion", immersionOn);
    row("Boiler (relay)", digitalRead(BOILER_RELAY_PIN)==LOW);
    row("Kitchen UFH Zone", ufhKitchenOn);
    row("Utility UFH Zone", ufhUtilityOn);
    row("Basement Rad Zone", radBasementOn);
    row("Lounge Rad Zone", radLoungeOn);
    row("Front Bedroom Rad Zone", radFrontBedroomOn);
    row("Back Bedroom Rad Zone", radBackBedroomOn);
    row("Office Rad Zone", radOfficeOn);
    row("Master Bedroom Rad Zone", radMasterBedroomOn);
    row("Bathroom Rad 24V", bathRadOn);
    row("Bathroom Towel 24V", bathTowelOn);
    row("Ensuite Towel 24V", ensuiteTowelOn);
    html += "</tbody></table></div>";

    // Enumerate sensors
    sensors.begin();
    sensors.requestTemperatures();
    int count = sensors.getDeviceCount();
    html += String("<div class='card'><h2>Temperature Sensors</h2><p class='muted'>Detected: ") + String(count) + "</p>";
    html += "<table class='table'><thead><tr><th>Name</th><th>Address</th><th>Resolution</th><th>Temp (°C)</th></tr></thead><tbody>";

    DeviceAddress addr;
    for (int i=0;i<count;i++) {
        if (sensors.getAddress(addr, i)) {
            float c = sensors.getTempC(addr);
            String name = nameForAddress(addr);
            html += String("<tr><td>") + name + "</td><td>" + getAddressString(addr) + "</td><td>" + String(sensors.getResolution(addr)) + "</td><td>" + String(c) + "</td></tr>";
        }
    }
    html += "</tbody></table></div>";

    html += "</div></body></html>";
    return html;
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


  void handleRadiatorSchedule(AsyncWebServerRequest *request) {
    request->redirect("/");
  }

void handleUnderfloorSchedule(AsyncWebServerRequest *request) { request->redirect("/"); }
  
void handleImmersionSchedule(AsyncWebServerRequest *request) { request->redirect("/"); }
void handleImmersionThreshold(AsyncWebServerRequest *request) { request->redirect("/"); }
  
void handleSetThreshold(AsyncWebServerRequest *request) {
    if (request->hasParam("threshold", true)) {
        String thresholdStr = request->getParam("threshold", true)->value();
        float newThreshold = thresholdStr.toFloat();
        daySetpointC = newThreshold;
        TankSetpointC = daySetpointC;
        saveSetpoints();
    }
    if (request->hasParam("day_src", true)) {
        String s = request->getParam("day_src", true)->value();
        daySourceBoiler = (s == "boiler");
        prefs.putBool("day_src_boiler", daySourceBoiler);
    }
    if (request->hasParam("d_start", true) && request->hasParam("d_end", true)) {
        String s = request->getParam("d_start", true)->value();
        String e = request->getParam("d_end", true)->value();
        dayStartHour = s.substring(0,2).toInt();
        dayStartMinute = s.substring(3,5).toInt();
        dayEndHour = e.substring(0,2).toInt();
        dayEndMinute = e.substring(3,5).toInt();
        saveDayWindow();
    }

    // Redirect back to the main page or a confirmation page
    request->redirect("/");
}

void handleSetNightSetpoint(AsyncWebServerRequest *request) {
    if (request->hasParam("night", true)) {
        String v = request->getParam("night", true)->value();
        nightSetpointC = v.toFloat();
        saveSetpoints();
    }
    if (request->hasParam("night_src", true)) {
        String s = request->getParam("night_src", true)->value();
        nightSourceBoiler = (s == "boiler");
        prefs.putBool("night_src_boiler", nightSourceBoiler);
    }
    if (request->hasParam("n_start", true) && request->hasParam("n_end", true)) {
        String s = request->getParam("n_start", true)->value();
        String e = request->getParam("n_end", true)->value();
        nightStartHour = s.substring(0,2).toInt();
        nightStartMinute = s.substring(3,5).toInt();
        nightEndHour = e.substring(0,2).toInt();
        nightEndMinute = e.substring(3,5).toInt();
        saveImmersionWindow();
    }
    request->redirect("/");
}

// removed standalone night window handler; saved with Night form

// removed night enabled handler

    // Definitions for other functions
}
