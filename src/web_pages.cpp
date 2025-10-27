// web_pages.cpp
#include "web_pages.h"
#include "pins.h"
#include "control.h"
#include "mqtt.h"

namespace WebPages {
    // Function to return the main page HTML
String getMainPageHtml() {
     using namespace Temps;

    String htmlContent = "<!doctype html><html><head><meta charset='UTF-8'><title>Heating System Core v2</title>";
    htmlContent += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    htmlContent += "<style>:root{--bg:#0f172a;--card:#111827;--text:#f8fafc;--muted:#94a3b8;--primary:#3b82f6;--line:#1f2937}body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,\"Helvetica Neue\",Arial;background:var(--bg);color:var(--text)}.container{max-width:980px;margin:0 auto;padding:16px}.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;margin-bottom:12px}.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap;margin-bottom:10px}.label{color:var(--muted);min-width:120px}.btn{appearance:none;border:none;background:var(--primary);color:#fff;padding:10px 14px;border-radius:8px;cursor:pointer}.temps{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:8px}.grid{display:grid;grid-template-columns:1fr;gap:8px}.grid2{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:8px}.temp{border:1px solid var(--line);border-radius:8px;padding:8px 10px;color:var(--text)}.input{background:#0b1220;border:1px solid var(--line);border-radius:8px;padding:8px 10px;color:var(--text);width:140px}.nav{background:#0b1220;border-bottom:1px solid var(--line)}.navwrap{max-width:980px;margin:0 auto;padding:10px 16px;display:flex;align-items:center;gap:16px}.nav a{color:var(--text);text-decoration:none;padding:6px 10px;border-radius:6px}.nav a.active{background:var(--primary)}.spacer{flex:1}.navtime{color:var(--muted);font-size:12px}</style></head><body>";
    time_t now; time(&now); struct tm timeinfo; localtime_r(&now, &timeinfo);
    char navTimeBuf[64]; strftime(navTimeBuf, sizeof(navTimeBuf), "%a %Y-%m-%d %H:%M", &timeinfo);
    htmlContent += String("<div class='nav'><div class='navwrap'><div><strong>Heating Core v2</strong></div><a href='/' class='active'>Home</a><a href='/config'>Control</a><a href='/firmware'>Firmware</a><div class='spacer'></div><div class='navtime'>") + String(navTimeBuf) + "</div></div></div>";
    htmlContent += "<div class='container'>";

    // Title and brief description
    htmlContent += "<div class='card'><h1 style='margin:0'>Heating System Core v2</h1><p style='color:var(--muted);margin:6px 0 0'>Monitor and control tanks, radiators, and UFH. Adjust setpoints and the night window below.</p></div>";

    // Time moved into header

    htmlContent += "<div class='card'><h2>Setpoints</h2>";
    htmlContent += "<form action='/setThreshold' method='post'><div class='row'><div class='label'>Day Setpoint (°C)</div><input class='input' type='text' name='threshold' value='" + String(daySetpointC, 1) + "' inputmode='decimal' pattern='[0-9]*[\\.,]?[0-9]+'></div><div class='row'><div class='label'>Day Source</div><label><input type='radio' name='day_src' value='boiler' " + String(daySourceMode==SOURCE_BOILER?"checked":"") + "> Boiler</label><label style='margin-left:12px'><input type='radio' name='day_src' value='immersion' " + String(daySourceMode==SOURCE_IMMERSION?"checked":"") + "> Immersion</label><label style='margin-left:12px'><input type='radio' name='day_src' value='none' " + String(daySourceMode==SOURCE_NONE?"checked":"") + "> None</label></div><div class='row'><div class='label'>Day Window</div><input type='time' name='d_start' value='" + String(dayStartHour<10?"0":"") + String(dayStartHour) + ":" + String(dayStartMinute<10?"0":"") + String(dayStartMinute) + "'> <span>to</span> <input type='time' name='d_end' value='" + String(dayEndHour<10?"0":"") + String(dayEndHour) + ":" + String(dayEndMinute<10?"0":"") + String(dayEndMinute) + "'></div><div class='row'><button class='btn' type='submit'>Save Day</button></div></form>";
    htmlContent += "<form action='/setNightSetpoint' method='post'><div class='row'><div class='label'>Night Setpoint (°C)</div><input class='input' type='text' name='night' value='" + String(nightSetpointC, 1) + "' inputmode='decimal' pattern='[0-9]*[\\.,]?[0-9]+'></div><div class='row'><div class='label'>Night Source</div><label><input type='radio' name='night_src' value='boiler' " + String(nightSourceMode==SOURCE_BOILER?"checked":"") + "> Boiler</label><label style='margin-left:12px'><input type='radio' name='night_src' value='immersion' " + String(nightSourceMode==SOURCE_IMMERSION?"checked":"") + "> Immersion</label><label style='margin-left:12px'><input type='radio' name='night_src' value='none' " + String(nightSourceMode==SOURCE_NONE?"checked":"") + "> None</label></div><div class='row'><div class='label'>Night Window</div><input type='time' name='n_start' value='" + String(nightStartHour<10?"0":"") + String(nightStartHour) + ":" + String(nightStartMinute<10?"0":"") + String(nightStartMinute) + "'> <span>to</span> <input type='time' name='n_end' value='" + String(nightEndHour<10?"0":"") + String(nightEndHour) + ":" + String(nightEndMinute<10?"0":"") + String(nightEndMinute) + "'></div><div class='row'><button class='btn' type='submit'>Save Night</button></div></form>";
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
    htmlContent += String("<div class='temp' style='background:") + colorFor(StoveFlow) + "'>Stove Flow: " + String(StoveFlow) + "&deg;C</div>";
    htmlContent += String("<div class='temp' style='background:") + colorFor(StoveReturn) + "'>Stove Return: " + String(StoveReturn) + "&deg;C</div>";
    htmlContent += "</div></div>";

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
    html += "<style>:root{--bg:#0f172a;--card:#111827;--text:#f8fafc;--muted:#94a3b8;--primary:#3b82f6;--line:#1f2937;--success:#10b981;--warning:#f59e0b}";
    html += "body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,\"Helvetica Neue\",Arial;background:var(--bg);color:var(--text)}";
    html += ".container{max-width:980px;margin:0 auto;padding:16px}";
    html += ".card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;margin-bottom:12px}";
    html += ".btn{appearance:none;border:none;background:var(--primary);color:#fff;padding:10px 14px;border-radius:8px;cursor:pointer;text-decoration:none;display:inline-block}";
    html += ".table{width:100%;border-collapse:collapse}.table th,.table td{border:1px solid var(--line);padding:8px;text-align:left}";
    html += ".muted{color:var(--muted)}";
    html += ".nav{background:#0b1220;border-bottom:1px solid var(--line)}.navwrap{max-width:980px;margin:0 auto;padding:10px 16px;display:flex;align-items:center;gap:16px}.nav a{color:var(--text);text-decoration:none;padding:6px 10px;border-radius:6px}.nav a.active{background:var(--primary)}.spacer{flex:1}.navtime{color:var(--muted);font-size:12px}";
    html += ".toggle{position:relative;display:inline-block;width:48px;height:24px}";
    html += ".toggle input{opacity:0;width:0;height:0}";
    html += ".slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#475569;transition:.3s;border-radius:24px}";
    html += ".slider:before{position:absolute;content:'';height:18px;width:18px;left:3px;bottom:3px;background-color:white;transition:.3s;border-radius:50%}";
    html += "input:checked+.slider{background-color:var(--success)}";
    html += "input:checked+.slider:before{transform:translateX(24px)}";
    html += "input:disabled+.slider{opacity:0.5;cursor:not-allowed}";
    html += ".badge{display:inline-block;padding:2px 6px;border-radius:4px;font-size:11px;font-weight:600;margin-left:6px}";
    html += ".badge-auto{background:var(--warning);color:#000}";
    html += ".badge-countdown{background:var(--primary);color:#fff}";
    html += ".output-row{display:flex;align-items:center;justify-content:space-between;padding:8px 0;border-bottom:1px solid var(--line)}";
    html += ".output-row:last-child{border-bottom:none}";
    html += ".output-name{flex:1}";
    html += ".output-control{display:flex;align-items:center;gap:8px}";
    html += "</style></head><body>";
    time_t now2; time(&now2); struct tm timeinfo2; localtime_r(&now2, &timeinfo2);
    char navTimeBuf2[64]; strftime(navTimeBuf2, sizeof(navTimeBuf2), "%a %Y-%m-%d %H:%M", &timeinfo2);
    html += String("<div class='nav'><div class='navwrap'><div><strong>Heating Core v2</strong></div><a href='/'>Home</a><a href='/config' class='active'>Control</a><a href='/firmware'>Firmware</a><div class='spacer'></div><div class='navtime'>") + String(navTimeBuf2) + "</div></div></div>";
    html += "<div class='container'>";
    html += "<div class='container'>";
    html += "<div class='card'><h1 style='margin:0'>Control Panel</h1><p class='muted' style='margin:6px 0 0'>Interactive zone control and system status</p></div>";

    // UFH Zones
    html += "<div class='card'><h2>Underfloor Heating</h2>";
    auto toggleRow = [&](const char* id, const char* name, bool state, bool readOnly, const char* badge, int countdown) {
        html += String("<div class='output-row'><div class='output-name'>") + name;
        if (badge && badge[0]) html += String("<span class='badge badge-auto'>") + badge + "</span>";
        if (countdown > 0) html += String("<span class='badge badge-countdown'>") + String(countdown) + " s</span>";
        html += "</div><div class='output-control'><label class='toggle'><input type='checkbox' id='" + String(id) + "' " + (state?"checked":"") + (readOnly?" disabled":"") + " onchange='toggleOutput(\"" + String(id) + "\")'><span class='slider'></span></label></div></div>";
    };
    
    int ufhWarmup = getUFHWarmupCountdown();
    int ufhRunon = getUFHRunonCountdown();
    toggleRow("ufh_kitchen", "Kitchen UFH Valve", ufhKitchenOn, false, nullptr, ufhWarmup && ufhKitchenOn ? ufhWarmup : 0);
    toggleRow("ufh_utility", "Utility UFH Valve", ufhUtilityOn, false, nullptr, ufhWarmup && ufhUtilityOn ? ufhWarmup : 0);
    toggleRow("ufh_mech", "UFH Valve (mechanical)", ufhPumpOn, true, "AUTO", ufhRunon);
    html += "</div>";

    // Radiator Zones
    html += "<div class='card'><h2>Radiators</h2>";
    // Per-zone warm-up countdowns use since timestamps
    auto zoneCountdown = [&](bool on, unsigned long sinceMs){
        if (!on || sinceMs==0) return 0;
        unsigned long elapsed = millis() - sinceMs;
        if (elapsed >= RADS_WARMUP_MS) return 0;
        return (int)((RADS_WARMUP_MS - elapsed)/1000);
    };
    int cd_basement = zoneCountdown(radBasementOn, radBasementOnSinceMs);
    int cd_lounge = zoneCountdown(radLoungeOn, radLoungeOnSinceMs);
    int cd_front = zoneCountdown(radFrontBedroomOn, radFrontBedroomOnSinceMs);
    int cd_back = zoneCountdown(radBackBedroomOn, radBackBedroomOnSinceMs);
    int cd_office = zoneCountdown(radOfficeOn, radOfficeOnSinceMs);
    int cd_master = zoneCountdown(radMasterBedroomOn, radMasterBedroomOnSinceMs);
    toggleRow("rad_basement", "Basement Rad", radBasementOn, false, nullptr, cd_basement);
    toggleRow("rad_lounge", "Lounge Rad", radLoungeOn, false, nullptr, cd_lounge);
    toggleRow("rad_front_bed", "Front Bedroom Rad", radFrontBedroomOn, false, nullptr, cd_front);
    toggleRow("rad_back_bed", "Back Bedroom Rad", radBackBedroomOn, false, nullptr, cd_back);
    toggleRow("rad_office", "Office Rad", radOfficeOn, false, nullptr, cd_office);
    toggleRow("rad_master_bed", "Master Bedroom Rad", radMasterBedroomOn, false, nullptr, cd_master);
    // Main valve shows no countdown; it is gated by earliest zone internally
    toggleRow("rads_main", "Rads Main Valve", radsMainValveOn, true, "AUTO", 0);
    html += "</div>";

    // 24V Zones
    html += "<div class='card'><h2>24V Towel Rails</h2>";
    int cd_bath = zoneCountdown(bathRadOn, bathRadOnSinceMs);
    int cd_towel = zoneCountdown(bathTowelOn, bathTowelOnSinceMs);
    int cd_ensuite = zoneCountdown(ensuiteTowelOn, ensuiteTowelOnSinceMs);
    toggleRow("bath_rad", "Bathroom Rad", bathRadOn, false, nullptr, cd_bath);
    toggleRow("bath_towel", "Bathroom Towel", bathTowelOn, false, nullptr, cd_towel);
    toggleRow("ensuite_towel", "Ensuite Towel", ensuiteTowelOn, false, nullptr, cd_ensuite);
    toggleRow("transformer_24v", "24V Transformer", transformer24VOn, true, "AUTO", 0);
    html += "</div>";

    // Boiler & Immersion
    html += "<div class='card'><h2>Heating Sources</h2>";
    toggleRow("boiler", "Boiler", digitalRead(BOILER_RELAY_PIN)==LOW, true, "AUTO", 0);
    toggleRow("immersion", "Immersion Heater", immersionOn, true, "AUTO", 0);
    html += "</div>";

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

    // JavaScript for toggle handling (no auto-refresh to avoid race); reload briefly after success
    html += "<script>";
    html += "function toggleOutput(id){";
    html += "fetch('/api/toggle?id='+id,{method:'GET',cache:'no-store'}).then(r=>r.json()).then(d=>{";
    html += "if(d.success){setTimeout(()=>location.reload(),200);}else{alert('Failed to toggle');}";
    html += "}).catch(e=>{alert('Error: '+e)});";
    html += "}";
    html += "</script>";

    html += "</div></div></body></html>";
    return html;
}

String getFirmwarePageHtml() {
    String html = "<!doctype html><html><head><meta charset='UTF-8'><title>Firmware - Heating System Core v2</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>:root{--bg:#0f172a;--card:#111827;--text:#f8fafc;--muted:#94a3b8;--primary:#3b82f6;--line:#1f2937}.nav{background:#0b1220;border-bottom:1px solid var(--line)}.navwrap{max-width:980px;margin:0 auto;padding:10px 16px;display:flex;align-items:center;gap:16px}.nav a{color:var(--text);text-decoration:none;padding:6px 10px;border-radius:6px}.nav a.active{background:var(--primary)}.spacer{flex:1}body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,\"Helvetica Neue\",Arial;background:var(--bg);color:var(--text)}.container{max-width:980px;margin:0 auto;padding:16px}.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:16px;margin-bottom:12px}.btn{appearance:none;border:none;background:var(--primary);color:#fff;padding:10px 14px;border-radius:8px;cursor:pointer}</style></head><body>";
    time_t now3; time(&now3); struct tm timeinfo3; localtime_r(&now3, &timeinfo3);
    char navTimeBuf3[64]; strftime(navTimeBuf3, sizeof(navTimeBuf3), "%a %Y-%m-%d %H:%M", &timeinfo3);
    html += String("<div class='nav'><div class='navwrap'><div><strong>Heating Core v2</strong></div><a href='/'>Home</a><a href='/config'>Control</a><a href='/firmware' class='active'>Firmware</a><div class='spacer'></div><div class='navtime'>") + String(navTimeBuf3) + "</div></div></div>";
    html += "<div class='container'>";
    html += "<div class='card'><h1 style='margin:0'>Firmware Update</h1><p class='muted' style='margin:6px 0 0'>Upload new firmware using the form below.</p></div>";
    html += String("<div class='card'>") + String(
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Update' class='btn' style='margin-left:8px'>"
      "</form>") + "</div>";
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
        if (s == "boiler") daySourceMode = SOURCE_BOILER; else if (s == "immersion") daySourceMode = SOURCE_IMMERSION; else daySourceMode = SOURCE_NONE;
        saveSourceModes();
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
        if (s == "boiler") nightSourceMode = SOURCE_BOILER; else if (s == "immersion") nightSourceMode = SOURCE_IMMERSION; else nightSourceMode = SOURCE_NONE;
        saveSourceModes();
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

void handleToggleOutput(AsyncWebServerRequest *request) {
    if (!request->hasParam("id")) {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"missing id\"}");
        return;
    }
    
    String id = request->getParam("id")->value();
    bool success = false;
    unsigned long now = millis();
    
    // Map ID to zone toggles
    if (id == "ufh_kitchen") { ufhKitchenOn = !ufhKitchenOn; success = true; }
    else if (id == "ufh_utility") { ufhUtilityOn = !ufhUtilityOn; success = true; }
    else if (id == "rad_basement") { radBasementOn = !radBasementOn; radBasementOnSinceMs = radBasementOn ? now : 0; success = true; }
    else if (id == "rad_lounge") { radLoungeOn = !radLoungeOn; radLoungeOnSinceMs = radLoungeOn ? now : 0; success = true; }
    else if (id == "rad_front_bed") { radFrontBedroomOn = !radFrontBedroomOn; radFrontBedroomOnSinceMs = radFrontBedroomOn ? now : 0; success = true; }
    else if (id == "rad_back_bed") { radBackBedroomOn = !radBackBedroomOn; radBackBedroomOnSinceMs = radBackBedroomOn ? now : 0; success = true; }
    else if (id == "rad_office") { radOfficeOn = !radOfficeOn; radOfficeOnSinceMs = radOfficeOn ? now : 0; success = true; }
    else if (id == "rad_master_bed") { radMasterBedroomOn = !radMasterBedroomOn; radMasterBedroomOnSinceMs = radMasterBedroomOn ? now : 0; success = true; }
    else if (id == "bath_rad") { bathRadOn = !bathRadOn; bathRadOnSinceMs = bathRadOn ? now : 0; success = true; }
    else if (id == "bath_towel") { bathTowelOn = !bathTowelOn; bathTowelOnSinceMs = bathTowelOn ? now : 0; success = true; }
    else if (id == "ensuite_towel") { ensuiteTowelOn = !ensuiteTowelOn; ensuiteTowelOnSinceMs = ensuiteTowelOn ? now : 0; success = true; }
    // Auto outputs are read-only, ignore toggle attempts
    
    if (success) {
        Serial.print("Toggle: "); Serial.print(id); Serial.print(" => ");
        if (id == "rad_basement") Serial.println(radBasementOn ? "ON" : "OFF");
        else if (id == "rad_lounge") Serial.println(radLoungeOn ? "ON" : "OFF");
        else if (id == "rad_front_bed") Serial.println(radFrontBedroomOn ? "ON" : "OFF");
        else if (id == "rad_back_bed") Serial.println(radBackBedroomOn ? "ON" : "OFF");
        else if (id == "rad_office") Serial.println(radOfficeOn ? "ON" : "OFF");
        else if (id == "rad_master_bed") Serial.println(radMasterBedroomOn ? "ON" : "OFF");
        else if (id == "bath_rad") Serial.println(bathRadOn ? "ON" : "OFF");
        else if (id == "bath_towel") Serial.println(bathTowelOn ? "ON" : "OFF");
        else if (id == "ensuite_towel") Serial.println(ensuiteTowelOn ? "ON" : "OFF");
        mqttPublishSwitchStates();
        request->send(200, "application/json", "{\"success\":true}");
    } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"invalid id or read-only\"}");
    }
}

    // Definitions for other functions
}
