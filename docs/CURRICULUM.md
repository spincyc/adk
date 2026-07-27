# First-class curriculum

This is the planned learning path for ADK. It records the canonical numbers
before the lessons are promoted; a row is not evidence that its interface or
lesson is supported yet. Material moved to `legacy/` is historical context,
not a prerequisite and not a statement of the current API. Published lessons
use the first-class RAII interfaces.

## Current status

Lessons 001--027 have first-class implementation work, deterministic host
tests, canonical Mega 2560 examples, lesson sources, and size evidence. Their
bench cards remain open. Lesson 028 is the active implementation boundary;
lessons 028--030 remain the
ordered delivery queue. The [work queue](WORK_QUEUE.md) records later
expansion, research, physical, and publication work.

All lessons remain experimental. Host verification, firmware compilation,
documentation publication, and size evidence do not imply that a circuit has
passed its Mega 2560 bench card. Physical acceptance is deliberately deferred
until measured results are recorded; that open hardware gate does not pause
implementation of later lessons.

The fixed resource registry uses 17 bytes: 11 ownership bytes plus six shared
timer counters. It uses no heap allocation.

## Uninterrupted delivery policy

Curriculum implementation proceeds through lesson 030 without waiting for the
physical bench campaign. Each lesson may advance through interface, lifecycle,
determinism, host, Arduino compile, size, documentation, packaging, and
publication gates. Its status remains **host verified; hardware acceptance
open** until the physical record exists.

An earlier open bench card does not block a later software or documentation
slice. An unresolved interface, deterministic test, electrical-safety rule, or
compile failure does block its consumers. Later work must not invent measured
voltages, timing tolerances, current, signal integrity, or physical behavior.

For every lesson, the queued implementation order is:

1. establish the deterministic behavior and resource boundary;
2. add the endpoint or component with rollback and safe shutdown;
3. add host fakes, replay traces, and failure tests;
4. add a narrative Mega example and enforce its firmware budget;
5. publish complementary HTML and PDF material with circuit-native evidence;
6. record the unperformed bench checks as open acceptance items;
7. continue to the next lesson when all non-hardware gates pass.

## Delivery queue

| Block | Status | Interface and composition boundary | Circuit-native evidence |
|---|---|---|---|
| 010--012 | Host verified; bench open | `ShiftRegisterOutput`, seven-segment presentation, explicit traffic timing, and `TrafficJunction` | Shift-clock/data/latch test points, display self-test, and an all-red fault state |
| 013 | Host verified; bench open | Validated temperature/humidity samples and scheduled acquisition | Sensor-health indicator and a measurable sample cadence |
| 014 | Host verified; bench open | LCD presentation and stable records | Display status field, enable test point, and acquisition LED |
| 015 | Host verified; bench open | Environmental-station composition | Display age/status field and sensor-health evidence |
| 016 | Host verified; bench open | Keypad events and matrix scanning | Key echo/status pattern and row scan test point |
| 017 | Host verified; bench open | Bounded servo intent and versioned configuration | Command-position marker and independent power evidence |
| 018 | Host verified; bench open | Inert access trainer | Soft-latch state indicator and bounded event-audit presentation |
| 019 | Host verified; bench open | Range validity and explicit echo timing | Echo timing point and distinct timeout/range evidence |
| 020 | Host verified; bench open | Motor intent, reversal dead time, and stop policy | Direction/enable indicators and stopped-state evidence |
| 021 | Host verified; bench open | Rover supervision and deterministic route policy | Independent stop and requested/applied motion evidence |
| 022 | Host verified; bench open | Owned `I2cBus`/`SpiBus`, explicit RTC state, and deterministic durable-record models | Bus activity points and durable record acknowledgement |
| 023 | Host verified; bench open | Constrained simulated loads and watering policy | Mutually exclusive load-state LEDs and a fault pattern |
| 024 | Host verified; bench open | Greenhouse controller; physical RTC/media adapters remain deferred | Deterministic decision and record replay |
| 025 | Host verified; bench open | Owned infrared capture and classic NEC-only evidence | Capture indicator and stable receive-only record |
| 026 | Host verified; bench open | Exact telemetry packets and freshness tracking | Timestamp, integrity, and age evidence from deterministic fixtures |
| 027 | Host verified; bench open | Scheduling and deterministic telemetry console | Stale-data alarm and stored replay record |
| 028--030 | Queued capstone | Injectable continuity/fault models, deterministic cue scheduling, operator confirmation, and inert show-cue simulator | Redundant state indicators, inert channel lamps, stop dominance, and a complete audit log |

