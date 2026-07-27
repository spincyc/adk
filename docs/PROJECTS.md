# Project cadence

Every third lesson is a complete project. The two preceding lessons introduce
one or two components; the project composes them with everything already
learned. Projects never hide new hardware behind unexplained example code.

This is the curriculum contract, not a promise that every named driver already
exists. A project is publishable only when its component interfaces, host
fakes, Mega 2560 sketch, HTML guide, PDF field lesson, and acceptance evidence
are complete.

## Current status

The Reaction Timer (003) and Simon engine (006) are implemented and host
verified. Their APIs and lessons are experimental; Mega 2560 bench acceptance
remains open. Projects 009--030 are briefs, not implemented claims.

## Common project rules

- Use the first-class ADK interfaces. Imported examples live under `legacy/`.
- Inject clocks, input traces, and sequence sources. Tests never wait on wall
  time or depend on entropy.
- Record the seed, configuration, timestamped inputs, state changes, and
  outputs needed to replay a run exactly.
- Derive outputs from explicit state. Avoid delays, hidden callbacks, and
  mutable globals.
- Test nominal behavior, boundaries, timestamp wraparound, initialization
  rollback, repeated shutdown, and injected hardware failures.
- Leave every endpoint inert after shutdown. Disconnect power before rewiring.
- Keep a debug LED, test point, or status pattern active beside the primary
  behavior. Budget its pin, current, timer use, claim conflicts, and safe state.
- Verify diagnostic and primary traces together. Serial may add detail but is
  never required to identify startup, ready, activity, and fault states.
- Use only low-voltage, current-limited kit circuits. Motors, relays, and other
  loads require rated drivers and separate supplies where specified.
- Keep HTML concise and searchable. Use PDFs for bench procedure, drawings,
  prediction tables, measurements, and printable acceptance records.

## Component runway

| Lessons | New material | Project |
|---|---|---|
| 001--003 | Output, input, `Button`, deterministic time | Reaction timer |
| 004--006 | `PwmOutput`, `RgbLed`, piezo sounder | Simon |
| 007--009 | `AnalogInput`, calibration, sampled filtering | Adaptive night light |
| 010--012 | Seven-segment display, shift register, finite-state timing | Traffic junction |
| 013--015 | Temperature/humidity sensor, LCD, serial records | Environmental station |
| 016--018 | Keypad, servo, persistent configuration | Inert access-control trainer |
| 019--021 | Distance sensor, motor driver, encoder | Bench rover |
| 022--024 | RTC, SD logging, buses, relay simulation | Greenhouse controller |
| 025--027 | Infrared and receive-only radio observation | Telemetry console |
| 028--030 | Operator panel, continuity simulation, event log | Inert show-cue simulator |

The exact sensor model may follow the kit inventory, but changing a part must
not change the deterministic behavior interface or its tests.

## Lesson 001 practice — Diagnostic beacon

**Build:** A button-free status beacon that runs a deterministic startup
self-test, blinks an LED without blocking, reports resource-claim failures as
a status pattern and optionally over Serial, and returns its pin to high
impedance on shutdown.

**Builds on:** Board context, lifecycle, pin claims, monotonic time,
`DigitalOutput`, and `MonoLed`.

**Kit:** Mega 2560, breadboard, LED, 220 Ω resistor, jumper wires.

**Evidence:** A table-driven fake clock proves the exact output trace, including
large time jumps and counter wraparound. Fault tests prove duplicate claims
fail without disturbing the first owner. A logic-analyzer or timestamped
serial trace checks the Mega timing tolerance.

**Safety:** The resistor is mandatory. Verify LED polarity and disconnect USB
before moving wires.

