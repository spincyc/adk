# Project 003: deterministic reaction timer

## Purpose

This is the first recurring project checkpoint. It follows the first three
component lessons and composes `DigitalOutput`, `DigitalInput`, `Button`, and
`MonoLed` into one circuit. The output path comes first so every later input
step has a visible diagnostic.

The project has two equivalent forms:

- a host simulation driven by timestamped samples; and
- a Mega 2560 circuit driven by the same state machine.

Neither form sleeps, calls `delay()`, reads a hidden clock, allocates dynamic
memory, uses exceptions, or depends on random timing. An input trace and
configuration completely determine every state and LED frame.

## Learning outcomes

After the project, a learner can:

1. distinguish electrical sampling, debouncing, events, policy, and output;
2. compose hardware-owning RAII components without sharing pins;
3. drive a nonblocking state machine with explicit monotonic time;
4. use a digital output as a standalone circuit diagnostic;
5. predict boundary behavior before running a test;
6. replay a host trace on the Mega; and
7. collect evidence that shutdown leaves the circuit safe.

## Prerequisites

The preceding lessons establish:

| Lesson | Interface | Evidence carried into this project |
|---|---|---|
| 001 | `DigitalOutput` | claim, safe state, write, shutdown, visible blink diagnostic |
| 002 | `DigitalInput` | pull-up input, raw level, explicit sample time, wiring diagnostic |
| 001 | `MonoLed` | semantic inactive state over owned output |
| 003 | `Button` | stable state, press/release snapshots, active-low semantics |

Lesson 003 introduces `Button` immediately before composing the project. The
legacy LED API is not a project dependency.

## Circuit

Reference wiring uses an external resistor-limited LED on D8 for the reaction
cue, the Mega 2560 built-in D13 LED for acquisition evidence only, and one
normally open pushbutton on D22:

| Role | Mega resource | Connection | Inactive state |
|---|---|---|---|
| Cue LED | D8 | pin through measured 330 Ω resistor and LED to ground | off |
| Acquisition LED | `LED_BUILTIN` / D13 | board LED through board resistor | off |
| Reaction button | D22 | pin to button, button to ground | internal pull-up, released |

The D13 pulse reports only successful acquisition. It does not report timer
state or prove shutdown.

```text
             Mega 2560

               D8 o----[330 ohm]---->|----GND
              D13 o----[board resistor]---->|----GND

              D22 o----------+----- internal pull-up
                            |
                         [ button ]
                            |
                           GND
```

Disconnect power before changing wiring. Never drive an LED without a
current-limiting resistor.

## Architecture

Hardware adapters own resources. Behavior owns rules. Observation crosses
between them as plain values.

```text
raw pin level
      |
      v
+--------------+    sample     +----------+    observation
| DigitalInput |-------------->|  Button  |----------------+
+--------------+               +----------+                |
                                                           v
explicit TimePoint -------------------------------->+---------------+
                                                    | ReactionTimer |
                                                    +---------------+
                                                           |
                                                      snapshot
                                                           |
                                                           v
                                              +---------+  owns  +---------------+
                                              | MonoLed |------->| DigitalOutput |
                                              +---------+        +---------------+
```

The timer does not know Arduino pins, electrical polarity, `millis()`, or
`digitalWrite()`. The adapter obtains one timestamp per cycle, updates input
once, advances policy once, then reconciles output once.

### Ownership

- `DigitalInput` exclusively claims the input pin.
- `Button` owns its `DigitalInput`; it does not borrow a mutable pin.
- `DigitalOutput` exclusively claims the output pin.
- `MonoLed` owns its `DigitalOutput` and defines semantic on/off behavior.
- The project adapter owns one `Button`, one `MonoLed`, and one
  `ReactionTimer`.
- Destruction occurs in reverse ownership order and is safe during external
  exception unwinding.

Initialization is transactional. If the second resource cannot be claimed,
the first claim is released and the output is returned to its documented safe
state. `shutdown()` is idempotent and non-throwing.

## Configuration

Use one immutable, validated configuration:

| Field | Introductory value | Constraint |
|---|---:|---|
| Ready duration | 1,000 ms | greater than zero |
| Wait duration | 2,000 ms | greater than zero |
| Response timeout | 1,500 ms | greater than zero |
| Result duration | 1,000 ms | greater than zero |
| Button debounce | 20 ms | less than response timeout |

The wait duration is fixed in this first project. Deterministic variation may
be added later through an injected, versioned sequence source. Do not seed from
wall time, boot state, or an unconnected analog pin.

All duration addition and deadline comparisons use the library's wrap-safe
time operations. A deadline is due when `now` is equal to or after it.

## State model

| State | LED | Accepted input | Exit |
|---|---|---|---|
| `Idle` | off | one new press starts | press |
| `AwaitRelease` | off | release only | all buttons released |
| `Ready` | slow diagnostic blink | none | ready deadline |
| `Wait` | off | premature press fails | wait deadline or press |
| `Cue` | on | first press succeeds | press or timeout |
| `Success` | two short pulses | none | result deadline |
| `Failure` | one long pulse | none | result deadline |

