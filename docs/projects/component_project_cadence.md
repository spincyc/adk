# Extended component project cadence

## Purpose

This is the expansion plan after lesson 030. It gives the Elegoo Mega 2560
Most Complete Starter Kit and the common 37-module sensor assortment useful
jobs without turning the curriculum into 37 disconnected demos.

Module assortments change. Before implementation, record the exact part number,
board markings, schematic, supply voltage, output range, and datasheet in
`docs/inventory/`. A familiar `KY-` name is not an electrical specification.

The cadence remains fixed:

```text
lesson 3n + 1: one endpoint or physical measurement
lesson 3n + 2: one semantic component or processing policy
lesson 3n + 3: one complete, replayable project
```

Each project uses earlier components, introduces no hidden driver, and has a
useful circuit-native display before Serial is connected.

## Selection rules

- Group modules by measurement or control problem, not retail package.
- Prefer one calibrated sensor over several interchangeable threshold boards.
- Use near-duplicates to compare physics, noise, and failure modes.
- Keep raw evidence beside filtered or classified state.
- Inject time, samples, identities, and sequences into host behavior.
- Store fixed-capacity traces; no project requires heap allocation.
- Treat every actuator command as intent until a bounded endpoint accepts it.
- Use inert loads for relay, access, alarm, and show-control lessons.
- Do not energize an unidentified laser, mains circuit, ignition load, or RF
  transmitter.

## Lessons 031--033: calibration console

This input-first block is canonical. The complete interface, experiment, and
delivery contract is in the
[input expansion plan](../design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md).

### 031 — Analog joystick

Add two-axis calibration, dead zones, saturation, center drift, and switch
events over two `AnalogInput` objects and one `Button`. Keep both raw samples
beside normalized intent.

### 032 — Quadrature encoder

Decode every valid Gray-code edge into a bounded signed count. Invalid
transitions, reversal, saturation, and the independent push button remain
observable.

### 033 — Project: calibration console

The joystick selects a field and supplies coarse intent. The encoder trims it.
Explicit commit and cancel events update two bounded values while the LCD,
RGB LED, and binary LEDs keep preview and committed state distinct.

```text
joystick samples --> field/coarse intent --+
encoder edges -----> trim/commit/cancel ----+--> console --> LCD/RGB/LEDs
explicit time ------------------------------+
```

Deterministic evidence:

- raw minima, centers, dead-zone edges, saturation, and axis inversion;
- every Gray-code phase, reversal, invalid jump, and count boundary;
- atomic commit, cancel rollback, simultaneous-event precedence, and timeout;
- fault recovery only through explicit reinitialization; and
- byte-identical replay of snapshots and presentation intents.

Circuit-native observation:

- joystick axes have named analog test points;
- encoder phases have named logic test points;
- RGB distinguishes selection, editing, commit, cancel, and fault;
- binary LEDs retain a preview if LCD presentation fails; and
- shutdown evidence is separate from input-interpretation evidence.

Primary kit coverage: joystick, rotary encoder, buttons, LCD, RGB LED, binary
LEDs, and optional passive buzzer.

## Lessons 034--036: magnetic passage logger

### 034 — Magnetic and contact sensing

Compare digital Hall, analog Hall, reed switch, and ball/tilt switch modules.
Add polarity, hysteresis, dwell, and explicit open/short/stuck observations.
Do not use a mercury switch; substitute a sealed ball switch.

### 035 — Qualified passage events

Compose Hall/reed edges with dwell, direction, duplicate suppression, and
timeout into one hardware-neutral passage record. Reuse the lesson 032 encoder
as optional position evidence; do not redefine quadrature decoding.

### 036 — Project: magnetic passage logger

A tabletop gate counts tagged carriers moving through a channel. Hall and reed
observations indicate entry; the encoder represents conveyor position; RTC and
SD produce stable records. An RFID reader may identify a carrier without
changing passage detection.

