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

Future circuit-native observation:

- joystick axes have named analog test points;
- encoder phases have named logic test points;
- RGB distinguishes selection, editing, commit, cancel, and fault;
- binary LEDs retain a preview if LCD presentation fails; and
- shutdown evidence is separate from input-interpretation evidence.

Planned specimen coverage: joystick, rotary encoder, buttons, LCD, RGB LED, binary
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

A tabletop gate counts carriers moving through a channel. Hall and reed
observations indicate entry; the encoder represents conveyor position; RTC and
SD produce stable records. Local buttons may assign one of a few fixed
learner-defined labels without introducing an identity-device driver.

```text
Hall/reed edges --> passage model --> count --> seven-segment display
encoder edges ----^        |                    status LED
button label --------------+------------------> RTC-stamped SD record
```

Deterministic evidence:

- timestamped approach/departure traces for both magnet polarities;
- bounce, chatter, missing encoder edge, reversal, and impossible transition;
- duplicate label events inside one passage window;
- RTC power-loss and SD full/partial-write faults; and
- restart from a committed fixed-size checkpoint.

Circuit-native observation:

- raw Hall and reed LEDs show the two sensing boundaries;
- the display shows the committed count;
- a latch pulse marks each accepted passage;
- the encoder A/B pins are named logic-analyzer points; and
- storage success/fault has a persistent LED pattern.

Planned family coverage: listed Linear Hall and Magnetic Spring families,
optional encoder, deterministic RTC/ledger fixtures, display, and LEDs. No
separate digital Hall, kit magnet, or removable-storage specimen is claimed.
Physical RTC and removable-media adapters remain deferred.

## Lessons 037--039: vibration and sound laboratory

The complete interface, fixture, experiment, resource, specimen, publication,
and acceptance contract is in the
[percussion laboratory plan](../design/LESSONS_037_039_PERCUSSION_PLAN.md).
Canonical teaching uses external C&K/Littelfuse and SparkFun reference
fixtures. Authorized Elegoo Tilt/Tap/Shock/Big/Small Sound families remain
planned substitution coverage and require their own conformance evidence.

### 037 — Contact dynamics

Give tilt, vibration switch, knock, tap, and shock modules one common
timestamped transition boundary. Preserve pulse width and refractory policy;
do not reduce every device to an unqualified Boolean.

### 038 — Acoustic envelope and event classification

Model microphone analog envelope, threshold output, baseline calibration,
clipping, quiet/noisy confidence, and a bounded event window. This is an
amplitude experiment, not speech recognition or sound-level metrology.

### 039 — Project: percussion sequencer

Four injected logical surface lanes drive a deterministic step sequencer.
Each eventual physical contact specimen remains separately gated. Qualified
contacts provide attack timing, the microphone supplies relative envelope,
a potentiometer sets tempo, and passive buzzer plus LEDs replay the recorded
pattern.

```text
contact edges -----> event window -----> fixed-capacity pattern
microphone samples -> intensity --------^          |
manual clock + tempo ------------------------------+--> light/tone frames
```

Deterministic evidence:

- recorded single tap, double tap, vibration, shock, and ambient-noise traces;
- exact refractory, quantization, simultaneous-hit, and buffer-full boundaries;
- injected source-unavailable, rail/stuck, and threshold-disagreement faults;
- deterministic skip-to-current playback under uneven update intervals; and
- a seed-free replay whose output frames match byte for byte.

Circuit-native observation:

- one LED per surface reports accepted rather than merely raw hits;
- the existing one-digit display shows the active hexadecimal step;
- a dedicated capture LED shows enabled envelope sampling;
- D52 reports ready/healthy execution and the bounded heartbeat cadence, not
  an unqualified runtime-fault code;
- named contact, envelope, and threshold test points preserve raw electrical
  evidence;
- host traces prove runtime fault attribution and cleared cue intent; and
- shutdown leaves the LEDs and display dark and the buzzer silent.

The canonical adapter owns four contact inputs, envelope and threshold inputs,
tempo input, play/clear buttons, four surface LEDs, D52 status, D53 capture,
the 74HC595-backed one-digit display, and the passive sounder. It acquires
owners in documented dependency order and releases them in reverse. A runtime
fault is not encoded on D52 unless a future bounded diagnostic state is
specified and tested; the current acceptance path uses retained host
provenance plus separately observed all-dark/silent shutdown.

Planned family coverage: separately qualified knock/vibration/shock/tap/tilt
contacts, sound/microphone envelope, passive buzzer, potentiometer, LEDs, and
one-digit display. Active buzzer is excluded because it cannot provide the
project's pitched replay.

## Lessons 040--042: optical course marshal

Provisional implementation-depth planning is recorded in the
[optical course marshal plan](../design/LESSONS_040_042_OPTICAL_COURSE_MARSHAL_PLAN.md).
Implementation and canonical publication remain blocked until Lessons 037--039
complete with status `done`, final reconciliation is recorded, the Lesson 042
arming decision is resolved, and exact specimens are qualified.

### 040 — Reflective and interrupted light

Treat listing-authorized line-tracking, obstacle-avoidance, photo-interrupter,
and photoresistor evidence as explicit optical observations without asserting
one universal hardware sensor. Teach ambient rejection, threshold calibration,
hysteresis, crosstalk limits, and surface dependence.

### 041 — Presence and passage

