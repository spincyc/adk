# Safety model

This file is the review authority for supported ADK hardware and projects.
`site/pages/safety.md` explains the rules to learners; lessons apply them.
When documents disagree, use the stricter boundary and stop the release until
the disagreement is fixed.

ADK is a low-voltage educational library. It is not a safety controller.
Software state, RAII cleanup, tests, and deterministic replay never replace
power isolation, rated protection, or qualified supervision.

## Non-negotiable boundaries

Supported work must not:

- connect learner-built circuits directly to mains, vehicle, building, medical,
  life-safety, or public infrastructure;
- power a motor, relay coil, solenoid, heater, servo, lamp, or radio from a
  Mega 2560 I/O pin;
- switch mains or an unknown load, even when a relay module claims isolation;
- defeat, emulate, or bypass a physical interlock or certified controller;
- transmit an unknown, captured, private, protected, or safety-relevant radio
  protocol;
- connect to a pyrotechnic launcher, initiator, ignition circuit, or live
  continuity circuit;
- use energetic, combustible, explosive, high-temperature, or sharp-moving
  loads as a lesson prop;
- describe a software command, destructor, reset, or high-impedance pin as an
  emergency stop.

An agent must reject code, diagrams, examples, links, or instructions that make
one of these paths easier. Reframing the work as testing, research, simulation,
or receive-only observation does not relax the boundary.

## Energy classes

Every hardware lesson declares one class. An undeclared lesson does not ship.

| Class | Allowed work | Required controls |
|---|---|---|
| E0 | Host simulation; unpowered inspection | No hardware side effect |
| E1 | Mega 2560, USB or documented low-voltage current-limited supply, LEDs, switches, passive sensors, piezo | Exact schematic, ratings, unpowered wiring check, stop conditions |
| E2 | Externally powered servo, motor, or rated low-voltage driver; inert relay load | E1 controls plus separate load supply, driver protection, physical power removal, restrained workspace, measured current limit |
| E3 | Mains, energetic devices, ignition, certified safety systems, unknown RF control | Outside ADK; no implementation or bench procedure |

E2 work remains draft until a named person records physical bench acceptance.
CI, compilation, photos, and an agent review cannot supply that evidence.

## Component gates

### Outputs and LEDs

- Construction is electrically inert.
- Capability checks precede claims and writes.
- Generic output shutdown becomes high impedance.
- A semantic output documents its inactive state before releasing its endpoint.
- Tests cover failed initialization, reset, repeated shutdown, and destruction.
- Instructions require power removal before wiring; cleanup is not permission
  to touch a powered circuit.

### Inputs and controls

- Pull policy, active level, valid voltage range, and floating-input behavior
  are explicit.
- Raw and interpreted state remain distinguishable.
- Stuck, disconnected, bouncing, simultaneous, and contradictory inputs are
  injectable faults.
- An operator confirmation is not an interlock. A button cannot be credited as
  an emergency stop unless independent rated hardware removes actuator power.

### PWM, tone, and timers

- Documentation calls PWM a switched waveform, not analog voltage.
- Pin and timer conflicts fail before hardware changes.
- Duty, frequency, current, and audible-exposure limits are bounded.
- Shutdown behavior is tested from every active mode.

### Analog and bus devices

- Input range, reference voltage, source impedance, and common-ground rules are
  stated.
- ADC counts are ratios to the selected reference. A lesson that discusses
  volts measures the reference rail instead of assuming it is exactly 5 V.
- External signals cannot back-power an unpowered board.
- Bus ownership, device supply, address or chip-select, and voltage compatibility
  are checked before connection.
- Storage and display failure cannot leave an actuator enabled.

### Lessons 007--009 analog circuits

These lessons are E1 and use only a USB-powered Mega 2560, passive divider
parts, and resistor-limited LEDs. Their supported bench circuits obey these
additional rules:

- Lesson 007 uses a 10 kΩ linear potentiometer between the Mega 5 V and GND
  rails, with its wiper on A0/TP1. Do not connect the wiper to a digital output.
- Lesson 008 retains that circuit so the raw and filtered observations describe
  the same physical signal. Filtering is interpretation, not electrical
  protection and not evidence that a wiring fault disappeared.