```text
Hall/reed edges --> passage model --> count --> seven-segment display
encoder edges ----^        |                    status LED
RFID identity -------------+------------------> RTC-stamped SD record
```

Deterministic evidence:

- timestamped approach/departure traces for both magnet polarities;
- bounce, chatter, missing encoder edge, reversal, and impossible transition;
- duplicate RFID scans inside one passage window;
- RTC power-loss and SD full/partial-write faults; and
- restart from a committed fixed-size checkpoint.

Circuit-native observation:

- raw Hall and reed LEDs show the two sensing boundaries;
- the display shows the committed count;
- a latch pulse marks each accepted passage;
- the encoder A/B pins are named logic-analyzer points; and
- storage success/fault has a persistent LED pattern.

Primary kit coverage: analog Hall, digital Hall, linear Hall, reed switch,
magnet, rotary encoder, RTC, RFID RC522, SD module, seven-segment display.

## Lessons 037--039: vibration and sound laboratory

### 037 — Contact dynamics

Give tilt, vibration switch, knock, tap, and shock modules one common
timestamped transition boundary. Preserve pulse width and refractory policy;
do not reduce every device to an unqualified Boolean.

### 038 — Acoustic envelope and event classification

Model microphone analog envelope, threshold output, baseline calibration,
clipping, quiet/noisy confidence, and a bounded event window. This is an
amplitude experiment, not speech recognition or sound-level metrology.

### 039 — Project: percussion sequencer

Four physical surfaces trigger a deterministic step sequencer. Contact sensors
provide attack timing, the microphone supplies relative intensity, a
potentiometer sets tempo, and passive buzzer plus LEDs replay the recorded
pattern.

```text
contact edges -----> event window -----> fixed-capacity pattern
microphone samples -> intensity --------^          |
manual clock + tempo ------------------------------+--> light/tone frames
```

Deterministic evidence:

- recorded single tap, double tap, vibration, shock, and ambient-noise traces;
- exact refractory, quantization, simultaneous-hit, and buffer-full boundaries;
- microphone rail, unplugged, and threshold-disagreement faults;
- stable playback under uneven update intervals; and
- a seed-free replay whose output frames match byte for byte.

Circuit-native observation:

- one LED per surface reports accepted rather than merely raw hits;
- the matrix or four-digit display shows the active step;
- a heartbeat LED separates a quiet system from a stalled scheduler;
- the microphone analog output is a named oscilloscope point; and
- silence is the safe buzzer state.

Primary kit coverage: knock, vibration, shock, tap, sound/microphone, tilt,
active buzzer, passive buzzer, potentiometer, LEDs, display.

## Lessons 040--042: optical course marshal

### 040 — Reflective and interrupted light

Unify line-tracking, obstacle-avoidance, photo-interrupter, and light-cup
modules as explicit optical observations. Teach ambient rejection, threshold
calibration, hysteresis, crosstalk, and surface dependence.

### 041 — Presence and passage

Compose PIR warm-up/motion state, optical beam edges, and ultrasonic range into
a presence model. Each source retains validity and age; disagreement is an
observable result, not silently voted away.

### 042 — Project: tabletop course marshal

A hand-moved card or unpowered model vehicle passes checkpoints. Reflective
markers identify lanes, an obstacle sensor guards the finish, PIR arms the
course, and ultrasonic range confirms the finish approach. The project times
runs and rejects impossible checkpoint order.

```text
PIR --------> armed
line gates --> checkpoint order --+
obstacle --------------------------+--> marshal state --> light/display
range ------> approach validity ---+                 --> timing record
```

Deterministic evidence:

- dark/light calibration fixtures and slowly crossing threshold ramps;
- legal course, skipped gate, reversed gate, simultaneous gates, and timeout;
- PIR warm-up, stuck motion, optical saturation, and ultrasonic no-echo;
- permutations proving pins do not define checkpoint meaning; and
- complete run replay including displayed time and fault pattern.

