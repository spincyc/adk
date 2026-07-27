# “37 sensor kit” taxonomy audit

Status: research only. A kit count, seller title, or `KY-` label is not a part
number and is never enough information for an ADK electrical contract.

## Finding

There is no canonical “37 sensor kit.” The phrase describes a price and bundle
size, not a stable module inventory. Three current manufacturer inventories
demonstrate the mismatch:

- [Keyestudio KS0487 v3.0][ks0487] has traffic lights, a motor module, soil
  moisture, pulse-rate, voltage-detection, and four-digit display modules.
- [Keyestudio KS0399/400/401 v2.0][ks0399] is a different 37-piece family;
  `KS0401` bundles a Mega 2560 while `KS0399` supplies no controller.
- [SunFounder Sensor Kit V2][sun-components] has BMP180, DS1302, PCF8591,
  I2C LCD1602, color, gas, and raindrop modules absent from the KS0487 list.
- [Elegoo’s upgraded inventory][elegoo] substitutes DS1307, GY-521, HC-SR501,
  two sound modules, two RGB modules, an LCD, and a breadboard supply.

Revisions also change membership. Therefore ADK should support an identified
electrical behavior and chip, not a vendor’s bundle position or marketplace
name.

## Required identity record

Record this before selecting an interface or applying power:

| Field | Why it matters |
|---|---|
| Vendor and kit SKU | Distinguishes unrelated “37” inventories |
| Kit revision and purchase date | Bundles change without changing the search name |
| PCB text on both sides | `KY-013`, `S`, `AO`, and `DO` are clues, not identity |
| Active-device marking | Separates a thermistor from DS18B20 or LM35 behavior |
| Connector labels and order | Three-pin modules do not share one pin order |
| Board schematic or traced circuit | Reveals pull-ups, comparators, inversion, and series parts |
| Supply and signal limits | “Arduino compatible” does not establish voltage limits |
| Observed inactive state | Establishes polarity without trusting a translated tutorial |

Photograph both PCB faces beside a written label. If the active-device marking
or pin order cannot be established, quarantine the module rather than probe it
under power. Use the durable
[exact-module inventory template](../inventory/exact-module.md); a planned
curriculum behavior or a host-verified generic endpoint is not an accepted
physical adapter.

## Canonical behavior taxonomy

The canonical name describes the circuit boundary. Parentheses show common
seller aliases, not interchangeable parts.

