// globals.cpp
#include "globals.h"
#include <time.h>

namespace Temps {
    float Tank1;
    float Tank2;
    float Tank3;
    float Tank4;
    float Tank5;
    float RadFlow;
    float RadReturn;
    float UnderfloorFlow;
    float UnderfloorFlowMix;
    float UnderfloorReturn1;
    float UnderfloorReturn2;
    float UnderfloorReturn3;
    float StoveFlow;
    float StoveReturn;
    float TankSolar;
    float RadFlowMix;
    float HotWater;
    float ColdWater;
    float GasFlow;
    float GasReturn;
}

Preferences prefs;

float TankSetpointC = 50.0f;
unsigned long boilerActivationTime = 0;

bool ufhKitchenOn = false;
bool ufhUtilityOn = false;
bool radBasementOn = false;
bool radLoungeOn = false;
bool radFrontBedroomOn = false;
bool radBackBedroomOn = false;
bool radOfficeOn = false;
bool radMasterBedroomOn = false;
bool bathRadOn = false;
bool bathTowelOn = false;
bool ensuiteTowelOn = false;

bool ufhPumpOn = false;
bool radsPumpOn = false;
bool radsMainValveOn = false;
bool transformer24VOn = false;
bool immersionOn = false;

unsigned long ufhAnyValveOpenSinceMs = 0;
unsigned long ufhPumpRunOnStartMs = 0;
unsigned long ufhPumpLastSwitchMs = 0;
unsigned long radsAnyZoneOpenSinceMs = 0;
unsigned long radsPumpRunOnStartMs = 0;
unsigned long radsPumpLastSwitchMs = 0;

bool getCurrentTime(int &hour, int &minute) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        hour = timeinfo.tm_hour;
        minute = timeinfo.tm_min;
        return true;
    }
    return false;
}

int nightStartHour = 0;
int nightStartMinute = 0;
int nightEndHour = 7;
int nightEndMinute = 0;
int dayStartHour = 7;
int dayStartMinute = 0;
int dayEndHour = 23;
int dayEndMinute = 59;
float nightSetpointC = 50.0f;
float daySetpointC = 45.0f;
bool daySourceBoiler = true;  // Day default to boiler
bool nightSourceBoiler = false; // Night default to immersion

void loadPreferences() {
    prefs.begin("heating", false);
    TankSetpointC = prefs.getFloat("tank_sp", 50.0f);
    // For backwards compatibility, TankSetpointC is the day setpoint
    daySetpointC = prefs.getFloat("day_sp", TankSetpointC);
    nightSetpointC = prefs.getFloat("night_sp", 50.0f);
    nightStartHour = prefs.getInt("n_sh", 0);
    nightStartMinute = prefs.getInt("n_sm", 0);
    nightEndHour = prefs.getInt("n_eh", 7);
    nightEndMinute = prefs.getInt("n_em", 0);
    dayStartHour = prefs.getInt("d_sh", 7);
    dayStartMinute = prefs.getInt("d_sm", 0);
    dayEndHour = prefs.getInt("d_eh", 23);
    dayEndMinute = prefs.getInt("d_em", 59);
    daySourceBoiler = prefs.getBool("day_src_boiler", true);
    nightSourceBoiler = prefs.getBool("night_src_boiler", false);
}

void saveTankSetpoint() {
    prefs.putFloat("tank_sp", TankSetpointC);
}

void saveImmersionWindow() {
    prefs.putInt("n_sh", nightStartHour);
    prefs.putInt("n_sm", nightStartMinute);
    prefs.putInt("n_eh", nightEndHour);
    prefs.putInt("n_em", nightEndMinute);
}

void saveDayWindow() {
    prefs.putInt("d_sh", dayStartHour);
    prefs.putInt("d_sm", dayStartMinute);
    prefs.putInt("d_eh", dayEndHour);
    prefs.putInt("d_em", dayEndMinute);
}

void saveSetpoints() {
    prefs.putFloat("day_sp", daySetpointC);
    prefs.putFloat("night_sp", nightSetpointC);
}
