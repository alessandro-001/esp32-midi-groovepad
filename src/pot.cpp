#include <Arduino.h>
#include "pot.h"
#include "pins.h"

namespace {
const int NUM_SAMPLES = 16;
// Hysteresis band, in raw ADC counts, one output step is 4096/128 = 32 counts.
// The stable raw only moves when a reading clears this band, so idle jitter
// below it is ignored (no flicker) while a real turn still crosses it.
const int HYSTERESIS = 32;
int stableRaw = -1000;
int lastMapped = -1;
}

void potInit() {
  // Default 12-bit resolution and 11db attenuation already cover 0-3.3V on ESP32-S3.
}

bool potPoll(int &value) {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(POT_PIN);
  }
  int raw = sum / NUM_SAMPLES;

  // Only accept the new raw once it has moved past the hysteresis band, this
  // freezes the value when the pot is at rest and kills boundary bouncing.
  if (abs(raw - stableRaw) > HYSTERESIS) {
    stableRaw = raw;
  }

  // Inverted so full left reads 0 and full right reads 127 (standard volume/gain
  // convention), matches this pot's physical wiring where raw ADC runs the other way.
  int mapped = map(stableRaw, 0, 4095, 127, 0);

  if (mapped != lastMapped) {
    lastMapped = mapped;
    value = mapped;
    return true;
  }
  return false;
}