After either result, return to `Idle`. A held button cannot restart a trial.
The adapter must observe release before accepting another start.

The result pulse pattern is derived from timer snapshot state and explicit
time. It must not be produced by a blocking loop.

```text
                  press
        +-----------------------+
        |                       v
     +------+              +--------------+
 +-->| Idle |              | AwaitRelease |
 |   +------+              +--------------+
 |                                |
 |                             release
 |                                v
 |                            +-------+
 |                            | Ready |
 |                            +-------+
 |                                |
 |                            deadline
 |                                v
 |   premature press          +------+
 |   +------------------------| Wait |
 |   |                        +------+
 |   v                            |
 | +---------+                 deadline
 | | Failure |                    v
 | +---------+                  +-----+
 |   |                timeout   | Cue |---press--->+---------+
 |   |                  +-------+-----+            | Success |
 |   |                  v                          +---------+
 |   |             +---------+                         |
 |   +------------>| Failure |                         |
 |                 +---------+                         |
 |                      |                              |
 +----------------------+----------result deadline-----+
```

## Timing example

For a trial started at 100 ms:

```text
time (ms)     100       180       1180              3180    3542
button        press-----release-----------------------------press
state         Idle  AwaitRelease  Ready-------------Wait----Cue
LED           off       off       blink-------------off-----on/off
deadline                           1180              3180    4680
reaction                                                   362 ms
```

The reaction interval begins at the exact transition into `Cue`, not when the
adapter happens to illuminate the LED later. With the prescribed update order,
both occur in the same cycle.

### Boundary rules

- A press sampled during `Wait`, even exactly at its ordinary wait deadline,
  is premature because input validation precedes the time transition.
- A press sampled exactly at the response deadline is a timeout.
- Repeated timestamps do not advance a deadline or duplicate an event.
- A large forward time step performs at most one externally observable state
  transition per update. The next update may advance again at the same time.
- Backward timestamps follow the common clock contract and must yield a
  diagnosable status rather than silently corrupting a result.

These rules make one trace row correspond to one transition and prevent loop
frequency from changing the outcome.

## Deterministic update cycle

```text
loop iteration
    |
    +-- now = clock.now()                 exactly once
    +-- button.update (now)               exactly once
    +-- input = button.snapshot()
    +-- timer.update (now, input)          exactly once
    +-- view = timer.snapshot()
    +-- led.reconcile (view.ledFrame)      exactly once
```

Button events are non-consuming snapshots. Logging or another observer may
inspect the same press and release values without stealing them from the game.
No callback performs an essential transition.

The timer snapshot exposes at least:

- current state;
- cue-visible flag;
- LED diagnostic frame;
- most recent outcome;
- cue timestamp, when present;
- reaction duration, after success;
- next deadline, when meaningful; and
- last validation or clock fault.

## Debugging progression

Build and accept each layer before adding the next:

1. Claim the output and blink it from explicit timestamps.
2. Verify semantic off and safe shutdown.
3. Claim the input and mirror its raw electrical level on the LED.
4. Display stable button state instead of raw state.
5. Flash once for each `pressed()` snapshot and twice for `released()`.
6. Run the timer with the diagnostic state pattern.
7. Compare a serial trace only after the LED-only circuit works.

This progression leaves a visible diagnostic available when input behavior is
wrong. Serial output is optional evidence, not a functional dependency.

## Host test plan

Tests use a fake pin backend and explicit timestamp rows. They never sleep.

| Area | Required cases |
|---|---|
| Configuration | known-valid preset; every zero value; debounce equal to or greater than the response timeout; overflow-safe deadlines |
| Lifecycle | successful claims; input/output conflicts; partial initialization rollback; repeated shutdown; destruction after partial initialization |
| Output | initial inactive state; requested frames; no redundant writes; high-impedance generic shutdown; semantic LED inactive shutdown |
| Input | pull-up polarity; raw observations; bounce shorter than debounce; stability exactly at debounce deadline |
| Events | one press snapshot; one release snapshot; held button does not repeat; two observers see identical events |
| Start | press from idle; held start waits for release; release begins ready interval once |
| Wait | no input until deadline; premature press one tick before and exactly at deadline; cue after deadline |
| Response | success just after cue; reaction value; success one tick before timeout; timeout exactly at deadline |
| Result | exact pulse boundaries; success and failure patterns; automatic return to idle; no restart while held |
| Time | repeated timestamp; large step; counter wrap; rejected backward timestamp |
| Replay | saved trace produces identical snapshots, pin modes, output writes, and reaction result |
| Unwinding | a surrounding throwing fixture destroys the adapter and observes released claims and safe output |

Tests compile with warnings as errors, exceptions disabled, RTTI disabled, and
optimization enabled. The one external-unwinding test may be a separate
exception-enabled translation unit; library code remains exception-free.

### Representative trace

