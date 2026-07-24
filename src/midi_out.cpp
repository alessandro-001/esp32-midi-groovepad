#include <Arduino.h>
#include <USB.h>
#include <USBMIDI.h>
#include "midi_out.h"

namespace {
USBMIDI MIDI;

// Buttons 1-6 map to notes 36-41 (C1 through F#1, General MIDI percussion
// range), channel 1.
const uint8_t BASE_NOTE = 36;
const uint8_t VELOCITY = 100;
}

void midiInit() {
  // This exact order matters, per the framework's own bundled USBMIDI example.
  MIDI.begin();
  USB.begin();
}

void midiNoteOn(int buttonIndex) {
  MIDI.noteOn(BASE_NOTE + buttonIndex, VELOCITY);
}

void midiNoteOff(int buttonIndex) {
  MIDI.noteOff(BASE_NOTE + buttonIndex, 0);
}

int midiNoteForButton(int buttonIndex) {
  return BASE_NOTE + buttonIndex;
}