- Lesson 009 uses one photoresistor and one 10 kΩ fixed resistor as a divider.
  The fixed resistor limits current to at most 0.5 mA at 5 V even if the
  photoresistor is shorted. The divider's Thevenin source resistance remains
  no greater than 10 kΩ, matching the ADC source-impedance guidance.
- Every LED channel has its own 220 Ω or larger series resistor. With the
  actual LED forward voltage recorded, the worksheet must show no more than
  15 mA per channel and must total all simultaneously active channels.
- TP1 is measured only relative to a named GND test point. Place meter clips
  with USB power removed, inspect them, then apply power; do not move bare
  probes around the powered breadboard.
- The default ADC reference is the measured board supply. TP1 must remain
  between GND and that reference, and no external source is connected.
- A rail reading can mean a legitimate sensor extreme or a short/open in the
  divider. A disconnected A0 lead can float to an apparently plausible value.
  Software therefore calls endpoints suspected faults, never claims complete
  disconnection diagnosis, and always bounds LED duty.
- A suspected fault turns the lamp off and selects the documented fault
  pattern. A floating value that escapes detection can request only the same
  bounded, resistor-limited LED output as a valid sample.

The bench card records the measured 5 V rail, TP1 at both potentiometer stops
or both controlled light extremes, each LED resistor, each active-channel
current, and total LED current. Stop on a hot part, unstable USB connection,
TP1 outside the rails, an uncommanded output, or disagreement between the
schematic and breadboard. Remove USB power; software shutdown is not the stop
method.

### Servos, motors, and relays

- Only documented low-voltage modules and inert loads are supported.
- Logic and load power paths are shown separately; grounding and isolation
  follow the module manufacturer's primary documentation.
- The lesson specifies driver current, stall current where applicable,
  protection, supply limit, wire rating, and stored-energy handling.
- Motion has a guarded envelope, no pinch or entanglement exposure, a stable
  fixture, a low-energy first test, and an independent means to remove load
  power.
- Direction reversal, conflicting commands, sensor loss, reset, timeout, and
  communication loss reach a documented inactive state.
- A relay exercise switches only an inert, current-limited low-voltage
  indicator. No ADK lesson provides mains wiring guidance.

### Infrared and radio

- Infrared transmission is limited to a documented, owned, harmless lab target.
- Radio curriculum is passive receive-only observation of lawful signals and
  may use prerecorded or synthetic captures.
- Lessons provide no replay, cloning, brute force, jamming, access-control
  bypass, protocol-to-command mapping, antenna amplification, or transmitter
  implementation for observed RF.
- Frequency, licensing, privacy, equipment, and location remain learner
  responsibilities; uncertainty stops the exercise.
- Prefer shielded, conducted, synthetic, or prerecorded evidence when it meets
  the learning objective.

### Cue simulation

- Cue identifiers have no electrical firing meaning or launcher protocol.
- Outputs are screen state, logs, or low-voltage indicators incapable of
  ignition.
- Continuity is synthetic or uses isolated inert fixtures only; no part of the
  project connects to an initiator or live firing circuit.
- Arming, confirmation, faults, and emergency shutdown are state-machine
  subjects, not claims of functional safety.
- ADK does not export a launcher driver, transmitter, waveform, pinout, adapter,
  or real-show operating procedure.

### Lessons 049--051 identity and carousel boundaries

Lessons 049--051 are E0 policy work until separately qualified hardware gates
are recorded. Their copied UID-shaped values are local identifiers, not
authentication, authorization, proof of identity, access control, or a
security boundary. Enrollment, lookup, key confirmation, presentation, and
audit results remain simulated decisions over caller-owned memory.

- E0 owns no pins, buses, timers, interrupts, storage medium, display, reader,
  sensor, motor, servo, or power path. Home and stop are copied evidence;
  positions, step vectors, coil patterns, gate commands, display records, and
  storage records are logical intents only. They do not prove physical
  position, motion, actuation, display, or persistence.
- A simulated durable-start admission may follow only the specified
  preview/export/acknowledge sequence over deterministic caller-owned record
  images. It demonstrates recovery policy, not a completed EEPROM, SD, flash,
  or other physical-media write.
- Once that model exposes an authorization-start record, stop must retain its
  operation identity and reconcile an attributable `Stopped` terminal; it
  cannot silently discard the open operation.