The coordinator promotes a row from queued to active only after its public
dependencies have landed. The queue fixes teaching order, not implementation
claims.

## Cadence

Two component lessons introduce one small layer at a time. Every third lesson
is a project that composes the preceding work with earlier components. A
project may deepen an interface, but it may not quietly introduce a new
hardware abstraction. Each lesson remains useful on its own; the sequence
supplies the shortest planned path through the library.

| Lesson | Kind | Subject | Required outcome |
|---:|---|---|---|
| 001 | Component | `DigitalOutput`, `MonoLed`, lifecycle, resource claims, and time | A diagnostic LED reaches a safe state and always releases its pin |
| 002 | Component | `DigitalInput`, pull-up policy, and raw switch observation | Pull-up wiring is sampled without hiding bounce |
| 003 | Project-bearing | `Button`, debounce, edges, and reaction timer | Recorded input and clock traces produce repeatable events and trials |
| 004 | Component | `PwmOutput`, duty cycle, and `RgbLed` | Color and brightness are bounded by board capability and current limits |
| 005 | Component | Piezo sounder, tone, and timer ownership | Audible cues coexist with PWM or fail with a useful resource error |
| 006 | Project-bearing | Deterministic Simon | Four buttons, four cues, sound, and a seeded sequence replay exactly |
| 007 | Component | `AnalogInput`, potentiometer, and calibration | Raw samples map to explicit engineering ranges without hidden filtering |
| 008 | Component | Calibration and sampled filtering | Filter state advances only from supplied samples; raw and filtered evidence remain visible |
| 009 | Project-bearing | Adaptive night light | Calibrated sensing controls color, brightness, hysteresis, and diagnostics |
| 010 | Component | Shift register and seven-segment display | Serialized output drives a readable display without blocking |
| 011 | Component | Finite-state timing and pedestrian requests | Timed transitions remain explicit, nonblocking, and testable |
| 012 | Project-bearing | Traffic junction | Conflicting greens are impossible and every failure forces all-red |
| 013 | Component | Temperature and humidity sensing | Transport failure and invalid measurements remain distinct |
| 014 | Component | Character LCD and stable serial records | Presentation is separate from state and records are locale-independent |
| 015 | Project-bearing | Environmental station | Scheduled sensors, display, and health records remain reproducible |
| 016 | Component | Keypad and operator-panel events | Key sequences and invalid chords are observable without global callbacks |
| 017 | Component | Servo, persistent configuration, and external power | Motion is bounded and corrupt configuration fails safely |
| 018 | Project-bearing | Inert access-control trainer | A soft latch, lockout, prompts, and audit state compose deterministically |
| 019 | Component | Distance sensing and explicit validity | Timeout and out-of-range remain distinct from valid measurements |
| 020 | Component | Motor driver and emergency-stop policy | Direction, enable, dead time, and shutdown are explicit and fail safe |
| 021 | Project-bearing | Bench rover | Sensing, motion, scripted routes, and emergency stop pass first with wheels raised |
| 022 | Component | RTC state, durable-record models, `I2cBus`, and `SpiBus` | Leased transactions restore bus state and deterministic records survive restart |
| 023 | Component | Relay-module simulation and constrained outputs | Inert fan, pump, and heater states enforce mutual exclusion |
| 024 | Project-bearing | Greenhouse controller | Schedules, sensor faults, simulated loads, and logs reproduce every decision |
| 025 | Component | Infrared receive and decoded command models | Captures preserve timing evidence while protocol policy stays separate |
| 026 | Component | Exact packet reception and freshness evidence | Synthetic packet observations become timestamped data without a physical-radio claim |
| 027 | Project-bearing | Multi-source telemetry console | Deterministic fixture sources, health evidence, acknowledgement, and storage share one scheduler |
| 028 | Component | Continuity, fault, and redundant-state simulation | Open, short, stale, and contradictory states are injectable and observable |
| 029 | Component | Cue scheduling, audit logs, and operator confirmation | A clocked cue engine is deterministic, inert, and fully replayable |
| 030 | Project-bearing capstone | Inert show-cue simulator | Redundant arming, inert loads, faults, shutdown, and evidence logs pass review |
| 031 | Component | Calibrated analog joystick | Raw axes, dead zones, saturation, and switch events remain visible |
| 032 | Component | Quadrature encoder | Valid Gray-code edges produce a bounded count while invalid transitions remain visible |
| 033 | Project-bearing | Calibration console | Joystick selection, encoder trim, commit, and cancel replay exactly |
| 034 | Component | Magnetic and contact sensing | Hall, reed, and ball-switch observations retain polarity, hysteresis, and faults |
| 035 | Component | Qualified passage events | Dwell, direction, duplicate suppression, and timeout produce stable records |
| 036 | Project-bearing | Magnetic passage logger | Contact and encoder evidence produce RTC-stamped counts without a later identity driver |
| 037 | Component | Contact dynamics | Tilt, vibration, knock, tap, and shock retain pulse and refractory evidence |
| 038 | Component | Acoustic envelopes | Raw amplitude, baseline, clipping, and event windows remain distinct |
| 039 | Project-bearing | Percussion sequencer | Contact timing and relative intensity replay through existing lights and sound |
| 040 | Component | Reflective and interrupted light | Optical observations retain calibration, ambient effects, and crosstalk |
| 041 | Component | Presence and passage | PIR, beam, and range observations retain validity, age, and disagreement |
| 042 | Project-bearing | Tabletop course marshal | Ordered checkpoints and finish evidence produce a replayable timed run |
| 043 | Component | Resistive environmental probes | Switched-power wetness observations expose calibration, corrosion, and faults |
| 044 | Component | Thermal and radiant observations | Units, uncertainty, validity, age, and controlled low-energy IR remain explicit |
| 045 | Project-bearing | Museum-case monitor | Sensor health, inert alarm intent, and records reproduce each decision |
| 046 | Component | Touch, proximity, and pulse demonstrations | Human-input observations retain noise and make no medical claim |
| 047 | Component | Bounded stepper motion | Coil frames, rate, travel, cancellation, and de-energized shutdown are explicit |
| 048 | Project-bearing | Kinetic light sculpture | Existing indicators mirror bounded motion intent before motor power is used |
| 049 | Component | Identity records | RFID observations and keypad entries form bounded local identifiers, not authentication |
| 050 | Component | Positioning and homing | Position remains unknown until bounded homing succeeds |
| 051 | Project-bearing | Tabletop parts carousel | Identity, confirmation, homing, and inert gate intent compose safely |
| 052 | Component | Captured IR pulse trains | Known, repeated, unknown, and malformed receive evidence remain distinct |
| 053 | Component | Known-code IR transmission | Only documented learner-created codes use a bounded, cancellable emitter |
| 054 | Project-bearing | IR command translator | Adjacent owned transmitter and receiver fixtures replay known commands |
| 055 | Component | Threshold-module descriptors | Identified low-voltage comparator modules share one explicit electrical descriptor |
| 056 | Component | Characterization runs | Supplied sweeps expose threshold, hysteresis, chatter, and disagreement |
| 057 | Project-bearing | Module characterization bench | One identified analog/comparator specimen produces a stable acceptance record |
| 058 | Component | Constraint and clue model | Fixed-capacity rules reject cycles and consume only explicit observations |
| 059 | Component | Fault-aware operator panel | Inputs, presentation, acknowledgement, stop, and storage compose deterministically |
| 060 | Project-bearing | Inert escape-room console | Replayable clues can request only lightweight, bounded actuator intent |
| 061 | Component | Revision-neutral inertial samples | Identified MPU6050 or QMI8658 adapters produce one validated sample value |
| 062 | Component | Orientation presentation | Supplied samples produce bounded pitch/roll and existing LED/tone intent |
| 063 | Project-bearing | Balance-table instrument | Tilt, sensitivity, freeze, health, and presentation replay without a later display driver |
| 064 | Component | Inertial record normalization | Device identity, range, calibration version, data-ready, and faults survive normalization |
| 065 | Component | Inertial source qualification | Explicit configuration qualifies one source without hidden voting or failover |
| 066 | Project-bearing | Interchangeable motion recorder | Revision-specific traces normalize and present through earlier endpoints |
| 067 | Component | Owned single-wire transactions | Reset, presence, slots, pull-up policy, timeout, rollback, and DS18B20 identity are explicit |
| 068 | Component | Qualified thermal probe sets | Fixed-capacity identities retain conversion, CRC, stale, and disappearance state |
| 069 | Project-bearing | Thermal gradient mapper | Safe tabletop probe observations produce stable displays and records |
| 070 | Component | Nonblocking multiplexed digits | Supplied-time digit frames expose polarity, blanking, overflow, and refresh loss |
| 071 | Component | MAX7219 matrix presentation | Owned SPI configuration and frames fail blank without hiding transport faults |
| 072 | Project-bearing | Dual-display timing desk | Multiplexed digits and matrix progress present one stopwatch snapshot |
| 073 | Component | Owned three-wire clock observations | DS1302 transactions retain direction, protection, oscillator, and calendar validity |
| 074 | Component | Pressure and external analog conversion | Identified BMP180 and PCF8591 observations retain raw and compensated evidence |
| 075 | Project-bearing | Pressure and analog station | Clocked pressure and controlled analog records reproduce trend decisions |
| 076 | Component | Identified color observations | One inventoried pulse or register mechanism produces explicit raw channels |
| 077 | Component | Calibrated color classification | Fixed dark/white calibration produces bounded labels and confidence |
| 078 | Project-bearing | Color sorting trainer | Paper swatches produce confirmed, replayable pointer intent and confusion records |
| 079 | Component | Bounded low-side load driver | Identified transistor intent, current limits, flyback, and all-off rollback are explicit |
| 080 | Component | Small indicator-module semantics | Identified low-energy indicators retain polarity, autonomy, and safe state |
| 081 | Project-bearing | Component qualification bench | One identified inert specimen passes review, bounded stimulus, and stable recording |

