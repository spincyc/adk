# Elegoo Mega kit coverage audit

Observed 2026-07-27. This audit uses the official Elegoo download as the
revisioned baseline for “Mega 2560 The Most Complete Starter Kit.” It is a
planning inventory, not a claim that every retail carton has identical contents.
Elegoo explicitly includes a sensor/module version notice in the bundle and
ships either a GY-521/MPU-6050 or QMI-8658 motion module in some revisions.

## Source identity

- [Current official product page][product] says “more than 200 components,”
  “more than 35 lessons,” and names the LCD1602 and GY-521. Its rendered
  “What's In Box” list is only a partial inventory.
- [Official tutorial page][tutorial-page], dated 2020-10-21, links the
  authoritative downloadable bundle.
- [Official tutorial bundle][tutorial-zip] is named
  `ELEGOO The Most Complete Starter Kit for MEGA V1.0.2022.03.24.zip`.
  That filename is the exact baseline revision used here.
- The bundle's English manual is named
  `The Most Complete Starter Kit for MEGA V1.0.2021.05.13.pdf`. Its cover says
  `V1.0.18.12.19`; PDF metadata reports modification on 2022-03-10. The bundle
  contains 33 component lessons plus installation material.
- The product page's [officially linked user manual][user-manual] is a second
  primary reference. It is not revision-named in its URL.

The official web page gives exact quantities for only this subset: one Mega
2560, USB cable, breadboard, 65-wire jumper set, 74HC595, active buzzer, tilt
switch, photoresistor, RGB LED, five each of red/green/blue/yellow LEDs, five
small buttons, five female-to-male wires, and ten each of 10 Ω, 100 Ω, 220 Ω,
330 Ω, 1 kΩ, 2 kΩ, 5.1 kΩ, 10 kΩ, 100 kΩ, and 1 MΩ resistors. The revisioned
tutorial confirms the remaining functional parts below but does not provide a
machine-readable carton quantity table. Counts must therefore remain
“one lesson unit” unless the official page states otherwise.

## Coverage vocabulary

- **Supported**: a first-class RAII interface and current lesson/example exist.
- **Composite**: usable through supported endpoints, but no part-specific
  adapter is warranted.
- **In progress**: first-class source exists in the current worktree but its
  full release boundary is not yet published.
- **Planned**: named by the canonical curriculum.
- **Missing**: no first-class adapter or committed curriculum boundary.
- **Infrastructure**: a construction or power item, not a software component.

## Board, construction, and discrete parts

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| Mega 2560 controller | 1 | Supported: `Mega2560Board`, `Runtime` | Physical bench records remain per lesson |
| USB cable | 1 | Infrastructure: CLI upload/monitor workflow | None; document data-capable cable diagnosis |
| 830-point breadboard | 1 | Infrastructure in lesson schematics | Add continuity/rail-break orientation lab |
| 65-piece male jumper set | 1 set | Infrastructure | No software adapter |
| Female-to-male DuPont wires | 5 | Infrastructure | No software adapter |
| Breadboard power-supply module | 1 lesson unit | Planned for E2 loads | Add supply-owner policy and measured rail checks |
| 9 V, 1 A adapter | 1 lesson unit | Planned for E2 loads | Never feed actuators from a Mega pin; bench acceptance required |
| 9 V battery/connector | Datasheet only | Not a preferred ADK supply | Explain voltage sag and prohibit motor lessons from relying on it |
| Prototype expansion shield | Datasheet only | Missing | Optional board-layout adapter; low priority |
| Resistor assortment | 10 of each listed value | Composite in every circuit | Add resistor selection/tolerance worksheet |
| Ceramic capacitor assortment | Bundle datasheet | Composite | Add decoupling lesson evidence |
| Electrolytic capacitor assortment | Bundle datasheet | Composite | Add polarity, discharge, and stored-energy guidance |
| 1N4007 diode | Bundle datasheet | Planned with inductive drivers | Add flyback orientation and fault-observation test |
| PN2222 transistor | Bundle datasheet | Missing | Add bounded low-side-driver component before motor work |
| S8050 transistor | Bundle datasheet | Missing | Treat as a separately rated low-side-driver variant |

## Human input, light, and sound

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| Red, green, blue, yellow 5 mm LEDs | 5 each | Supported: `DigitalOutput`, `MonoLed`, lessons 001–003 | Add per-color forward-voltage measurement |
| White LED | Bundle datasheet | Composite: `MonoLed` | Require resistor calculation from measured forward voltage |
| RGB LED | 1 | Supported: `PwmOutput`, `RgbLed`, lesson 004 | Current code assumes the documented common-cathode circuit |
| Small pushbuttons | 5 | Supported: `DigitalInput`, `Button`, lessons 002–003 | None |
| Tilt-ball switch | 1 | Composite: `DigitalInput`/`Button` | Add named `TiltSwitch` semantics and orientation trace |
| Rotary encoder module | 1 lesson unit | Planned indirectly with operator input | Missing quadrature decoder and push-button composition |
| 4×4 membrane keypad | 1 lesson unit | In progress: `Keypad`, `MatrixKeypad`, lesson 016 | Complete release and Mega bench record |
| 10 kΩ potentiometer | 1 lesson unit | Supported: `AnalogInput`, lesson 007 | ADC reference ownership is intentionally deferred |
| Two-axis joystick with switch | 1 lesson unit | Composite from two `AnalogInput`s and one `DigitalInput` | Add calibrated `Joystick` value type and dead-zone lesson |
| Photoresistor | 1 | Supported composition in lessons 008–009 | A typed nonlinear sensor adapter remains optional |
| NTC thermistor | 1 lesson unit | Planned in lesson 008 | Add divider model and fixed-point temperature conversion |
| Active buzzer | 1 | Missing | Add simple `ActiveBuzzer` digital semantic component |
| Passive buzzer | 1 lesson unit | Supported: `PiezoSounder`, lesson 005 | Bench acceptance must identify the actual transducer |
| Sound-sensor module | 1 lesson unit | Composite analog/digital input | Add envelope/calibration adapter; never imply calibrated SPL |
| IR receiver and remote | 1 each | Planned: receive policy in lesson 025 | Add receive-only capture and decoded-command boundary |

