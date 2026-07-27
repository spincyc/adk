# Deterministic Simon midpoint project

## Purpose

Simon is ADK's midpoint composition project. It combines four buttons, four
matching LEDs, explicit time, a finite state machine, and a reproducible cue
source. The first version deliberately excludes sound, RGB effects, callbacks,
dynamic allocation, and hardware randomness. Those are later variations, not
requirements for understanding or testing the game.

The project has two equal uses:

- a host simulation whose complete behavior is reproducible; and
- a Mega 2560 circuit using the same game rules at human-scale timings.

It demonstrates the central ADK method: establish small hardware-owning
components, keep policy in a hardware-neutral behavior object, drive time
explicitly, and make observable state sufficient to test every decision.

## Agreed design

- The physical build has four active-low, internal-pull-up buttons and four
  matching, current-limited LEDs.
- The game uses neutral cue identities rather than colors:
  `Cue::One` through `Cue::Four`.
- A hardware mapping associates each cue with its button, LED, physical
  position, and later an optional tone.
- `update(TimePoint now)` is the only operation that advances game time.
- Button events are non-consuming snapshots. Every observer sees the same
  `pressed()` and `released()` values until the next component update.
- A press must be followed by release before another cue is accepted. A held
  button cannot create repeated input.
- Two or more active buttons in one update are one explicit invalid input. Scan
  order never chooses a winner.
- Playback does not accept player input.
- Sequence storage is fixed-capacity and uses no heap allocation.
- Configuration is immutable after construction and is validated before play.
- Introductory, standard, and advanced configurations are named values, not
  subclasses.
- A documented, versioned pseudorandom algorithm provides production
  sequences. An algorithm version and seed reproduce the same sequence across
  boards and future ADK releases.
- Tests can instead inject a fixed cue sequence.
- A replay record contains the algorithm version, seed, configuration identity
  or values, and the complete timestamped input trace.
- ADK does not use exceptions or RTTI. All component cleanup remains safe when
  a surrounding exception-enabled application unwinds.

## Current prerequisites

These interfaces must be implemented, host-tested, demonstrated on the Mega
2560, and documented before Simon itself is implemented:

1. lifecycle and exclusive resource claims;
2. `DigitalInput`;
3. `Button`, including raw diagnostics, debounce, and stable event snapshots;
4. `DigitalOutput`;
5. `MonoLed`;
6. monotonic `TimePoint` and `Duration` arithmetic with defined wraparound
   behavior; and
7. a nonblocking update discipline.

The current imported LED compatibility API is not the target Simon dependency.
Simon should wait for the new endpoint and component hierarchy rather than
make the compatibility registration mechanism permanent.

Sound lessons may precede Simon in the overall curriculum, but sound is not a
prerequisite for the first Simon build.

## Future interfaces

The names below describe intended responsibilities, not a frozen public API.
They should be introduced one interface at a time after their prerequisite
lessons establish the invariants.

### Cue and cue sets

`Cue` is a small, fixed-width enumeration with exactly four valid values.
Conflicting input is represented as a cue set or explicit input observation,
not coerced into a single `Cue`.

### Simon configuration

An immutable `SimonConfig` carries:

- cue-on duration;
- inter-cue gap;
- player input timeout;
- optional press indication duration or policy;
- starting sequence length;
- sequence growth per successful round; and
- maximum sequence length, bounded by compile-time storage capacity.

Validation rejects zero or contradictory timing values, an impossible sequence
range, and values that exceed fixed storage. Validation returns a compact
result; it does not throw.

Named introductory, standard, and advanced presets provide known-valid
configurations. Tests exercise both preset values and every validation error.

### Sequence source

A narrow sequence source produces neutral cues. Two implementations are
planned:

- a fixed source for examples and tests; and
- a versioned seeded generator for hardware play.

The generator specification must define its integer widths, state transition,
cue extraction, seed handling, and golden vectors. It must not depend on
`std::rand()`, library distribution behavior, native word size, wall-clock
time, analog noise, or boot state. A future generator requires a new explicit
algorithm version; an existing version never silently changes.

### Game engine

The game engine owns game policy and fixed-capacity sequence state. It does not
own Arduino pins, read a global clock, call `delay()`, register callbacks, or
directly drive LEDs.

Each update receives:

- the current `TimePoint`; and
- one complete, already-debounced input observation for the update cycle.

The engine exposes an observable snapshot containing at least:

- phase;
- currently displayed cue, if any;
- round and sequence lengths;
- expected player index;
- last outcome or fault;
- whether input is presently accepted; and
- the next relevant deadline where meaningful.

The adapter derives LED output from this snapshot. Tests use the same snapshot
without hardware. No essential result exists only as a callback side effect.

### Mega hardware adapter

The adapter owns or composes four `Button` objects and four `MonoLed` objects,
updates inputs once per loop, forms one input observation, updates the game
once, then reconciles all outputs from the resulting snapshot. A table maps
neutral cues to physical components. Wiring order and color never enter the
game engine.

## State model

The exact type names may evolve, but the behavior requires these phases:

1. **Idle** — outputs inactive; waits for an explicit start action.
2. **PlaybackOn** — one sequence cue is visible until its deadline.
3. **PlaybackGap** — all cues are inactive between playback cues.
4. **AwaitPress** — accepts exactly one unambiguous new press before timeout.
5. **AwaitRelease** — the accepted button may remain held; no new cue is
   accepted until all buttons are released.
6. **RoundSuccess** — records success and grows the sequence if capacity allows.
7. **GameSuccess** — maximum configured sequence length was completed.
8. **GameFailure** — mismatch, simultaneous input, or timeout is observable.

Start and restart semantics must be explicit events, not accidental presses
during playback or a failure display. A reset establishes the same state for
the same configuration, sequence source state, and timestamp.

At a timestamp that exactly equals a deadline, one documented boundary rule
must apply everywhere. The implementation lesson should choose and illustrate
that rule before code is written. Time comparisons must use the ADK wrap-safe
clock operation rather than direct unsigned ordering.

## Deterministic update order

The hardware loop and host harness use the same ordering:

1. obtain one timestamp;
2. update all four buttons with that timestamp;
3. snapshot the complete four-button state and edge events;
4. call the game update exactly once;
5. read the game snapshot; and
6. reconcile all four LEDs.

No component obtains a second hidden timestamp during the cycle. A trace row
therefore completely specifies one game step. Repeating the same initial state
and trace yields the same snapshots and outputs.

## Replay trace

A terse text or structured fixture format should represent:

| Field | Meaning |
|---|---|
| Algorithm version | Exact cue-generator contract |
| Seed | Initial generator value |
| Configuration | All timing and sequence bounds |
| Timestamp | Time supplied to one update |
| Active cue set | Complete debounced input state |
| Pressed cue set | New press events for this cycle |
| Released cue set | Release events for this cycle |
| Expected snapshot | Optional executable assertion |

Raw electrical samples may be recorded in a separate diagnostic trace, but
game replay begins at the debounced input boundary. This separation lets a
button lesson diagnose contact bounce without making the game engine depend on
electrical details.

## Test matrix

Table-driven traces are both correctness tests and examples of proper API use.
Every row supplies explicit time and complete input; no test sleeps.

| Area | Required cases and assertions |
|---|---|
| Configuration | All presets validate; each zero, contradictory, excessive, or out-of-capacity value returns the intended error |
| Generator | Golden vectors for every supported version; same seed repeats; several seeds diverge; every result is a valid cue; replay metadata round-trips |
| Reset | Known initial snapshot; repeat reset is deterministic; no stale deadline, event, or sequence position survives |
| Playback | Exact first-cue onset; on-to-gap and gap-to-next-cue boundaries; complete sequence order; no accepted press during playback |
| Input | Correct cue advances exactly once; held press does not repeat; release enables the next press; press and release snapshots are non-consuming |
| Conflict | Every pair, triple, and four-button simultaneous set produces `InvalidInput`; mapping or scan order does not affect the result |
| Mismatch | Each wrong cue at each player position fails deterministically and records expected versus observed input |
| Timeout | Just before, exactly at, and just after the deadline follow the documented boundary rule |
| Round growth | Successful rounds grow by the configured amount; playback reuses the existing prefix; source consumption is exact |
| Capacity | Maximum length is playable; success at the limit enters `GameSuccess`; no out-of-bounds write or extra source read occurs |
| Restart | Restart from idle, success, failure, playback, and held-input conditions follows the documented policy |
| Time | Large forward steps, repeated timestamp, and counter wraparound preserve correct deadlines; backward time is handled by the clock contract |
| Mapping | Permuting colors and Mega pins changes only adapter output, never the neutral cue trace |
| Lifecycle | Partial hardware initialization rolls back; shutdown is idempotent; outputs enter their safe inactive states; external unwinding invokes cleanup |
| Regression | A saved seed/configuration/trace reproduces every expected state snapshot and LED frame |

