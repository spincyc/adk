# Lessons 017--018 servo and inert-trainer safety gates

Status: isolated design authority; implementation and bench acceptance open  
Energy class: E2  
Reference board: Mega 2560 Rev3  
Reference load: one exact, documented 4.8 V-class micro servo

This note tightens the executable design in
[`LESSONS_016_018.md`](LESSONS_016_018.md). It does not admit an arbitrary
“SG90-compatible” servo or power module. The lesson remains blocked until the
owned servo's manufacturer, markings, pin order, voltage range, pulse range,
idle/running/stall current, and mechanical travel have been recorded from a
primary source and verified at the bench.

Lesson 017 teaches bounded command intent and an external power boundary.
Lesson 018 composes that intent with an inert state-machine trainer. Neither
lesson implements a lock, verifies physical position, protects property, or
provides a functional-safety mechanism.

## Required physical system

The accepted first fixture contains:

- one identified micro servo rated for the selected regulated supply;
- one bench supply with adjustable current limiting, or another documented
  regulated supply plus independently rated current protection;
- a dedicated physical switch or removable connector in the **servo-positive**
  lead, reachable without entering the motion envelope;
- Mega GND and servo-supply negative connected at one named star/common point;
- Mega D44 connected only to the servo signal input;
- a TP-S logic test point at D44, TP-V at the servo supply, and a documented
  current-measurement point in the servo-positive lead;
- a resistor-limited status LED powered by the Mega logic domain;
- a paper pointer on the horn over a printed card, with no linkage, latch,
  door, stored spring energy, hard end stop, or useful load.

The servo positive lead does not connect to the Mega 5 V or I/O pins. The
external supply does not connect to Mega 5 V, VIN, barrel input, or USB 5 V.
The common ground is a signal reference, not permission to join positive
rails. Connect common ground first, signal second, and servo positive last;
remove servo positive first.

Do not use a rectangular 9 V battery, an unidentified breadboard power module,
or a computer USB port as the servo source. Do not choose a supply limit from
internet folklore. The initial current limit comes from the exact servo's
primary documentation and is then compared with measured idle, movement, and
brief controlled stall/start evidence. No deliberate sustained stall is
permitted.

## Electrical state and power sequencing

Construction and every rewire are all-power-off. Before servo load power:

1. identify wires from the selected servo documentation, not color alone;
2. inspect polarity, common ground, the open load-power switch, and unobstructed
   pointer travel;
3. initialize all logic resources and validate calibration;
4. show logic-ready on the status LED;
5. observe TP-S for the bounded command waveform with the servo still
   unpowered;
6. set the external supply voltage and current limit, then close load power
   while clear of the motion envelope.

Shutdown reverses the sequence: open physical servo power, verify current is
zero, stop the pulse endpoint, make D44 high impedance, release Timer5 and D44,
then remove logic power. `shutdown()`, timer release, a closed command, or a
destructor is not the physical stop method.

The design explicitly tests these asymmetric power cases:

- servo supply off, Mega on: signal must not back-power the servo;
- servo supply on, Mega off/resetting: no current may enter an unpowered Mega
  through D44; the supported operating procedure prevents this state by opening
  servo power before reset/upload;
- brownout or repeated Mega reset: the learner opens servo power immediately
  and does not continue until the cause is measured;
- loss of common ground: stop, remove servo power, and inspect; software cannot
  diagnose the resulting command reference reliably.

## Pulse, timer, and resource gate

The first supported Mega endpoint uses D44/OC5C and owns D44 plus **Timer5
exclusively**. It claims and configures nothing unless calibration, pin
capability, and the complete resource set are valid. A conflict returns a
status before D44 changes mode or produces a pulse.

Timer5 ownership conflicts with PWM on D44, D45, and D46. The official Arduino
Servo library also allocates Mega 2560 16-bit timers in the sequence Timer5,
Timer1, Timer3, Timer4 and states that associated `analogWrite()` PWM is
disabled when a timer is seized. First-class ADK code therefore does not call
the global Servo library behind the resource registry. It uses an ADK pulse
driver, or a future adapter that makes the same timer ownership visible.