- Lesson 051's atomic one-step logical composition intentionally publishes
  zero coil intent because `holdAtRest` is false. This is neither an energized
  coil observation nor evidence of physical motion; Lesson 047 retains the
  staged logical-coil teaching surface.
- Startup and every restart begin closed and off, with physical position and
  home unknown. Persisted identity or audit data cannot restore position
  authority, energize motion, open the gate, or bypass a fresh qualified
  homing sequence.
- E1 requires separate qualification of the exact RFID reader and owned cards,
  keypad matrix and polarity, home and stop inputs, display and its bus, and
  storage device and medium. Record supply voltage, logic-level compatibility,
  pin and bus ownership, pull policy, current, authoritative schematics,
  unpowered wiring inspection, observation points, failure injection, and
  restart/recovery behavior. E1 retains the stepper, servo, and their load
  supplies physically absent.
- E2 requires the exact stepper, rated driver, servo, restrained lightweight
  carousel, guarded travel envelope, separate current-limited load supplies,
  and an independent means to remove actuator power. Bench acceptance records
  measured idle, moving, stall, and simultaneous-load current; driver and
  supply temperature; home repeatability; travel bounds; stop, reset, sensor
  loss, jam, and communication-loss behavior; and proof that removing load
  power leaves the mechanism closed and inactive.

### Lessons 055--057 clue and escape-console boundaries

Lessons 055--057 are E0 policy work until the separately qualified E1 and E2
gates are recorded. The constraint model, operator-panel policy, audit image,
and escape-console coordinator are puzzle-teaching models. They are not access
control, authentication, authorization, security, confinement, egress,
emergency release, life-safety, or protective interlock systems, and no person
or animal may depend on them to enter, leave, summon help, or remain safe.

- E0 owns no endpoint, pin, interrupt, timer, display, storage medium, servo,
  relay, lamp, latch, door, lock, or power path. Copied clues, button evidence,
  presentation, audit, latch, lamp, and stop values are logical evidence or
  semantic intents only.
- A clue result cannot identify or authenticate a person, grant access, secure
  property, confine an occupant, or establish that a physical room is safe.
  The six Lesson 057 clue families are fictional puzzle categories only.
- Operator acknowledgement may clear only an acknowledgeable diagnostic or
  audit condition. It cannot override stop, invalid configuration, internal
  failure, stale or contradictory evidence, a full or indeterminate audit
  image, or an invalid input chord.
- Restart, torn or corrupt audit-image recovery, storage failure, display
  failure, source loss, timing ambiguity, and contradictory clues leave latch
  intent inert and fault presentation requested. Software stop state is not an
  emergency stop and never substitutes for physical power removal.
- E1 may qualify only exact passive inputs, low-voltage current-limited
  indicators, and presentation hardware while servo, relay, latch, lock, and
  other powered actuation remain physically absent.
- E2 may use only a restrained demonstration servo or an inert,
  current-limited low-voltage relay-and-lamp load with independent physical
  load-power removal. It must not attach to a door, gate, lock, occupied
  enclosure, alarm, emergency lighting, egress route, or life-safety system.

### Lessons 067--069 inertial-recorder boundaries

Lessons 067--069 are E0 copied-record work until each separately scoped E1
gate is recorded. Normalization, source qualification, record assembly,
presentation intent, and persistence intent operate only on caller-supplied
records and memory. They do not prove that a sensor was powered, sampled,
mounted, oriented, calibrated, displayed, or written to durable media.

- E0 owns no pin, bus, interrupt, timer, sensor, display, button, real-time
  clock, storage device, storage medium, or power path. Source identities and
  measurements are copied provenance, and display, indicator, control, clock,
  and storage values are semantic evidence or intents only.
- One explicitly configured copied source is admitted per recorder session.
  Source matching, freshness checks, frame mapping, qualification, sequence
  checks, and record integrity demonstrate deterministic policy, not physical
  device identity, correct mounting, calibration, timing accuracy, or sensor
  health.
- A prepared, exported, or acknowledged record demonstrates only the specified
  in-memory transaction policy. It is not evidence of an SD-card, flash,
  EEPROM, or other physical-media write, and restart does not imply that a
  record survived.
- E1a separately qualifies the exact inertial sensor, carrier revision, supply
  and logic levels, bus address and ownership, pull-ups and straps, mounting
  axes, data-ready behavior, configured ranges and rates, conversion rules,
  calibration, observation points, and source-loss behavior. The display,
  controls, clock, and storage hardware remain physically absent.
