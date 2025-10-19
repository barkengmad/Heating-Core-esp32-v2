// control.h
#ifndef CONTROL_H
#define CONTROL_H

// Timing constants (ms)
static const unsigned long UFH_VALVE_WARMUP_MS = 180000; // 3 min
static const unsigned long RADS_WARMUP_MS = 180000; // 3 min
static const unsigned long PUMP_RUNON_MS = 60000; // 60 s
static const unsigned long PUMP_MIN_TOGGLE_MS = 10000; // 10 s
static const float HYSTERESIS_C = 2.0f;

void controlBoiler();
void controlUFH();
void controlRads();
void controlImmersion();
void updateOutputsFromState();


#endif