The calibration record contains:

- format version and record length;
- minimum and maximum absolute pulse bounds supported by the admitted servo;
- closed and open command pulses strictly inside or equal to those bounds;
- direction/endpoint identity;
- integrity field over every safety-relevant byte.

Integrity detects accidental corruption; it is not authentication. Parsing
uses a fixed-width byte representation with explicit endianness and rejects
unknown version, length, reserved bits, checksum, non-monotonic endpoints,
out-of-range values, duplicate endpoints, and partial records. Rejection emits
no pulse and makes the fault visible. Do not silently clamp, repair, migrate,
or fall back to a wider default range.

The pulse endpoint bounds both high time and frame interval. Tests establish
the exact allowable jitter and prove no high pulse can remain asserted after
timer failure, shutdown, destruction, or rollover. The public component speaks
in `ServoPosition`/permille; only the endpoint translates to microseconds.

## Motion and current gate

The learner marks the two observed pointer endpoints only after a center/neutral
command succeeds with no horn attachment, then fits the paper pointer with
servo power removed. The permitted software envelope is narrower than any
observed mechanical stop. The exercise never discovers a pulse limit by
driving into a stop.

The pointer card lies flat, is taped down, and has a keep-clear arc. Hair,
clothing, cables, and fingers remain outside it whenever servo power is
present. Nothing is attached that can pinch, trap, lift, propel, secure, or
damage an object. A hot, buzzing, chattering, stalled, or unexpectedly moving
servo is a stop condition.

The bench record measures:

- supply voltage with no servo, at idle, during each endpoint transition, and
  at the largest observed transient;
- idle/holding current, current for both directions, startup peak when the
  instrument can capture it, and the exact configured current limit;
- TP-S high time and frame interval at center and both endpoints;
- position-marker agreement after each bounded command;
- load-power-open current, D44 high-impedance behavior, and no back-power in
  the accepted shutdown sequence.

A hobby servo normally has no position feedback to the controller. The paper
pointer is observation, not closed-loop proof. “Commanded closed” and
“physically observed at the closed marker” remain separate claims. Holding a
closed command can draw current and cannot be called de-energized; detaching
pulses can let the shaft move and cannot be called mechanically locked.

## Restart, persistence, and corruption

Construction starts inert: no pulse, servo power open, and status LED showing
uninitialized. On every boot, reset, watchdog restart, upload, or return from a
power interruption:

1. keep servo load power physically open;
2. validate the complete calibration/configuration record;
3. acquire D44 and Timer5 transactionally;
4. expose logic-ready and the intended closed command at TP-S;
5. require a deliberate local bench confirmation before the learner applies
   servo power.

Firmware never actuates merely because a stored record says the previous state
was granted/open. Transient access state and credentials are not restored.
Lockout persistence, if later taught, is a separate versioned record and must
define monotonic-time loss; corruption cannot shorten a lockout or open the
soft latch. A missing, erased, truncated, future-version, checksum-failed, or
semantically invalid record enters `Fault`, requests closed intent, emits no
servo pulse until explicit recovery, and remains visibly distinct from
`Denied`.

Two copies plus sequence numbers may improve recovery from interrupted writes,
but redundancy is not assumed correct until tests cover every byte tear,
one-copy/two-copy corruption, sequence rollover, and power loss at every write
boundary. A compiled-in known bench calibration is acceptable for the first
lesson; persistent calibration should not be introduced merely to satisfy the
project theme.

## Lesson 018 inert access-trainer boundary

The “latch” is only the paper pointer/foam flag. The project is not connected
to a door, cabinet, lock, alarm, access strike, relay, solenoid, real
credential, valuable property, or remote command source. The teaching
credential is fixed or explicitly supplied test data and is never presented as
secure storage or authentication.