- E1b separately qualifies the exact display, current-limited indicators, and
  passive controls, including their supply, bus or endpoint ownership,
  polarity, bounded current, inactive presentation, and failure behavior. The
  inertial sensor, clock, and storage hardware are not thereby qualified.
- E1c separately qualifies the exact real-time clock and storage device and
  medium, including supply and logic levels, bus or chip-select ownership,
  timestamp provenance, capacity, removal, write-failure, torn-write,
  corruption, and restart/recovery behavior. E1c does not retroactively
  qualify E1a or E1b, and no combined powered fixture ships until all three
  gates and their integration evidence are accepted.

### Lessons 070--072 threshold-module characterization boundaries

Lessons 070--072 are E0 copied-descriptor and copied-sweep work until an exact
low-voltage analog/comparator specimen passes its separate E1 gate. At E0 the
descriptor retains a declared specimen reference copied from the caller; it
does not identify a retail board, establish ratings, authorize power, or prove
that its analog and comparator outputs are safe or truthful.

- E0 owns no pin, ADC channel, interrupt, timer, bus, sensor, comparator,
  threshold potentiometer, display, indicator, supply, or power path. Raw
  values, comparator states, controlled-sample labels, timing, threshold
  crossings, hysteresis, chatter, stuck-output findings, disagreements, and
  volatile characterization evidence records are caller-supplied evidence or
  semantic intents only. E0 reports raw lower-rail and upper-rail evidence and
  never diagnoses an open or short; only separately instrumented E1 acceptance
  may do so.
- Unknown, ambiguously marked, conflicting, damaged, or incompletely sourced
  modules remain unpowered and are rejected. A kit name, PCB color, connector
  shape, copied descriptor, or resemblance to a qualified board does not
  establish electrical identity or extend acceptance to another revision.
- At E0, Lesson 072 accepts one declared specimen reference per copied
  characterization run; at E1, it accepts one exactly identified physical
  specimen per run. It does not provide blanket support for a product family
  or every board sold in a "sensor kit," and evidence for one specimen cannot
  qualify another specimen or revision.
- E1a separately qualifies the exact module and PCB revision using primary
  device documentation plus the authoritative module schematic, or primary
  device documentation plus a reviewed trace of the populated module when no
  authoritative module schematic exists. Record supply and logic ranges;
  analog/comparator output topology and limits; the digital-output pull-up rail
  and whether the pull is onboard, external, or forbidden; threshold-pot
  direction and end stops; startup, warm-up, settling, source impedance,
  pinout, polarity, current, and named analog/digital test points. Record the
  selected ADC reference, its measured voltage, AREF handling, and the
  source-impedance and settling proof for that ADC configuration. Analyze every
  powered/unpowered combination so neither AO, DO, a pull-up, nor a
  supply-control signal back-powers the Mega or module.
- Switched module power is optional, not an E1 requirement, and a module is
  never sourced from a Mega GPIO. If a characterization genuinely requires
  switching, qualify an exact rated high-side or load switch and record its
  current limit, peak and inrush behavior, inactive default, reverse-current
  and back-power behavior, output discharge, timing, thermal margin, and
  observed power-removed state. Otherwise use a documented current-limited
  board supply and remove physical power to stop. In either case, perform an
  all-power-off wiring inspection and place meter clips before the first
  powered check. Stop on heat, odor, unstable current, rail violation,
  unexpected output drive, or disagreement with the authoritative schematic
  or reviewed populated trace.
- E1 characterization uses only a documented harmless stimulus within the
  exact specimen's ratings and family boundary: room light or an already
  qualified, enclosed low-energy visible/IR source for light, tracking,
  avoidance, photo-interrupter, and passive-radiant boards; an ordinary
  conversational sound or bounded recorded sounder below the lesson's audible
  exposure limit for microphone boards; a small retained permanent magnet
  moved by hand for Hall boards; a dry, electrically inert, comfortably
  touchable object within the documented temperature range for thermistors;
  an insulated conductive test piece rather than a person for Metal Touch; and
  gentle hand displacement or a soft target for contact, vibration, and
  obstacle boards. Keep optical sources out of the eye line and magnets away
  from magnetic media and medical devices. Do not use direct sun, lasers,
  flame, hot objects, liquids, gas, smoke, combustion products, hazardous
  chemicals, body contact, physiological measurement, ionizing sources,
  energetic motion, unknown optical emission, or environmental claims that
  require a separately scoped boundary. Register devices and active emitters
  are outside this arc.