Compose PIR warm-up/motion state, optical beam edges, and ultrasonic range into
a presence model. Each source retains validity and age; disagreement is an
observable result, not silently voted away.

### 042 — Project: tabletop course marshal

A hand-moved card or unpowered model vehicle passes checkpoints. Reflective
markers identify lanes, an obstacle sensor guards the finish, PIR arms the
course only under the final reviewed arming policy, and ultrasonic range
confirms the finish approach. The provisional plan recommends an existing
button start with PIR as eligibility evidence; PIR-only start authority remains
an explicit user-decision gate. The project times runs and rejects impossible
checkpoint order.

```text
PIR --------> eligibility
reviewed start policy -> armed
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

Planned family coverage: listing-authorized Tracking, Avoidance,
Photo-Interrupter, Photo-Resistor, HC-SR501, HC-SR04, and existing displays.
The light-cup alias is not in the authorized Elegoo union and is excluded
unless a later source decision admits it.

## Lessons 043--045: balance-table instrument

This host-verified engagement-first block brings visible, responsive motion
sensing forward without changing its specimen gate or revision-neutral
design. Its
[implementation-depth plan](../design/LESSONS_043_045_BALANCE_TABLE_PLAN.md)
authorizes E0 replay only: copied synthetic inertial values, pure policies,
deterministic composition, and presentation intent. It does not authorize a
powered adapter, I2C transaction, wiring table, formal schematic, or E1
acceptance claim. Exact MPU6050 and QMI8658 variants remain independently
gated.

The published API is `InertialObservationPolicy` followed by
`OrientationPolicy` and `BalancePresentationPolicy`, then
`BalanceInstrument`. The canonical Mega 2560 compile-only replays measure
6,682/949, 13,740/748, and 26,398/1,898 bytes of flash/static SRAM for Lessons
043, 044, and 045 respectively. The first two stay below 16,384 bytes of flash
and 1,024 bytes of SRAM; the project stays below 28,672 bytes of flash and
2,048 bytes of SRAM.

### 043 — Revision-neutral inertial samples

Validate one copied, explicitly identified six-axis sample. Source and sample
identity, configured range, conversion and calibration revisions, data-ready
state, producer failure, age, and saturation remain distinct. The E0
implementation accepts only synthetic-fixture provenance. Separate future
MPU6050 and QMI8658 adapters require their own exact-specimen, primary-source,
electrical, and register-contract gates.

### 044 — Orientation presentation

Map supplied inertial samples into bounded pitch/roll and light/tone
presentation intent. Board orientation is explicit configuration. Presentation
depends only on copied samples, project configuration, and supplied time; it
owns no endpoint, bus, or clock and makes no heading, position, stabilization,
or navigation claim.

### 045 — Project: balance-table instrument

A stationary, hand-operated tabletop instrument maps copied tilt evidence to
light and tone intent, uses copied joystick events to adjust sensitivity, and
uses an explicit copied button event to freeze one measurement for comparison.
It is not handheld, moving, actuated, or safety relevant.

```text
copied joystick events ----+
                            +--> sensitivity --> orientation --> light intent