```text
# time_ms raw_low expected_state expected_led expected_outcome
0         false   Idle           Off          None
100       true    Idle           Off          None
120       true    AwaitRelease   Off          None
180       false   AwaitRelease   Off          None
200       false   Ready          PulseSlow    None
1200      false   Wait           Off          None
3200      false   Cue            On           None
3562      true    Cue            On           None
3582      true    Success        PulseDouble  Success(382ms)
4582      false   Idle           Off          Success(382ms)
```

Raw changes become button events only after the configured stable interval.
The executable fixture should include every intermediate row needed to prove
that timing.

## Mega 2560 acceptance

Record the board, toolchain, library commit, selected pins, and measured binary
size. Then verify:

- power-on leaves the cue LED inactive;
- the output-only diagnostic works before the button is installed;
- raw and debounced input diagnostics distinguish wiring from bounce;
- holding the button creates one press and no repeats;
- pressing during `Wait` always produces the failure pattern;
- pressing after the cue produces the success pattern and a plausible result;
- pressing at documented boundaries matches the host trace;
- ten replays of the published trace produce the same state sequence;
- unplugging or miswiring the button is diagnosable and does not energize an
  output indefinitely;
- explicit shutdown and object destruction leave the LED inactive and release
  both claims; and
- flash and static RAM remain within the project's recorded budgets.

Human reaction time cannot validate exact boundaries. Use a second output pin
looped to the input through an appropriate test connection, or a scripted test
adapter, for timestamp acceptance on hardware.

## Rich lesson plan

Suggested duration is two 60-minute sessions plus optional extension work.
The HTML guide carries API links, executable traces, and troubleshooting. The
PDF carries the bench procedure, writable predictions, pencil-style physical
orientation drawing, state graph, timing worksheet, and acceptance record.

### Session A: observe and compose

1. **Predict (10 min):** mark safe output and released input levels on the
   circuit drawing.
2. **Output diagnostic (10 min):** wire and accept the cue LED alone.
3. **Raw input (10 min):** mirror the electrical sample and explain active-low.
4. **Debounce (15 min):** sketch a bouncing waveform, predict stable events,
   and run the matching trace.
5. **Composition (15 min):** identify ownership boundaries and prove there is
   exactly one owner per pin.

Evidence: completed wiring annotation, output acceptance, raw/stable trace,
and one explanation of why an event snapshot is non-consuming.

### Session B: model and verify

1. **Model (10 min):** walk a paper token through the state graph.
2. **Boundaries (10 min):** predict just-before, exact, and just-after cases.
3. **Host replay (15 min):** run the published trace and one premature-press
   trace.
4. **Hardware run (15 min):** perform normal, early, timeout, and held-button
   trials.
5. **Reason (10 min):** connect each claim to host or hardware evidence.

Evidence: annotated state trace, reaction calculation, fault diagnosis, size
record, and signed hardware checklist.

### Instructor prompts

- Why is the LED usable before the button abstraction exists?
- Which layer knows that electrical low means pressed?
- What changes if `timer.update()` reads its own clock?
- Why does input win over the wait deadline at an equal timestamp?
- Which object must restore pin safety during external stack unwinding?
- Can a serial logger observe an event without changing the game?

### Common faults

| Observation | Likely cause | Isolation step |
|---|---|---|
| LED never lights | polarity, claim, or pin mapping | run output-only diagnostic |
| LED always lights | unsafe initialization or inverted semantics | request semantic off before input setup |
| Raw input changes, stable state does not | debounce interval or missing updates | print timestamped raw/stable pairs |
| One press repeats | level treated as event | inspect consecutive event snapshots |
| Immediate failure after start | release gate omitted | trace `AwaitRelease` |
| Host and Mega disagree | hidden clock or different update order | log the single cycle timestamp and snapshot |
| Works until clock rollover | direct unsigned comparison | run the published wrap trace |

## Assessment

A complete submission includes:

- one passing host trace for success, premature press, and timeout;
- boundary and lifecycle test results;
- a labeled circuit record;
- Mega acceptance results and size measurements;
- one claim-evidence-reasoning paragraph about deterministic behavior; and
- one proposed extension that preserves the hardware-neutral engine.

Rubric: 40% deterministic correctness, 25% safe ownership and lifecycle, 20%
evidence and diagnosis, and 15% clear circuit documentation.

## Extensions

After the baseline passes:

- show reaction time on a serial console or display adapter;
- store a fixed-capacity history and report minimum/median values;
- inject a versioned wait-sequence source;
- add a second button and reject simultaneous reactions;
- replace the LED mapping without changing timer policy;
- add a buzzer cue through a later semantic output component; or
- replay recorded input traces through a desktop visualization.

Extensions must keep explicit time, bounded storage, safe shutdown, and the
same documented boundary rules.

## Completion criteria

Project 003 is host verified. It becomes hardware supported when:

- its four prerequisite interfaces are first-class, not legacy wrappers;
- all host tests and the Mega compile pass;
- a published replay fixture matches the documented snapshots;
- hardware acceptance records output-first diagnosis and shutdown safety;
- HTML and PDF materials link to the relevant API references and sources;
- the PDF diagrams have been visually checked; and
- future-agent documentation records any changed name, invariant, or boundary
  rule before later projects depend on it.
