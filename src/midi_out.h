#pragma once

void midiInit();

// buttonIndex is 0-based (Button 1 = index 0), maps to a fixed MIDI note.
void midiNoteOn(int buttonIndex);
void midiNoteOff(int buttonIndex);
int midiNoteForButton(int buttonIndex);

// value is 0-127 (the pot's mapped range), sent as CC 74 (filter cutoff).
void midiControlChange(uint8_t value);
