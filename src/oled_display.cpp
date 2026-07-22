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
