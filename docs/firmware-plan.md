# ESP32-S3 USB-MIDI Groove Pad, Firmware and Build Plan

## Role instructions for the coding agent

Act as a senior electrical and embedded engineer working alongside someone who is comfortable with code but wants rigor on the hardware side. For every stage:

- State assumptions explicitly before writing code (pin numbers, voltage levels, component ratings) and flag anything that should be confirmed with a multimeter or datasheet before power is applied.
- Never assume a resistor, transistor, or wiring choice is "close enough." If a component value is wrong for the job (for example, a resistor that will make an LED too dim or a GPIO that cannot safely source a motor's current), say so and propose the correct part or workaround rather than proceeding.
- Call out strapping pins, ADC-only pins, and USB-reserved pins on the ESP32-S3 by name whenever they are relevant.
- After each stage, provide a concrete test procedure with an expected result, not just "upload and see." Include what to check with a multimeter where relevant (continuity, voltage at a specific point) before applying power.
- Do not move to the next stage until the current one's test procedure is confirmed working.
- When uploading on github do not add Claude as contributor, when about to commit always ask confirmation.

## Goal

Build firmware for an ESP32-S3 (native USB) that turns a breadboard rig into a class-compliant USB-MIDI controller for playing Strudel (browser-based livecoding music, using the Web MIDI API). The device reads buttons, a potentiometer, and a mic amplitude signal, and drives an addressable LED, discrete LEDs, and an OLED display as feedback.

Vibration motor haptic feedback was in the original scope but has been dropped, we could not find enough reliable information about the specific motor module on hand (voltage rating, current draw, whether it already includes a driver transistor) to wire it safely. All motor driver wiring, BOM entries, and the Stage 6 haptics step below are removed accordingly.

This is a staged build. Each stage should compile, upload, and be verified on hardware, with its test passing, before starting the next one.

## Hardware inventory

- ESP32-S3 (native USB variant, confirm exact board silkscreen/model once in hand, this changes GPIO numbering)
- 6 bare pushbuttons (the 4-button PCB module, marked 3030368_Y137_220714, was dropped, its header pins are on the top face and can't reach a breadboard)
- 1x potentiometer, marked WL / B5K, 3 pins, treat as a 5K linear pot
- 1x addressable RGB LED, 3 pins (VCC, GND, DATA). Confirm this is WS2812B-style before wiring, look for a tiny black driver chip inside the dome
- Assorted discrete single-color LEDs
- Resistors: 10K, 1K, 220R, 110R (see Bill of Materials below for what each value is used for)
- Mini OLED screen, I2C, 0.96" 128x64, marked JMD0.96D on the back, 2-color (yellow top strip, blue bottom). This module is commonly SSD1306-based, still confirm the I2C address (commonly 0x3C) with the Stage 3 I2C scanner sketch, and swap to an SH1106 library if characters come out wrong
- MAX4466 electret mic amp (Adafruit), adjustable gain via onboard trimpot
- Breadboards, jumper wires
- Not used in this build: both SRD-05VDC-SL-C relay modules, second ESP32 boards (S3 Zero spare, NodeMCU ESP32 v1.1), vibration motor (dropped from scope, not enough reliable info on the specific module to wire it safely)

## Bill of materials

Full parts list for the whole build, with quantities and exactly which resistor value from your stock (10K, 1K, 220R, 110R) goes where. Nothing here needs ordering, it is all drawn from what you already have.

| Part | Quantity | Used for |
|---|---|---|
| ESP32-S3 (native USB board) | 1 | main controller |
| Breadboard | 1 | |
| Jumper wires (M-M, plus M-F if any module has header sockets) | assorted | all connections |
| Pushbutton | 6 | main trigger pads, each with its own 10K pull-down resistor |
| Potentiometer, B5K, 3-pin | 1 | MIDI CC control |
| Addressable RGB LED, 3-pin (WS2812-style, confirm) | 1 | color feedback |
| Discrete LED | 3 (or however many you want to wire) | VU meter / status feedback |
| MAX4466 mic amp module | 1 | amplitude sensing |
| Mini OLED display, I2C | 1 | status/value display |
| Multimeter | 1 | continuity and voltage checks at every stage |

### Resistor assignment (from your stock: 10K, 1K, 220R, 110R)

| Value | Used for | Why this value |
|---|---|---|
| 220R | Discrete LED current limiting, one per LED (x3) | at 3.3V gives roughly (3.3-2)/220 ~ 5.9mA, safe and reasonably bright for a standard 5mm LED |
| 220R | WS2812 data line series resistor (optional) | reduces ringing on the data line if you see occasional glitches, not required at short breadboard wire lengths |
| 110R | Alternate LED resistor if you want a noticeably brighter LED | gives roughly 11.8mA at 3.3V, still comfortably under the typical 20mA max for a standard LED, use this instead of 220R for any LED you want to stand out (for example a "beat" indicator) |
| 10K | Pull-down for each pushbutton (x6) | external pull-down chosen over the ESP32's internal pull-up, gives a physical resistor per button to verify with a multimeter, and matches the active-HIGH wiring used throughout |

1K and the second 110R were reserved for the motor driver (base/gate resistor), no longer needed now that the motor is out of scope. Both stay spare.

Do not use 10K for LED current limiting, it under-drives an LED to roughly 0.13mA at 3.3V and the LED will appear off.

## Digital debouncing

Confirmed: `ezButton` by ArduinoGetStarted, this is what the user's past project used. Simple digital debounce with `.isPressed()` / `.isReleased()` state methods, read the pin, require the new state to hold stable for a debounce window (typically 20 to 50ms) before reporting a change, not analog smoothing.

## Software stack

- Framework: Arduino core for ESP32-S3, via PlatformIO in VS Code
- USB mode: TinyUSB, configured for USB-MIDI class device (not CDC serial). This is set in `platformio.ini` board config, confirm the exact flags for your board variant before Stage 5
- Libraries:
  - `ezButton` (digital debounce for all buttons)
  - Adafruit TinyUSB Arduino (USB-MIDI transport)
  - FastLED or Adafruit NeoPixel (addressable LED)
  - Adafruit SSD1306 plus Adafruit GFX (OLED, swap to an SH1106 library if the controller chip turns out to be SH1106 instead)
  - Native `analogRead` for pot and mic, no extra library needed
- Keep all pin assignments in a single `pins.h` header

## Suggested project structure

```
/src
  main.cpp
  pins.h
  buttons.cpp / buttons.h
  pot.cpp / pot.h
  mic.cpp / mic.h
  led_feedback.cpp / led_feedback.h
  oled_display.cpp / oled_display.h
  haptics.cpp / haptics.h
  midi_out.cpp / midi_out.h
/platformio.ini
```

## Reference pin assignments

These assume a generic ESP32-S3 DevKitC-1 style board. Confirm against your specific board's printed pinout diagram before wiring, since some S3 boards number pins differently. Avoid GPIO0, GPIO3, GPIO45, GPIO46 (strapping pins, can prevent boot if loaded at power-up) and GPIO19/GPIO20 (native USB D-/D+, do not reuse these for anything else).

| Function | Suggested GPIO | Notes |
|---|---|---|
| I2C SDA (OLED) | GPIO8 | confirm against board silkscreen |
| I2C SCL (OLED) | GPIO9 | confirm against board silkscreen |
| Button 1 | GPIO4 | digital input, external 10K pull-down, active-HIGH |
| Button 2 | GPIO5 | |
| Button 3 | GPIO6 | |
| Button 4 | GPIO7 | |
| Button 5 | GPIO15 | |
| Button 6 | GPIO16 | |
| Pot wiper | GPIO1 | ADC1 channel, analog input only |
| Mic amp OUT | GPIO2 | ADC1 channel, analog input only |
| WS2812 RGB LED DATA | GPIO17 | single wire, optional 220R in series if you see flicker, not strictly required at short wire lengths |
| Discrete LED 1 | GPIO18 | through a 220R resistor |
| Discrete LED 2 | GPIO21 | through a 220R resistor |
| Discrete LED 3 | GPIO10 | through a 220R resistor (or 110R if you want this one brighter, for example a beat indicator). Reassigned from the originally suggested GPIO35, this board has Octal PSRAM, which reserves GPIO33-37 internally (SPIIO4-7/SPIDQS), those pins are not available for anything else on this board |

## Breadboard wiring instructions, step by step

Work one sub-circuit at a time. After each one, do the multimeter check listed before moving on.

### 1. Power rails
- Insert the ESP32-S3 straddling the breadboard's center gap so pins on each side are accessible.
- Run a wire from the board's 3V3 pin to the breadboard's red (+) rail, and from a GND pin to the blue/black (-) rail.
- Multimeter check: power the board over USB, measure across the red and blue rails, expect approximately 3.3V. If you see 0V, check the 3V3 pin location on your specific board (it is sometimes labeled differently).

### 2. Buttons (6 bare pushbuttons, each with its own 10K pull-down)
- One leg of each button to the red (+) rail (3.3V), the other leg to its assigned GPIO (see table above).
- A 10K resistor from that same GPIO junction to the blue (-) rail (GND). This is a pull-down, so idle reads LOW and a press reads HIGH. Pin mode in code is plain `INPUT`, not `INPUT_PULLUP`, the internal pull-up would fight the external pull-down and leave the pin at an ambiguous idle voltage.
- Multimeter check: with the board unpowered, continuity mode confirms the button itself closes its two legs when pressed, and confirms the 10K resistor reads roughly 10K ohm between the GPIO junction and the blue rail.

### 3. Potentiometer
- Outer pin 1 to the red (+) rail (3.3V).
- Outer pin 2 to the blue (-) rail (GND).
- Middle pin (wiper) to GPIO1.
- Multimeter check: with the board powered, measure voltage on the wiper pin while turning the pot, expect a smooth sweep from near 0V to near 3.3V across its full range.

### 4. MAX4466 mic amp
- VCC to red (+) rail, GND to blue (-) rail.
- OUT to GPIO2.
- Multimeter check: with the board powered and the room quiet, measure OUT to GND, expect a steady DC value around half the supply voltage (roughly 1.6 to 1.7V), this is the bias point the mic signal rides on. Clap near the mic and watch for the reading to flicker, confirming it is picking up sound (a multimeter is slow, this is a rough check, real verification happens in Stage 4's code test).

### 5. Addressable RGB LED
- VCC to red (+) rail, GND to blue (-) rail.
- DATA to GPIO17.
- No series resistor required at breadboard scale for a single LED. If you have a small ceramic or electrolytic capacitor (10uF or larger) handy, place it across VCC and GND close to the LED, this is good practice but not mandatory for one LED on a short wire.
- Visual check only needed here, code test comes in Stage 2.

### 6. Discrete LEDs
- Long leg (anode) through a 220R resistor (or 110R for a brighter one) to the assigned GPIO.
- Short leg (cathode) directly to the blue (-) rail.
- Multimeter check: with the board unpowered, use diode-test mode (if your multimeter has it) across each LED to confirm orientation and that it is not damaged, you should see a voltage drop reading only in one direction.

### 7. OLED display
- VCC to red (+) rail, GND to blue (-) rail.
- SDA to GPIO8, SCL to GPIO9.
- Multimeter check: confirm 3.3V is present at the OLED's VCC pin while powered. Real verification of the I2C link happens with an I2C scanner sketch in Stage 3.

## Stage plan with tests

### Stage 0: project scaffolding
- Set up PlatformIO project targeting the correct ESP32-S3 board definition.
- Write a basic blink sketch on any free GPIO with a spare LED and its resistor.
- Test: LED blinks at a visibly steady 1 second interval, serial monitor prints a heartbeat message once per second. If the blink rate looks wrong, check for the wrong board definition in `platformio.ini` before touching wiring.

### Stage 1: buttons and pot input
- Wire buttons and pot per the instructions above (6 pushbuttons, each with its own 10K pull-down, plus the pot).
- Use `ezButton` for all button reads, poll each button's `.loop()` and `.getState()` once per main loop pass, not inside an interrupt. Wiring is active-HIGH, so edge detection is done manually against `getState()` rather than via `ezButton`'s own `isPressed()`/`isReleased()`, which assume the opposite (active-LOW) polarity.
- Read the pot with `analogRead`, averaged over a few samples, map to 0 to 127.
- Test: serial monitor shows one clean "pressed" and one clean "released" event per physical press, with no repeated or chattering events, for all 6 buttons. Turning the pot slowly should print a smoothly increasing or decreasing number, not a jumpy or noisy one.

### Stage 2: addressable LED and discrete LEDs
- Confirm the RGB LED responds correctly to a basic FastLED or NeoPixel color-cycle test (red, then green, then blue, then off, one second each).
- Drive discrete LEDs through a simple test pattern (all on, all off, one at a time).
- Test: colors on the RGB LED match what the code requested, in the correct order, no flicker. Discrete LEDs light clearly and at a comfortable brightness in the expected pattern.

### Stage 3: OLED display
- Run an I2C scanner sketch first, confirm the OLED's address shows up (commonly 0x3C).
- Display static text, then a live-updating value (use the pot reading as the test source).
- Test: text is legible, no garbled characters, and the live value updates smoothly as the pot turns, with no visible flicker or refresh tearing.

### Stage 4: mic amplitude sensing
- Sample the mic ADC at a fast, consistent rate (a simple timer or a tight loop with a short delay is fine to start, aim for roughly 1kHz).
- Compute a rolling amplitude (for example, track a running max over a short window, or a simple envelope follower).
- Drive discrete LEDs as a standalone VU meter from this value, no MIDI involved yet.
- Test: clap near the mic and confirm LEDs visibly react in near real time. Adjust the MAX4466's onboard gain trimpot if the response is too weak or if it is pinned at maximum from ambient noise.

### Stage 5: USB-MIDI transport
- Switch the board's USB mode to MIDI (TinyUSB), reflash, and confirm the host computer sees a new MIDI device when you check its OS sound or MIDI device list.
- Send a single test note-on then note-off from one button press.
- Test: the device shows up by name in your OS's MIDI device list. A basic MIDI monitor tool (or Strudel itself, see Strudel setup below) shows a note-on event the instant you press the button, and a matching note-off on release.

### Stage 6: full integration
- Map all buttons to distinct MIDI notes.
- Map the pot to a MIDI CC number.
- Drive LED and OLED feedback from live mic amplitude while Strudel plays.
- Test: play a full round trip, press a button, hear the corresponding Strudel sound with no perceptible lag, turn the pot and hear the mapped parameter change smoothly, watch LEDs and OLED respond to the audio.

## Setting up Strudel

1. Go to `https://strudel.cc` in a desktop browser that supports the Web MIDI API (Chrome or Edge are the most reliable choices, Firefox does not support Web MIDI by default).
2. Plug in your ESP32-S3 over USB after it is flashed with Stage 5's MIDI firmware. Your OS should recognize it as a MIDI device before you open the browser tab, or you may need to reload the tab after plugging in.
3. In Strudel's interface, open the settings panel (usually a gear or settings icon) and look for a MIDI or "MIDI in" option. Enable it and select your ESP32 device from the dropdown if one appears.
4. Write a minimal test pattern using Strudel's `midin` (or equivalent MIDI input function, check Strudel's current docs since function names can change between versions) bound to a note number matching one of your button's MIDI notes, and confirm pressing that button triggers sound.
5. Once basic note triggering works, expand the pattern to map your pot's MIDI CC to a parameter such as filter cutoff (`lpf`) or tempo (`cps`), and confirm turning the pot audibly changes that parameter live.
6. Save your working Strudel pattern (Strudel supports exporting or copying the pattern code) so you do not lose it between sessions, since it runs entirely in the browser tab.

## Open questions to resolve during implementation

- Exact GPIO pin numbers for each peripheral, once the specific ESP32-S3 board variant's pinout is confirmed against its silkscreen
- Whether the 3-pin RGB LED is genuinely WS2812-style or a mislabeled 4-pin part
- OLED I2C address (commonly 0x3C), confirm with the Stage 3 I2C scanner sketch, and whether the JMD0.96D module is actually SSD1306 or needs the SH1106 library instead
- Final choice of what the pot's MIDI CC controls in the Strudel pattern
- Exact Strudel MIDI-input function name and syntax, confirm against the current Strudel docs since this can change between versions

## Formatting and code style notes for Claude Code

- No em dashes in code comments or documentation, use commas, periods, or "and" instead
- Keep each stage's code compiling and uploadable on its own, do not write ahead into later stages before the current one is verified working
- Flag any pin conflicts or power budget concerns (mic amp, OLED, LED sharing the ESP32-S3's 3.3V rail) rather than silently working around them
- Confirm debounce timing (start at 20 to 50ms) is tuned per button if any button feels laggy or chattery in testing
