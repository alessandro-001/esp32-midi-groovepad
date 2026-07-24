#include <Arduino.h>
#include "mic.h"
#include "pins.h"

namespace {
const uint32_t WINDOW_MS = 50;
uint32_t windowStart = 0;
int windowMin = 4095;
int windowMax = 0;

int amplitude = 0;   // raw peak-to-peak
int energy = 0;      // amplitude above the noise floor, drives level + bar
int level = 0;

// Raw peak-to-peak silence sits well above zero (ADC + ambient noise), measured
// ~130-185 in the room. NOISE_FLOOR is subtracted so quiet reads ~0 on both the
// LEDs and the OLED bar. ponytail: fixed constant, re-tune per environment if
// the room changes a lot, auto-calibration (track a slow running minimum) is
// the upgrade path if this drifts too much.
const int NOISE_FLOOR = 185;

// Thresholds on the floor-subtracted energy. Keyboard typing (~raw 190-307,
// energy 5-120) stays green at most, talking/clapping climb into yellow/red.
const int L1 = 40;    // green
const int L2 = 200;   // + yellow
const int L3 = 400;   // + red
}

void micInit() {
  // Default 12-bit resolution and 11db attenuation already cover the
  // MAX4466's 3.3V-biased output range, no extra ADC config needed.
}

// One sample per call, folded into a running min/max. ponytail: this samples
// at whatever rate the main loop runs, not a fixed timer, plenty fast enough
// for a VU meter's loudness envelope. Upgrade path if it ever looks coarse:
// sample on a micros() interval or a hardware timer instead.
void micPoll() {
  int sample = analogRead(MIC_PIN);
  if (sample < windowMin) windowMin = sample;
  if (sample > windowMax) windowMax = sample;

  uint32_t now = millis();
  if (now - windowStart >= WINDOW_MS) {
    // Peak-to-peak cancels the mic's DC bias automatically, no need to know
    // or calibrate the exact bias voltage.
    amplitude = windowMax - windowMin;
    energy = amplitude - NOISE_FLOOR;
    if (energy < 0) energy = 0;

    if (energy < L1) {
      level = 0;
    } else if (energy < L2) {
      level = 1;
    } else if (energy < L3) {
      level = 2;
    } else {
      level = 3;
    }

    windowStart = now;
    windowMin = 4095;
    windowMax = 0;
  }
}

int micGetLevel() { return level; }
int micGetAmplitude() { return amplitude; }
int micGetEnergy() { return energy; }