Lessons 025--030 do not clone transmitters, replay unknown radio protocols,
control pyrotechnic launchers, or energize ignition circuits. A real show
remains the responsibility of a certified commercial controller and its
documented safety interface.

## Prerequisite graph

The main spine is intentionally simple:

```text
001 -> 002 -> 003 -> 004 -> 005 -> 006
                         \          |
                          +---------+

006 -> 007 -> 008 -> 009 -> 010 -> 011 -> 012
       |      |      |                    |
       +------+------+

012 -> 013 -> 014 -> 015 -> 016 -> 017 -> 018
              |             |      |
              +-------------+------+

018 -> 019 -> 020 -> 021 -> 022 -> 023 -> 024
       |      |             |      |
       +------+-------------+------+

024 -> 025 -> 026 -> 027 -> 028 -> 029 -> 030
       |      |             |      |
       +------+-------------+------+

030 -> 031 -> 032 -> 033 -> ... -> 079 -> 080 -> 081
       |      |                               |      |
       +------+       every third is project +------+
```

Project prerequisites are strict: every project-bearing lesson requires all
earlier lessons. A learner may enter at a later component lesson after
passing the earlier lesson acceptance checks. Bus lessons also require the
resource and lifecycle contract from `001`; motor and relay lessons require
the external-power boundary introduced in `017`.

