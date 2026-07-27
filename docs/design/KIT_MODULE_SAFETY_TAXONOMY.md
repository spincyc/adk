# Kit module safety and observability taxonomy

Status: design inventory; not a supported-parts list  
Board reference: Mega 2560 Rev3-compatible, 5 V logic  
Scope: likely parts in Elegoo Mega “Most Complete” and 37-sensor kits

This document is a triage map for future component work. Kit names are not
electrical identities: suppliers change sensors, PCB layouts, regulators,
pull-ups, polarity, and pin order without changing the marketing name. A
physical lesson names and photographs the exact board revision, reads its
markings, traces its power path, and obtains the primary device and module
documentation before wiring. An unidentified or contradictory module is
blocked, not “tested carefully.”

The Elegoo lists establish likely inventory only. They do not override a
component datasheet, module schematic, measured pinout, or
[`SAFETY_MODEL.md`](../SAFETY_MODEL.md).

## Taxonomy keys

Power classes:

- **P0 — passive:** no supply; switches, resistors, passive transducers.
- **P1 — board logic:** documented 5 V-compatible device within a measured
  Mega/USB current budget.
- **P2 — low-voltage translated:** 3.3 V device or mixed-voltage module;
  regulator, pull-ups, and every signal direction must be known.
- **P3 — external load power:** motor, servo, relay, lamp, heater, or other
  load has a separate current-limited supply and rated driver.
- **PX — blocked pending identity:** pinout, voltage, current, protection, or
  module topology is unknown.

Hazard flags:

- **I:** inductive energy/back-EMF
- **H:** heating, hot surface, or ignition source
- **O:** optical exposure
- **M:** motion/pinch/entanglement
- **C:** relay contacts or external circuit
- **W:** liquid, corrosion, or conductive contamination
- **R:** intentional radio/infrared emission

Evidence channels:

- **TP:** named voltage/current/logic test point
- **VI:** resistor-limited visible indicator
- **AO:** bounded audible output
- **DO:** primary display or mechanical observation
- **FI:** independent fault indicator
- **PR:** physical power-removal verification

Serial is supporting evidence only. A module LED may show that its comparator
or power rail changed; it does not prove the external stimulus, sampled value,
load current, or safe shutdown.

## Rules that apply to every family

1. Wire only with all power removed. Record module identity, markings, revision,
   pin order, supply range, signal voltage, quiescent/peak current, and primary
   source before initialization.
2. A Mega pin is a logic endpoint, not a load supply. Use 20 mA as the board
   pin ceiling and design ordinary LED channels below it; never treat the
   ATmega2560's 40 mA absolute maximum as an operating target.
3. Inputs remain between GND and the selected reference/supply. Never drive an
   unpowered board. Do not drive AREF while the default AVCC reference is in
   use. ADC voltage reconstruction uses measured `Vref / 1024`; code 1023 is
   the top conversion interval, not exactly Vref.
4. A 3.3 V regulator does not prove the signal pins are 5 V tolerant. Onboard
   pull-ups can connect a bus to the wrong rail. Trace or measure every path.
5. External supplies need documented voltage/current limits and a common ground
   only where the selected driver requires it. Connect ground before logic;
   remove load power before logic power. Prevent back-power through signal pins.
6. Inductive loads require a named driver and a complete recirculation path.
   “The module has a diode” is accepted only after its topology and return rail
   are verified.
7. Startup, resource failure, active operation, sensor fault, shutdown, reset,
   and power removal each need an observable prediction. Software cleanup is
   not physical power removal.
8. A physical acceptance card records predicted and measured idle/active
   voltage, supply and load current, evidence pattern, fault injection, safe
   state, stop conditions, reviewer, and date.

## Family matrix

