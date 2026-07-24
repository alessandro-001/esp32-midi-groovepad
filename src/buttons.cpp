#include <Arduino.h>
#include <ezButton.h>
#include "buttons.h"
#include "pins.h"
#include "midi_out.h"
#include "oled_display.h"

namespace {
const int NUM_BUTTONS = 6;

// External 10K pull-down on each button, so wiring is active-HIGH (pressed = HIGH).
// ezButton's plain INPUT mode is not one of its recognized mode constants, it only
// calls pinMode() for INTERNAL_PULLUP/INTERNAL_PULLDOWN/EXTERNAL_PULLUP/EXTERNAL_PULLDOWN.
// EXTERNAL_PULLDOWN is the library's dedicated mode for this exact wiring: sets the
// pin to plain INPUT (no internal pull-up fighting the external 10K) and tells the
// library pressed = HIGH.
ezButton buttons[NUM_BUTTONS] = {
  ezButton(BUTTON_1_PIN, EXTERNAL_PULLDOWN),
  ezButton(BUTTON_2_PIN, EXTERNAL_PULLDOWN),
  ezButton(BUTTON_3_PIN, EXTERNAL_PULLDOWN),
  ezButton(BUTTON_4_PIN, EXTERNAL_PULLDOWN),
  ezButton(BUTTON_5_PIN, EXTERNAL_PULLDOWN),
  ezButton(BUTTON_6_PIN, EXTERNAL_PULLDOWN),
};

int lastState[NUM_BUTTONS];
}

void buttonsInit() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].setDebounceTime(50);
    lastState[i] = LOW;
  }
}

void buttonsPoll() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].loop();
    int state = buttons[i].getState();

    // Active-HIGH wiring, do our own edge detection rather than trust ezButton's
    // isPressed()/isReleased(), which assume the opposite (active-LOW) polarity.
    if (state == HIGH && lastState[i] == LOW) {
      Serial.print("Button ");
      Serial.print(i + 1);
      Serial.println(" pressed");
      midiNoteOn(i);
      oledShowNote(midiNoteForButton(i));
    } else if (state == LOW && lastState[i] == HIGH) {
      Serial.print("Button ");
      Serial.print(i + 1);
      Serial.println(" released");
      midiNoteOff(i);
      oledShowNote(-1);
    }
    lastState[i] = state;
  }
}