## Component coverage

| Capability | Introduced | First composition | Later evidence |
|---|---:|---:|---|
| Resource-safe lifecycle | 001 | 003 | Every lesson |
| Digital output and LED diagnostics | 001 | 003 | 006, 009, 012 |
| Digital input and buttons | 002--003 | 003 | 006, 012, 018 |
| PWM, RGB, and sound | 004--005 | 006 | 009 |
| Analog sensing and filtering | 007--008 | 009 | 015, 024 |
| Displays and serialized output | 010, 014 | 012, 015 | 018, 027 |
| Rich operator input | 016 | 018 | 024, 030 |
| Servo and external-power boundaries | 017 | 018 | 021 |
| Range and motor interfaces | 019--020 | 021 | 030, inert only |
| I2C, SPI, storage, RTC, and relay simulation | 022--023 | 024 | 027, 030 |
| Infrared and packet observation | 025--026 | 027 | Receive-only fixture evidence |
| Fault injection and cue scheduling | 028--029 | 030 | Capstone evidence |
| Joystick and encoder input | 031--032 | 033 | 048, 060, 063 |
| Contact, acoustic, optical, and environmental observations | 034--044 | 036--045 | 057, 060 |
| Stepper, identity, homing, and owned IR output | 047--053 | 048--054 | 060 |
| Descriptor, rule, and operator-panel models | 055--059 | 057--060 | 081 |
| Revision-neutral inertial observations | 061--065 | 063--066 | Recorded comparison |
| Single-wire thermal observations | 067--068 | 069 | Recorded mapping |
| Multiplexed and MAX7219 displays | 070--071 | 072 | Timing presentation |
| Three-wire clock, pressure, and external conversion | 073--074 | 075 | Acquisition records |
| Color observation and classification | 076--077 | 078 | Inert sorting intent |
| Low-side driver and indicator descriptors | 079--080 | 081 | Inert qualification |

## Circuit-native debugging thread

Every lesson keeps its primary circuit observable without requiring Serial.
Prefer the circuit's primary behavior when it proves the claim unambiguously.
Compose an existing indicator when internal state needs another channel, and
reserve a dedicated debug LED or test point when the primary behavior cannot
separate likely causes. Name its pin or channel, current and timer cost,
inactive state, and claim conflicts. Its pattern must identify startup, ready,
activity, and fault states without changing primary timing. Serial may add
detail, but remains optional. Host traces and the bench card verify the
diagnostic signal alongside the primary outputs.