| Likely kit family and examples | Power | Principal gate | Required circuit-native evidence |
|---|:---:|---|---|
| Bare buttons, joystick switch, membrane keypad, tilt/ball switch, knock/shock spring, reed switch, rotary encoder | P0/P1 | Use pull-up-to-5 V and switch-to-GND unless the exact module requires otherwise; expose bounce, open, stuck, and chords | TP at input plus VI state/event pattern; open and stuck traces |
| Mercury tilt switch | PX, H/W | Do not use. Quarantine a glass or mercury-marked part; substitute a sealed ball-tilt sensor | Inventory record showing substitution |
| Potentiometer, thermistor, photoresistor, analog Hall element | P0/P1 | Divider current, ADC source impedance, Vref, saturation, and floating lead explicit | TP at divider and VI mapped output; both rail endpoints and disconnected-lead limitation |
| Comparator sensor boards: sound, light, flame, tracking, obstacle, Hall, touch, rain/water threshold | PX→P1/P2 | Identify sensor and LM393-like board, supply, comparator pull-up rail, potentiometer direction, and AO/DO pin order | TP for raw analog and digital threshold where present; VI for interpreted state and FI for invalid/saturated input |
| Bare microphone/electret and sound modules | PX→P1 | Never connect a bare microphone as if it were an amplified module; identify bias/amplifier and bound output to ADC rails | TP bias and envelope; VI threshold; prerecorded host trace beside live observation |
| Analog temperature IC, thermistor, DS18B20-style digital thermometer, DHT11 module | PX→P1/P2 | Exact part/pinout; supply and pull-up topology; distinguish invalid, stale, reset-default, and transport faults; no medical/safety claim | TP supply/data plus VI health pattern and primary temperature/humidity display |
| Soil-moisture, water-level, rain and conductive probes | P1, W | De-energize between samples where supported; current, corrosion, electrolysis, spills, and drying procedure explicit; never immerse electronics | TP probe excitation, VI valid/fault, visual corrosion inspection; current before/after test |
| PIR motion module | PX→P1 | Identify module supply/regulator and output-high voltage; warm-up and retrigger modes; no security or life-safety claim | TP output plus VI warm-up/ready/motion/fault pattern |
| HC-SR04-style ultrasonic module | PX→P1 | Exact module and timing; 5 V Echo only to 5 V board; no presence/safety interlock claim | TP Trigger/Echo or logic analyzer plus VI range-valid/timeout/out-of-range |
| IR receiver, line tracker and photo-interrupter | PX→P1/P2 | Supply/output levels and active polarity; ambient-light and missing-signal faults | TP demodulated/blocked output plus VI activity and FI timeout |
| IR emitter/remote | P1, O/R | Only documented, owned, harmless target; resistor/driver current bounded; no unknown-command replay | VI transmit intent plus camera/photodiode or receiver indication; inactive-state test |
| Laser emitter module | PX, O/H | Excluded from normal curriculum. Do not energize an unidentified laser; no eye exposure, reflective targets, alignment, or unattended operation | Unpowered identity/rating record only unless separately reviewed outside ADK |
| Bare single/two-color/RGB LEDs, LED bar, 7-segment and 8×8 matrix | P1 | One calculated resistor per independently driven path; per-pin, port, and aggregate current; multiplex duty; polarity | VI is primary, TP current/voltage on representative active paths, all-off safe state |
| WS2812/other addressable LED (if present) | PX→P2/P3 | Exact voltage, peak aggregate current, decoupling, signal level, and power injection; never power a long strip from a pin | VI test pattern, TP supply current/voltage at both ends, FI brownout, PR |
| Passive piezo disc/sounder | P0/P1 | Confirm passive part; bounded frequency/duty and optional series resistor; no startling or prolonged loud exposure | AO plus TP waveform; silent/high-impedance shutdown |
| Active buzzer or siren module | PX→P1/P3 | Identify polarity, operating voltage, current, onboard transistor, and sound level; a speaker is not a buzzer | VI command plus AO, measured current, silent shutdown and PR where externally powered |
| Character LCD, OLED, MAX7219 matrix, TM1637 display | PX→P1/P2 | Exact controller/module voltage; backlight/segment current; onboard pull-ups; 3.3/5 V bus compatibility | DO self-test and fault glyph/pattern; TP supply/bus; blank or documented safe display on shutdown |
| 74HC595 and other bare logic | P1 | Supply, decoupling, input levels, output and package-total currents; use drivers for loads | VI walking-one pattern plus TP clock/latch/data and outputs-disabled state |
| RTC modules | PX→P1/P2 | Identify RTC, pull-up rails, charging circuit, and installed cell chemistry; never charge a primary coin cell | DO time/health, TP bus and backup rail, FI oscillator/power-loss status |
| SD/microSD module | PX→P2 | Card and module are 3.3 V; verify regulator and level shifting in every signal direction; corruption/power-loss explicit | VI activity separate from commit success, DO read-back, TP 3.3 V and SPI |
| GY-521/MPU-6050-style IMU | PX→P2 | Identify regulator and I²C pull-up rail; no 5 V signal assumption; not a safety or navigation instrument | TP rails/bus, VI sample-health, stationary gravity/self-test record |
| RFID RC522-style board | P2, R | 3.3 V supply/signals unless exact module proves otherwise; authorized tags only; no access-control claim | VI reader state, DO tag-test result, TP 3.3 V/SPI; inactive RF state |
| IR receiver/transmitter pair and 315/433 MHz receiver modules | PX→P1/P2, R | Receive-only for unknown RF; exact frequency, voltage, legality, and privacy required; no replay/transmitter implementation | VI receive activity plus TP logic output; synthetic/prerecorded capture reproduces decoder |
| Servo | P3, M/I | Separate current-limited supply sized from measured stall current; signal must not back-power; soft fixture and independent disconnect | VI command/health, DO bounded motion, TP supply/current, FI timeout, PR |
| Small DC motor/fan | P3, M/I | Rated H-bridge/transistor, separate supply, stall current, flyback path, guarded fixture; never direct GPIO or 5 V pin | VI direction/enable, DO unloaded motion, TP motor current/voltage, FI stall/timeout, PR |
| 28BYJ-48 stepper with ULN2003 board | P3, M/I | Exact motor voltage/current; separate supply; verify ULN2003 COM connects clamp diodes to motor rail; thermal and sequence limits | VI phase pattern, DO restrained motion, TP coil current, FI sequence fault, PR |
| L293D/L298-class motor-driver board | P3, M/I/H | Exact IC/module; logic/load supplies separate; L293D has internal clamps while bare L293 requires external clamps; heatsink/current limits do not come from marketing | VI enable/direction, TP both rails and load current, DO wheels raised/unloaded first, FI overcurrent/stall, PR |
| Relay module | P3, I/C | Inert current-limited extra-low-voltage load only; identify coil supply, driver, flyback, optocoupler topology, active level, and contact separation; no mains | VI command distinct from contact-state TP, measured coil/load current, stuck/open fault, PR |
| Heating element, power resistor, thermoelectric module, flame source/sensor demonstration | PX, H | No flame, ignition, hot-wire, heater, combustible target, or unattended thermal experiment in ADK | Host simulation or ambient unheated sensing only |
| Breadboard power module, 9 V battery lead, wall adapter, barrel supply | PX→P1/P3 | Identify polarity/regulator/jumpers; current-limit before connection; never put 9 V on a 5 V rail; avoid simultaneous sources/backfeed | TP unloaded and loaded rails, measured current, VI power-good if independent, PR |
| Capacitors, transistor, diode and resistor assortments | P0/P1/P3 | Verify polarity/value/rating; discharge capacitors; transistor pinouts vary; diode orientation and load-energy rating come from exact datasheet | TP charge/discharge or switched node, VI bounded load, power-off residual-voltage check |

