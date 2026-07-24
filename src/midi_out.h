#pragma once

void midiInit();

// buttonIndex is 0-based (Button 1 = index 0), maps to a fixed MIDI note.
void midiNoteOn(int buttonIndex);
void midiNoteOff(int buttonIndex);
int midiNoteForButton(int buttonIndex);
