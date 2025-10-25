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
        if ((millis() - boilerActivationTime) >= MIN_ON_OFF_MS) {
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
        if ((millis() - boilerActivationTime) >= MIN_ON_OFF_MS) {
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
        if ((millis() - boilerActivationTime) >= MIN_ON_OFF_MS) {
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
            if (!ufhPumpOn && (now - ufhPumpLastSwitchMs) >= MIN_ON_OFF_MS) {
                ufhPumpOn = true;
                ufhPumpLastSwitchMs = now;
            }
        }
    } else {
        ufhAnyValveOpenSinceMs = 0;
        if (ufhPumpOn && ufhPumpRunOnStartMs == 0) {
            ufhPumpRunOnStartMs = now; // start run-on timer
        }
        if (ufhPumpRunOnStartMs != 0 && (now - ufhPumpRunOnStartMs) >= UFH_RUNON_MS) {
            if (ufhPumpOn && (now - ufhPumpLastSwitchMs) >= MIN_ON_OFF_MS) {
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
    bool anyZone = anyRadZone || any24v;

    unsigned long now = millis();

    // Update per-zone since timestamps
    auto upd = [&](bool on, unsigned long &since){ if (on) { if (since==0) since=now; } else { since=0; } };
    upd(radBasementOn, radBasementOnSinceMs);
    upd(radLoungeOn, radLoungeOnSinceMs);
    upd(radFrontBedroomOn, radFrontBedroomOnSinceMs);
    upd(radBackBedroomOn, radBackBedroomOnSinceMs);
    upd(radOfficeOn, radOfficeOnSinceMs);
    upd(radMasterBedroomOn, radMasterBedroomOnSinceMs);
    upd(bathRadOn, bathRadOnSinceMs);
    upd(bathTowelOn, bathTowelOnSinceMs);
    upd(ensuiteTowelOn, ensuiteTowelOnSinceMs);

    // Earliest ON time among currently ON zones
    unsigned long earliestOn = 0;
    auto consider = [&](unsigned long since){ if (since!=0 && (earliestOn==0 || since<earliestOn)) earliestOn = since; };
    consider(radBasementOnSinceMs);
    consider(radLoungeOnSinceMs);
    consider(radFrontBedroomOnSinceMs);
    consider(radBackBedroomOnSinceMs);
    consider(radOfficeOnSinceMs);
    consider(radMasterBedroomOnSinceMs);
    consider(bathRadOnSinceMs);
    consider(bathTowelOnSinceMs);
    consider(ensuiteTowelOnSinceMs);

    // Rads main valve ON only after earliest ON zone has completed warm-up; OFF immediately if none
    if (earliestOn != 0 && (now - earliestOn) >= RADS_WARMUP_MS) {
        radsMainValveOn = true;
    } else if (!anyZone) {
        radsMainValveOn = false;
    } else {
        radsMainValveOn = false;
    }
    digitalWrite(RADS_VALVE_PIN, radsMainValveOn ? LOW : HIGH);

    // 24V Transformer - auto derived from 24V valves only, with warmup countdown gating for display
    transformer24VOn = any24v; // still immediate on/off as requested earlier
    digitalWrite(TRANSFORMER_24V_ENABLE_PIN, transformer24VOn ? LOW : HIGH);

    // Pump logic if defined
#ifdef RADS_PUMP_PIN
    if (anyZone) {
        if (radsAnyZoneOpenSinceMs == 0) {
            radsAnyZoneOpenSinceMs = now;
        }
        radsPumpRunOnStartMs = 0;
        if ((now - radsAnyZoneOpenSinceMs) >= RADS_WARMUP_MS) {
            if (!radsPumpOn && (now - radsPumpLastSwitchMs) >= MIN_ON_OFF_MS) {
                radsPumpOn = true;
                radsPumpLastSwitchMs = now;
            }
        }
    } else {
        radsAnyZoneOpenSinceMs = 0;
        // Stop pump immediately when all zones off (no run-on for rads pump per request to avoid pumping against closed valves)
        if (radsPumpOn && (now - radsPumpLastSwitchMs) >= MIN_ON_OFF_MS) {
            radsPumpOn = false;
            radsPumpLastSwitchMs = now;
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
        if (immersionOn && (now - immersionLastToggleMs) >= MIN_ON_OFF_MS) {
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
        if (!immersionOn && (now - immersionLastToggleMs) >= MIN_ON_OFF_MS) {
            immersionOn = true;
            digitalWrite(IMMERSION_RELAY_PIN, HIGH); // active-HIGH
            immersionLastToggleMs = now;
        }
    } else if (stopImm || !useImmersion) {
        if (immersionOn && (now - immersionLastToggleMs) >= MIN_ON_OFF_MS) {
            immersionOn = false;
            digitalWrite(IMMERSION_RELAY_PIN, LOW);
            immersionLastToggleMs = now;
        }
    }
}

// Countdown functions for UI display (returns seconds remaining, 0 if not active)
int getUFHWarmupCountdown() {
    if (ufhAnyValveOpenSinceMs == 0) return 0;
    unsigned long elapsed = millis() - ufhAnyValveOpenSinceMs;
    if (elapsed >= UFH_VALVE_WARMUP_MS) return 0;
    return (UFH_VALVE_WARMUP_MS - elapsed) / 1000;
}

int getUFHRunonCountdown() {
    if (ufhPumpRunOnStartMs == 0) return 0;
    unsigned long elapsed = millis() - ufhPumpRunOnStartMs;
    if (elapsed >= UFH_RUNON_MS) return 0;
    return (UFH_RUNON_MS - elapsed) / 1000;
}

int getRadsWarmupCountdown() {
    if (radsAnyZoneOpenSinceMs == 0) return 0;
    unsigned long elapsed = millis() - radsAnyZoneOpenSinceMs;
    if (elapsed >= RADS_WARMUP_MS) return 0;
    return (RADS_WARMUP_MS - elapsed) / 1000;
}

int getRadsRunonCountdown() {
    if (radsPumpRunOnStartMs == 0) return 0;
    unsigned long elapsed = millis() - radsPumpRunOnStartMs;
    if (elapsed >= RADS_RUNON_MS) return 0;
    return (RADS_RUNON_MS - elapsed) / 1000;
}

int get24vWarmupCountdown() {
    if (radsAnyZoneOpenSinceMs == 0) return 0;
    unsigned long elapsed = millis() - radsAnyZoneOpenSinceMs;
    if (elapsed >= RADS_WARMUP_MS) return 0;
    return (RADS_WARMUP_MS - elapsed) / 1000;
}