Circuit-native observation:

- each checkpoint has a local accepted-event LED;
- display self-test precedes timing;
- all-red means invalid or unsafe course state;
- trigger, echo, and sensor digital outputs are named test points; and
- the project functions without a serial terminal.

Primary kit coverage: line tracker, obstacle detector, photo-interrupter,
light-cup/light-blocking module, PIR, HC-SR04, LDR, displays.

## Lessons 043--045: leak and thermal alarm trainer

### 043 — Resistive environmental probes

Add water-level, rain, and soil-moisture adapters over switched-power
`AnalogInput`. Teach corrosion-aware duty cycles, dry/wet calibration,
contamination, disconnected probes, and the limits of absolute readings.

### 044 — Thermal and radiant observations

Compare thermistor, analog temperature, digital temperature, and flame/radiant
IR modules. Preserve units, uncertainty, threshold state, validity, and age.
Use a TV remote or controlled low-energy IR source for the radiant experiment;
an open flame is unnecessary.

### 045 — Project: museum-case monitor

A model display case watches for liquid, excessive temperature, abrupt radiant
IR, and unauthorized opening. It presents health locally, logs timestamped
events, and drives only an inert relay indicator.

```text
wetness probes ---+
temperature ------+--> validated hazard model --> LCD/status/alarm intent
radiant IR -------+             |                --> inert relay lamp
reed contact -----+             +---------------> RTC/SD audit record
```

Deterministic evidence:

- dry-to-wet ramps, probe open/short, contamination drift, and stale samples;
- temperature boundary, disagreement, and sensor transport failures;
- radiant pulse traces and ambient saturation;
- alarm latch, acknowledgement, cooldown, restart, and log-write interruption;
- invariant: invalid sensing can never request the “healthy” indication.

Circuit-native observation:

- RGB state is healthy/warning/alarm/fault with a grayscale-safe blink code;
- LCD always includes sample age or fault;
- probe supply has a measurable duty-cycle test point;
- the inert relay drives a current-limited lamp only; and
- the alarm output is inactive after shutdown.

Primary kit coverage: water sensor, rain sensor, soil probe, thermistor, analog
temperature, digital temperature, flame/radiant sensor, reed, relay, LCD, RTC,
SD, RGB LED.

## Lessons 046--048: tactile kinetic sculpture

### 046 — Touch, proximity, and human gestures

Add capacitive touch, metal-touch, finger-heartbeat, and gesture-like switch
modules. Heartbeat modules are treated as noisy pulse demonstrations, never
medical devices.

### 047 — Bounded stepper motion

Add the 28BYJ-48/ULN2003 as four owned outputs plus a deterministic coil-frame
sequencer. Direction, phase, rate, travel budget, cancellation, and
de-energized shutdown are explicit.

### 048 — Project: kinetic light sculpture

Touch selects a pattern, a noisy pulse sensor modulates it, the joystick or
tilt sensor changes direction, and a stepper rotates a lightweight paper
element while RGB and matrix output mirror the commanded phase.

```text
touch/pulse/tilt --> gesture model --> bounded motion intent --> stepper
explicit time ---------------------^          |             --> RGB/matrix
stop button ----------------------------------+
```

Deterministic evidence:

- touch chatter, long hold, pulse dropout, impossible pulse rate, and tilt
  transitions;
- exact coil vectors, direction reversal, cancellation, and wrap boundaries;
- stop dominance at every state and no queued motion after shutdown;
- external-power loss and driver-fault injection; and
- frame-for-frame light and motion-intent replay.

Circuit-native observation:

- matrix frame mirrors commanded coil phase before the motor is powered;
- independent stop LED is active whenever motion is inhibited;
- each ULN2003 channel LED exposes coil intent;
- external motor power is physically separable from logic power; and
- the motor is de-energized on shutdown.