## Sensors and time

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| HC-SR04 ultrasonic module | 1 lesson unit | Planned: lesson 019 | Missing pulse-timing endpoint, timeout, and validity model |
| DHT11 temperature/humidity module | 1 lesson unit | In progress: `Dht11Sensor`, `ClimateSensor`, lesson 013 | Complete deterministic transport faults and bench record |
| GY-521/MPU-6050 motion module | Revision-dependent | Missing | Add owned `I2cBus`, register transport, calibration, and sample model |
| QMI-8658 motion module | Revision-dependent alternative | Missing | Separate device adapter behind the same inertial-sample value type |
| HC-SR501 PIR module | 1 lesson unit | Composite: `DigitalInput` | Add warm-up, retrigger, and stale-observation semantics |
| Water-level sensor board | 1 lesson unit | Composite: `AnalogInput` | Add corrosion/duty-cycle warning; no unattended leak-safety claim |
| DS1307 or DS3231 RTC module | Revision-dependent | Planned: lesson 022 | Add `I2cBus`, validated calendar value, and clock-validity state |

## Displays, storage, and identification

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| LCD1602 parallel display | 1 | In progress: `CharacterDisplay`; curriculum lessons 014–015 | Add HD44780 transport adapter and release lesson |
| 74HC595 shift register | 1 | Supported: `ShiftRegister`, lesson 010 | Bench acceptance open |
| One-digit 7-segment display | 1 lesson unit | Supported: `SevenSegmentDisplay`, lesson 010 | Confirm common-anode/cathode variant on the bench |
| Four-digit 7-segment display | 1 lesson unit | Planned extension of lesson 010 | Missing multiplexed display owner and nonblocking refresh |
| MAX7219 8×8 LED matrix module | 1 lesson unit | Missing | Add shared SPI transport and bounded frame buffer |
| RC522 RFID module, card, and key fob | 1 lesson unit/set | Missing | Add SPI device lease and UID observation only; no access-security claim |

## Motion and switched loads

These are E2 parts. A supported adapter requires an external load supply,
physical power removal, a restrained first test, current evidence, and explicit
failure states. None may be powered directly from a Mega I/O pin.

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| SG90 servo | 1 lesson unit | Planned: lesson 017 | Missing timer-owned pulse endpoint, bounded angle, and external-power contract |
| 3–6 V DC motor and fan blade | 1 set | Planned: lessons 020–021 | Missing motor-driver adapter, dead time, current limit, and encoder feedback |
| L293D H-bridge IC | 1 lesson unit | Planned: lesson 020 | Add typed direction/enable driver with all-off failure state |
| 5 V relay | 1 lesson unit | Planned inert simulation: lessons 023–024 | Only a low-voltage inert indicator load; never mains |
| 28BYJ-48 stepper motor | 1 lesson unit | Missing | Add bounded stepper state machine after motor safety foundation |
| ULN2003 stepper-driver module | 1 lesson unit | Missing | Add four-channel driver ownership and all-off shutdown |

## Recommended implementation order

The kit does not justify flattening ADK into one wrapper per retail part.
Interfaces should follow shared electrical mechanisms:

1. finish analog sampling/filtering/night-light work;
2. finish `MatrixKeypad`, DHT11, and character-display release boundaries;
3. add an owned `I2cBus`, then RTC and one motion-device adapter;
4. add an owned `SpiBus`, then MAX7219 and RC522 device leases;
5. add pulse timing, then HC-SR04 and PIR semantics;
6. add rotary decoding and joystick composition;
7. add externally powered servo and motor endpoints under the E2 gate;
8. leave relay and stepper work until the driver, supply, and shutdown evidence
   is reusable.

This order makes kit breadth an integration test of the hierarchy instead of
creating unrelated component classes. Circuit-native verification should
remain part-specific: visible display patterns, LED state, audible cue,
electrical test points, or bounded inert motion. Serial remains supplemental.

## Known uncertainty

The live product page and revisioned tutorial do not expose one consistent,
complete, textual carton manifest. This audit therefore distinguishes exact
web-listed quantities from “one lesson unit.” Before a lesson names a specific
module, compare its markings and pin labels with the learner's actual kit and
the bundle's `Sensor&Module Version Notification.jpg`. A later Elegoo download
revision should produce a new dated audit rather than silently rewriting this
baseline.

[product]: https://www.elegoo.com/en-gb/collections/equipments-others/products/elegoo-mega-2560-the-most-complete-starter-kit
[tutorial-page]: https://www.elegoo.com/blogs/arduino-projects/elegoo-mega-2560-the-most-complete-starter-kit-tutorial
[tutorial-zip]: https://download.elegoo.com/01%20STEM%20Kits/02%20Mega%202560/Complete/ELEGOO%20The%20Most%20Complete%20Starter%20Kit%20for%20MEGA%20V1.0.2022.03.24.zip
[user-manual]: https://m.media-amazon.com/images/I/D1oC-c3G5TS.pdf