## Hazard-specific decisions

### Heating, flame, and laser

“Flame sensor” means passive observation of ordinary ambient IR changes. It
does not authorize creating a flame. Use a resistor-limited IR LED or a replayed
sample trace as the controlled stimulus. Heating modules, hot-wire elements,
thermoelectric loads, and ignition sources are outside ADK.

Laser modules remain unpowered until wavelength, output class, driver current,
and enclosure are independently documented. Because the same optical lesson
can use an LED, ADK's default is substitution, not a laser bench procedure.

### Relays

A relay module does not make its contacts touch-safe and “10 A” printing does
not rate the breadboard, jumpers, connector spacing, power source, or learner
procedure. Supported curriculum simulates relay outputs with LEDs. A later
physical relay acceptance may switch only an inert extra-low-voltage,
current-limited indicator after module-level review; it never switches mains,
a heater, motor, solenoid, launcher, or unknown circuit.

### Motion and inductive loads

The load supply, driver, motor/coil, clamp path, wiring, and mechanical fixture
form one reviewed system. Driver silicon maximum ratings are not project
operating points. Acceptance begins at the lowest practical current with a
servo linkage removed, motor unloaded, stepper restrained without trapping
fingers, or rover wheels raised. Reversal, reset, unplugged logic, stale
command, and loss of sensing must reach inactive drive. Physical power removal
is independent of firmware.

