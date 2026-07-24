# ESP32-S3 MIDI Groove Pad

![Platform](https://img.shields.io/badge/platform-ESP32--S3-E7352C?logo=espressif&logoColor=white)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D?logo=arduino&logoColor=white)
![Built with PlatformIO](https://img.shields.io/badge/built%20with-PlatformIO-FF7F00?logo=platformio&logoColor=white)
![USB MIDI](https://img.shields.io/badge/output-USB--MIDI-4B0082)
![Status](https://img.shields.io/badge/status-work%20in%20progress-yellow)

A breadboard MIDI controller built on an ESP32-S3 with native USB. It reads six
buttons, a potentiometer, and a microphone amplitude signal, and drives an RGB
LED, three discrete status LEDs, and an OLED display as feedback, all sent out
as class-compliant USB-MIDI for playing [Strudel](https://strudel.cc),
browser-based livecoding music, over the Web MIDI API.

## Features

- **6 pushbuttons** with clean digital debounce, external 10K pull-downs, active-HIGH, each sends a USB-MIDI note-on/note-off
- **Potentiometer** read on the ADC, mapped 0 to 127 with hysteresis to kill jitter
- **4-pin RGB LED** driven by per-channel PWM (green is dimmed since it reads brighter to the eye)
- **3 discrete LEDs** wired green, yellow, red as a live microphone VU meter
- **0.96" I2C OLED** (SSD1306, 128x64) showing pot value and a mic level bar, mounted upside-down with a software 180 degree rotation
- **MAX4466 electret mic**, peak-to-peak amplitude with noise-floor subtraction, drives the VU meter and OLED bar
- **USB-MIDI transport** over the native USB port (built-in `USBMIDI` class, no extra libraries), class-compliant, no drivers needed

## Hardware

| Component | Detail |
|---|---|
| Board | ESP32-S3-DevKitC-1 N16R8 (16MB flash, 8MB octal PSRAM), native USB |
| Buttons | 6 bare pushbuttons, one 10K pull-down each |
| Potentiometer | B5K linear, 3-pin |
| RGB LED | bare 4-pin common-cathode, one 220R per color channel |
| Discrete LEDs | 3x through 220R (green, yellow, red) |
| OLED | JMD0.96D, I2C SSD1306 128x64, 2-color (yellow/blue) |
| Mic | MAX4466 electret amp module |

### Bill of materials

| Qty | Part | Used for |
|---|---|---|
| 1 | ESP32-S3-DevKitC-1 N16R8 (native USB) | main controller |
| 2 | Breadboard | joined, to reach headers on both sides of the board |
| 6 | Pushbutton (momentary) | trigger pads |
| 1 | Potentiometer, B5K linear, 3-pin | control knob |
| 1 | RGB LED, 4-pin common-cathode | color feedback |
| 3 | Discrete LED (green, yellow, red) | VU meter / status |
| 1 | MAX4466 electret mic amp module | amplitude sensing |
| 1 | OLED 0.96" I2C SSD1306, 128x64 | status display |
| 6 | Resistor, 220R | 3 RGB channels + 3 discrete LEDs |
| 6 | Resistor, 10K | one pull-down per button |
| — | Jumper wires | assorted |

### Pin map

| Function | GPIO | Function | GPIO |
|---|---|---|---|
| Buttons 1-6 | 4, 5, 6, 7, 15, 16 | RGB red / green / blue | 17 / 11 / 12 |
| Potentiometer wiper | 1 | Discrete LEDs (G / Y / R) | 18 / 21 / 10 |
| Mic amp out | 2 | OLED SDA / SCL | 8 / 9 |
| Onboard status LED | 48 | | |

> Note: this board uses octal PSRAM, which reserves GPIO33-37 internally. Those
> pins are avoided throughout. GPIO0/3/45/46 (strapping) and GPIO19/20 (native
> USB D-/D+) are also left free.

### MIDI note map

Channel 1, velocity 100. Needed if you're writing a Strudel pattern to trigger these:

| Button | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| MIDI note | 36 | 37 | 38 | 39 | 40 | 41 |

## Build and flash

This is a [PlatformIO](https://platformio.org) project.

```bash
# build
pio run

# flash and open the serial monitor (adjust the port to your board)
pio run -t upload --upload-port COM6
pio device monitor -b 115200
```

The board exposes two USB-C ports, `USB` (native) and `COM` (UART bridge). Use the
`COM` port for flashing and the serial monitor, both cables can stay plugged in
at once. The native `USB` port is the MIDI interface, no drivers needed on a
modern OS, it enumerates as a class-compliant MIDI device.

## Build stages

The firmware is built and hardware-verified one stage at a time. Each stage must
compile, flash, and pass its test before the next begins.

- [x] **Stage 0** Project scaffolding, onboard LED heartbeat
- [x] **Stage 1** Buttons and potentiometer input
- [x] **Stage 2** RGB LED and discrete LEDs
- [x] **Stage 3** OLED display
- [x] **Stage 4** Microphone amplitude sensing (VU meter)
- [x] **Stage 5** USB-MIDI transport
- [ ] **Stage 6** Full integration with Strudel
