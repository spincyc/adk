# Project cadence

Every third lesson is a complete project. The two preceding lessons introduce
one or two components; the project composes them with everything already
learned. Projects never hide new hardware behind unexplained example code.

This is the curriculum contract, not a promise that every named driver already
exists. A project is publishable only when its component interfaces, host
fakes, Mega 2560 sketch, HTML guide, PDF field lesson, and acceptance evidence
are complete.

## Current status

Lessons 001--039 are host verified with their bench cards open. Lessons
037--039 use documented external reference fixtures; incoming conformance and
E1 acceptance remain open. The earlier Elegoo exact-specimen requirements
were superseded only as blockers for canonical publication and remain
historical optional substitution-conformance work. Lessons 040--042 are the
next queued block and have a
[provisional implementation-depth brief](design/LESSONS_040_042_OPTICAL_COURSE_MARSHAL_PLAN.md)
that remains gated by the Lesson 042 arming decision and its own
exact-specimen requirements.
See the [authoritative work
queue](WORK_QUEUE.md) for the complete ledger.

Every API and lesson remains experimental. Mega 2560 bench acceptance is open
for every project. Development continues through the full project sequence
without treating an unperformed bench card as a software blocker; no project
may claim hardware verification until its measured record is published.

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
| 016--018 | Keypad, bounded servo, supplied configuration | Inert access-control trainer |
| 019--021 | Distance sensor and motor driver | Bench rover |
| 022--024 | Owned buses, deterministic records, inert-load simulation | Greenhouse controller |
| 025--027 | Infrared and exact packet observation | Telemetry console |
| 028--030 | Operator panel, continuity simulation, event log | Inert show-cue simulator |
| 031--033 | Joystick, encoder, calibration policy | Calibration console |
| 034--036 | Magnetic/contact sensing and passage policy | Magnetic passage logger |
| 037--039 | Contact dynamics and acoustic envelopes | Percussion sequencer |
| 040--042 | Optical observations and presence policy | Tabletop course marshal |
| 043--045 | Revision-neutral inertial samples and orientation | Balance-table instrument |
| 046--048 | Authorized tactile/directional inputs and bounded stepper motion | Kinetic light sculpture |
| 049--051 | Local identity records and bounded homing | Tabletop parts carousel |
| 052--054 | Known-kit IR capture and exactly qualified transmission | IR command translator |
| 055--057 | Constraint model and fault-aware panel | Inert escape-room console |
| 058--060 | Multiplexed digits and MAX7219 presentation | Dual-display timing desk |
| 061--063 | Authorized Water Level, thermistor, Digital Temperature, radiant, and reed observations | Museum-case monitor |
| 064--066 | Single-wire transport and qualified 18B20 probes | Thermal gradient mapper |
| 067--069 | Inertial records and source qualification | Interchangeable motion recorder |
| 070--072 | Identified threshold descriptors and sweeps | Module characterization bench |
| 073--075 | Authorized-family replacements pending | Project pending re-scope |
| 076--078 | Authorized-family replacements pending | Project pending re-scope |
| 079--081 | Low-side driver and indicator semantics | Component qualification bench |

The exact sensor model may follow the kit inventory, but changing a part must
not change the deterministic behavior interface or its tests.

## Project delivery queue

Each row is a dependency boundary. Complete its non-hardware gates before
landing the next project consumer, while retaining the bench card as an
explicit open item.

