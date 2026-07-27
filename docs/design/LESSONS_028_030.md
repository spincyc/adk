# Lessons 028--030 design: inert show-cue simulator

Status: executable design; host implementation and physical acceptance deferred.

This capstone teaches contradiction detection, deterministic scheduling,
operator confirmation, audit records, and replay. It does not control a show.
Every physical output is a resistor-limited low-voltage LED. No interface in
this slice represents ignition, continuity of an initiator, launcher channels,
radio transmission, captured-protocol replay, or an adapter to such equipment.

## Safety boundary

The supported circuit is E0/E1:

- one USB-powered Mega 2560;
- eight LEDs, each with its own 330 ohm or larger resistor;
- four momentary buttons wired with internal pull-ups;
- optional character LCD from lesson 014;
- named multimeter or logic-analyzer test points at LED outputs.

Unsupported connections include relays, transistors driving external loads,
isolated firing modules, initiators, commercial launcher connectors, antennas,
RF emitters, and unknown equipment. The library exports no generic callback
for cue actuation. A cue changes a value in a snapshot; the canonical example
alone translates that value to visible LEDs.

```text
schedule -> decision snapshot -> inert LED view -> audit record
```

There is intentionally no `fire()`, `ignite()`, `launch()`, raw waveform,
protocol address, transmitter, or external-output sink.

## Dependency narrative

```text
028 synthetic observations -> assessed channel state
029 inert cue plan         -> confirmation + deterministic schedule + audit
030 both                   -> reviewed simulator + exact replay
```

The example loop mirrors the argument:

```cpp
simulator.observe  (now, readOperatorInput (), readSyntheticChannels ());
simulator.decide   (now);
simulator.present  (now);

showInertCues      (simulator.snapshot ());
showSimulatorState (simulator.snapshot ());
```

Only the example owns output components. The core receives values and returns
values.

## Lesson 028: synthetic channel assessment

### Learning outcome

Classify open, closed, stale, short-simulated, and contradictory observations
without pretending that a single reading proves an energetic circuit safe.

### Public model

The word `continuity` is avoided in public types because the lesson observes
only a learner-operated simulation network.

```cpp
using InertChannelId = uint8_t;

enum struct InertObservation : uint8_t
{
    Open,
    Closed,
    ShortSimulated,
    Unavailable
};

struct InertChannelObservation
{
    InertChannelId  channel;
    InertObservation primary;
    InertObservation redundant;
    TimePoint        observedAt;
};

enum struct InertChannelState : uint8_t
{
    Open,
    Closed,
    ShortSimulated,
    Stale,
    Contradictory,
    Unavailable
};

struct InertChannelAssessment
{
    InertChannelId  channel;
    InertChannelState state;
    TimePoint       observedAt;
};

struct InertChannelAssessor
{
    explicit InertChannelAssessor (Duration staleAfter) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (TimePoint now,
                       const InertChannelObservation* observations,
                       uint8_t observationCount) noexcept;

    Result<InertChannelAssessment> assessment
        (InertChannelId channel, TimePoint now) const noexcept;
};
```

The first implementation has eight fixed channels numbered 0--7 and no heap.
The constructor is inert. Configuration rejects a zero stale interval.
`update()` rejects null storage with nonzero count, duplicate channel IDs,
out-of-range IDs, and future observations more than half the 32-bit timestamp
space away. Failed updates leave the prior complete set unchanged.

Assessment rules are explicit:

| Primary | Redundant | Result |
|---|---|---|
| same usable value | same | that state |
| different usable values | different | `Contradictory` |
| one unavailable | any | `Unavailable` |
| either observation too old | any | `Stale` |

`ShortSimulated` means only that the learner selected the short position in the
simulation network. It is not an electrical diagnosis and never enables an
output.

### Deterministic input source

`RecordedInertObservationSource` walks a caller-owned, fixed array of timestamped
observation sets. It has no global clock and never mutates source storage.
Repeated timestamps expose the same stable set. Advancing past multiple entries
selects the latest due complete set rather than emitting a burst.

### Circuit-native evidence

Two button banks select primary and redundant simulated observations. Eight
LEDs display the assessments one at a time:

- off: `Open`;
- steady green: `Closed`;
- steady amber: `ShortSimulated`;
- slow amber pulse: `Stale`;
- alternating red/amber: `Contradictory`;
- two red pulses: `Unavailable`.

TP28 on the selected input and TP29 on its indicator output let the learner
separate observation from interpretation. The acquisition check is the
startup sweep. The safe-state check separately confirms every indicator pin is
high impedance after shutdown.

Host cases cover every state pair, duplicates, count/capacity boundaries,
transactional invalid input, exact stale boundary, rollover, repeated update,
recovery, replay, repeated shutdown, and destruction.

## Lesson 029: inert cue schedule and audit

### Learning outcome

Advance a plan from supplied time, require explicit operator confirmation,
produce a stable decision snapshot, and encode every decision as a replayable
record.

### Cue plan

```cpp
using InertCueId = uint8_t;

struct InertCue
{
    InertCueId id;
    Duration   offset;
    Duration   visibleFor;
};

struct InertCuePlan
{
    static const uint8_t capacity = 32;

    InertCue cues[capacity];
    uint8_t count;
};

enum struct CueDecision : uint8_t
{
    Waiting,
    ConfirmationRequired,
    Active,
    Complete,
    Skipped,
    Held,
    Cancelled
};
```

IDs are opaque display labels from 0--31. They have no pin, frequency,
protocol, address, or external-channel meaning. A valid plan has at least one
cue; strictly increasing offsets; nonzero visible durations; no overlapping
active intervals; and an end within the unambiguous 32-bit duration window.

### Operator input

```cpp
struct CueOperatorInput
{
    bool reviewHeld;
    bool runPressed;
    bool confirmPressed;
    bool skipPressed;
    bool cancelPressed;
};
```

`runPressed`, `confirmPressed`, `skipPressed`, and `cancelPressed` are debounced
edge events. A chord or repeated held button is invalid. `reviewHeld` is a
level gate: releasing it immediately enters `Held` and clears pending
confirmation. The simulator cannot start from one button action:

1. hold review;
2. press run;
3. inspect the named next cue;
4. press confirm.

This is instructional redundancy, not a functional-safety claim.

### Scheduler

```cpp
enum struct CueSchedulerPhase : uint8_t
{
    Idle,
    Review,
    Waiting,
    Confirmation,
    Active,
    Held,
    Complete,
    Cancelled,
    Fault
};

struct CueSchedulerSnapshot
{
    CueSchedulerPhase phase;
    CueDecision       decision;
    InertCueId        cue;
    uint8_t           cueIndex;
    Duration          planElapsed;
    Duration          cueElapsed;
    Status            status;
};

struct InertCueScheduler
{
    InertCueScheduler (const InertCuePlan& plan,
                       Duration confirmationWindow) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (TimePoint now,
                       const CueOperatorInput& input) noexcept;

    CueSchedulerSnapshot snapshot () const noexcept;
};
```

The scheduler is a pure deterministic state machine. Time begins only after
confirmation. Due cues request confirmation rather than becoming active
automatically. A confirmation outside the configured window skips that cue and
records the reason. A delayed loop coalesces elapsed inactive time; it never
activates past cues in a catch-up burst. Cancel is terminal until reinitialize.
Invalid input or impossible plan state enters `Fault`, whose snapshot has no
active cue.

### Audit record

Every accepted transition creates one value:

```cpp
enum struct CueAuditEvent : uint8_t
{
    Initialized,
    ReviewStarted,
    RunRequested,
    ConfirmationRequested,
    Confirmed,
    CueShown,
    CueHidden,
    CueSkipped,
    Held,
    Resumed,
    Cancelled,
    Faulted,
    Shutdown
};

struct CueAuditEntry
{
    uint32_t      sequence;
    TimePoint     recordedAt;
    CueAuditEvent event;
    InertCueId    cue;
    uint8_t       cueIndex;
    Status        status;
};
```

`CueAuditBuffer` is a caller-provided fixed ring. The scheduler does not own a
stream and never invokes a callback during cleanup. When storage is full, it
enters `Held`, reports `CapacityExceeded`, and preserves all existing records.
It cannot continue unrecorded.

Stable text grammar version 1 is:

```text
adk-cue,1,<sequence>,<milliseconds>,<event>,<cue>,<index>,<status>\n
```

The formatter is allocation-free, ASCII, locale-independent, and never
truncates. Serial is one optional sink. The canonical evidence remains the LED
state plus a downloadable host replay trace.

### Circuit-native evidence

The selected cue's inert LED is lit only during `Active`. The RGB status is:

- blue: review;
- amber: confirmation required;
- green: cue visible;
- violet pulse: held;
- red: cancelled or fault;
- off: idle or safe shutdown.

TP29 on the selected LED output proves the electrical output. The LCD or cue
selection LEDs name which cue is pending before confirmation. Serial does not
participate in the decision.

Host cases cover plan validation, every phase and event, chords, release gate,
confirmation edges, timeout edges, delayed loops, rollover, audit exhaustion,
formatter boundaries, exact replay, cancel from each phase, shutdown from each
phase, and reinitialization.

## Lesson 030 project: reviewed inert show-cue simulator

### Learning outcome

Compose assessed synthetic channels, a reviewed cue schedule, visible outputs,
and an audit/replay workflow. Explain what each observation proves and what it
cannot prove.

### Project policy

```cpp
enum struct InertShowState : uint8_t
{
    Startup,
    Review,
    Ready,
    Running,
    Held,
    Complete,
    Cancelled,
    Fault
};

enum struct InertShowFault : uint8_t
{
    None,
    ObservationUnavailable,
    ObservationStale,
    ObservationContradictory,
    AuditFull,
    InvalidInput,
    InternalInvariant
};

struct InertShowSnapshot
{
    InertShowState             state;
    InertShowFault             fault;
    InertChannelAssessment     selectedChannel;
    CueSchedulerSnapshot       schedule;
    uint32_t                   auditSequence;
    uint32_t                   traceDigest;
};
```

The capstone core coordinates values only:

```cpp
struct InertShowSimulator
{
    InertShowSimulator (InertChannelAssessor& assessor,
                        InertCueScheduler&     scheduler,
                        CueAuditBuffer&        audit) noexcept;
    ~InertShowSimulator () noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status observe    (TimePoint now,
                       const InertChannelObservation* observations,
                       uint8_t observationCount,
                       const CueOperatorInput& input) noexcept;
    Status decide     (TimePoint now) noexcept;

    InertShowSnapshot snapshot () const noexcept;
};
```

The simulator allows a cue to become visibly active only when its selected
synthetic channel assessment is `Closed`. `Open`, `ShortSimulated`, `Stale`,
`Contradictory`, and `Unavailable` all hold the schedule with no active cue.
This rule exists to teach evidence gating; it does not model a real launcher.

`observe()` transactionally captures one complete input frame. `decide()`
updates assessment, schedule, fault, and audit exactly once for that frame.
Calls at the same timestamp with the same frame have no effect. A changed frame
at the same timestamp is rejected to preserve replay identity.

### Trace and replay

An `InertShowTraceFrame` contains only:

- timestamp;
- complete synthetic observation set;
- debounced operator events;
- expected state, fault, cue decision, and audit sequence.

The versioned trace file contains no electrical pin maps, radio data, waveform,
or external protocol field. `traceDigest` is a small deterministic integrity
check for accidental corruption, not authentication. Replay verifies:

1. input frame schema and monotonic trace order;
2. each resulting snapshot;
3. every audit record byte;
4. final digest;
5. zero active cues after shutdown.

The host CLI should expose:

```text
make inert-show-test
make inert-show-replay TRACE=tests/traces/inert_show_happy.csv
make inert-show-audit TRACE=tests/traces/inert_show_faults.csv
```

These targets never require an Arduino or network.

### Narrative Mega example

Objects appear in dependency order:

```cpp
Runtime                 runtime;
Button                  operatorButtons[4];
MonoLed                 inertCueLeds[8];
RgbLed                  simulatorHealth;
InertChannelAssessor    channelAssessor (...);
InertCueScheduler       cueScheduler (...);
CueAuditBuffer          cueAudit (...);
InertShowSimulator      simulator (...);
```

High-level flow appears before pin mechanics:

```cpp
void loop ()
{
    const TimePoint now = TimePoint (millis ());

    observeSimulator (now);
    decideSimulator  (now);
    showInertCues    ();
    showHealth       (now);
}
```