Primary kit coverage: capacitive touch, metal touch, pulse/heartbeat,
tilt/ball switch, joystick, stepper, ULN2003, RGB LED, matrix, stop button.

## Lessons 049--051: identity-controlled parts carousel

### 049 — Identity records and bounded enrollment

Turn RFID UID observations and keypad entries into fixed-size identity records.
Enrollment, duplicate identity, unknown identity, lockout, and corrupt storage
are explicit outcomes. A UID is an identifier, not proof of security.

### 050 — Positioning and homing

Compose stepper position, Hall/reed home sensor, travel limits, and
power-interruption recovery. Position is unknown until a bounded homing
procedure succeeds.

### 051 — Project: tabletop parts carousel

An RFID token requests one of several paper-part bins. The keypad confirms the
selection, the carousel homes and moves, the servo opens a lightweight gate,
and LCD/LEDs report each phase.

Deterministic evidence:

- known, unknown, duplicate, and rapidly repeated identities;
- homing success, missing home, stuck home, interrupted movement, and restart;
- authorization expiry and keypad conflict boundaries;
- exact step/servo command vectors with motor power absent; and
- invariant: no gate-open intent before identity, confirmation, and position.

Circuit-native observation:

- position LEDs and the home-sensor LED work before motors are powered;
- LCD names requested and confirmed bins;
- coil and servo intents are mirrored on LEDs in inert acceptance mode;
- stop removes actuator intent independently of UI state; and
- audit records survive a simulated interrupted write.

Primary kit coverage: RFID, keypad, stepper, Hall/reed, servo, LCD, EEPROM/SD,
buttons, status LEDs.

## Lessons 052--054: infrared protocol workbench

### 052 — Captured pulse trains

Extend receive-only IR work into bounded carrier-demodulated pulse records,
decoder confidence, repeat frames, unknown frames, and fixture export.

### 053 — Known-code infrared transmission

Add an IR LED endpoint for documented, learner-created codes only. Carrier
timer ownership, duty, burst duration, cancellation, and eye-safe
current-limited operation are explicit. This does not authorize replay of
access controls or unknown devices.

### 054 — Project: IR command translator

A keypad chooses one of several locally defined commands. The station sends it
between two adjacent breadboards, decodes it, and displays source, command,
confidence, and round-trip timing. Unknown remote captures are displayed but
cannot enter the transmit table.

Deterministic evidence:

- golden encode/decode vectors and malformed/truncated/noisy pulse trains;
- repeat handling, timeout, timer conflict, cancellation, and queue capacity;
- receiver saturation and missing carrier;
- source policy proving unknown captures are never transmitted; and
- round-trip replay at exact timestamp boundaries.

Circuit-native observation:

- transmit and receive activity use separate LEDs;
- a phototransistor or phone camera can confirm IR LED activity;
- LCD displays decoded command and confidence;
- timer conflict produces a distinct fault pattern; and
- all emitters are inactive after cancellation and shutdown.

Primary kit coverage: IR receiver, IR emitter, remote, keypad, LCD, timers,
buttons, LEDs.

## Lessons 055--057: modular sensor test bench

### 055 — Descriptor-driven threshold modules

Add a compact descriptor for the many modules that expose analog and
comparator outputs. It records polarity, range, pull requirement, warm-up,
settling, and threshold-pot direction without inventing a new class per PCB.

### 056 — Characterization runs

Add a deterministic sweep recorder and classifier comparison. A learner
supplies controlled samples; the behavior reports threshold crossing,
hysteresis, chatter, stuck output, and analog/digital disagreement.

### 057 — Project: module characterization bench

The bench accepts one low-voltage sensor module at a time, guides a safe test,
shows raw and threshold state, and emits a stable characterization record.
This is where vendor substitutions and remaining 37-kit variants are handled
honestly rather than forced into unrelated applications.

Deterministic evidence:

- ascending and descending ramps for active-high and active-low descriptors;
- chatter, rail, open, short, stale, and comparator disagreement;
- descriptor validation and unknown-module rejection;
- stable record serialization and interrupted storage; and
- replay from recorded raw samples without the physical module.

Circuit-native observation:

- seven-segment or LCD raw reading beside comparator LED;
- RGB validity state;
- named analog, digital, power, and ground test points;
- switched sensor power with an inactive default; and
- no module is connected before its voltage and pinout are identified.

Primary coverage: remaining analog/digital light, sound, Hall, temperature,
flame/radiant, touch, vibration, and obstacle board variants.

## Lessons 058--060: cooperative escape-room console

### 058 — Constraint and clue model

Add a fixed-capacity rule graph whose predicates consume existing semantic
observations. Rules are data, cycles are rejected, and hidden time is
forbidden.

### 059 — Fault-tolerant operator panel

Compose keypad, RFID, joystick, encoder, buttons, display, storage, and
redundant stop/acknowledgement behavior into one deterministic panel model.

### 060 — Project: tabletop escape-room console

Players solve optical, magnetic, motion, sound, identity, and ordering clues.
Success moves only a lightweight servo latch and lights an inert relay lamp.
Every clue can be replayed in host tests, so the capstone tests composition
rather than electrical improvisation.

Deterministic evidence:

- one golden trace per clue and a complete successful run;
- every clue permutation, repeated event, simultaneous event, and timeout;
- stale sensor, bus loss, storage full, reset, and power-removal recovery;
- property tests proving a failed prerequisite cannot unlock a dependent rule;
- invariant: stop, invalid configuration, or contradictory state forces all
  actuator intents inactive.

Circuit-native observation:

- each clue station has raw, accepted, and fault indications;
- the panel shows current rule and stale-input age;
- actuator intent is mirrored before external power is connected;
- stop state is electrically independent and visually dominant; and
- an append-only audit summary can be inspected after restart.

Primary coverage: deliberate reuse of all earlier endpoint families and a
meaningful integration test for the common kit.

## Deferred lessons 061--063: balance-table instrument

This block follows the canonical 031--060 kit expansion. Its numbers are
reserved so MPU6050 work cannot collide with the input-first block.

### 061 — MPU6050 inertial samples

Add an owned I2C accelerometer/gyroscope adapter, validated sample units, bias
calibration, and deterministic complementary filtering. Transport failure,
stale data, and physical saturation remain distinct.

### 062 — Orientation presentation

Map supplied inertial samples into bounded pitch/roll intent and an 8x8 matrix
cursor. Board orientation is explicit configuration. Matrix frames and tone
intent depend only on samples, calibration, and supplied time.

### 063 — Project: balance-table instrument

A handheld instrument maps tilt to a matrix cursor, uses the joystick to adjust
sensitivity, and freezes one measurement for comparison.

```text
joystick events ----+
                    +--> calibration --> orientation --> matrix frame
inertial samples ---+                         |       --> tone intent
explicit time --------------------------------+
```

Evidence includes stationary, tilt, rotation, saturation, dropout, I2C NACK,
stale-sample, axis-permutation, timestamp-wrap, and byte-identical replay
fixtures. Matrix self-test, RGB health, SDA/SCL test points, and joystick-axis
test points provide the non-Serial path.

Primary kit coverage: MPU6050, joystick, 8x8 matrix, RGB LED, passive buzzer,
button, potentiometer, I2C bus, and shift/display transport.

## Coverage ledger

This ledger prevents quiet omissions. “Primary” means a project depends on the
module's distinctive behavior; “variant” means the characterization bench
compares electrically similar retail boards.