The state engine owns no pin, timer, storage, display, or servo driver. It emits
`softLatchOpen` intent. The composition translates that intent to one of the
two already validated `BoundedServo` endpoints.

Fault policy is fail-inert, not fail-secure:

- keypad chord/stuck/fault, clock rollback, corrupt configuration, display
  failure, audit failure, actuator error, unexpected restart, or resource loss
  enters `Fault`;
- `Fault` requests closed intent and a red visible pattern, but the UI says
  **close requested**, never **closed**, until the pointer is observed;
- actuator failure never becomes a granted state;
- audit failure does not hide the visible fault;
- reset clears entered digits and granted/open state, and does not move until
  lesson-017 restart gates are repeated;
- physical servo-power removal overrides every software state.

RGB status cannot be the only actuator evidence. Required simultaneous
observations are RGB state, display prompt/count, paper-pointer marker, TP-S,
and supply current. Serial/audit records add replay detail but prove none of
those physical effects by themselves.

## Deterministic test gates

No implementation is complete without strict no-exception/no-RTTI host tests
for:

- every calibration field at, inside, and outside its boundary;
- every record byte corrupted, truncation, erased storage, unknown version,
  interrupted update, and deterministic recovery;
- D44 conflict, Timer5 conflict, claim rollback, repeated initialization,
  repeated shutdown, destruction while active, and resource reuse;
- pulse endpoints, frame interval, allowed jitter, update gaps, rollover, and
  injected endpoint failures while high and low;
- restart from every access state, with no restored grant/open actuation;
- failure of keypad, clock, display, audit sink, configuration, and servo from
  every state;
- commanded/observed position kept as separate snapshot/evidence fields;
- exact lockout and grant deadlines, including one tick before/at/after and
  timestamp wrap;
- identical replay producing byte-identical snapshots, output intents, fault
  patterns, and audit intents.

Host tests establish logic only. Hardware status remains “bench acceptance
open” until a named person completes the E2 record with the exact servo,
supply, current limit, fixture, instruments, observations, deviations, date,
and reviewer.

## Stop conditions

Open the physical servo-power switch immediately on:

- unexpected, reversed, oscillating, chattering, or runaway motion;
- contact with a marked endpoint/obstruction or a person entering the arc;
- hot servo, connector, wire, breadboard, supply, or odor;
- unexpected current-limit activation, current above the admitted value, or
  supply sag outside the servo rating;
- Mega reset, USB disconnect/reconnect loop, corrupt/fault display, loss of
  common ground, missing status LED, malformed TP-S waveform, or disagreement
  between command and pointer.

Do not investigate while load power remains present. Record the stop, remove
all power, inspect, and return to the first stage. Software retry is not the
recovery procedure.

## Primary references

- [Arduino Servo library](https://docs.arduino.cc/libraries/servo/) — external
  supply/common-ground guidance
- [Arduino Servo implementation and timer behavior](https://github.com/arduino-libraries/Servo/blob/master/src/Servo.h)
- [Mega 2560 AVR servo timer allocation](https://github.com/arduino-libraries/Servo/blob/master/src/avr/ServoTimers.h)
- [Arduino Mega 2560 Rev3 hardware resources](https://docs.arduino.cc/hardware/mega-2560)
- [Arduino Mega 2560 full pinout](https://docs.arduino.cc/resources/pinouts/A000067-full-pinout.pdf)
- [Microchip ATmega2560 device documentation](https://www.microchip.com/en-us/product/atmega2560)
- [TowerPro SG90 manufacturer page](https://towerpro.com.tw/product/sg90-7/)

The TowerPro page illustrates why exact identity matters: it describes its
current product as 4.8 V, externally powered, and approximately 0.5--2 A
operating current, while visually similar kit servos may be clones with
different limits. Those figures are not copied onto an unidentified Elegoo
servo's acceptance card.
