// control.cpp
#include "globals.h"
#include "control.h"
#include "pins.h"

// Boiler control with hysteresis on Tank2
void controlBoiler() {
    // During night window, boiler should be OFF (immersion takes over)
    int h=0,m=0; getCurrentTime(h,m);
    int current = h*60+m;
    int nStart = nightStartHour*60 + nightStartMinute;
    int nEnd = nightEndHour*60 + nightEndMinute;
    int dStart = dayStartHour*60 + dayStartMinute;
    int dEnd = dayEndHour*60 + dayEndMinute;
    bool inNight = (nStart <= nEnd) ? (current >= nStart && current < nEnd)
                                    : (current >= nStart || current < nEnd);
    bool inDay = (dStart <= dEnd) ? (current >= dStart && current < dEnd)
                                  : (current >= dStart || current < dEnd);

    if (!inNight && !inDay) {
        // No active window → ensure boiler off
        if ((millis() - boilerActivationTime) >= PUMP_MIN_TOGGLE_MS) {
            digitalWrite(BOILER_RELAY_PIN, HIGH);
        }
        return;
    }

    // Determine active window with Day priority on overlap
    bool activeIsDay = false;
    if (inDay && !inNight) activeIsDay = true;
    else if (inNight && !inDay) activeIsDay = false;
    else if (inDay && inNight) activeIsDay = true; // Day wins on overlap

    float targetSp = activeIsDay ? daySetpointC : nightSetpointC;
    TankSetpointC = targetSp; // reflect active target

    bool useBoiler = activeIsDay ? daySourceBoiler : nightSourceBoiler;
    bool demandHeat = useBoiler && (Temps::Tank2 < (targetSp - HYSTERESIS_C));
    bool stopHeat = (Temps::Tank2 > (targetSp + HYSTERESIS_C));

    static bool boilerIsOn = false;

    if (!useBoiler) {
        // If not using boiler for this window, force boiler off
        if ((millis() - boilerActivationTime) >= PUMP_MIN_TOGGLE_MS) {
            digitalWrite(BOILER_RELAY_PIN, HIGH);
            boilerIsOn = false;
        }
        return;
    }

    if (demandHeat) {
        digitalWrite(BOILER_RELAY_PIN, LOW);
        boilerIsOn = true;
        boilerActivationTime = millis();
    } else if (stopHeat) {
        // enforce minimal on-time of 10s to avoid chatter
        if ((millis() - boilerActivationTime) >= PUMP_MIN_TOGGLE_MS) {
            digitalWrite(BOILER_RELAY_PIN, HIGH);
            boilerIsOn = false;
        }
    }
}

// UFH control: valves immediate, pump after 3 min warmup, 60s run-on
void controlUFH() {
    bool anyValve = (ufhKitchenOn || ufhUtilityOn);

    // Drive valves (active-LOW outputs)
    digitalWrite(KITCHEN_UFH_VALVE_PIN, ufhKitchenOn ? LOW : HIGH);
    digitalWrite(UTILITY_UFH_VALVE_PIN, ufhUtilityOn ? LOW : HIGH);

    unsigned long now = millis();
    if (anyValve) {
        if (ufhAnyValveOpenSinceMs == 0) {
            ufhAnyValveOpenSinceMs = now;
        }
        ufhPumpRunOnStartMs = 0; // cancel run-on countdown
        if ((now - ufhAnyValveOpenSinceMs) >= UFH_VALVE_WARMUP_MS) {
            if (!ufhPumpOn && (now - ufhPumpLastSwitchMs) >= PUMP_MIN_TOGGLE_MS) {
                ufhPumpOn = true;
                ufhPumpLastSwitchMs = now;
            }
        }
    } else {
        ufhAnyValveOpenSinceMs = 0;
        if (ufhPumpOn && ufhPumpRunOnStartMs == 0) {
            ufhPumpRunOnStartMs = now; // start run-on timer
        }
        if (ufhPumpRunOnStartMs != 0 && (now - ufhPumpRunOnStartMs) >= PUMP_RUNON_MS) {
            if (ufhPumpOn && (now - ufhPumpLastSwitchMs) >= PUMP_MIN_TOGGLE_MS) {
                ufhPumpOn = false;
                ufhPumpLastSwitchMs = now;
            }
        }
    }

    digitalWrite(UFH_VALVE_PUMP_PIN, ufhPumpOn ? LOW : HIGH);
}