- E1b separately qualifies the exact low-voltage display, resistor-limited
  indicator, and passive control fixture, including endpoint ownership,
  current budget, inactive/fault presentation, and a non-Serial observation
  path. Presentation failure cannot convert an invalid or ambiguous
  characterization into acceptance.
- Raw lower-rail evidence, raw upper-rail evidence, stale, chatter, comparator
  disagreement, and threshold-pot extremes remain explicit E0 outcomes. E0
  never diagnoses an open or short. Physical open and short become E1 findings
  only when the acceptance record names the independent instrument, test
  points, method, and observed values. Software classification, filtering,
  hysteresis, cleanup, or reset is neither electrical protection nor evidence
  that a wiring or specimen fault disappeared; physical power removal is the
  stop method.
- A generated volatile characterization evidence record demonstrates only the
  conditions and declared specimen reference represented by its copied input.
  A physical E1 acceptance additionally records the exact specimen, primary
  device documents, and authoritative module schematic or reviewed populated
  trace; descriptor and firmware revisions; supply and current limit; ADC
  reference and source impedance; stimulus and bounds; instruments and test
  points; AO/DO and pull-rail observations; warm-up and settling measurements;
  current/inrush and power-removal observations; deviations; human reviewer;
  and date. It is not calibration certification, metrology traceability,
  product approval, environmental safety monitoring, or evidence that the
  module is suitable for control or protective service.

## Project hazard gates

Every project first passes all component gates it composes. It then records the
following project-specific evidence.

| Lesson | Project | Class | Release gate |
|---:|---|:---:|---|
| 003 | Reaction timer | E1 | No startling output; stuck, bounce, false-start, timeout, and shutdown traces |
| 006 | Simon | E1 | Bounded light and sound; chord, timeout, rollover, and shutdown traces |
| 009 | Adaptive night light | E1 | Recognized rail faults force zero duty; floating or plausible faults remain bounded by the resistor-limited output |
| 012 | Traffic junction | E1 | Conflicting greens remain impossible; every failure forces all-red |
| 015 | Environmental station | E1 | Invalid or missing sensor data remains explicit; no safety-alarm claim |
| 018 | Inert access trainer | E2 | Soft latch first; restrained servo; independent load-power removal |
| 021 | Bench rover | E2 | Wheels raised first; bounded area; physical load-power disconnect; loss-of-control stop test |
| 024 | Greenhouse controller | E1 | LEDs simulate loads; schedules and faults cannot enable an unsafe combination |
| 027 | Telemetry console | E1 | Receive-only or synthetic radio; stale data is explicit; no safety dispatch claim |
| 030 | Show-cue simulator | E0/E1 | Inert outputs only; no launcher/initiator connection; complete deterministic fault and audit trace |
| 045 | Balance-table instrument | E0/E1 | Stationary hand-operated tabletop tilt/orientation demonstrator; E0 replay has no hardware side effect; stale, saturated, or unsteady evidence forces a distinct visible ineligible diagnostic and silent tone; qualify the exact E1 circuit before using indicators or controls; remove physical power to stop |
| 048 | Kinetic sculpture | E0/E1/E2 | E0 uses memory-only intent and has no hardware side effect; E1 qualifies the exact inert inputs and indicators with the motor absent; E2 requires the exact restrained motor, ULN driver, separate current-limited load supply, and independent physical load-power removal before motion |
| 051 | Tabletop parts carousel | E0/E1/E2 | E0 admits only copied identity, home, stop, key, record-image, and logical actuator intent; E1 separately qualifies the exact RFID, keypad, home/stop, display, and storage fixture with actuators absent; E2 requires restrained stepper/servo motion, independent actuator-power removal, and measured bench acceptance |
| 054 | Infrared command translator | E0/E1 | E0 is inert and produces no physical output; E1 requires an exact current-limited, burst-bounded, fail-off fixture aimed only at an owned harmless adjacent target and admits only fixed local catalog commands—never consumer/security control or unknown capture replay; cancellation and missed service force emission off, physical power removal remains the stop method, and acquisition, inactive-state, and optical-emission evidence are recorded separately |
| 057 | Inert escape-room console | E0/E1/E2 | E0 is copied puzzle policy with inert latch and lamp intent; it provides no access-control, security, confinement, egress, or life-safety function; E1 admits exact passive inputs and presentation with actuators absent; E2 admits only a restrained demonstration servo or inert low-voltage relay/lamp load with independent physical load-power removal and no door, lock, occupied enclosure, or safety-system connection |
| 060 | Dual-display timing desk | E0/E1 | E0 proves supplied-time policy and copied command/receipt evidence only; E1a and E1b independently qualify the exact multiplex and MAX7219 fixtures before E1c combines them with worst-case current, rail-droop, thermal, refresh, optical-agreement, blanking, and physical-power-removal evidence |
| 069 | Interchangeable motion recorder | E0/E1 | E0 accepts one configured copied inertial source per session and produces only memory-backed records and presentation, clock, control, and storage intents; E1a sensor acquisition, E1b presentation and controls, and E1c clock and storage are independently qualified, and no powered or durable-recording claim is permitted until all applicable gates and the combined integration record are accepted |
| 072 | Module characterization bench | E0/E1 | E0 replays one declared specimen reference and caller-supplied analog/comparator sweeps into a volatile characterization evidence record with no hardware side effect, reports raw lower/upper-rail evidence, and never diagnoses open or short; E1a independently qualifies the exact low-voltage specimen, authoritative schematic or reviewed populated trace, acquisition, supply and optional rated load switch, and bounded harmless family-specific stimulus, E1b independently qualifies presentation and passive controls, and open/short findings require instrumented human acceptance while unknown modules, gas/smoke/chemical exposure, physiological use, register devices, active emitters, and blanket kit-family claims remain outside the project |

