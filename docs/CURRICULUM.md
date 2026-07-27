# First-class curriculum

This is the planned learning path for ADK. It records the canonical numbers
before the lessons are promoted; a row is not evidence that its interface or
lesson is supported yet. Material moved to `legacy/` is historical context,
not a prerequisite and not a statement of the current API. Published lessons
use the first-class RAII interfaces.

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
| 008 | Component | Photoresistor, thermistor, and sampled filtering | Filter state advances only from supplied samples and time |
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
| 020 | Component | Motor driver, encoder, and emergency-stop policy | Direction, enable, dead time, and shutdown are explicit and fail safe |
| 021 | Project-bearing | Bench rover | Sensing, motion, scripted routes, and emergency stop pass first with wheels raised |
| 022 | Component | RTC, SD records, `I2cBus`, and `SpiBus` | Leased transactions restore bus state and records survive restart |
| 023 | Component | Relay-module simulation and constrained outputs | Inert fan, pump, and heater states enforce mutual exclusion |
| 024 | Project-bearing | Greenhouse controller | Schedules, sensor faults, simulated loads, and logs reproduce every decision |
| 025 | Component | Infrared receive and decoded command models | Captures preserve timing evidence while protocol policy stays separate |
| 026 | Component | Radio receive-only observation and spectrum records | Lawful, passive observations become timestamped data without transmit support |
| 027 | Project-bearing | Multi-room telemetry console | Wired sensors, receive-only links, displays, alarms, and storage share one scheduler |
| 028 | Component | Continuity, fault, and redundant-state simulation | Open, short, stale, and contradictory states are injectable and observable |
| 029 | Component | Cue scheduling, audit logs, and operator confirmation | A clocked cue engine is deterministic, inert, and fully replayable |
| 030 | Project-bearing capstone | Inert show-cue simulator | Redundant arming, inert loads, faults, shutdown, and evidence logs pass review |

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
| Range, motor, and encoder interfaces | 019--020 | 021 | 030, inert only |
| I2C, SPI, storage, RTC, and relay simulation | 022--023 | 024 | 027, 030 |
| Infrared and passive radio observation | 025--026 | 027 | Observation only |
| Fault injection and cue scheduling | 028--029 | 030 | Capstone evidence |

## Lesson package

Every component lesson publishes:

- a clean public header and mostly out-of-line implementation;
- a host fake, deterministic examples, and correctness tests;
- a Mega 2560 sketch with wiring table and exact schematic;
- a pencil-style orientation drawing that never substitutes for a schematic;
- flash and static-RAM measurements;
- an HTML reference page and a printable lesson PDF;
- source links, datasheets, electrical limits, and a hardware acceptance card;
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
   size budget, names the tested board and supply, and passes its bench card.
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
