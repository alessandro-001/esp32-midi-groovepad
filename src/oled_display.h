#pragma once

void oledInit();
void oledShowPotValue(int value);
void oledShowMicLevel(int amplitude);

// note < 0 shows "--" (no button currently held).
void oledShowNote(int note);