Project 018 does not secure property. Project 021 carries no person, animal,
hot item, sharp tool, or hazardous material. Project 027 does not monitor a
safety-critical condition. Project 030 does not operate a real show. Project
045 is not an actuator, navigation, stabilization, interlock, or safety-control
system. Any eventual physical platform is lightweight, stable on the tabletop,
free of sharp edges, and carries no loose or rolling load.
Project 048 makes no emergency-stop or position-feedback claim. Its software
state, input controls, and indicators do not replace independent physical
load-power removal, and commanded steps do not prove that the mechanism moved
or reached a position.
Project 051 does not secure property or authenticate a person. Its local
identifier match and key confirmation are demonstration prerequisites only.
Software stop state is not an emergency stop, logical position is not physical
position, and a durable-start acknowledgment is not evidence of a physical
write. Restart leaves the gate closed, actuator intent off, and position
unknown until the applicable E1 and E2 gates are separately accepted.
Project 057 does not secure property, control access, confine an occupant, or
provide emergency release, egress, alarm, or life-safety behavior. Puzzle
completion and operator acknowledgement can request only inert semantic intent
at E0. Any later demonstration actuator remains incapable of controlling a
real door or occupied enclosure and requires independent physical load-power
removal.
Project 072 is a teaching and specimen-characterization bench, not a universal
component tester, calibration laboratory, environmental monitor, exposure
system, or safety controller. A passing run applies only to the exact
identified specimen, descriptor revision, wiring, supply, stimulus, and
conditions recorded; it cannot authorize another module or powered
application.

## Lesson release record

The lesson owner records:

1. energy class, exact board, parts, supplies, ratings, and primary sources;
2. authoritative schematic and an all-power-off inspection;
3. normal, startup, reset, failure, shutdown, and power-removed states;
4. current limit and measured voltage/current where relevant;
5. foreseeable shorts, reversals, disconnections, stalls, conflicts, and stale
   data;
6. explicit stop conditions and the physical power-removal method;
7. simulation or inert-load evidence before E2 hardware;
8. instruments, observations, deviations, reviewer, and date.

Do not mark hardware accepted without this record. Do not infer electrical
safety from a successful compile, unit test, diagram, LED, or destructor.

## Stop and escalation rule

Stop development or publication when identity, rating, polarity, supply,
isolation, safe state, legality, load behavior, or physical acceptance is
unknown. Record the gap as blocked. Resolve it with the manufacturer's primary
documentation and competent human review; do not discover a safety limit by
experiment.