// Radiators control: zones immediate, main valve follows, pump warmup/run-on, 24V transformer for 24V valves
void controlRads() {
    // Zone outputs (active-LOW)
    digitalWrite(BASEMENT_RAD_ZONE_PIN, radBasementOn ? LOW : HIGH);
    digitalWrite(LOUNGE_RAD_ZONE_PIN, radLoungeOn ? LOW : HIGH);
    digitalWrite(FRONT_BEDROOM_RAD_ZONE_PIN, radFrontBedroomOn ? LOW : HIGH);
    digitalWrite(BACK_BEDROOM_RAD_ZONE_PIN, radBackBedroomOn ? LOW : HIGH);
    digitalWrite(OFFICE_RAD_ZONE_PIN, radOfficeOn ? LOW : HIGH);
    digitalWrite(MASTER_BEDROOM_RAD_ZONE_PIN, radMasterBedroomOn ? LOW : HIGH);

    // 24V valves
    digitalWrite(BATHROOM_RAD_24V_PIN, bathRadOn ? LOW : HIGH);
    digitalWrite(BATHROOM_TOWEL_24V_PIN, bathTowelOn ? LOW : HIGH);
    digitalWrite(ENSUITE_TOWEL_24V_PIN, ensuiteTowelOn ? LOW : HIGH);

    bool anyRadZone = radBasementOn || radLoungeOn || radFrontBedroomOn || radBackBedroomOn || radOfficeOn || radMasterBedroomOn;
    bool any24v = bathRadOn || bathTowelOn || ensuiteTowelOn;

    // Main valve
    radsMainValveOn = anyRadZone;
    digitalWrite(RADS_VALVE_PIN, radsMainValveOn ? LOW : HIGH);

    // Transformer
    transformer24VOn = any24v;
    digitalWrite(TRANSFORMER_24V_ENABLE_PIN, transformer24VOn ? LOW : HIGH);

    // Pump logic if defined
#ifdef RADS_PUMP_PIN
    unsigned long now = millis();
    if (anyRadZone) {
        if (radsAnyZoneOpenSinceMs == 0) {
            radsAnyZoneOpenSinceMs = now;
        }
        radsPumpRunOnStartMs = 0;
        if ((now - radsAnyZoneOpenSinceMs) >= RADS_WARMUP_MS) {
            if (!radsPumpOn && (now - radsPumpLastSwitchMs) >= PUMP_MIN_TOGGLE_MS) {
                radsPumpOn = true;
                radsPumpLastSwitchMs = now;
            }
        }
    } else {
        radsAnyZoneOpenSinceMs = 0;
        if (radsPumpOn && radsPumpRunOnStartMs == 0) {
            radsPumpRunOnStartMs = now;
        }
        if (radsPumpRunOnStartMs != 0 && (now - radsPumpRunOnStartMs) >= PUMP_RUNON_MS) {
            if (radsPumpOn && (now - radsPumpLastSwitchMs) >= PUMP_MIN_TOGGLE_MS) {
                radsPumpOn = false;
                radsPumpLastSwitchMs = now;
            }
        }
    }
    digitalWrite(RADS_PUMP_PIN, radsPumpOn ? LOW : HIGH);
#endif
}

void updateOutputsFromState() {
    // This function is intentionally kept to allow future abstraction; actual writes are in control functions
}

// Immersion control: active at night window using Tank2 hysteresis
void controlImmersion() {
    int h=0,m=0; getCurrentTime(h,m);
    int current = h*60+m;
    int nStart = nightStartHour*60 + nightStartMinute;
    int nEnd = nightEndHour*60 + nightEndMinute;
    int dStart = dayStartHour*60 + dayStartMinute;
    int dEnd = dayEndHour*60 + dayEndMinute;
    bool inNight = (nStart <= nEnd) ? (current >= nStart && current < nEnd)
                                    : (current >= nStart || current < nEnd);
    bool inDay = (dStart <= dEnd) ? (current >= dStart && current < dEnd)
                                  : (current >= dStart || current < dEnd);

    if (!inNight && !inDay) {
        // No active window → ensure immersion off
        static unsigned long immersionLastToggleMs = 0;
        unsigned long now = millis();
        if (immersionOn && (now - immersionLastToggleMs) >= PUMP_MIN_TOGGLE_MS) {
            immersionOn = false;
            digitalWrite(IMMERSION_RELAY_PIN, LOW);
            immersionLastToggleMs = now;
        }
        return;
    }

    // Determine active window with Day priority on overlap
    bool activeIsDay = false;
    if (inDay && !inNight) activeIsDay = true;
    else if (inNight && !inDay) activeIsDay = false;
    else if (inDay && inNight) activeIsDay = true; // Day wins on overlap

    float targetSp = activeIsDay ? daySetpointC : nightSetpointC;
    bool useImmersion = !(activeIsDay ? daySourceBoiler : nightSourceBoiler);
    bool demandImm = useImmersion && (Temps::Tank2 < (targetSp - HYSTERESIS_C));
    bool stopImm = (Temps::Tank2 > (targetSp + HYSTERESIS_C));

    static unsigned long immersionLastToggleMs = 0;
    unsigned long now = millis();
    if (demandImm) {
        if (!immersionOn && (now - immersionLastToggleMs) >= PUMP_MIN_TOGGLE_MS) {
            immersionOn = true;
            digitalWrite(IMMERSION_RELAY_PIN, HIGH); // active-HIGH
            immersionLastToggleMs = now;
        }
    } else if (stopImm || !useImmersion) {
        if (immersionOn && (now - immersionLastToggleMs) >= PUMP_MIN_TOGGLE_MS) {
            immersionOn = false;
            digitalWrite(IMMERSION_RELAY_PIN, LOW);
            immersionLastToggleMs = now;
        }
    }
}

