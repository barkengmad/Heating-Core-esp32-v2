// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <EEPROM.h>
#include <Preferences.h>
#include <DallasTemperature.h>

namespace Temps {
    extern float Tank1;  // Declaration of the global variable
    extern float Tank2;
    extern float Tank3;
    extern float Tank4;
    extern float Tank5;
    extern float RadFlow;
    extern float RadReturn;
    extern float UnderfloorFlow;
    extern float UnderfloorFlowMix;
    extern float UnderfloorReturn1;
    extern float UnderfloorReturn2;
    extern float UnderfloorReturn3;
    extern float StoveFlow;
    extern float StoveReturn;
    extern float TankSolar;
    extern float RadFlowMix;
    extern float HotWater;
    extern float ColdWater;
    extern float GasFlow;
    extern float GasReturn;
}

extern float TankSetpointC; // Tank 2 setpoint (°C)
extern unsigned long boilerActivationTime;

// Switch states (commanded by HA)
extern bool ufhKitchenOn;
extern bool ufhUtilityOn;
extern bool radBasementOn;
extern bool radLoungeOn;
extern bool radFrontBedroomOn;
extern bool radBackBedroomOn;
extern bool radOfficeOn;
extern bool radMasterBedroomOn;
extern bool bathRadOn;
extern bool bathTowelOn;
extern bool ensuiteTowelOn;

// Derived outputs
extern bool ufhPumpOn;
extern bool radsPumpOn;
extern bool radsMainValveOn;
extern bool transformer24VOn;
extern bool immersionOn;

// Timers
extern unsigned long ufhAnyValveOpenSinceMs;
extern unsigned long ufhPumpRunOnStartMs;
extern unsigned long ufhPumpLastSwitchMs;
extern unsigned long radsAnyZoneOpenSinceMs;
extern unsigned long radsPumpRunOnStartMs;
extern unsigned long radsValveRunOnStartMs; // run-on timer for main valve
extern unsigned long radsPumpLastSwitchMs;

// Per-zone warm-up timestamps (ms since boot), 0 means inactive
extern unsigned long radBasementOnSinceMs;
extern unsigned long radLoungeOnSinceMs;
extern unsigned long radFrontBedroomOnSinceMs;
extern unsigned long radBackBedroomOnSinceMs;
extern unsigned long radOfficeOnSinceMs;
extern unsigned long radMasterBedroomOnSinceMs;
extern unsigned long bathRadOnSinceMs;
extern unsigned long bathTowelOnSinceMs;
extern unsigned long ensuiteTowelOnSinceMs;

bool getCurrentTime(int &hour, int &minute);

// Preferences
extern Preferences prefs;
void loadPreferences();
void saveTankSetpoint();
void saveImmersionWindow();
void saveSetpoints();
void saveDayWindow();

// Day/Night windows and setpoints
extern int dayStartHour;
extern int dayStartMinute;
extern int dayEndHour;
extern int dayEndMinute;
extern int nightStartHour;
extern int nightStartMinute;
extern int nightEndHour;
extern int nightEndMinute;
extern float nightSetpointC;
extern float daySetpointC;
// Day/Night heat source selection (true=Boiler, false=Immersion)
extern bool daySourceBoiler;
extern bool nightSourceBoiler;

// Expose DallasTemperature bus and known sensor addresses
extern DallasTemperature sensors;
extern DeviceAddress tempTank1ID;
extern DeviceAddress tempTank2ID;
extern DeviceAddress tempTank3ID;
extern DeviceAddress tempTank4ID;
extern DeviceAddress tempTank5ID;
extern DeviceAddress tempRadFlowID;
extern DeviceAddress tempRadReturnID;
extern DeviceAddress tempUnderfloorFlowID;
extern DeviceAddress tempUnderfloorFlowMixID;
extern DeviceAddress tempUnderfloorReturn1ID;
extern DeviceAddress tempUnderfloorReturn2ID;
extern DeviceAddress tempUnderfloorReturn3ID;
extern DeviceAddress tempStoveFlowID;
extern DeviceAddress tempStoveReturnID;

// Legacy EEPROM helpers removed (moved to Preferences)


#endif
