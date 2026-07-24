#include <Arduino.h>
#include <FastLED.h>
#include "pins.h"
#include "buttons.h"
#include "pot.h"
#include "led_feedback.h"
#include "oled_display.h"
#include "mic.h"

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

const uint32_t TOGGLE_INTERVAL_MS = 1000;
uint32_t lastToggle = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
  buttonsInit();
  potInit();
  ledFeedbackInit();
  oledInit();
  micInit();
}

void loop() {
  uint32_t now = millis();
  if (now - lastToggle >= TOGGLE_INTERVAL_MS) {
    lastToggle = now;
    ledOn = !ledOn;
    leds[0] = ledOn ? CRGB::Blue : CRGB::Black;
    FastLED.show();
  }

  buttonsPoll();
  ledFeedbackPoll();

  micPoll();
  ledVuMeter(micGetLevel());

  int potValue;
  if (potPoll(potValue)) {
    Serial.print("Pot: ");
    Serial.println(potValue);
    oledShowPotValue(potValue);
  }

  // Throttled tuning print for the mic thresholds in mic.cpp, quiet down or
  // remove once the L1/L2/L3 values are set from real readings.
  static uint32_t lastMicPrint = 0;
  if (now - lastMicPrint >= 150) {
    lastMicPrint = now;
    Serial.print("Mic raw: ");
    Serial.print(micGetAmplitude());
    Serial.print("  energy: ");
    Serial.print(micGetEnergy());
    Serial.print("  level: ");
    Serial.println(micGetLevel());
    oledShowMicLevel(micGetEnergy());
  }
}
