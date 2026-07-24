#include <Arduino.h>
#include "led_feedback.h"
#include "pins.h"

namespace {
// Bare 4-pin RGB LED, not addressable, no driver chip. Starts assuming
// common-cathode (common leg to GND, color legs driven HIGH to light).
// If the Stage 2 hardware test shows colors inverted (dark when they should
// be lit, or lit during the "off" step), this is actually common-anode:
// move the common leg's wire from the black rail to the red rail, and flip
// this to false.
const bool RGB_COMMON_IS_CATHODE = true;

// PWM duty for "on", 0-255, per channel. Green LEDs read as much brighter
// than red/blue at the same current (human eyes are most sensitive around
// green wavelengths), so it gets its own, lower duty rather than sharing
// one value. Tune to taste.
const int RGB_RED_DUTY = 20;
const int RGB_GREEN_DUTY = 6;
const int RGB_BLUE_DUTY = 20;

int applyPolarity(int duty) {
  return RGB_COMMON_IS_CATHODE ? duty : 255 - duty;
}

void setRgb(bool red, bool green, bool blue) {
  int offDuty = RGB_COMMON_IS_CATHODE ? 0 : 255;
  analogWrite(RGB_RED_PIN, red ? applyPolarity(RGB_RED_DUTY) : offDuty);
  analogWrite(RGB_GREEN_PIN, green ? applyPolarity(RGB_GREEN_DUTY) : offDuty);
  analogWrite(RGB_BLUE_PIN, blue ? applyPolarity(RGB_BLUE_DUTY) : offDuty);
}

void setDiscreteLeds(bool led1, bool led2, bool led3) {
  digitalWrite(DISCRETE_LED_1_PIN, led1 ? HIGH : LOW);
  digitalWrite(DISCRETE_LED_2_PIN, led2 ? HIGH : LOW);
  digitalWrite(DISCRETE_LED_3_PIN, led3 ? HIGH : LOW);
}

const uint32_t STEP_INTERVAL_MS = 1000;
const int NUM_STEPS = 4;
uint32_t lastStep = 0;
int step = 0;

// 4-step repeating RGB cycle, 1 second per step, kept as a simple "alive"
// indicator. The discrete LEDs moved to the mic VU meter (ledVuMeter) as of
// Stage 4, they're no longer part of this self-test.
void applyStep(int s) {
  switch (s) {
    case 0: setRgb(true, false, false); break;
    case 1: setRgb(false, true, false); break;
    case 2: setRgb(false, false, true); break;
    case 3: setRgb(false, false, false); break;
  }
}
}

void ledFeedbackInit() {
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(DISCRETE_LED_1_PIN, OUTPUT);
  pinMode(DISCRETE_LED_2_PIN, OUTPUT);
  pinMode(DISCRETE_LED_3_PIN, OUTPUT);
  applyStep(0);
}

void ledFeedbackPoll() {
  uint32_t now = millis();
  if (now - lastStep >= STEP_INTERVAL_MS) {
    lastStep = now;
    step = (step + 1) % NUM_STEPS;
    applyStep(step);
  }
}

// level: 0 (quiet) to 3 (loud). Green lights at 1+, yellow joins at 2+, red
// joins at 3, a standard rising VU meter.
void ledVuMeter(int level) {
  setDiscreteLeds(level >= 1, level >= 2, level >= 3);
}
