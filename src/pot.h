#pragma once

void potInit();

// Returns true and sets value (0-127) when the mapped reading has changed.
bool potPoll(int &value);