### Liquids and conductive probes

Only the sensing end approaches a small tray of clean water. The board,
breadboard, USB connector, battery, and mains-powered computer remain outside
the spill area. Use a drip loop where relevant, energize exposed electrodes
only for a bounded observation, dry them afterward, and discard visibly
corroded parts. These modules do not detect potable water, flooding, or a
safety condition.

## Admission checklist for a specific module

A module adapter can enter a lesson only after all answers are recorded:

- exact kit edition, module photographs, PCB markings, component markings,
  connector pinout, and primary sources;
- supply range, logic-high output, input tolerance, onboard regulator,
  pull-ups, indicator LEDs, protection parts, and idle/peak current;
- external-load supply, stall/coil/heater current, driver dissipation, clamp
  path, stored energy, and mechanical/thermal boundary when applicable;
- construction-inert state, initialized inactive state, active bounds,
  resource-conflict behavior, sensor-invalid state, shutdown state, and
  power-removed state;
- a circuit-native evidence channel for each claim, including what it cannot
  prove;
- an all-power-off inspection, current-limited first energization, explicit
  stop conditions, physical disconnect, measurements, reviewer, and date.

Any missing identity, rating, schematic, or protection answer assigns **PX** and
blocks wiring. A tutorial screenshot, seller listing, PCB color, or similar
module is not a substitute.

## Primary references

- [Elegoo Mega 2560 Most Complete kit](https://us.elegoo.com/products/elegoo-mega-2560-the-most-complete-starter-kit)
  and [official tutorial download](https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-mega-2560-the-most-complete-starter-kit-tutorial)
- [Elegoo upgraded 37-in-1 tutorial and datasheet download](https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-upgraded-37-in-1-sensor-modules-kit-tutorial)
- [Arduino Mega 2560 Rev3 hardware resources](https://docs.arduino.cc/hardware/mega-2560)
  and [official full pinout](https://docs.arduino.cc/resources/pinouts/A000067-full-pinout.pdf)
- [Microchip ATmega640/1280/1281/2560/2561 datasheet](https://www.microchip.com/en-us/product/atmega2560)
- [Microchip ADC input-circuit guidance](https://onlinedocs.microchip.com/oxy/GUID-6178B82B-1DE0-484B-ACA5-153F78A78E41-en-US-1/GUID-085B188C-E869-4105-8411-EB13F0E66D65.html)
- [TI L293/L293D motor-driver datasheet](https://www.ti.com/lit/ds/symlink/l293d.pdf)
- [TI ULN2003A Darlington-array datasheet](https://www.ti.com/lit/ds/symlink/uln2003a.pdf)
- [ST L298 dual full-bridge datasheet](https://www.st.com/resource/en/datasheet/l298.pdf)
- [Analog Devices DS18B20 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf)
- [TI LM35 temperature-sensor datasheet](https://www.ti.com/lit/ds/symlink/lm35.pdf)
- [NXP MFRC522 reader IC datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)

These references establish board and underlying-device limits. Clone-module
admission still requires the exact module schematic or a documented electrical
inspection because an underlying IC datasheet does not describe the PCB around
it.