Lessons 007--009 make the evidence chain explicit:

```text
shaft or light level
    -> voltage at the named analog test point
        -> raw ADC count
            -> calibrated and filtered value
                -> PWM duty and visible lamp state
```

Lesson 007 uses a potentiometer because it gives a controlled, repeatable
voltage before environmental variability is introduced. Lesson 008 preserves
raw and processed values side by side. Lesson 009 then applies the same method
to a photoresistor, where open, short, saturation, noise, and threshold
hysteresis become project evidence.

## Lesson package

Every component lesson publishes:

- a clean public header and mostly out-of-line implementation;
- a host fake, deterministic examples, and correctness tests;
- a Mega 2560 sketch with wiring table and exact schematic;
- a pencil-style orientation drawing that never substitutes for a schematic;
- flash and static-RAM measurements;
- an HTML reference page and a printable lesson PDF;
- source links, datasheets, electrical limits, and a hardware acceptance card;
- a debug LED, test point, or status-pattern table with resource and safe-state
  costs;
- explicit prerequisites, cleanup behavior, and next uses.

Every project additionally publishes its dependency map, state graph, timing
diagram, replay traces, fault matrix, bill of materials, staged build checks,
and a final claim-evidence-reasoning report.

### HTML and PDF complement policy

HTML is the maintained navigation and reference surface. It favors searchable
API contracts, short wiring tables, source links, test traces, errata, and
copyable commands. The PDF is the bench companion. It favors prediction
prompts, pencil diagrams, worksheets, measurement tables, troubleshooting
trees, and space for recorded evidence.

Neither format may be the sole source of a safety limit, prerequisite, pin
assignment, acceptance criterion, or API contract. Each lesson links directly
to its counterpart and to downloadable source. Differences are deliberate and
listed under “Use with” rather than concealed as incomplete duplication.

## Acceptance gates

A lesson is first-class only when all applicable gates pass:

1. **Interface:** names and formatting match project style; public headers are
   small; ownership, units, valid ranges, and failure states are explicit.
2. **Lifecycle:** construction is inert; initialization is transactional;
   shutdown is idempotent; destruction is `noexcept`; every claim is released.
3. **Determinism:** time, samples, and sequences are injected; a saved trace
   reproduces state, events, outputs, and errors exactly.
4. **Host verification:** strict-warning, no-exception, no-RTTI builds pass;
   tests cover success, boundary, conflict, rollback, and destruction paths.
5. **Hardware verification:** the Mega 2560 sketch compiles, stays within its
   size budget, names the tested board and supply, and passes its primary and
   circuit-native diagnostic traces. Serial is optional.
6. **Electrical safety:** current, voltage, power, isolation, inactive state,
   and external-power rules cite authoritative sources.
7. **Documentation:** HTML, PDF, links, diagrams, metadata, and downloadable
   sources build cleanly; their shared facts agree.
8. **Accessibility:** semantic HTML, keyboard use, contrast, alt text, zoom,
   grayscale printing, reading order, and tagged-PDF checks are recorded.
9. **Integration:** all earlier tests still pass and the next scheduled project
   demonstrates the component through its public interface.

A project also needs deterministic end-to-end traces, injected fault tests,
power-removal recovery, an emergency-stop path where motion is present, and a
complete learner evidence record. A failed gate keeps the lesson in draft.

## Capstone progression

The projects grow one engineering habit at a time:

```text
observable output
    -> measured human input
    -> deterministic game
    -> calibrated feedback
    -> operator instrument
    -> persistent experiment
    -> bounded actuator
    -> mobile system
    -> distributed observation
    -> safety-reviewed inert cue system
```

The final capstone is therefore not a leap into a new domain. It reuses the
same ownership, deterministic scheduling, operator input, diagnostic output,
logging, interlock, and fault-injection contracts already proven in smaller
projects. Its deliverable is an inert simulator plus an auditable engineering
record, not a launcher controller.

## Maintenance rules

- Keep lesson numbers stable after publication; record replacements instead of
  silently reusing a number.
- Schedule a project at every number divisible by three.
- Add a component only with a named future project that proves its composition.
- Update this file, the roadmap, site navigation, and deferred-work ledger in
  the same curriculum change.
- Do not promote legacy code or examples as current interface guidance.
- Preserve deterministic host operation even when hardware timing is involved.
- Link primary datasheets and standards; date-check external guidance during
  each release audit.
