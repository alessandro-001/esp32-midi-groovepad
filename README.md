# ESP32-S3 MIDI Groove Pad

![Platform](https://img.shields.io/badge/platform-ESP32--S3-E7352C?logo=espressif&logoColor=white)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D?logo=arduino&logoColor=white)
![Built with PlatformIO](https://img.shields.io/badge/built%20with-PlatformIO-FF7F00?logo=platformio&logoColor=white)
![USB MIDI](https://img.shields.io/badge/output-USB--MIDI-4B0082)
![Status](https://img.shields.io/badge/status-work%20in%20progress-yellow)

A breadboard MIDI controller built on an ESP32-S3 with native USB. It reads six
buttons, a potentiometer, and a microphone amplitude signal, and drives an RGB
LED, three discrete status LEDs, and an OLED display as feedback. The end goal is
a class-compliant USB-MIDI device for playing [Strudel](https://strudel.cc),
browser-based livecoding music, over the Web MIDI API.

## Features

- **6 pushbuttons** with clean digital debounce, external 10K pull-downs, active-HIGH
- **Potentiometer** read on the ADC, mapped 0 to 127 with hysteresis to kill jitter
- **4-pin RGB LED** driven by per-channel PWM (green is dimmed since it reads brighter to the eye)
- **3 discrete LEDs** wired green, yellow, red for a VU-meter style readout
- **0.96" I2C OLED** (SSD1306, 128x64) showing live status, mounted upside-down with a software 180 degree rotation
- **MAX4466 electret mic** for amplitude sensing (in progress)

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

Full parts list, wiring, and per-stage build notes live in
[docs/firmware-plan.md](docs/firmware-plan.md).

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
`COM` port for flashing and the serial monitor during development. The native USB
port becomes the MIDI interface once the USB-MIDI stage lands.

## Build stages

The firmware is built and hardware-verified one stage at a time. Each stage must
compile, flash, and pass its test before the next begins.

- [x] **Stage 0** Project scaffolding, onboard LED heartbeat
- [x] **Stage 1** Buttons and potentiometer input
- [x] **Stage 2** RGB LED and discrete LEDs
- [x] **Stage 3** OLED display
- [ ] **Stage 4** Microphone amplitude sensing (VU meter)
- [ ] **Stage 5** USB-MIDI transport
- [ ] **Stage 6** Full integration with Strudel
