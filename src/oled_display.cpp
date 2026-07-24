#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "oled_display.h"
#include "pins.h"

namespace {
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Light smoothing (EMA) so the bar doesn't jitter between windows, this is a
// display-only concern, mic.cpp's raw amplitude stays untouched for tuning.
int smoothedMicAmplitude = 0;
// Energy value (amplitude above noise floor) that fills the bar completely.
// Above mic.cpp's L3 (red) threshold so a loud hit fills it without pinning.
const int MIC_BAR_MAX = 450;

// Scans the I2C bus and prints any device addresses found, this is the
// spec's "I2C scanner sketch" step folded into normal startup. Returns the
// first address found, or 0 if nothing responded.
uint8_t scanI2C() {
  uint8_t firstFound = 0;
  Serial.println("I2C scan:");
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  found device at 0x");
      Serial.println(addr, HEX);
      if (firstFound == 0) {
        firstFound = addr;
      }
    }
  }
  if (firstFound == 0) {
    Serial.println("  no I2C devices found");
  }
  return firstFound;
}
}

void oledInit() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  // 400kHz fast mode, the full 1KB framebuffer push at the default 100kHz is
  // slow enough to see as flicker when the number redraws. SSD1306 handles it.
  Wire.setClock(400000);
  uint8_t found = scanI2C();

  uint8_t tryAddress = found != 0 ? found : 0x3C;
  if (!display.begin(SSD1306_SWITCHCAPVCC, tryAddress)) {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }

  // Panel is physically mounted upside-down (space constraint, yellow strip
  // ends up at the bottom instead of the top). Rotate 180 degrees in software
  // so text still renders right-side-up to the viewer.
  display.setRotation(2);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("MIDI Groovepad");
  display.setCursor(0, 16);
  display.println("Pot:");
  display.setCursor(0, 52);
  display.print("Mic:");
  display.display();
}

void oledShowPotValue(int value) {
  const int x = 0;
  const int y = 28;
  const int w = 60;
  const int h = 16;
  display.fillRect(x, y, w, h, SSD1306_BLACK);
  display.setTextSize(2);
  display.setCursor(x, y);
  display.print(value);
  display.display();
}

// Drawn at y=50-59, the bottom 16 rows of our (rotated) drawing coordinates.
// After the 180-degree rotation fix, that's where the yellow strip physically
// sits on this upside-down-mounted panel. Starts at x=28 to clear the "Mic:"
// label (4 chars * 6px = 24px, drawn once in oledInit()) to its left.
void oledShowMicLevel(int amplitude) {
  smoothedMicAmplitude += (amplitude - smoothedMicAmplitude) / 4;

  const int barX = 28;
  const int barY = 50;
  const int barW = 96;
  const int barH = 10;

  int clamped = constrain(smoothedMicAmplitude, 0, MIC_BAR_MAX);
  int fillW = map(clamped, 0, MIC_BAR_MAX, 0, barW - 2);

  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
  display.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, SSD1306_BLACK);
  if (fillW > 0) {
    display.fillRect(barX + 1, barY + 1, fillW, barH - 2, SSD1306_WHITE);
  }
  display.display();
}