copied inertial samples ----+                         |       --> tone intent
copied button events -------+-------------------------+       --> freeze state
explicit time ----------------------------------------+
```

E0 evidence includes level, tilt, unsteady, saturation, stale, producer-fault,
axis-permutation, sensitivity, freeze, timestamp-wrap, simultaneous-event, and
byte-identical replay fixtures. The host harness records distinct
acquisition/health, orientation, freeze, light-intent, and tone-intent result
cells as the Serial-independent evidence surface. Compile-only Mega
examples add no physical observation claim.

AVR GCC 7.3 measures `InertialObservationPolicy` at 80 bytes,
`OrientationPolicy` at 38 bytes, `BalancePresentationPolicy` at 95 bytes,
`BalanceInstrument` at 339 bytes, caller-owned `BalanceFrameStorage` at 102
bytes, and the worst live composition at 741 bytes. The compiler-derived
`BalanceInstrument::update()` frame is 399 bytes; these static/compiler
figures are capacity evidence, not a runtime stack measurement.

Future E1 specimen coverage may include one exactly qualified MPU6050 or
QMI8658 variant plus the joystick, RGB LED, passive buzzer, button, and
shift-register LEDs. Until those independent gates close, the block contains
no powered sensor, I2C transaction, pin assignment, wiring table, schematic,
or physical acceptance.

## Lessons 046--048: tactile kinetic sculpture

This host-verified block publishes E0 copied-evidence and logical-intent
policy under the
[implementation-depth plan](../design/LESSONS_046_048_KINETIC_SCULPTURE_PLAN.md).
The public sequence is `InteractionIntentPolicy`,
`BoundedStepperSequence`, then `KineticLightSculpture`. Canonical Mega
compile-only replays measure 6,956/733, 8,068/1,053, and 22,216/1,470 bytes
of flash/static SRAM for Lessons 046, 047, and 048. Static stack proofs are
263, 212, and 687 bytes respectively. These are E0 capacity results, not
powered or physical verification.

### 046 — Touch and gesture-like switches

`InteractionIntentPolicy` validates copied contact and joystick evidence,
preserving source identity, polarity-derived contact state, chatter, hold
duration, direction, saturation, age, and explicit validity. Powered exact
tactile/directional adapters and their independent indicators remain E1-open;
no capacitive, gesture, proximity, or physiological interface is claimed.

### 047 — Bounded stepper motion

`BoundedStepperSequence` is a pure logical-intent policy with explicit
direction, phase, rate, travel budget, cancellation, stop, and de-energized
shutdown intent. It owns no output, timer, or clock. Powered indicators remain
E1-open; the exact 28BYJ-48/ULN2003 electrical system and energized motion
remain E2-open.

### 048 — Project: kinetic light sculpture

`KineticLightSculpture` transactionally composes one copied contact action,
copied directional evidence, logical motion, and light intent. E0 replay
proves authorization binding, cancellation, stop dominance, fault handling,
and byte-identical outcomes without powered endpoints.

```text
touch/tilt -----> gesture model --> bounded motion intent --> stepper
joystick -------^                     |                    --> RGB/LEDs
explicit time ------------------------^
stop button --------------------------+
```

Deterministic evidence:

- touch chatter, long hold, invalid source, and tilt transitions;
- exact coil vectors, direction reversal, cancellation, and wrap boundaries;
- stop dominance at every state and no queued motion after shutdown;
- frame-for-frame light and motion-intent replay.

Future E1/E2 acceptance:

- E1 must show shift-register LEDs mirroring logical coil intent and an
  independent stop LED whenever motion is inhibited;
- E2 must show each ULN2003 channel indicator exposing applied coil state;
- E2 must inject driver fault and external-power loss with the exact powered
  system;
- E2 must prove external motor power is physically separable from logic
  power; and
- E2 must prove the motor is de-energized on stop, fault, and shutdown.

Future E1 coverage may qualify listed tactile/contact and joystick inputs,
stop input, and independent RGB/shift-register indicators. Future E2 coverage
may qualify the exact 28BYJ-48/ULN2003, separable motor power, and lightweight
sculpture. Until those gates close, no powered endpoint, wiring table,
authoritative schematic, energized motion, or physical acceptance is claimed.
Capacitive-touch and heartbeat modules remain excluded.

## Lessons 049--051: identity-controlled parts carousel

### 049 — Identity records and bounded enrollment

Turn copied synthetic identifier and key evidence into fixed-size local
identity records. Enrollment, duplicate identity, unknown identity, lockout,
and malformed supplied record images are explicit outcomes. An identifier is
a local lesson token, not a credential, authentication result, or proof of a
person. E0 records are caller-owned and volatile; an explicitly supplied
two-slot fixed image models deterministic commit markers, generations,
checksums, interrupted-write cuts, and reconstruction. This is simulated
durability semantics, not an EEPROM/SD transaction or physical persistence
claim.

### 050 — Positioning and homing

Compose copied synthetic home and stop evidence, logical travel limits, and
exact step-intent vectors in a pure bounded homing policy. A successful E0
fixture establishes logical zero only for that replay session; issued steps
are intent, not evidence of physical movement or position. Physical position
remains unknown until the exact E2 mechanism completes a recorded bounded
homing acceptance.

### 051 — Project: tabletop parts carousel

Copied local-identifier evidence requests one of several named paper-bin
records, copied key evidence confirms the selection, and the coordinator
publishes bounded logical home, travel, gate, presentation, and audit intents.
The E0 carousel is an inert deterministic fixture: it owns no endpoint, moves
nothing, opens no gate, writes no persistent medium, and makes no powered
display or sensor claim.

Deterministic evidence:

- known, unknown, duplicate, and rapidly repeated identities;
- synthetic logical-home success, missing/stuck home evidence, interrupted
  logical travel, and restart with position unknown;
- authorization expiry and keypad conflict boundaries;
- exact logical step and bounded gate-intent vectors with no powered endpoint;
- deterministic two-slot reconstruction from supplied valid, torn, corrupt,
  duplicate-generation, ambiguous, erased, and full fixed record images;
- durable-model authorization-start admission before any logical motion is
  eligible, followed by terminal reconciliation under the same operation
  identity; and
- invariant: no gate-open intent before identity, confirmation, and known
  logical target position in the same valid operation epoch.

E0 retained evidence:

- caller-owned result cells retain identity disposition, logical phase,
  authorization epoch, home evidence, logical position, and fault attribution;
- fixture-owned mirrors retain exact coil, bounded gate, presentation, stop,
  and all-off intents;
- the project retains the Lesson 050 preview and applies each Lesson 047
  command as one atomic step; with project-fixed `holdAtRest = false`, the
  published coil mirror is intentionally zero, while Lesson 047 retains the
  staged nonzero-coil teaching surface;
- fixed-capacity audit records and explicitly supplied two-slot restart images
  make authorization-start admission, terminal reconciliation, every
  interrupted-write cut, and replay byte-checkable;
- once a start record is exposed, stop requires an attributable `Stopped`
  terminal for the same operation rather than abandoning reconciliation;
- an indeterminate simulated commit is reconstructed before retry so one
  logical operation cannot be duplicated under a new identity; and
- stop clears logical motion and gate intent independently of presentation or
  audit diagnostics.

E0's durable image is a pure fixed-memory model. It establishes ordering and
recovery semantics only: authorization-start must be resolved in that model
before logical motion eligibility, and a terminal record with the same
operation identity must remain attributable until resolved or reconstructed.
It does not establish that bytes reached, survived on, or were recovered from
any physical medium.

Future E1 may separately qualify exact RFID/keypad/home inputs, an existing
display and LEDs, and a named nonvolatile adapter while motor and servo power
remain absent. Future E2 may separately qualify the exact stepper/driver and
servo with switchable current-limited actuator power, a restrained lightweight
mechanism, independent stop and power removal, and measured homing, travel,
gate, fault, and shutdown acceptance. LEDs and an LCD may present intent after
E1 qualification, but cannot prove physical home, movement, gate position, or
de-energization.

Planned specimen coverage after those independent gates: RFID, keypad,
Hall/reed home input, display, LEDs, and nonvolatile medium at E1; exact
stepper/driver, servo, and restrained paper-bin mechanism at E2.

## Lessons 052--054: infrared protocol workbench

### 052 — Copied IR capture evidence

Extend receive-only IR work into bounded carrier-demodulated pulse records,
categorical evidence strength, repeat frames, unknown frames, and fixture
export. This reuses the Lesson 025 capture and decoder stack rather than
creating a second receiver path.

### 053 — Closed symbolic IR transmission intent

Add a pure logical emission policy for documented, learner-created codes only.
A future exactly qualified IR LED endpoint makes carrier-timer ownership, duty,
burst duration, cancellation, and current-limited, exposure-bounded operation
explicit; no eye-safety claim is made. This does not authorize replay of
captures, access controls, appliances, or unknown devices.

### 054 — Project: IR command translator

At E0, the station translates one allowlisted, known-valid synthetic receive
symbol into a different locally defined logical response and presentation
intent. A future E1 station sends those known harmless commands between two
adjacent owned fixtures only after exact transmitter and receiver
qualification. It displays source, command, categorical evidence, and
round-trip timing. Repeat, unknown, malformed, self-echo, and unlisted captures
remain observable but cannot enter the transmit catalog.

Deterministic evidence:

- golden encode/decode vectors and malformed/truncated/noisy pulse trains;
- repeat handling, timeout, cancellation, and one-operation busy capacity;
- source policy proving unknown captures are never transmitted; and
- E0 known-code fixture traces at exact timestamp boundaries.

Future E1 evidence adds timer conflicts, receiver saturation, missing carrier,
and measured adjacent-fixture round trips.

Circuit-native observation:

- logical transmit intent and receive activity use separate LEDs;
- an exactly qualified adjacent receiver is the optical activity evidence; a
  phone camera is only an optional alignment hint;
- LCD displays decoded command and categorical evidence;
- a future E1 timer conflict produces a distinct fault pattern; and
- E0 emission intent is inactive after cancellation and shutdown; future E1
  evidence must prove measured carrier and optical output are inactive.

Planned specimen coverage: exact IR receiver; externally sourced exact IR
emitter whose future E1 selection remains open; owned harmless remote, keypad,
LCD, timers, buttons, and LEDs.

## Lessons 055--057: cooperative escape-room console

### 055 — Constraint and clue model

Add a fixed-capacity rule graph whose predicates consume existing semantic
observations. Rules are data, cycles are rejected, and hidden time is
forbidden.

### 056 — Fault-tolerant operator panel

Compose keypad, RFID, joystick, encoder, buttons, display, storage, and
redundant stop/acknowledgement behavior into one deterministic panel model.

### 057 — Project: tabletop escape-room console

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

Planned composition coverage: deliberate reuse of all earlier endpoint families and a
meaningful integration test for the common kit.

## Lessons 058--060: display transport laboratory

### 058 — Nonblocking multiplexed digits

Add a fixed E0 multiplex policy whose supplied-time refresh emits bounded,
ordered blank/segment/digit intent. Common-anode/cathode metadata, independent
digit-select polarity, leading zeros, decimal points, overflow, refresh loss,
and shutdown blank intent are explicit. A future E1 endpoint owns every
digit-select and segment resource it drives and must qualify the exact driver
topology before power.

### 059 — Register-driven display presentation

Add a MAX7219 presentation policy over a recording transport seam.
Configuration, intensity, scan limit, decode mode, row writes, chip-select
intent, transport faults, partial-prefix attribution, and blank-on-shutdown
request remain explicit. Frame generation stays independent. A future E1
adapter reuses the existing owned `SpiDevice` boundary after terminal bus-fault
recovery is resolved.

### 060 — Project: dual-display timing desk

One deterministic stopwatch model drives a multiplexed four-digit display and
a MAX7219 matrix progress dial. Buttons control start, lap, and reset; the
project reports presentation disagreement rather than silently trusting one
display.

Deterministic evidence covers every glyph and digit phase, refresh jitter,
timestamp wrap, SPI failure at each register, partial frame attribution,
presentation-intent disagreement, and shutdown. E0 uses named result cells;
future E1 digit-select, segment, clock, data, and chip-select test points
expose transport. Both displays run a distinct safe self-test before either
presents time.

The specimen gate records display polarity and resistor network, MAX7219
marking/module schematic, supply and logic levels, matrix orientation, maximum
segment current, and measured current budget. Unknown modules remain blank and
unpowered.

Planned specimen coverage: four-digit seven-segment display, MAX7219 matrix,
buttons, SPI, and status LEDs. PCF8574 LCD backpacks remain inventory-gated
planning work and are not claimed by this arc.

## Lessons 061--063: museum-case monitor

### 061 — Water-level observations

Qualify copied Water Level acquisition evidence without owning an ADC channel
or excitation endpoint. Teach corrosion-aware per-cycle duty limits, dry/wet
calibration, contamination, discharged/off evidence, disconnected predicates,
and the limits of absolute readings. An exact switched-power `AnalogInput`
adapter, electrical identity, current, corrosion, spills, and drying remain
E1a work.

### 062 — Thermal and radiant observations

Compare copied already-converted thermistor evidence, copied categorical
evidence from the distinct authorized Digital Temperature family, and copied
raw radiant-threshold evidence. Preserve each role's identity, calibration or
configuration revision, units where justified, uncertainty, threshold state,
validity, age, disagreement, and saturation. Exact powered specimens and
adapters remain E1b; any radiant stimulus is a controlled harmless low-energy
IR source, never a flame, heater, laser, or unknown-protocol replay.

### 063 — Project: museum-case monitor

A model display case watches for liquid, excessive temperature, abrupt radiant
IR, and a reed-open condition. At E0 it consumes copied qualified observations
and emits only health, presentation, alarm, bounded audit-record, and
relay-lamp intent. It owns no clock, persistent medium, display, sounder, or
relay.

```text
water level ------+
temperature ------+--> validated hazard model --> copied presentation intent
radiant IR -------+             |                --> inert relay-lamp intent
reed contact -----+             +---------------> copied audit intent/receipt
```

Deterministic evidence:

- dry-to-wet ramps, sensor open/short, contamination drift, and stale samples;
- temperature boundary, disagreement, and sensor transport failures;
- radiant pulse traces and ambient saturation;
- alarm latch, acknowledgement, cooldown, restart, and log-write interruption;
- invariant: invalid sensing can never request the “healthy” indication.

Planned physical observation after independent specimen qualification:

- RGB state is healthy/warning/alarm/fault with a grayscale-safe blink code;
- LCD always includes sample age or fault;
- water-sensor supply has a measurable duty-cycle test point;
- an exact extra-low-voltage relay may drive a current-limited inert lamp only;
  and
- the alarm output is inactive after shutdown.

E1a independently qualifies the Water Level fixture; E1b independently
qualifies the thermistor, distinct Digital Temperature, and radiant fixtures;
E1c qualifies reed and local presentation; and E1d qualifies their passive
combination. Durable RTC/media hardware remains deferred under Lessons 022 and
024. E2 separately qualifies an exact extra-low-voltage inert relay/lamp
fixture with independent removal; no mains, security, access-control,
preservation, or life-safety function is claimed. Rain and soil-moisture
probes are not claimed.

## Lessons 064--066: multi-probe thermal mapper

The controlling
[implementation-depth plan](../design/LESSONS_064_066_THERMAL_MAPPER_PLAN.md)
defines the E0 contracts, capacities, deterministic fixtures, and resource
gates. E0 is copied-policy work only: it owns no pin, bus, endpoint, supply,
display, clock, or storage medium.

### 064 — Owned single-wire transactions

Add a bounded transaction policy over copied line receipts with explicit reset,
presence, bit-slot, external-power policy, timeout, and rollback. A future E1
adapter may own an exact single-wire endpoint; E0 emits inert transaction intent
and cannot drive a pin or strong pull-up. DS18B20 identity, scratchpad, CRC,
resolution, and conversion-state handling remain above transport in Lesson 065.

### 065 — Qualified thermal probe sets

Compose exactly four configured ROM identities and copied transaction results
into a qualified thermal snapshot. This exact-four source contract is distinct
from Lesson 066's configured two-to-four-probe spatial subset. Duplicate
identities, disappearance, conversion-in-progress, CRC failure, stale values,
implausible steps, and mixed resolutions remain visible.

### 066 — Project: thermal gradient mapper

At E0, two to four identities selected from Lesson 065's exact four-probe
source are arranged in configured spatial order. The mapper consumes copied
observations and controls, then emits inert presentation intent and a
fixed-field record image inside the caller-owned result. It does not write RTC,
SD, EEPROM, or any other medium. A future E1 fixture may page an LCD and LEDs
through probe identity, temperature, age, and validity, and may separately
qualify a durable adapter that keys records by stable identity rather than
discovery order.

Deterministic evidence includes reset/presence slots, ROM search fixtures,
CRC vectors, resolution-dependent deadlines, duplicate and disappearing
probes, timestamp wrap, and exact caller-owned record images. E0 result cells
and byte-stable replay are the non-Serial observation path. Future E1 work may
qualify the data line, a direct probe-VDD test point, and a
conversion-activity LED; no switched probe rail is promised. A fault pattern
cannot be mistaken for a cold reading.

All powered work is E1-only. Its specimen gate records the exact DS18B20
marking, package, pinout, pull-up, waterproof-probe construction if present,
supply mode, direct VDD test point, and datasheet. No immersion, hot surface,
parasite-power, or unknown three-pin module is used until its electrical and
material construction is established.

Planned E1 specimen coverage: DS18B20 variants, LCD, RTC, SD, buttons, and
LEDs.

## Lessons 067--069: interchangeable motion recorder

### 067 — Inertial record normalization

Normalize copied, revision-specific inertial record fixtures into one
source-frame contract while retaining source identity, configured range,
data-ready state, saturation, calibration and conversion revisions, sample
time, sequence, and producer status. Lesson 043 supplies a revision-neutral
copied-sample contract, not a powered device adapter. E0 owns no bus or
endpoint and performs no runtime probe or configuration write.

### 068 — Inertial source qualification

Qualify one explicitly configured copied-record source using stationary bias,
an explicit right-handed source-to-qualification-frame mapping, sample age,
sequence, and range checks. Source selection is configuration, not voting,
hot switching, or automatic failover. Recorded fixtures can drive the
qualifier without a device driver.

### 069 — Project: interchangeable motion recorder

At E0 the learner replays the same hand-motion script from independently
identified copied-record fixtures, compares normalized traces on the host,
and produces inert presentation and record intents. One configured source is
used for each recorder session. No I2C endpoint, RTC, SD medium, character
display, RGB LED, or button is owned or physically verified.

Deterministic E0 evidence includes golden copied traces for each represented
revision, identity and contract-revision mismatch, copied producer faults,
stale/data-ready disagreement, range saturation, axis permutations, session
capacity and lifecycle boundaries, and byte-identical normalized records.
Caller-owned result cells expose qualification, recording, and presentation
intent without Serial output or a physical observation claim.

Future E1 work may independently qualify an exact MPU6050 or QMI8658 adapter,
then separately qualify its display, RGB/button, clock, and durable-media
endpoints. Before power is applied, the inventory must name the sensor, PCB
markings, address strap, regulator and level-shifter population, logic voltage,
and primary register-map revision. E1 acceptance must expose SDA, SCL,
interrupt, and sensor-rail acquisition evidence; keep display self-test
separate from changing orientation; and make RGB fault dominate valid
orientation. An unidentified revision remains unpowered.

Planned E1 specimen coverage: one exactly identified revision-dependent
MPU6050 or QMI8658 fixture, I2C, character display, RGB LED, button, RTC, and
SD. Qualifying one motion family does not qualify the other.

## Lessons 070--072: modular sensor test bench

The
[implementation-depth plan](../design/LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md)
authorizes E0 stateless descriptor and frame validation, copied attributed
points, streaming bounded legs, immutable envelope consumption, inert
presentation and script intent, and one caller-owned volatile image. It does
not authorize module discovery, a powered specimen, ADC or digital
acquisition, switched power, a display endpoint, or persistence.

### 070 — Descriptor-driven threshold modules

`ModuleThresholdDescriptor` declares one low-voltage analog/comparator fixture
without inventing a class per PCB, and `ModuleThresholdFrame` retains
attributable copied channels. Stateless validation covers schema, declared
specimen reference and revisions, topology, supply and signal ranges, raw
domain, comparator stage and polarity, pull requirement and rail, threshold
control, warm-up, and settling. A declaration, pin label, or observed value
does not identify a physical module or authorize power.

### 071 — Characterization runs

`ModuleCharacterizationPolicy` streams explicitly correlated
`ModuleCharacterizationPoint` values through ascending, descending, and then
verification legs without retaining a point array. The first two legs freeze
transition brackets and conservative guaranteed-active, guaranteed-inactive,
and ambiguity intervals; the third checks copied points only against those
frozen intervals. The policy retains witnesses, identity, run and leg
correlation, time, sequence, readiness, and producer status. Raw endpoint
codes are rail evidence, not `OpenLike` or `ShortLike` diagnoses, and the
policy publishes no exact threshold or negative hysteresis scalar.

### 072 — Project: module characterization bench

`InertModuleCharacterizationBench` atomically consumes one immutable
`ModuleCharacterizationEnvelope` containing matching descriptor and terminal
three-leg evidence with domain-separated digests. It owns no Lesson 071 child
and retains no frames. One configured descriptor is used per session while
the bench emits inert review-script and fault-dominant presentation intent.
Only explicit final preparation writes one canonical, exactly 192-byte
`ModuleCharacterizationRecordImage` atomically into caller memory.
`recordPrepared` proves neither persistence nor physical acceptance.

Deterministic E0 evidence includes:

- ascending and descending ramps for active-high and active-low descriptors;
- a verification leg with consistent, ambiguous, and disagreeing points;
- chatter, rail, stale, direction, time, and sequence outcomes;
- descriptor validation and unknown-module rejection;
- exact bracket and interval boundaries and terminal immutability;
- envelope correlation, digest mismatch, and atomic rejection;
- exact 192-byte framing, reserved bytes, semantic values, and CRC; and
- replay from recorded raw samples without the physical module.

Future E1a may separately qualify one exact low-voltage analog/comparator
specimen, its family-specific harmless stimulus, and its acquisition fixture.
Acceptance then requires:

- exact markings, photographs, and primary device documentation, plus an
  authoritative module schematic or, when none exists, a reviewed trace of
  the populated module circuit;
- pinout, voltage, current, topology, pull, threshold control, ADC reference,
  and impedance evidence;
- named analog, digital, power, and ground test points;
- measured supply, signal, current, warm-up, and settling limits; and
- no module is connected before its voltage and pinout are identified.

Switched power is optional and, when used, requires an exactly rated high-side
switch; a GPIO never powers a module. Otherwise use a qualified board supply
and physical power removal. Future E1b display, RGB, and button endpoints
require their own ownership, current, self-test, fault-dominance, and rollback
qualification. They present raw value and comparator state but do not close
acquisition or characterization accuracy. Durable storage and persistence are
outside this arc.

Candidate specimen coverage is limited to exact, independently qualified
low-voltage analog/comparator revisions from authorized light, sound, Hall,
thermistor, passive radiant, Metal Touch, vibration, and obstacle families.
This is not family-wide qualification. Register devices, generic
analog-temperature and capacitive-touch boards, PCF8574 backpacks, gas
exposure, flames, heaters, lasers, unknown emitters, and physiological claims
are not admitted by this boundary.

## Lessons 073--075: RTC integrity

The
[RTC integrity re-scope decision](../design/LESSONS_073_075_RTC_INTEGRITY_RESCOPE.md)
selects this arc for implementation-depth planning:

### 073 — Copied RTC Transaction Evidence

Model copied, attributed RTC register-transaction evidence without owning an
I2C endpoint or claiming that a physical clock produced it. Preserve register
bytes, transaction status, source identity, observation time, and sequence so
malformed calendar encoding and producer failures remain distinguishable.

### 074 — Qualified Clock Observation

Qualify copied clock evidence for calendar encoding, oscillator and power-loss
state, freshness, sequence, bounded jumps, rollover, and monotonic use. A valid
civil-time value is not by itself proof of clock accuracy, continuity, or
physical acceptance.

### 075 — Project: Inert Time-Warp Detective Desk

Compose copied transaction evidence and qualified clock observations into a
replayable fault-investigation desk. The learner classifies stopped,
power-loss, stale, malformed, jumping, and rollover cases and produces inert
display, indicator, and record intent without owning a clock, display, or
storage endpoint.

This is a planning-selected curriculum direction, not an implementation-ready
contract. Implementation remains blocked on an implementation-depth plan,
per-component architecture stress passes, resource budgets, deterministic
acceptance matrices, and publication design. Exact-specimen E1 work also
remains open: it must identify the precise DS1307 or documented DS3231-family
module, register-map revision, board topology, pull-up rails, backup supply and
cell chemistry, oscillator behavior, wiring, primary sources, and measured
bench acceptance. In particular, no backup cell may be installed until any
charging path is identified and shown compatible with that cell.

The former assignments of DS1302, BMP180, PCF8591, and a color sensor remain
historical exclusions. The official Elegoo Mega Most Complete plus Upgraded
37-in-1 union does not authorize those families, and this selection does not
restore them.

## Lessons 076--078: tabletop sonar desk

The replacement selection is canonical at planning depth. Its provenance,
rejected alternatives, scope boundaries, and activation gates are recorded in
the [tabletop sonar re-scope decision](../design/LESSONS_076_078_TABLETOP_SONAR_RESCOPE.md).
The arc deliberately deepens the authorized HC-SR04 and servo families instead
of inventing an unused sensor family or restoring the excluded color sensor.

### 076 — Copied Sweep-Range Frames

Pair attributable, copied HC-SR04-shaped range observations with copied,
bounded-servo angle intent in one deterministic sweep frame. Preserve source,
sequence, freshness, range validity, angle validity, and the distinction
between requested angle and observed range. This component acquires no echo
and commands no motion.

### 077 — Bounded Polar Occupancy Map

Reduce qualified copied sweep frames into a fixed-capacity polar-bin map.
Mapping policy freezes bin boundaries, update order, replacement, confidence,
age, disagreement, saturation, and reset behavior. It preserves enough
provenance to explain each occupied, clear, unknown, or rejected bin rather
than presenting a synthetic picture as a physical scan.

### 078 — Inert Tabletop Sonar Desk

Compose copied sweep frames and the bounded polar map into explainable
presentation intent and a volatile record intent. Learners replay a tabletop
scene, inspect why bins changed, and distinguish missing, stale, invalid, and
contradictory evidence without owning a ranger, servo, display, or persistent
store.

```text
copied range observation --+
copied bounded angle intent +--> sweep frame --> polar map --> display intent
explicit time/provenance ---+          |               +--> volatile record
```

Deterministic evidence covers:

- exact angle/range boundary pairs, bin edges, scan reversal, and sequence
  rollover;
- no-echo, out-of-range, stale, duplicated, missing, and contradictory copied
  observations;
- fixed-capacity exhaustion, replacement ties, confidence and age transitions,
  and reset;
- permutation tests proving arrival order or pin identity does not redefine
  spatial meaning; and
- byte-identical replay of map, explanation, presentation, and volatile-record
  intent.

The dependency boundary is strict. Lesson 076 reuses Lesson 017 bounded-servo
semantics and Lesson 019 ultrasonic semantics, but does not repeat their
endpoint acquisition. Lesson 078 is an inert evidence desk, not the autonomous
rover behavior of Lesson 021 or the presence and course policy of Lessons
040--042.

At E0, all frames are copied or synthetic and no physical scan, distance,
motion, display, or persistence claim is allowed. A future E1 ranger adapter
requires its own exact-specimen electrical and bench gate. Any future E2 servo
composition separately requires external-power, common-ground, restraint,
travel-limit, current, safe-state, and motion acceptance. Circuit-native
observation and named trigger, echo, servo-intent, and rail test points belong
to those later physical gates, not to this E0 selection.

Implementation remains blocked on one implementation-depth plan, individual
architecture stress passes for Lessons 076, 077, and 078, and frozen
provenance, mapping, resource, fixture, failure, publication, and promotion
semantics. This selection creates no code, lesson package, powered specimen,
or support claim.

## Lessons 079--081: low-energy component qualification bench

### 079 — Bounded low-side load driver

Add a low-side-driver intent and endpoint for an identified PN2222 or S8050
specimen. Base current, load-current ceiling, active polarity, flyback policy,
resource ownership, and all-off rollback are configuration. Tests use a fake
endpoint; first hardware uses only a current-limited LED or identified small
inductive fixture under the E2 gate.

### 080 — Small indicator-module semantics

Compose existing digital and light endpoints into explicit active-buzzer,
traffic-light, dual-color, auto-flash, and voltage-indicator observations.
Autonomous waveforms remain observations rather than scheduler claims.
Descriptors record polarity, resistor/driver population, warm-up, and safe
state; there is no universal “three-pin module” driver.

### 081 — Project: component qualification bench

The bench guides one identified low-voltage specimen through pinout review,
inactive-state measurement, bounded stimulus, and a stable acceptance record.
It compares direct LED loads, transistor-switched inert loads, and small
indicator modules without mains, high energy, unattended operation, or an
unknown emitter.

Deterministic evidence covers descriptor rejection, current-budget arithmetic,
active-high/low behavior, open/stuck endpoint faults, cancellation, flyback
requirement, autonomous waveform observations, partial records, and restart.
Separate power, raw input, accepted state, driver intent, and persistent fault
indicators expose every stage; named base/gate, collector/load, rail, and
flyback test points support measured acceptance.

Every specimen requires both-face photographs, active-device marking, traced
pin order, supply/signal limits, resistor and driver population, load identity,
energy class, and primary source. Missing identity assigns PX and prohibits
power. Relay contacts, mains, lasers, gas exposure, physiological claims, and
motor/fan blades are outside this bench.

Planned specimen coverage: PN2222 and S8050 variants, 1N4007, active buzzer,
traffic-light and dual-color LED modules, auto-flash LED, voltage detector,
breadboard supply evidence, resistors, capacitors, buttons, and status display.

## Coverage ledger

This planning ledger prevents quiet omissions. A listed lesson is the intended
first project dependency, not a support or physical-verification claim.
“Variant” means the characterization bench may compare electrically similar,
individually identified retail boards.

| Module family | Primary project | Later reuse |
|---|---:|---:|
| LEDs, RGB, buttons, buzzers | 003/006 | all diagnostic paths |
| Potentiometer, photoresistor | 009 | 039, 042, 045 |
| Shift register and one-digit 7-segment | 012 | 036, 039, 045, 048 |
| DHT | 015 | no required reuse |
| LCD | 015 | 045, 051, 054, 057, 063, 066; 069 and 072 future E1 only |
| Keypad, servo | 018 | 051, 054, 057 |
| Ultrasonic, PIR, DC motor/driver | 021 | 042 |
| RTC and deterministic durable records; physical RTC/media deferred | 024 | 036, 057, 063 |
| IR receiver and remote | 027 | 054 |
| Receive-capable RF | 027 passive only | no transmit project |
| Continuity and cue panel | 030 inert only | 057 fault model |
| Joystick, rotary encoder | 033 | 036, 045, 048, 057 |
| Copied inertial evidence; exact MPU6050 and QMI8658 adapters deferred | 045 E0 policy; E1 fixtures open | 069 E0 records; E1 adapters open |
| Hall variants, reed | 036 | 051, 057, 063; 072 copied descriptor at E0, exact comparator fixture E1-open |
| Tilt, knock, vibration, shock, sound | 039 | 048, 057; 072 copied descriptor at E0, exact comparator fixture E1-open |
| Tracking, Avoidance, Photo-Interrupter | 042 | 057; 072 copied descriptor at E0, exact comparator fixture E1-open |
| Water Level copied-observation policy; exact sensor deferred | 063 E0 policy; E1a fixture open | no 072 support claim without an exact comparator specimen |
| Thermistor and distinct Digital Temperature copied evidence; exact modules deferred | 063 E0 policy; E1b fixtures open | 072 copied thermistor/comparator descriptor only; exact fixture E1-open |
| Passive radiant copied evidence; exact detector deferred | 063 E0 policy; controlled low-energy IR only at E1b | 072 copied comparator descriptor only; exact passive fixture E1-open |
| Listed Metal Touch, contact/tilt switches, and joystick | 048 | 057; 072 copied comparator descriptor only, exact fixture E1-open |
| Stepper and ULN2003 | 048 | 051, 057 |
| RFID | 051 | 057 |
| IR emitter | 054, known local codes only | none required |
| Identified analog/comparator module descriptors; exact variants deferred | 072 E0 copied policy; E1 fixtures open | inventory acceptance only after exact qualification |
| DS18B20 and single-wire variants | 066 | thermal records |
| Four-digit display, MAX7219 matrix | 060 | timing presentation |
| DS1307 RTC transaction and clock-integrity policy | 073--075 planning selected; implementation not started | copied E0 evidence only; exact DS1307 and separately identified DS3231 specimens remain gated |
| HC-SR04/servo copied sweep and polar-map evidence | 076--078 planning selected; implementation not started | E0 replay only; ranger E1 and servo E2 gates remain separate |
| PN2222/S8050, diode, indicator variants | 081 inert loads only | inventory acceptance |
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
