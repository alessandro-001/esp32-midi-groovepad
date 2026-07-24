#pragma once

void micInit();
void micPoll();

// 0 (quiet) to 3 (loud), drives the VU meter.
int micGetLevel();

// Raw peak-to-peak over the current window, for tuning the level thresholds.
int micGetAmplitude();

// Amplitude above the noise floor (0 at silence), drives the OLED bar.
int micGetEnergy();