**Comparable exemplars:** Arduino's
[Blink Without Delay](https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay/)
establishes nonblocking timing; Mbed's
[`DigitalOut`](https://os.mbed.com/docs/mbed-os/v6.16/apis/digitalout.html)
shows a small endpoint abstraction.

## Lesson 003 — Reaction timer

**Build:** The beacon waits a seeded interval, signals “go,” accepts one
debounced press, displays the measured reaction time over serial, and rejects
early or simultaneous input.

**Builds on:** Diagnostic beacon, `DigitalInput`, `Button`, pull-up wiring,
debounced snapshots, and explicit state machines.

**Kit:** Prior project plus two pushbuttons and jumper wires.

**Evidence:** Fixed input traces cover bounce, early presses, timeout, exact
deadline presses, release-before-repress, simultaneous buttons, and timestamp
wraparound. Seed plus trace reproduces every score.

**Safety:** Use internal pull-ups and switches to ground. Never connect a pin
configured as an output directly to a supply rail.

**Comparable exemplars:** Arduino's
[Debounce](https://docs.arduino.cc/built-in-examples/digital/Debounce/) and
[State Change Detection](https://docs.arduino.cc/built-in-examples/digital/StateChangeDetection/)
introduce the underlying observations; ADK adds ownership and deterministic
replay. Compare the result with this
[Arduino reaction timer](https://projecthub.arduino.cc/vinikon/arduino-reaction-timer-my-kids-love-playing-this-3ad546),
which combines buttons, LEDs, and an LCD.

## Lesson 006 — Simon

**Build:** Four colored controls present a growing cue sequence. A host
simulator and the Mega run the same engine with fixed cue identifiers,
versioned pseudorandom generation, nonblocking sound, and explicit timing.

**Builds on:** Buttons, LEDs, PWM color, sounder, clocks, event snapshots, and
finite-state behavior.

**Kit:** Four buttons, four LEDs or one RGB LED plus indicators, resistors,
passive piezo, breadboard, Mega 2560.

**Evidence:** Golden vectors lock the sequence algorithm. Trace tests cover cue
timing, correct rounds, mismatch, timeout, restart, maximum length, chords, and
wraparound. Hardware acceptance replays a published trace.

**Safety:** Drive only a passive piezo within pin-current limits. A speaker
requires a transistor driver.

**Comparable exemplars:** Arduino's
[tone melody example](https://docs.arduino.cc/built-in-examples/digital/toneMelody/)
demonstrates scheduled notes, while the
[Arduino Simon project](https://projecthub.arduino.cc/Arduino_Scuola/6f7fefa6-6ce8-45cc-af59-fc3de55510d4)
provides a directly comparable circuit. ADK replaces sketch globals and
unrepeatable randomness with an independently tested engine.

## Lesson 009 — Adaptive night light

**Build:** A potentiometer calibrates a photoresistor threshold. The RGB lamp
fades on with hysteresis, exposes raw and filtered readings, and supports a
button-selected diagnostic mode.

**Builds on:** Simon's operator controls plus `AnalogInput`, sampled values,
calibration, filtering, PWM, and configuration snapshots.

**Kit:** Photoresistor, 10 kΩ resistor, potentiometer, RGB LED, button, current
limiting resistors.

**Evidence:** Recorded sample streams prove filter, hysteresis, mapping,
saturation, calibration bounds, and mode changes. Host tests inject noisy,
stuck, and out-of-range samples.

**Safety:** Confirm divider wiring before power. Each LED channel needs its own
resistor; remain within per-pin and aggregate board current limits.

**Comparable exemplars:** Arduino's
[Analog Input](https://docs.arduino.cc/built-in-examples/analog/AnalogInput/) and
[Smoothing Readings](https://docs.arduino.cc/built-in-examples/analog/Smoothing/)
provide the physical experiments that ADK turns into testable components.

## Lesson 012 — Traffic junction

**Build:** A two-direction traffic signal with pedestrian request, countdown
display, night mode, and a deterministic maintenance self-test. A shift
register expands outputs without changing the state engine.

**Builds on:** Timed state machines, buttons, light sensing, LEDs,
seven-segment display, and `ShiftRegisterOutput`.

**Kit:** Red/yellow/green LEDs, resistors, button, photoresistor,
seven-segment display, 74HC595, breadboard.

**Evidence:** Model-based tests assert that conflicting greens never occur,
minimum clearance intervals hold, requests are eventually served, and failure
forces all-red. Pin-level fake traces verify shift-register bit order.

**Safety:** This is a tabletop model, never a road controller. Calculate the
display and LED current budget; do not drive high-current lamps.

**Comparable exemplars:** Arduino's
[shiftOut tutorial](https://docs.arduino.cc/tutorials/communication/guide-to-shift-out/)
explains the 74HC595 connection; the state engine remains independent of the
chosen output transport.

## Lesson 015 — Environmental station

**Build:** A scheduled station samples temperature, humidity, and light,
presents current and min/max values on an LCD, and emits a stable serial record
format with sensor-health flags.

**Builds on:** Analog sampling, filters, displays, shared buses, scheduled
tasks, and explicit validity.

**Kit:** Supported temperature/humidity sensor, photoresistor, 16×2 LCD
(parallel or I²C backpack), buttons.

**Evidence:** Fixture streams cover valid samples, CRC or transport errors,
missing data, implausible changes, min/max reset, and display pagination.
Golden records verify locale-independent serialization.

**Safety:** Check each module's voltage and pinout rather than relying on wire
color. Do not expose hobby sensors to condensation or use readings for
life-safety decisions.

**Comparable exemplars:** Arduino's
[LiquidCrystal examples](https://docs.arduino.cc/learn/electronics/lcd-displays/)
show display wiring, and Adafruit's
[DHT guide](https://learn.adafruit.com/dht) documents sensor limitations and
sampling constraints. Adafruit's
[data-logger shield guide](https://learn.adafruit.com/adafruit-data-logger-shield)
is a useful later extension with timestamped durable records.

## Lesson 018 — Inert access-control trainer

**Build:** A keypad accepts a configurable code, an LCD shows prompts, and a
servo moves a cardboard latch model. Lockout timing and configuration storage
are deterministic and auditable.

**Builds on:** Operator panels, display composition, servo positioning,
persistent records, and fault-aware state machines.

**Kit:** Matrix keypad, hobby servo, LCD, buttons, cardboard mechanism, separate
regulated servo supply with common ground.

**Evidence:** Tests cover correct and incorrect codes, incomplete entries,
lockout boundaries, reset, corrupt storage, power-loss checkpoints, and servo
command limits. No test stores a real credential.

**Safety:** Demonstration only—not a security product. Use a soft cardboard
latch, keep fingers clear, and never power a servo from an I/O pin.

**Comparable exemplars:** Arduino's
[Sweep](https://docs.arduino.cc/learn/electronics/servo-motors/) introduces
servo control; the official
[EEPROM library](https://docs.arduino.cc/learn/built-in-libraries/eeprom/)
provides storage primitives whose failure policy ADK wraps explicitly. This
[Mega door-lock project](https://projecthub.arduino.cc/jayesh_nawani/door-lock-system-with-arduino-54d18a)
offers a direct component comparison, not a security design.

## Lesson 021 — Bench rover

**Build:** A two-wheel rover follows a scripted route, reports encoder motion,
stops for nearby obstacles, and can replay its behavior in a host simulation.
The first build runs with wheels raised.

**Builds on:** Distance sampling, motor driver, encoder counts, scheduled
control, and supervisory state machines.

**Kit:** DC motors and wheels, rated H-bridge module, distance sensor,
encoders if available, separate battery pack, chassis, emergency-stop button.

**Evidence:** A small kinematic fake checks route states, stopping distance,
timeouts, reversal dead time, stalled encoders, impossible range data, and
emergency stop. Hardware acceptance starts with unloaded motors.

**Safety:** No mains, roads, stairs, pets, or unattended operation. Use a rated
driver and fused/current-limited motor supply; never drive a motor from a GPIO.

**Comparable exemplars:** Arduino's
[Ping ultrasonic example](https://docs.arduino.cc/built-in-examples/sensors/Ping/)
shows time-of-flight measurement. The
[Arduino Motor Shield Rev3](https://docs.arduino.cc/hardware/motor-shield-rev3/)
documents a representative rated driver boundary.

## Lesson 024 — Greenhouse controller

**Build:** An environmental logger evaluates configurable schedules and
thresholds, records decisions to SD with RTC timestamps, and controls LEDs that
simulate fan, irrigation, and heater relays. Physical loads remain optional and
out of scope for the lesson.

**Builds on:** Environmental station, RTC, SD card, configuration, diagnostics,
and mutually constrained outputs.

**Kit:** Prior sensors, RTC, SD module, LCD, buttons, three LEDs; optionally a
rated low-voltage relay module driving inert test loads.

**Evidence:** A virtual clock and sensor fixtures cover daily schedules,
rollover, missing RTC, full or corrupt storage, hysteresis, sensor failure,
mutual exclusion, and safe restart. Golden logs reproduce every decision.

**Safety:** Default to LEDs. Do not switch mains, heaters, pumps, or unattended
loads. A relay's contact rating does not make breadboard wiring safe.

**Comparable exemplars:** Arduino's
[Datalogger](https://docs.arduino.cc/built-in-examples/communication/Datalogger/)
introduces SD records; the
[Arduino MKR ENV Shield guide](https://docs.arduino.cc/tutorials/mkr-env-shield/mkr-env-shield-basic/)
is a useful example of composing several environmental measurements.

## Lesson 027 — Telemetry console

**Build:** A desktop console combines wired sensors, infrared observations,
lawful receive-only radio records, health indicators, a display, and durable
logs under one explicit scheduler.

**Builds on:** Environmental records, bus ownership, storage, operator panels,
protocol-independent observations, and fault-aware presentation.

**Evidence:** Recorded input streams reproduce every displayed value, alarm,
and log record. Tests cover missing receivers, stale samples, malformed
captures, storage failure, clock wrap, and conflicting observations.

**Safety:** Radio work remains passive and lawful. The project has no transmit,
replay, remote-cloning, access-control bypass, or protected-service decoding
path.

## Lesson 030 — Inert show-cue simulator

**Build:** An operator panel loads a fixed cue schedule, requires staged
arming, simulates continuity, counts down, emits only LED/sound cues, logs every
decision, and enters a latched safe state on stop or fault.

**Builds on:** All earlier input, output, display, storage, timing,
configuration, logging, and deterministic state-machine work.

**Kit:** Keypad, LCD, buttons including a prominent stop, LEDs and resistors,
passive piezo, RTC, SD module. Continuity channels are switches and LEDs only.

**Evidence:** Property and trace tests prove no cue without all arming
conditions, stop dominance from every state, immutable schedules while armed,
deterministic simultaneous-event ordering, restart lockout, corrupted-file
rejection, complete logs, and safe shutdown.

**Safety:** This is an inert teaching simulator. It must not contain ignition
drivers, energetic material, transmitter cloning, RF replay, or launcher
control. Any real display must remain behind a certified commercial controller
and applicable law, training, site procedure, and manufacturer documentation.

**Comparable exemplars:** The
[NIST finite-state-machine guidance](https://www.nist.gov/publications/finite-state-machine-approach-digital-event-systems)
motivates explicit auditable state. The
[Open Lighting Architecture](https://docs.openlighting.org/doc/latest/index.html)
provides a mature event-driven show-control comparison, while the Arduino
[SD library examples](https://docs.arduino.cc/learn/programming/sd-guide/) show
local event recording. These are design references, not authorization for
pyrotechnic control.

## Publication gate

Before a project lesson is numbered complete:

1. all depended-on component lessons are complete and linked;
2. host tests pass with exceptions and RTTI disabled;
3. the Mega 2560 sketch compiles within its documented size budget;
4. the deterministic trace format and at least one replay fixture are public;
5. hardware acceptance records measured values, not “it worked”;
6. HTML links API contracts, source, tests, parts data, and troubleshooting;
7. the PDF supplies pencil-style orientation art, exact schematic, bench
   worksheet, expected measurements, exercises, and sign-off;
8. shutdown and every documented fault leave the circuit in its named safe
   state; and
9. an independent review confirms the project introduces no unexplained
   component or hidden safety assumption.