Host tests compile with warnings as errors, exceptions disabled, RTTI disabled,
and optimization enabled. Sanitizer-enabled tests should run where the host
toolchain supports them. Mega compile checks and hardware acceptance do not
replace host tests.

## Hardware acceptance

The reference build targets the Arduino Mega 2560. Exact pins should be chosen
by the board-profile and circuit lessons, then recorded in a mapping table.
Acceptance requires:

- all four raw inputs report wiring faults intelligibly;
- debounced press and release indicators match physical actions;
- every LED can be identified independently;
- simultaneous presses fail rather than select the lowest scanned pin;
- a known algorithm version and seed produce the published cue sequence;
- a captured host trace and the Mega exhibit the same state progression;
- shutdown leaves every LED inactive and owned pins in their documented safe
  states; and
- flash and static RAM size are recorded as curriculum evidence.

## Lesson and PDF plan

Simon should be taught as a short sequence of rich lessons, followed by one
integrated project PDF. Each lesson contains learning goals, prerequisite
checks, prediction prompts, an exact schematic, a pencil-drawn orientation
diagram, incremental code, deterministic tests, failure diagnosis, extension
work, and a claim-evidence-reasoning acceptance record.

### Lesson 1: neutral cues and physical mapping

Students wire and identify four buttons and four LEDs, then create a mapping
between neutral cues and physical positions. The lesson contrasts semantic
components with pins and demonstrates why color does not belong in game logic.

Diagrams:

- pencil-drawn Mega, breadboard, four-button, and four-LED physical layout;
- exact four-input/four-output schematic; and
- cue-to-position mapping table.

### Lesson 2: explicit time and finite-state behavior

Students model idle, playback, gaps, input, and terminal outcomes without
hardware delays. They predict transitions from trace rows before running host
tests.

Diagrams:

- pencil-drawn timeline annotated like an engineering notebook;
- exact state graph; and
- cue-on, gap, press, release, and timeout timing diagrams.

### Lesson 3: reproducible sequence generation

Students implement or exercise the documented versioned generator, verify
golden vectors, and explain why platform randomness is unsuitable for an
executable lesson. Fixed and seeded sources are compared through the same
interface.

Diagrams:

- pencil-drawn sequence notebook showing seed-to-cue progression; and
- exact generator state-transition and replay-metadata diagram.

### Lesson 4: conflict, fault, and boundary testing

Students build table-driven tests for mismatch, timeout, simultaneous presses,
maximum length, and timestamp wraparound. Each fault is predicted, injected,
observed, and explained.

Diagrams:

- pencil-drawn test bench with deliberately conflicting button presses;
- exact deadline boundary diagram; and
- trace-to-state-to-output data-flow diagram.

### Lesson 5: Mega integration and acceptance

Students assemble the complete circuit, replay a published game, compare host
and Mega observations, measure binary size, and produce an acceptance record.
The game remains operable without a serial monitor.

Diagrams:

- pencil-drawn completed Simon control panel;
- exact final schematic and pin table; and
- update-order sequence diagram.

### Integrated project PDF

The final PDF gathers the stable interface contracts, validated configuration
presets, state graph, generator specification and golden vectors, trace format,
complete test matrix, exact Mega schematic, pencil orientation plates,
troubleshooting decision tree, size report, and extension rubric. It must
remain useful when ADK is consumed as a library and when the repository is
explored locally.

## Later extensions

Extensions compose around the stable neutral engine:

- four tones using the existing cue mapping;
- one RGB LED or illuminated arcade buttons;
- color-vision-friendly labels and alternative physical layouts;
- adjustable, validated speed presets;
- serial trace export and replay;
- a display-based score or diagnostics panel;
- a deterministic attract mode;
- persistent high-score storage with explicit integrity rules; and
- property-based generation of legal and adversarial traces.

These extensions must not change the meaning of an existing algorithm version,
introduce hidden time, or couple game policy to particular pins, colors, or
Arduino APIs.

## Completion criteria

Simon is complete only when:

- the prerequisite component hierarchy and lessons are complete;
- all host tests and Mega compile checks pass;
- the hardware acceptance record is reproducible;
- the documented seed and version reproduce published golden sequences;
- replay fixtures demonstrate success, mismatch, timeout, conflict, capacity,
  restart, and wraparound behavior;
- the lesson PDFs build from source and their schematics have been visually
  checked; and
- the public documentation clearly distinguishes stable interfaces from future
  proposals.
