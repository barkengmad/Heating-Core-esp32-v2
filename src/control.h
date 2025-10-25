// control.h
#ifndef CONTROL_H
#define CONTROL_H

// Timing constants (ms)
static const unsigned long UFH_VALVE_WARMUP_MS = 180000; // 3 min (180 s)
static const unsigned long UFH_RUNON_MS = 60000; // 60 s
static const unsigned long RADS_WARMUP_MS = 180000; // 3 min (180 s)
static const unsigned long RADS_RUNON_MS = 60000; // 60 s
static const unsigned long MIN_ON_OFF_MS = 10000; // 10 s anti-chatter
static const unsigned long TRANSFORMER_24V_RUNON_MS = 0; // No run-on for 24V transformer
static const float HYSTERESIS_C = 2.0f;

void controlBoiler();
void controlUFH();
void controlRads();
void controlImmersion();
void updateOutputsFromState();

// Get countdown values in seconds (0 if not active)
int getUFHWarmupCountdown();
int getUFHRunonCountdown();
int getRadsWarmupCountdown();
int getRadsRunonCountdown();
int get24vWarmupCountdown();

#endif