| Project | Delivery state | Required deterministic artifact | Required physical observation |
|---:|---|---|---|
| 012 | Host verified; bench open | Traffic trace proving clearance timing, request service, all-red failure, and shift-register bit order | Display self-test plus mutually exclusive red/yellow/green indications |
| 015 | Host verified; bench open | Timestamped sensor fixtures, health transitions, display pages, min/max reset, and stable record vectors | Sensor-health indicator and displayed sample age beside optional Serial records |
| 018 | Host verified; bench open | Key-event traces, lockout boundaries, supplied configuration, restart, and bounded audit intent | Key acknowledgement, visible policy state, and inert soft-latch intent |
| 021 | Host verified; bench open | Range, route, stale/fault, reversal-dead-time, and stop-dominance traces | E1 direction/enable indicators; later wheels-raised direction and physical stop state |
| 024 | Host verified; bench open | Schedule, sensor-fault, RTC/storage-fault, hysteresis, exclusion, restart, and golden-log traces | Inert load LEDs, bus/storage activity, and unmistakable safe/fault combinations |
| 027 | Host verified; bench open | Configured packet fixtures, stale-data handling, scheduler order, and byte-stable record replay | Selection, health, acknowledgement, and storage states visible without Serial |
| 030 | Host verified; bench open | Immutable cue schedule, arming order, simulated continuity, simultaneous-event policy, stop dominance, restart lockout, and audit replay | Inert channel lamps, redundant armed/fault/stop states, and no energizing output path |
| 033 | Host verified; bench open | Input and commit/cancel traces | Preview, committed, and fault indications |
| 036 | Host verified; bench open | Passage, bounce, direction, recovery, and record traces | Raw contacts, accepted passage, and count |
| 039 | Host verified; bench/incoming conformance open | Sequencer core, attribution, atomic admission, replay, indexed-hit evidence, and exact external reference publication complete | Canonical Mega schematic/example, visible pattern evidence, and E1 acceptance card; incoming conformance remains open |
| 042 | [Queued; provisional implementation-depth plan complete](design/LESSONS_040_042_OPTICAL_COURSE_MARSHAL_PLAN.md) | Calibration, checkpoint-order, range, and timeout traces; implementation remains gated | Local checkpoints and invalid-run state |
| 045 | Queued; detailed plan required | Revision-specific inertial, orientation, and freeze traces | Existing LED/tone health and bus test points |
| 048 | Queued; detailed plan required | Authorized tactile/directional input, coil-frame, stop, and power-loss traces | Shift-register intent mirror and independent stop |
| 051 | Queued; detailed plan required | Identity, homing, confirmation, and interruption traces | Position, home, gate-intent, and stop evidence |
| 054 | Queued; detailed plan required | Encode/decode, malformed-frame, policy, and cancellation traces | Separate transmit/receive/fault indications |
| 057 | Queued; detailed plan required | Clue, permutation, fault, reset, and stop traces | Raw/accepted/fault clue states and inert intent |
| 060 | Queued; detailed plan required | Digit refresh, SPI failure, disagreement, and shutdown traces | Independent display self-tests and transport points |
| 063 | Queued; detailed plan required | Water Level, thermistor, Digital Temperature, radiant, reed, latch, and record traces | Health/fault display and inert alarm lamp |
| 066 | Queued; detailed plan required | Single-wire, CRC, conversion, identity, and record traces | Data/rail/activity test points and explicit invalid state |
| 069 | Queued; detailed plan required | Normalization, provenance, qualification, and replay traces | Display self-test, orientation, and dominant fault |
| 072 | Queued; detailed plan required | Descriptor, sweep, disagreement, and record traces | Raw, comparator, validity, and switched-power evidence |
| 075 | Re-scope required | Deterministic artifacts depend on the authorized families selected for 073--074 | Observation paths depend on the authorized replacement specimens |
| 078 | Re-scope required | Deterministic artifacts depend on the authorized families selected for 076--077 | Observation paths depend on the authorized replacement specimens |
| 081 | Queued; detailed plan required | Descriptor, current budget, endpoint fault, and record traces | Separate power, raw, accepted, intent, and fault evidence |

Each project package still includes the public component interfaces, host
tests, narrative Mega example, HTML reference, rich PDF, size evidence, and
open hardware card required by the development contract. “Queued” authorizes
planning only; it does not promote an interface or lesson.

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

**Build:** A potentiometer first establishes a known analog evidence chain.
The project then applies explicit calibration and filtering to a photoresistor.
The PWM lamp fades on with hysteresis and enters a distinct fault state for an
invalid sample.

**Builds on:** Simon's operator controls plus `AnalogInput`, sampled values,
calibration, filtering, PWM, and configuration snapshots.

**Kit:** Photoresistor, 10 kΩ resistor, potentiometer, RGB LED, button, current
limiting resistors.

**Circuit-native evidence:** Predict and measure the divider voltage at the
named analog test point, then observe the PWM lamp. Steady off, variable
brightness, and the documented fault indication distinguish normal and invalid
paths without Serial. Optional serial logging places the raw ADC count,
calibrated value, filtered value, controller mode, and duty beside those
physical observations.

**Host evidence:** Recorded sample streams prove filter, hysteresis, mapping,
saturation, calibration bounds, and mode changes. Tests inject noisy, stuck,
stale, and out-of-range samples and prove an invalid input requests zero lamp
duty.

