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

- **Host verified; bench open**: a first-class RAII interface, deterministic
  tests, Mega example, and lesson package exist, but the exact specimen has no
  recorded physical acceptance.
- **Composite**: usable through host-verified endpoints, but no part-specific
  adapter is warranted.
- **Planned**: named by the canonical curriculum.
- **Missing**: no first-class adapter or committed curriculum boundary.
- **Infrastructure**: a construction or power item, not a software component.

These terms describe ADK artifacts, not carton contents. A planned behavior
does not support a module, and a host-verified model does not support an exact
adapter until its completed [inventory record][inventory-template] and bench
card identify the physical specimen.

## Board, construction, and discrete parts

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| Mega 2560 controller | 1 | Host verified; bench open: `Mega2560Board`, `Runtime` | Physical bench records remain per lesson |
| USB cable | 1 | Infrastructure: CLI upload/monitor workflow | None; document data-capable cable diagnosis |
| 830-point breadboard | 1 | Infrastructure in lesson schematics | Add continuity/rail-break orientation lab |
| 65-piece male jumper set | 1 set | Infrastructure | No software adapter |
| Female-to-male DuPont wires | 5 | Infrastructure | No software adapter |
| Breadboard power-supply module | 1 lesson unit | Infrastructure; exact board blocked pending inventory | No generic supply adapter is planned: record regulator, jumpers, polarity, source priority, backfeed behavior, and loaded rails before use |
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
| Red, green, blue, yellow 5 mm LEDs | 5 each | Host verified; bench open: `DigitalOutput`, `MonoLed`, lessons 001–003 | Add per-color forward-voltage measurement |
| White LED | Bundle datasheet | Composite: `MonoLed` | Require resistor calculation from measured forward voltage |
| RGB LED | 1 | Host verified; bench open: `PwmOutput`, `RgbLed`, lesson 004 | Current code assumes the documented common-cathode circuit |
| Small pushbuttons | 5 | Host verified; bench open: `DigitalInput`, `Button`, lessons 002–003 | Exact switches still need bench records |
| Tilt-ball switch | 1 | Composite: `DigitalInput`/`Button` | Add named `TiltSwitch` semantics and orientation trace |
| Rotary encoder module | 1 lesson unit | Planned indirectly with operator input | Missing quadrature decoder and push-button composition |
| 4×4 membrane keypad | 1 lesson unit | Host verified; bench open: `Keypad`, `MatrixKeypad`, lesson 016 | Record exact tail order and Mega bench evidence |
| 10 kΩ potentiometer | 1 lesson unit | Host verified; bench open: `AnalogInput`, lesson 007 | ADC reference ownership is intentionally deferred |
| Two-axis joystick with switch | 1 lesson unit | Composite from two `AnalogInput`s and one `DigitalInput` | Add calibrated `Joystick` value type and dead-zone lesson |
| Photoresistor | 1 | Supported composition in lessons 008–009 | A typed nonlinear sensor adapter remains optional |
| NTC thermistor | 1 lesson unit | Planned in lesson 008 | Add divider model and fixed-point temperature conversion |
| Active buzzer | 1 | Missing | Add simple `ActiveBuzzer` digital semantic component |
| Passive buzzer | 1 lesson unit | Host verified; bench open: `PiezoSounder`, lesson 005 | Bench acceptance must identify the actual transducer |
| Sound-sensor module | 1 lesson unit | Composite analog/digital input | Add envelope/calibration adapter; never imply calibrated SPL |
| IR receiver and remote | 1 each | Host verified; bench open: `PulseCapture`, `InfraredDecoder`, lesson 025 | Exact receiver electrical adapter and known-remote bench trace remain open; unknown replay and transmit are excluded |

## Sensors and time

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| HC-SR04 ultrasonic module | 1 lesson unit | Host verified; bench open: `PulseInput`, `UltrasonicRanger`, lesson 019 | Exact module timing and Mega bench evidence remain open |
| DHT11 temperature/humidity module | 1 lesson unit | Host verified; bench open: `Dht11Sensor`, `ClimateSensor`, lesson 013 | Exact device identity, electrical timing, and bench record remain open |
| GY-521/MPU-6050 motion module | Revision-dependent | Missing | Add owned `I2cBus`, register transport, calibration, and sample model |
| QMI-8658 motion module | Revision-dependent alternative | Missing | Separate device adapter behind the same inertial-sample value type |
| HC-SR501 PIR module | 1 lesson unit | Composite: `DigitalInput` | Add warm-up, retrigger, and stale-observation semantics |
| Water-level sensor board | 1 lesson unit | Composite: `AnalogInput` | Add corrosion/duty-cycle warning; no unattended leak-safety claim |
| DS1307 or DS3231 RTC module | Revision-dependent | Host-verified bus and RTC state models only: `I2cBus`, `Rtc`, lesson 022 | No DS1307/DS3231 register adapter is claimed; identify chip, pull-ups, charging circuit, and cell first |