| Canonical behavior | Common aliases | Electrical contract to establish | Closest ADK support |
|---|---|---|---|
| Momentary contact input | button, push key | dry contact or conditioned output; active level; pull policy | `DigitalInput`, `Button` |
| Passive contact input | reed, magnetic spring, tilt ball, shake | contact topology, bounce, normally open/closed | `DigitalInput`, `Button`; semantic wrappers planned |
| Conditioned threshold input | tracking, line follower, obstacle, flame alarm, knock, touch | comparator polarity, threshold control, hysteresis, onboard indicator loading | `DigitalInput`; device wrappers planned |
| Linear analog source | potentiometer, analog rotation, joystick axis | output range, source impedance, rail behavior | `AnalogInput`, calibration |
| Resistive-divider source | photocell, photoresistor, thermistor, analog temperature | divider orientation/value, transfer direction, open/short behavior | `AnalogInput`, sampled filtering; sensor wrappers planned |
| Analog plus threshold source | analog Hall, sound, flame, vibration | separate `AO`/`DO` meanings, comparator polarity and threshold | endpoints exist; composed wrapper planned |
| Self-oscillating indicator | auto-flash LED, seven-color flash | supply/current and autonomous waveform | no dedicated wrapper |
| Current-limited light output | white LED, dual-color LED, RGB LED, traffic light | common terminal, polarity, resistor presence and value | `MonoLed`, `RgbLed`, `DigitalOutput` |
| Self-oscillating sounder | active buzzer | enable polarity, module driver, current, fixed internal tone | `DigitalOutput` is electrically close; semantic wrapper needed |
| Externally excited sounder | passive buzzer, piezo | drive method, frequency/current limits, timer ownership | `PiezoSounder` |
| Pulse/edge source | rotary encoder, photo-interrupter, pulse-rate | output stage, pull-up, debounce versus edge timing | digital endpoints exist; encoder/pulse abstraction planned |
| Timed pulse transducer | ultrasonic ranger | trigger level/width, echo voltage, timeout | planned `PulseInput`/`UltrasonicRanger` |
| Single-wire digital sensor | DS18B20, “18B20,” digital temperature | exact chip, bus pull-up, conversion timing and resolution | not implemented |
| Proprietary timed sensor | DHT11, humiture, temperature/humidity | exact device, timing, stale/invalid sample policy | `Dht11Sensor` exists; bench acceptance still required |
| Register bus device | BMP180, GY-521/MPU6050, DS1302/DS1307 RTC, PCF8591, color sensor, LCD1602 backpack | bus type, address, voltage, register identity, pull-ups | `I2cBus`/`SpiBus` and generic RTC/display models are host verified; these chip/module adapters remain planned |
| Switched load output | relay, motor module | input polarity, isolation truth, coil/load supply, safe state | outside E1; inert-load wrappers planned |
| Infrared receive | IR receiver | carrier/demodulator identity, output polarity, timing | receive-only abstraction planned |
| Infrared or laser emit | IR transmitter, laser emitter | optical limits and documented harmless target | no generic support; review required |
| Exposed conductive probe | rain, water level, soil moisture, steam | excitation method, corrosion, contamination, analog range | `AnalogInput` can sample; long-term sensor contract absent |
| Human physiological indication | pulse-rate, heart-rate | module identity, placement, signal conditioning | research only; no medical claim or supported component |
| Power source/regulator | breadboard power module, voltage detector | input topology, polarity, rail selection, regulator limits | not a sensor; inspect separately before use |
| Heated gas-response board | MQ-family, gas, smoke, air quality | exact sensing element, heater supply/duty, analog/comparator circuit, controlled stimulus, ventilation, disposal, calibration and claim boundary | no physical adapter; lesson 057 can characterize only identified low-voltage outputs and cannot claim gas concentration or safety |

## Names that must remain distinct

### Active and passive buzzer

An active buzzer contains an oscillator and accepts an enable-like signal. A
passive piezo requires a waveform. Both are sold as “buzzer modules,” yet they
need different APIs and timer behavior. ADK’s `PiezoSounder` models the latter,
not both.

### Analog Hall, Hall switch, and reed switch

These are three contracts. SunFounder describes a linear Hall output plus a
comparator output, a Hall switch with Boolean output, and a passive reed contact
separately. Its analog Hall lesson states that `A0` varies while `D0` becomes
low past the adjustable threshold; its Hall-switch lesson describes an
active-low conditioned output. A reed is a mechanical contact and can bounce.
[SunFounder analog Hall][hall-linear],
[Hall switch][hall-switch], and [reed switch][reed] are useful examples of why
“magnetic sensor” is not a type.

### Analog temperature, thermistor, DS18B20, and DHT11

“Temperature sensor” may mean a bare divider, a linear-output semiconductor, a
1-Wire digital device, or a timed temperature/humidity device. They do not
share sampling, calibration, fault, or ownership behavior. Identify the active
device before choosing an interface.

### Big sound, small sound, analog sound, and knock

Board size is not behavior. Microphone modules may expose an analog envelope,
a comparator threshold, or both; a “knock” board may instead use a piezo or
spring contact. Record output labels and trace the comparator before naming the
component.

### IR tracking, obstacle avoidance, receiver, and transmitter

Tracking and obstacle boards illuminate a local target and emit a conditioned
digital result. A receiver demodulates a remote carrier. A transmitter emits a
carrier. Sharing “IR” in a seller name does not make their endpoints or safety
policy interchangeable.