**Safety:** Use one photoresistor and one 10 kΩ fixed resistor in the divider;
the fixed resistor limits current to 0.5 mA at 5 V even if the photoresistor
shorts. Confirm divider wiring with USB power removed. Each LED channel needs
its own 220 Ω or larger resistor, a calculated and measured current no greater
than 15 mA, and inclusion in the simultaneous-output current total. Rail
samples force the lamp off, but a floating A0 lead can look plausible and is
not claimed as reliably detectable. Remove USB power on heat, resets, an
out-of-rail TP1 reading, or an uncommanded output.

**Comparable exemplars:** Arduino's
[Analog Input](https://docs.arduino.cc/built-in-examples/analog/AnalogInput/) and
[Smoothing Readings](https://docs.arduino.cc/built-in-examples/analog/Smoothing/)
provide the physical experiments that ADK turns into testable components.

## Research runway — switched high-speed media and peripherals

These are architecture investigations, not Arduino lesson promises. Their
deterministic controller models and design documents have landed, but no
high-speed endpoint is implemented or supported. An Arduino Mega 2560 cannot
terminate, switch, or transport HDMI 2.1-class video or USB 3.x data. Its
useful role is a low-speed management controller: buttons, status indicators,
environmental telemetry, reset sequencing, and deterministic control-plane
tests around dedicated high-speed hardware.

### 8K HDMI over a switched network

**Research question:** Can a modular endpoint carry full-quality 8K HDMI
through a packet-switched fabric while preserving display discovery, content
protection, audio, timing, and bounded switching behavior?

**Likely system boundary:** Dedicated HDMI receiver/transmitter silicon,
FPGA/adaptive-SoC video pipelines, JPEG XS or another justified codec, and
25/100 GbE-class network interfaces perform the data plane. A Linux-class
processor at each attachment unit manages the endpoint. One durable controller
running on an ordinary Linux computer manages identities, authorization,
profile admission, route epochs, and audit records, but never carries media.
Computer-side receivers interpret video, audio, timing, and metadata; room-side
transmitters reconstruct a fresh HDMI link. ADK may provide a deterministic,
electrically isolated operator panel and health display; it does not touch
protected media keys or high-speed lanes.

HDMI routes share the household switched network with USB and ordinary LAN
traffic. Named profiles make resolution, refresh, color, compression,
bandwidth, and latency explicit. A route may pin one profile or use an explicit
ordered fallback set. Endpoints display the applied profile and fault state;
an active pinned route blanks and mutes rather than silently degrading when its
contract cannot be maintained.

**First proof:** Begin with synthetic, unprotected video and recorded control
traffic. Measure bandwidth, end-to-end latency, frame integrity, clock recovery,
multicast behavior, link loss, and route changes. Do not claim 8K, lossless
transport, HDMI compliance, or interoperability until instruments and licensed
test equipment establish them.

Active 8K60 RGB 8-bit pixels alone require about 47.776 Gb/s before blanking,
packet, audio, and resilience overhead. Use 100 GbE as the uncompressed
correctness baseline. Active 10-bit 4:4:4 pixels require about 59.72 Gb/s,
already beyond HDMI 2.1's 48 Gb/s link rate; an HDMI 2.1 endpoint must accept
a permitted reduced-chroma/depth or compressed mode instead of promising that
input format. A more practical first product would evaluate JPEG XS carried as
ST 2110-22 over 25 GbE, with measured image quality and latency.

**Stages:** Build a command-line packet model; prototype a 4K FPGA link; prove
unprotected generated 8K media; add matrix routing and NMOS; exercise link
loss, failover, and recovery; only then assess a licensed interoperable
product. Treat HDCP licensing, HDMI adopter requirements, EDID, CEC, audio,
PTP timing, electromagnetic compatibility, and patent/licensing review as
first-order work.

**Primary references:** [HDMI 2.1 overview][hdmi21],
[SMPTE standards catalog][smpte], [AMWA IS-04 discovery][is04],
[AMWA IS-05 connection management][is05], [JPEG XS 8K FPGA evaluation
brief][jpeg-xs], and an [AMD 100G-capable adaptive-SoC platform][amd-vek385].
The detailed [feasibility study](research/HDMI_8K_NETWORK.md) and
[phased architecture](research/hdmi_8k_switched_network.md), plus the
[dynamic HDMI mesh](research/HDMI_MESH_ARCHITECTURE.md) and
[shared-fabric contract](research/SHARED_USB_HDMI_FABRIC.md), keep the
assumptions, calculations, and deferred compliance work reviewable.

[hdmi21]: https://www.hdmi.org/spec2sub/res-bandwidth
[smpte]: https://www.smpte.org/standards/recently-updated-documents
[is04]: https://specs.amwa.tv/is-04/releases/v1.3.3/docs/Overview.html
[is05]: https://specs.amwa.tv/is-05/v1.1/docs/Overview.html
[jpeg-xs]: https://www.intopix.com/Ressources/Solution%20Brief/jpeg-xs-8k-fpga-evaluation-kit-solution-brief.pdf
[amd-vek385]: https://docs.amd.com/r/en-US/ug1712-vek385-eval-bd/Transceivers

### Transparent USB 3 mesh over a switched network

**Research question:** Can an unmodified Windows or Linux computer connect
through physical USB to an attachment unit, traverse the shared switched
network, and reach the exact peripheral or user-provided hub topology attached
to a room-side unit?

Treat this as a dynamically reconfigurable endpoint mesh rather than a fixed
crossbar or a Linux USB/IP product. The computer requires no custom driver,
service, kernel module, or virtual host controller. A `ComputerAttachmentUnit`
(`Cau`) presents one selected remote topology through one physical USB 3
Type-B computer connection. A `PeripheralAttachmentUnit` (`Pau`) provides four
independently powered USB 3 Type-A topology roots. A user-provided hub and all
of its descendants remain one atomic topology; the product inserts no hidden
hub and consumes no additional computer USB ports.

One durable controller runs on a normal Linux computer and stays out of the
data path. Each topology has at most one active `Cau`. Moves are
generation-fenced, break-before-make, cold-power-cycle the PAU-supplied VBUS,
and appear to Windows or Linux as native unplug/replug. High availability is
deferred; controller loss fails route changes closed.

**Likely system boundary:** Terminate and reconstruct USB at the two appliance
edges; never packet-switch USB physical-layer symbols. The PAU performs USB
host duties and observes the real topology. The CAU behaves as USB device-side
hardware and reconstructs equivalent descriptors, endpoints, topology changes,
and transactions. Both baseline appliances use one Cat6A-qualified 10GBASE-T
RJ45 carrying data and primary PoE++, with explicit auxiliary-DC fallback. The
one-port CAU is an isolated fault domain, senses but never draws operating
power from or backfeeds computer VBUS, and qualifies Type-B ESD and SuperSpeed
signal integrity. Every PAU port has protected, measured, controllable VBUS. The
Mega may operate buttons, displays, power telemetry, and deterministic
control-plane tests; it cannot proxy the SuperSpeed protocol or physical
layers.

**First proof:** Use owned, non-sensitive loopback and mass-storage test
devices on an isolated lab network. Measure enumeration, sustained and burst
throughput, latency, reset, disconnect, endpoint recovery, and route changes.
Expose physical port power, local ownership, network assignment, and fault
state without depending on Serial.

**Stages:** Use USB/IP only as a data-plane learning prototype. Build a
controlled transparent HID topology, then bulk loopback, storage, composite
devices, audio/video isochronous traffic, and user-provided hubs. Every stage
uses physical CAU-to-computer USB with native Windows and Linux stacks. Keep
authorization, confidentiality, hostile peripherals, firmware trust, USB-IF
compliance, signal integrity, and protected per-port power inside the
architecture from the start.

USB and HDMI share the ordinary household network. Profile-based admission
reserves bandwidth, latency, jitter, endpoint capacity, PoE budget, and
ordinary-LAN headroom before applying a route. A pinned profile either holds
or fails; ordered alternatives are explicit and never silent. If an active USB
contract fails, the CAU disconnects, the PAU removes VBUS, stale transfers are
fenced, and automatic fresh enumeration waits for the profile's configurable
stable-path interval. Endpoint displays show the applied profile, recovery
state, and exact fault without relying on Serial.

Deterministic fault injection is a disabled-by-default lab mode with separate
authorization, bounded duration, conspicuous `TEST` indication, and real
faults taking precedence. Production failure policy and deliberate injection
remain separate concepts.

The decoded payload ceiling is roughly 4.0 Gb/s for USB 3 Gen 1, 9.697 Gb/s
for Gen 2, and 19.394 Gb/s for Gen 2x2 before tunneling overhead and
headroom. That points to 10, 25, and 50 GbE respectively for one saturated
direction; four simultaneous Gen 2 ports justify a 100 GbE investigation.

**Primary references:** [USB 3.2 specification][usb32],
[Linux USB/IP protocol][usbip], [Linux USB host-side API][linux-usb], and the
[Mega 2560 Rev3 datasheet][mega-datasheet]. The canonical product decisions are
in the [transparent USB contract](research/USB_TRANSPARENT_PRODUCT.md);
the [mesh architecture](research/USB3_MESH_ARCHITECTURE.md),
[endpoint hardware plan](research/USB_MESH_ENDPOINT_HARDWARE.md), and
[shared-fabric contract](research/SHARED_USB_HDMI_FABRIC.md) retain the
research detail. The earlier
[USB/IP-oriented working note](research/USB3_NETWORK_MATRIX.md) is a prototype
reference, not the product contract.

[usb32]: https://www.usb.org/usb-32-0
[usbip]: https://docs.kernel.org/usb/usbip_protocol.html
[linux-usb]: https://docs.kernel.org/driver-api/usb/usb.html
[mega-datasheet]: https://docs.arduino.cc/resources/datasheets/A000067-datasheet.pdf

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

**Build:** A scheduled station turns validated temperature/humidity samples
into numbered LCD records and an RGB climate color. Missing DATA produces a
visible fault and restoring it recovers the complete story. A fixed two-minute
run ends in an automatic all-off shutdown; RESET replays it.

**Builds on:** The conditional DHT11-module path, parallel character display,
stable records, RGB presentation, scheduled tasks, and explicit validity.

**Kit:** An independently identified 5 V DHT11 module, independently identified
parallel LCD1602 with pins 1--16 documented, 10 kΩ contrast potentiometer,
common-cathode RGB LED, and three 330 Ω channel resistors. Family listings do
not establish an exact specimen or pin order. LCD A/K remain open until the
exact backlight is qualified.

**Learner progression:** Climate color, LCD hello, first numbered record,
humidity-reactive color, reversible missing-DATA fault, and automatic recovery.
Repository-only fixtures cover valid samples, transport and validation errors,
stale data, timing rollover, lifecycle rollback, and deterministic replay.

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

**Build:** A keypad accepts a configured demonstration sequence, an LCD shows
prompts, and an LED presents an inert soft-latch intent. Lockout timing,
component-fault handling, and bounded audit presentation are deterministic.
No credential storage or physical latch is part of this lesson.

**Builds on:** Operator panels, display composition, bounded servo intent, and
fault-aware state machines.

**Kit:** Matrix keypad, LCD, resistor-limited LED, buttons, and cardboard
pointer. The servo and its load supply remain disconnected.

**Evidence:** Tests cover correct and incorrect sequences, incomplete entries,
lockout boundaries, reset, component faults, shutdown, and bounded audit
intent. No test stores a real credential.

**Safety:** Demonstration only—not a security product. The published circuit
is LED-only and cannot secure or move anything. Lesson 017 separately describes
the open physical acceptance work for a soft paper pointer.

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

**Builds on:** Distance sampling, motor intent, scheduled control, and
supervisory state machines.

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
Arduino's data-logging example pattern
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

**Build:** An operator panel runs a fixed cue schedule, requires review and
confirmation, supplies complete synthetic continuity frames, emits only
resistor-limited LED cues, logs scheduler decisions, and enters a latched safe
state on cancel or fault.

**Builds on:** Buttons, mono and RGB LEDs, synthetic inert-channel assessment,
cue scheduling, bounded audit storage, wrap-safe time, and deterministic
state-machine work.

**Kit:** Mega 2560, nine momentary switches, eight cue LEDs, one
common-cathode RGB LED, and one 1 kΩ resistor per LED channel. Continuity is
edited as synthetic values; there is no external continuity connector.

**Evidence:** Unit and replay tests prove no cue without Closed selected-channel
evidence and confirmation, cancellation dominance, deterministic
simultaneous-event ordering, same-timestamp identity, malformed-frame
rejection, bounded complete logs, and safe shutdown.

**Safety:** This is an inert teaching simulator. It must not contain ignition
drivers, energetic material, transmitter cloning, RF replay, or launcher
control. Any real display must remain behind a certified commercial controller
and applicable law, training, site procedure, and manufacturer documentation.

**Comparable exemplars:** The
formal finite-state-machine guidance
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