## Displays, storage, and identification

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| LCD1602 parallel display | 1 | Host verified; bench open: `CharacterDisplay`, lessons 014–015 | Exact controller/pinout and physical acceptance remain open |
| LCD1602 with PCF8574 backpack | Revision-dependent variant | Planned behavior only | `CharacterDisplay` does not imply PCF8574 support; inventory address straps, expander mapping, pull-up rails, backlight driver, and controller separately |
| 74HC595 shift register | 1 | Host verified; bench open: `ShiftRegisterOutput`, lesson 010 | Exact IC and load-current bench acceptance open |
| One-digit 7-segment display | 1 lesson unit | Host verified; bench open: `SevenSegmentDisplay`, lesson 010 | Confirm common-anode/cathode variant on the bench |
| Four-digit 7-segment display | 1 lesson unit | Planned extension of lesson 010 | Missing multiplexed display owner and nonblocking refresh |
| MAX7219 8×8 LED matrix module | 1 lesson unit | Missing | Add shared SPI transport and bounded frame buffer |
| RC522 RFID module, card, and key fob | 1 lesson unit/set | Missing | Add SPI device lease and UID observation only; no access-security claim |

## Motion and switched loads

These are E2 parts. A supported adapter requires an external load supply,
physical power removal, a restrained first test, current evidence, and explicit
failure states. None may be powered directly from a Mega I/O pin.

| Kit part | Official quantity | ADK coverage | Adapter or instructional gap |
|---|---:|---|---|
| SG90 servo | 1 lesson unit | Host verified; bench open: `ServoOutput`, `BoundedServo`, lesson 017 | Exact servo and external-power bench acceptance remain open |
| 3–6 V DC motor and fan blade | 1 set | Host-verified intent and rover models: lessons 020–021 | No exact motor is supported until driver, supply, current, motion, and bench evidence pass |
| L293D H-bridge IC | 1 lesson unit | Host verified; bench open: `HBridgeOutput`, lesson 020 | Exact IC wiring, clamp behavior, load current, and thermal evidence remain open |
| 5 V relay | 1 lesson unit | Host-verified inert simulation only: lessons 023–024 | No physical relay adapter or contact load is claimed; never mains |
| 28BYJ-48 stepper motor | 1 lesson unit | Missing | Add bounded stepper state machine after motor safety foundation |
| ULN2003 stepper-driver module | 1 lesson unit | Missing | Add four-channel driver ownership and all-off shutdown |

## Recommended implementation order

The kit does not justify flattening ADK into one wrapper per retail part.
Interfaces should follow shared electrical mechanisms:

1. preserve the host-verified lessons through 039 while completing their exact
   specimen bench cards;
2. front-load optical work in 040–042, then one identified inertial revision
   and the balance-table instrument in 043–045;
3. use the listing-authorized Metal Touch family only after exact identity,
   with a contact or existing joystick fallback, alongside bounded stepper
   motion in 046–048;
4. add the RC522 identity carousel in 049–051 and identified, known-code IR
   work in 052–054;
5. compose the cooperative escape-room console in 055–057, then add the
   four-digit and MAX7219 display laboratory in 058–060;
6. place the lower-immediacy Water Level, thermistor, unidentified Digital
   Temperature, passive radiant/flame, and reed work in 061–063;
7. keep listed 18B20 single-wire work distinct in 064–066, then reuse the
   identified inertial adapter for normalized records in 067–069;
8. publish descriptor and copied-sweep E0 policy in 070–072, qualify any exact
   powered analog/comparator variant only in a separate E1 boundary, and retain
   073–078 for later authorized-family re-scoping; and
9. leave the low-energy qualification bench at 079–081.

This order makes kit breadth an integration test of the hierarchy instead of
creating unrelated component classes. Circuit-native verification should
remain part-specific: visible display patterns, LED state, audible cue,
electrical test points, or bounded inert motion. Serial remains supplemental.

The 2026-07-27 audit historically discussed rain, soil-moisture, generic
capacitive-touch, and heartbeat/pulse candidates while comparing broader
seller taxonomies. The cited Elegoo manifests do not authorize those families,
so the engagement-first curriculum removes them from planned coverage.
`Digital Temperature` and `18B20 Temp` are separate first-party inventory
labels: the former remains unidentified pending specimen proof, while the
latter belongs to the single-wire thermal arc. Metal Touch is authorized only
at listing level until exact topology and safe stimulus are established.

## Explicit non-coverage and aliases

- PCF8574 is a future, inventory-gated I2C expander adapter. A working parallel
  LCD lesson does not establish its address, backpack pin map, pull-up voltage,
  or backlight circuit.
- Breadboard power boards and prototype shields are construction specimens,
  not interchangeable software components. Their regulator, jumper, polarity,
  backfeed, and thermal behavior must be inventoried per board revision.
- Gas-board names such as `MQ-2`, “gas,” “smoke,” or “air quality” are aliases,
  not supported measurements. Lessons 070–072 may characterize an identified
  low-voltage analog/comparator output with harmless supplied traces; it does
  not authorize gas exposure, heating a sensor, concentration units, alarms,
  or safety claims.
- Rain, soil-moisture, generic capacitive-touch, and heartbeat/pulse boards
  are absent from the cited manifests. Historical planning references do not
  authorize them. Water Level, Metal Touch, and the two separately labelled
  temperature families must not be broadened into those absent categories.
- `KY-` identifiers, seller lesson numbers, PCB color, and bundle position are
  searchable aliases only. They never select a C++ type or waive exact
  electrical identity.
- Mercury switches, unidentified lasers or emitters, physiological claims,
  unknown RF/IR command replay, mains relay loads, ignition, and pyrotechnic
  control remain excluded.

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
[inventory-template]: ../inventory/exact-module.md