### RGB LED variants

Through-hole, SMD, common-anode, common-cathode, current-limited, and
unresisted RGB boards appear under the same name. ADK’s current `RgbLed`
lesson requires the documented channel polarity and external current limiting;
it must not infer either from color or connector count.

### PCF8574 backpack and parallel LCD

`CharacterDisplay` behavior and a parallel LCD circuit do not establish a
PCF8574 backpack adapter. Backpacks vary in expander suffix, address straps,
LCD bit mapping, pull-up rail, and backlight polarity. Record each of those
fields before an I2C adapter is selected.

### Power board and regulated supply

A breadboard power module is a construction specimen, not a generic power
component. Similar boards can differ in regulator, input polarity, USB/barrel
priority, jumper mapping, reverse-current behavior, current limit, and thermal
performance. Keep it unpowered until the exact board record is complete.

### Gas, smoke, and air-quality boards

Seller aliases do not establish a sensing chemistry or calibrated unit. A
future descriptor may expose identified raw and comparator behavior, but it
does not by itself authorize heater power, gas exposure, concentration claims,
alarm use, or life-safety conclusions.

## ADK admission rule

Support a module alias only after one physical specimen has:

1. a recorded identity and authoritative schematic or verified trace;
2. measured supply, inactive output, and signal polarity;
3. an energy-class review;
4. a canonical behavior mapping above;
5. host fault vectors for open, short, stuck, saturation, and timing where
   applicable;
6. a Mega 2560 narrative example with non-Serial observation;
7. a bench card that names the exact vendor, SKU, revision, and specimen.

Aliases belong in documentation metadata, not C++ type names. For example,
`KY-013` may be recorded as an alias of a verified resistive thermistor module;
it must not become `Ky013Sensor`, because another PCB carrying that marketplace
label may use different values or pin order.

## Recommended implementation order

The inventory maps naturally onto the curriculum without adding one wrapper per
PCB:

1. finish `AnalogInput`, calibration, sampling, and night-light evidence;
2. add reusable conditioned-threshold and analog-plus-threshold compositions;
3. add contact semantics for reed, tilt, and vibration while reusing `Button`
   debounce policy explicitly;
4. add pulse capture, then rotary encoder and ultrasonic ranging;
5. add owned I2C and single-wire transports before chip-specific sensors;
6. admit DHT11 and display adapters only after exact hardware acceptance;
7. defer relays, motors, lasers, gas/flame alarms, and physiological modules to
   their stronger safety and claim boundaries.

This yields a small hierarchy based on observable electrical behavior while
still letting lessons say which kit modules can instantiate it.

## Source limits

Manufacturer lesson pages are useful inventory and wiring evidence, but are not
silicon datasheets. Some omit chip markings, resistor values, absolute limits,
or board revisions. Marketplace copies can silently change those details.
Before hardware support, follow the PCB’s active-device marking to its
manufacturer datasheet and reconcile it with the actual board circuit.

[ks0487]: https://docs.keyestudio.com/projects/KS0487/en/latest/ks0487.html
[ks0399]: https://docs.keyestudio.com/projects/KS0399-KS0400-KS0401/en/latest/KS0399%2C0400%2C0401.html
[sun-components]: https://docs.sunfounder.com/projects/sensorkit-v2-arduino/en/latest/components.html
[elegoo]: https://www.elegoo3dprinters.com/product/elegoo-upgraded-37-in-1-sensor-modules-kit-with-tutorial-compatible-with-arduino-ide-uno-r3-mega-nano/
[hall-linear]: https://docs.sunfounder.com/projects/sensorkit-v2-arduino/en/latest/lesson_2.html
[hall-switch]: https://docs.sunfounder.com/projects/sensorkit-v2-arduino/en/latest/lesson_3.html
[reed]: https://docs.sunfounder.com/projects/sensorkit-v2-arduino/en/latest/lesson_18.html