| Module family | Primary project | Later reuse |
|---|---:|---:|
| LEDs, RGB, buttons, buzzers | 003/006 | all diagnostic paths |
| Potentiometer, photoresistor | 009 | 039, 042, 063 |
| Shift register, 7-segment, matrix | 012 | 036, 039, 063 |
| DHT, LCD | 015 | 045 |
| Keypad, servo | 018 | 051, 054, 060 |
| Ultrasonic, PIR, DC motor/driver | 021 | 042 |
| RTC, SD, relay | 024 | 036, 045, 060 |
| IR receiver and remote | 027 | 054 |
| Receive-capable RF | 027 passive only | no transmit project |
| Continuity and cue panel | 030 inert only | 060 fault model |
| Joystick, rotary encoder | 033 | 036, 048, 060, 063 |
| MPU6050 | 063 | future inertial projects |
| Hall variants, reed | 036 | 051, 060 |
| Tilt, knock, vibration, shock, sound | 039 | 048, 060 |
| Line, obstacle, interrupter, light cup | 042 | 057, 060 |
| Water, rain, soil | 045 | 057 |
| Thermistor, analog/digital temperature | 045 | 057 |
| Flame/radiant detector | 045, controlled IR only | 057 |
| Touch, metal touch, pulse demonstration | 048 | 060 |
| Stepper and ULN2003 | 048 | 051 |
| RFID | 036 | 051, 060 |
| IR emitter | 054, known local codes only | none required |
| Analog/comparator module variants | 057 | inventory acceptance |
| Laser emitter | none until classified | optional disabled optical fixture |

The last laser row is intentional. A module with unknown wavelength, optical
power, class, or labeling is not necessary to meet a learning goal. Once an
identified module has a cited safe-use procedure, it may replace the
current-limited visible LED in a fully enclosed optical fixture; otherwise it
remains unpowered.

## Common deterministic project contract

Every project behavior accepts one complete input frame:

```text
Frame {
    TimePoint now;
    SampleSet samples;
    EventSet events;
    FaultSet injectedFaults;
}
```

One frame causes at most one externally observable state transition. A
snapshot contains state, validity and age, next deadline, bounded actuator
intents, diagnostic intent, and the last outcome. Tests never sleep.

Required fixtures:

1. nominal golden trace;
2. just-before, exact, and just-after deadline trace;
3. timestamp repeat, large step, wrap, and backward-time trace;
4. every input conflict and resource conflict;
5. open, short, rail, stale, transport, and power-loss faults where applicable;
6. reset and partial-initialization rollback;
7. explicit shutdown and external exception-unwinding cleanup; and
8. serialized replay metadata with configuration and algorithm version.

Fuzz or property tests may generate additional frames, but must print the seed
and minimize a failure into a checked-in trace.

## Common observability contract

Each wiring plan reserves observation before feature wiring:

| Channel | Required evidence |
|---|---|
| Power | named logic and external-power boundaries; current budget |
| Raw input | voltage, pulse, bus, or edge test point |
| Accepted input | LED/display indication distinct from raw activity |
| Scheduler | heartbeat whose pattern cannot alter application timing |
| State | primary display or compact, documented LED code |
| Actuator | command-intent mirror usable with load power removed |
| Fault | persistent, dominant indication with safe output intent |

Serial and stored logs add precision. They are never the only way to tell
ready from stalled, accepted from raw, or safe from energized.

## Delivery record for other agents

Before implementing one three-lesson arc, an agent must:

1. reconcile the physical inventory with the coverage ledger;
2. cite authoritative component and board limits;
3. add endpoint/component contracts before project behavior;
4. define trace and snapshot schemas before hardware code;
5. reserve pins, timers, buses, current, RAM, and flash in the Mega profile;
6. land commits in endpoint, component, behavior, tests, example, lesson,
   project, and publication order;
7. leave bench-only claims explicitly open until measured; and
8. update the ledger when a retail module is substituted or omitted.

The roadmap is successful when every supported module teaches a distinct
engineering idea, every project is useful without contrived parts, and a
complete run can be explained from physical evidence plus a deterministic
trace.