The example must not offer a configurable output adapter. `showInertCues()`
addresses only the eight statically constructed `MonoLed` objects. Shutdown
turns every LED off before releasing pins.

### Fault response

| Fault | Decision | Visible evidence |
|---|---|---|
| Observation open/short simulated | hold | selected LED off, amber status |
| Observation stale | hold | selected LED off, slow amber pulse |
| Contradictory pair | fault | all cue LEDs off, alternating red/amber |
| Invalid operator chord | fault | all cue LEDs off, two red pulses |
| Audit capacity exhausted | hold | all cue LEDs off, violet pulse |
| Internal invariant | fault | all cue LEDs off, steady red |
| Cancel | terminal cancel | all cue LEDs off, red status |
| Shutdown/reset | safe state | all LEDs off, then pins high impedance |

No fault path produces an active cue. Recovery from observation faults requires
a fresh consistent frame and explicit renewed review; it never resumes by
itself. Cancel and internal invariant faults require reinitialization.

### Required deterministic traces

- reviewed happy path through every cue;
- each assessed channel state at each scheduler phase;
- confirmation exactly inside and outside its window;
- review release immediately before and during a visible cue;
- invalid chord, skip, cancel, hold, and explicit renewed review;
- stale boundary and 32-bit timestamp rollover;
- delayed loop with no catch-up activation;
- audit capacity at one-before-full and full;
- transaction rejection for a changed same-time frame;
- restart after every recoverable fault;
- reinitialization after each terminal state;
- shutdown from every state with no active cue;
- two complete replays with byte-identical snapshots and audit text.

### Circuit-native verification

The acceptance worksheet follows an evidence chain:

```text
button level at TP28
    -> debounced operator event
        -> synthetic channel assessment
            -> scheduler decision
                -> cue LED voltage at TP29
                    -> audit entry
```

Each step states what it does not prove. A lit cue LED proves only that the
simulator selected a visible state. It proves nothing about an external load,
radio, real channel, show readiness, or functional safety.

Resource acquisition and safe-state evidence remain separate:

- startup sweep proves every expected LED endpoint was acquired;
- individual selection proves cue-to-indicator mapping;
- measured zero output proves the inactive electrical level;
- shutdown measurement proves high impedance;
- removal of USB power is the physical stop.

### Lesson package

HTML provides exact APIs, state and fault tables, trace schemas, source links,
copyable replay commands, corrections, and a prominent safety boundary. PDFs
provide:

- pencil orientation diagram of buttons and resistor-limited LEDs;
- authoritative pin-by-pin schematic;
- full state graph and timing diagrams;
- audit reconciliation worksheets;
- trace prediction and manual replay exercises;
- fault-injection cards;
- claim-evidence-reasoning report;
- an unsupported-connections page.

Every page that shows the circuit labels it “inert LED simulator.” Neither
format uses fireworks, launcher, initiator, or radio imagery as decoration.

## Implementation boundaries

The pure deterministic core may be implemented without Arduino dependencies:

1. `simulation: assess inert channels`
2. `schedule: add reviewed inert cues`
3. `audit: encode cue decisions`
4. `project: compose inert show simulator`
5. `examples: show inert cue evidence`
6. `lessons: teach reviewed cue simulation`
7. `site: publish inert capstone`

Core commits contain public values, out-of-line implementations, and host
tests. Example output composition follows only after the decision core passes.
Shared integration files (`Adk.h`, make fragments, site navigation, curriculum,
release metadata, and agent status) remain the integrator's responsibility.

## Non-goals enforced in review

Reject a change if it adds:

- an ignition, firing, launcher, or transmitter verb;
- a generic output callback, pin map, protocol field, waveform, or RF payload;
- continuity claims about real energetic circuits;
- automatic confirmation, catch-up activation, or unrecorded progress;
- relay, MOSFET, optocoupler, external connector, or antenna instructions;
- a route from cue IDs to anything except the canonical resistor-limited LEDs;
- language implying regulatory, functional-safety, or real-show readiness.

Future work may improve simulation, formal verification, accessibility, or
replay tooling. It may not turn this curriculum into a control system.
