# Lessons 040--042 optical course marshal plan

Status: implementation boundary reconciled, 2026-07-28.
Lessons 037--039 are published with status `done`, and their final public
headers, examples, stress passes, measured budgets, vocabulary, and
publication inventories have been reconciled against this record. Their
interfaces remain unchanged and are not dependencies of the new optical
types. The Lesson 040 E0 copied-value policies, deterministic fixtures, and
stress passes described here are authorized for implementation. Measured AVR
stress results block Lesson 041 and 042 API fixation until their owned state is
redesigned and every resulting type and object measures no more than 128
bytes.
Powered adapters, wiring tables, formal schematics, E1 examples, and electrical
support claims remain gated on exact-specimen qualification and measured
composition evidence.

## Scope, authority, and vocabulary

This arc preserves the canonical titles and kinds:

| Lesson | Kind | Title |
|---:|---|---|
| 040 | Component | Reflective and interrupted light |
| 041 | Component | Presence and passage |
| 042 | Project-bearing | Tabletop course marshal |

The authorized inventory families relevant to planning are `Photo-Resistor`,
`Photo-Interrupter`, `Tracking`, `Avoidance`, `HC-SR501`, and `HC-SR04`.
Family listing authorizes only curriculum planning. Before a powered adapter,
Mega example, wiring table, or formal schematic exists, the selected specimen
must establish its markings, pin order, supply, output rails, polarity,
pull-ups, emitter behavior, current, and authoritative electrical source.

`Light-cup` is not in the authorized Elegoo union and is excluded unless a
later source decision explicitly admits it. `Laser Emit` remains unpowered and
excluded. An IR receiver is not an interrupted-light gate. Retail words such as
line, tracking, avoidance, obstacle, and beam do not establish electrical
identity or a safety function.

The learner evidence chain is:

```text
raw optical sample
    -> calibration-bound observation
        -> qualified reflective state or beam edge
            -> separately aged presence evidence
                -> semantic checkpoint event
                    -> ordered run record and presentation
```

This arc makes no lux, reflectance, distance-accuracy, occupancy, security,
navigation, collision-avoidance, traffic-enforcement, identity, or
life-safety claim. The project uses a hand-moved card or unpowered model only.

## Design decisions before API fixation

1. Lesson 040 has two narrow pure policies with a shared output vocabulary:
   `ReflectiveObservationPolicy` handles scalar ADC evidence, while
   `BeamObservationPolicy` handles interrupted-light levels and edges. There is
   no universal hardware `OpticalSensor`, optional-field mega-record, driver
   hierarchy, or retail-label switch.
2. Exact adapters are separate endpoint compositions. An AO+DO adapter, if an
   exact specimen proves both outputs, owns its analog and digital pins
   transactionally and preserves both samples. It does not make the two
   Lesson 040 policies one hardware abstraction.
3. Lesson 041 uses an arc-local `TimedRangeEvidence` copied-value bridge.
   `RangeReading` and `UltrasonicRanger` remain unchanged. The bridge stamps
   completion in the course millisecond clock while retaining the measurement
   epoch and latency from the microsecond acquisition. It must not imply that
   delayed range and newer optical/PIR samples were simultaneous.
4. A dedicated pure `PirObservationPolicy` is the sole owner of PIR warm-up,
   active-polarity, retrigger, stuck-motion, and recovery semantics. The
   `PresenceModel` only combines its qualified output with optical and range
   evidence; it never requalifies PIR or optical inputs.
5. Lesson 041 retains every source's status, validity, age, provenance, and
   disagreement. It never votes, resamples, or silently substitutes one source
   for another. Domain outcomes such as stale or disagreement remain value
   quality; malformed evidence and source/timing failures use `Status`.
6. Lesson 042 accepts semantic checkpoint slots and stable events, never pins
   or retail modules. Its fixed-capacity volatile record and presentation
   intent are separate. It does not reuse or widen the magnetic-specific
   `PassageQualifier`, `PassageLedger`, `MagneticPassageLogger`, telemetry
   `ObservationTracker`, or RTC/storage contracts.
7. Monotonic supplied time is authoritative for elapsed run time. RTC data, if
   a later project adds it, is metadata only and requires a separately scoped
   persistence/recovery design.
8. A debounced explicit button press is the sole start authority. PIR supplies
   eligibility evidence but never authorizes or synthesizes a start. Eligibility
   requires a present, fresh, valid, Ok PIR observation in `PirPhase::Motion`;
   `ReadyClear`, `Warming`, `StuckMotion`, and `Fault` are ineligible. An
   arc-local `CourseStartPolicy` accepts the button press and copied PIR state
   in one frame and emits the only start event consumed by Lesson 042.

Changing an existing observation type, creating a repository-wide generic
timestamped-observation framework, adding shared emitter scheduling, adding
persistence, or making PIR an implicit start authority is not local cleanup.
It requires an architectural-remediation discussion and durable decision.

## 040 -- Reflective and interrupted light

Energy class: E0 for the pure policies and synthetic fixtures. A passive LDR
divider may become E1 after its resistor/current/source qualification. A
powered optical module remains PX until its exact specimen closes the
applicable P1/P2 supply, output, pull-up, emitter, and aggregate-current gates;
only then may its bounded fixture become E1.

Provisional files:

```text
src/optical_observation.h
src/optical_observation.cpp
tests/test_optical_observation.cpp
docs/design/stress-passes/optical-observation.md
```

### Public values

```cpp
enum struct OpticalSourceKind : uint8_t
{
    ReflectiveAnalog,
    InterruptedDigital
};

enum struct OpticalQuality : uint8_t
{
    Unqualified,
    Valid,
    BelowQualifiedRange,
    AboveQualifiedRange,
    DegenerateCalibration,
    SourceFault,
    TimingFault
};

struct OpticalProvenance
{
    uint8_t  sourceId;
    uint16_t calibrationRevision;
    TimePoint observedAt;
};

struct ReflectiveSample
{
    OpticalProvenance provenance;
    uint16_t          raw;
    Status            status;
};

struct BeamSample
{
    OpticalProvenance provenance;
    Level             rawLevel;
    Status            status;
};

struct ReflectiveObservation
{
    OpticalProvenance provenance;
    uint16_t          raw;
    uint16_t          darkReference;
    uint16_t          lightReference;
    uint16_t          normalizedPermille;
    bool              markerActive;
    bool              activationEvent;
    bool              deactivationEvent;
    Duration          stableFor;
    OpticalQuality    quality;
    Status            status;
};

struct BeamObservation
{
    OpticalProvenance provenance;
    Level             rawLevel;
    bool              interrupted;
    bool              interruptionEvent;
    bool              restorationEvent;
    Duration          stableFor;
    OpticalQuality    quality;
    Status            status;
};

struct ReflectiveObservationConfig
{
    uint8_t  sourceId;
    uint16_t calibrationRevision;
    uint16_t qualifiedMinimum;
    uint16_t qualifiedMaximum;
    uint16_t darkReference;
    uint16_t lightReference;
    uint16_t activatePermille;
    uint16_t releasePermille;
    Duration dwell;
    bool     darkerIsActive;
};

struct BeamObservationConfig
{
    uint8_t  sourceId;
    uint16_t calibrationRevision;
    Level    interruptedLevel;
    Duration interruptDwell;
    Duration restoreDwell;
};

struct ReflectiveObservationPolicy
{
    explicit ReflectiveObservationPolicy (
        const ReflectiveObservationConfig& config) noexcept;

    Status                initialize () noexcept;
    void                  reset      () noexcept;
    Status                update     (const ReflectiveSample& sample) noexcept;
    ReflectiveObservation snapshot   () const noexcept;
    bool                  initialized() const noexcept;
};

struct BeamObservationPolicy
{
    explicit BeamObservationPolicy (
        const BeamObservationConfig& config) noexcept;

    Status          initialize () noexcept;
    void            reset      () noexcept;
    Status          update     (const BeamSample& sample) noexcept;
    BeamObservation snapshot   () const noexcept;
    bool            initialized() const noexcept;
};
```

Both policies are inert, noncopyable/nonmovable pure copied-state owners. They
own no endpoint,
claim, bus, timer, callback, emitter, or clock, and have no `shutdown()`.
Endpoint adapters retain the ordinary lifecycle and shut down separately.

References are supplied configuration, not hidden or persisted calibration.
They must be distinct and inside the qualified ADC range. Normalization uses
widened integer arithmetic and clamps to 0--1000. Hysteresis ordering is
validated relative to `darkerIsActive`; the public observation remains
polarity-independent. `raw` is always preserved. Out-of-range evidence clears
a dwell candidate and cannot diagnose an open, short, floating wire, ambient
change, or surface material.

Ambient rejection is taught through controlled paired fixtures and comparison
of calibration revisions; one scalar does not prove ambient compensation.
`crosstalk` is not an output flag unless a later exact adapter supplies a
qualified emitter-on/off or paired-channel schedule. Until then, correlated
changes are a worksheet observation, not a component diagnosis.

Lesson 040 owns calibration-bound optical state and event semantics over a
qualified copied source. Lessons 070--072 own specimen-neutral module
descriptors, electrical characterization sweeps, and characterization records.
Neither boundary widens or silently absorbs the other; Lesson 040 does not own
potentiometer, pull-up, comparator, or module-topology characterization.

Time validation precedes enum and source processing. Identical same-time
samples are idempotent; changed same-time evidence, backward apparent time, or
an exact/greater unsigned half-range jump reports `TimingFault` without partial
mutation. Events last one snapshot and clear on the next later update. Counts
are deliberately absent; Lesson 042 owns course policy.

`sourceId` is unique only within its typed source kind. Any identity that can
cross reflective, interrupted, PIR, or range kinds includes that kind
explicitly; equal numeric IDs from different kinds never alias.

### Deterministic fixture and failure matrix

- Both reference orderings, active polarity, and every valid source ID.
- Qualified ADC minimum/maximum and reference/threshold values at minus one,
  exact, and plus one; degenerate/reversed calibration; normalized endpoints.
- Dark/light ramps, slow crossings, hysteresis entry/exit, ambient offsets,
  drift fixtures, bright/dark saturation, and surface-dependent traces.
- Interrupted/restored levels, pulses shorter than, equal to, and longer than
  each dwell, bounce, chatter, stuck clear, and stuck interrupted.
- Separate analog and digital source faults; invalid levels; unavailable
  evidence; no invented runtime hardware failure from endpoints whose
  `update()` cannot report one.
- Same-time identical/changed frames, ordering permutations, rollover,
  half-range, backward time, reset recovery, and field-stable replay.
- Exact-adapter unsupported/busy pin, AO+DO second-claim failure and reverse
  rollback, repeated initialize/shutdown, destruction, and claim reuse.
- Adjacent-source fixtures with alternating quiet/active intervals; correlated
  changes remain evidence without being mislabeled as proven crosstalk.

### Provisional adapter and resource envelopes

One selected Lesson 040 circuit uses either one analog reflective input or one
digital interrupted-light input, never an inferred all-family circuit.
Provisional planning pins are A0/`TP-OA` or D22/`TP-OB`, plus D30 raw,
D31 qualified-event, and D32 ready/fault LEDs through 1 kOhm. An exact AO+DO
specimen may reserve both A0 and D22 transactionally.

There is no interrupt, timer, bus, heap, emitter output, or ADC-reference
change. At most three direct LEDs are active, each planned below 5 mA; selected
module/base/aggregate current remains blank until qualification. Pin numbers
are provisional and do not become wiring guidance through this plan.

### Narrative and publication

```text
setup: acquire exact adapter -> acquire indicators -> show ready
loop:  observe one source -> qualify optical evidence -> present evidence
```

Stages are synthetic trace replay; unpowered identity inspection; controlled
dark/light or clear/interrupted endpoints; threshold/hysteresis; paired
ambient/crosstalk comparison; fault evidence; shutdown and separate
high-impedance/safe-indicator checks.

Pencil visuals cover orientation, beam direction, surface/marker examples,
ambient scenes, calibration plots, traces, timing, state flow, and staged
builds. Only the conventional net/component drawing for one exact qualified
circuit may be marked `% ADK visual: schematic` and electrically
authoritative. Breadboard, pin locator, waveform, state machine, and block
diagram remain `% ADK visual: pencil`.

## 041 -- Presence and passage

Energy class: E0 for copied-value policy. Eventual adapters may be E1 after
separate PIR, optical, and HC-SR04 qualification.

Architecture status: **API fixation blocked by measured SRAM buckling.** The
provisional `PresenceModel` layout measures 233 bytes on AVR, above the
128-byte largest-object ceiling. The declarations below preserve required
semantics for redesign discussion; they are not an authorized final layout.

Provisional files:

```text
src/presence_model.h
src/presence_model.cpp
tests/test_presence_model.cpp
docs/design/stress-passes/presence-model.md
```

### Public values and contract

```cpp
enum struct PirPhase : uint8_t
{
    Warming,
    ReadyClear,
    Motion,
    StuckMotion,
    Fault
};

enum struct PresenceQuality : uint8_t
{
    Unqualified,
    Valid,
    Stale,
    Disagreement,
    SourceFault,
    TimingFault
};

struct PirSample
{
    uint8_t   sourceId;
    TimePoint observedAt;
    Level     rawLevel;
    Status    status;
};

struct PirObservation
{
    uint8_t   sourceId;
    TimePoint observedAt;
    Level     rawLevel;
    PirPhase  phase;
    bool      motionEvent;
    bool      clearEvent;
    Duration  stableFor;
    Status    status;
};

struct PirObservationConfig
{
    uint8_t  sourceId;
    Level    motionLevel;
    Duration warmup;
    Duration qualifyMotion;
    Duration qualifyClear;
    Duration stuckMotion;
};

struct PirObservationPolicy
{
    explicit PirObservationPolicy (
        const PirObservationConfig& config) noexcept;

    Status         initialize () noexcept;
    void           reset      () noexcept;
    Status         update     (const PirSample& sample) noexcept;
    PirObservation snapshot   () const noexcept;
    bool           initialized() const noexcept;
};

struct TimedRangeEvidence
{
    uint8_t              sourceId;
    TimePoint            startedAt;
    TimePoint            completedAt;
    MicrosecondTimePoint measurementStartedAt;
    MicrosecondDuration  measurementLatency;
    RangeReading         reading;
    Status               status;
};

struct OptionalPirObservation
{
    bool           present;
    PirObservation value;
};

struct OptionalBeamObservation
{
    bool            present;
    BeamObservation value;
};

struct OptionalReflectiveObservation
{
    bool                  present;
    ReflectiveObservation value;
};

struct OptionalTimedRangeEvidence
{
    bool               present;
    TimedRangeEvidence value;
};

struct PresenceInput
{
    TimePoint                     observedAt;
    OptionalPirObservation        pir;
    OptionalBeamObservation       beam;
    OptionalReflectiveObservation finishGuard;
    OptionalTimedRangeEvidence    range;
};

enum struct PresenceSourceBit : uint8_t
{
    Pir         = 1U << 0,
    Beam        = 1U << 1,
    FinishGuard = 1U << 2,
    Range       = 1U << 3
};

struct PresenceModelConfig
{
    uint8_t  requiredSources;
    uint8_t  agreementSources;
    Duration pirMaximumAge;
    Duration beamMaximumAge;
    Duration finishGuardMaximumAge;
    Duration rangeMaximumAge;
    Duration beamPassageWindow;
    Duration agreementWindow;
    uint16_t approachMinimumMm;
    uint16_t approachMaximumMm;
};

struct PirPresenceState
{
    bool           available;
    PirObservation evidence;
    Duration       age;
    bool           valid;
    bool           stale;
};

struct OpticalPresenceState
{
    bool              available;
    OpticalProvenance provenance;
    OpticalQuality    quality;
    Duration          age;
    bool              valid;
    bool              stale;
    bool              active;
    bool              activationEvent;
    bool              deactivationEvent;
    Status            status;
};

struct RangePresenceState
{
    bool               available;
    TimedRangeEvidence evidence;
    Duration           age;
    bool               valid;
    bool               stale;
    bool               approachValid;
};

struct PresenceSnapshot
{
    PirPresenceState     pir;
    OpticalPresenceState beam;
    OpticalPresenceState finishGuard;
    RangePresenceState   range;
    bool                 pirEligible;
    bool                 passageEvent;
    bool                 disagreement;
    Duration             disagreementFor;
    PresenceQuality      quality;
    Status               status;
};

struct PresenceModel
{
    explicit PresenceModel (const PresenceModelConfig& config) noexcept;

    Status           initialize () noexcept;
    void             reset      () noexcept;
    Status           update     (const PresenceInput& input) noexcept;
    PresenceSnapshot snapshot   () const noexcept;
    bool             initialized() const noexcept;
};
```

For `beam`, `OpticalPresenceState::active`, `activationEvent`, and
`deactivationEvent` map respectively to interrupted, interruption, and
restoration. For `finishGuard`, they map to marker active, marker activation,
and marker deactivation. The shared state is copied vocabulary, not a claim
that the two source policies have identical hardware or calibration.

`PresenceModel` takes a required-source bitmask, per-source maximum ages,
approach range, source-agreement window, and beam passage window. It provides
the pure-policy lifecycle `initialize/reset/update/snapshot/initialized`.
Qualified PIR and beam observations and `RangeReading` are consumed rather
than requalified. A passage event is an optical semantic event with retained
PIR/range context; it is not magnetic direction policy.

Each optional wrapper has one canonical absent value: `present=false` and an
all-zero value payload whose status is Ok. An absent optional source publishes
`available=false`, `valid=false`, `stale=false`, zero age, canonical evidence,
and is excluded from age validation, source-fault precedence, disagreement,
and event deduplication. An absent required source makes the aggregate
`Unqualified` with Ok status and emits no event; absence is not a hardware
failure. A present malformed or non-Ok source follows ordinary fault
precedence. `requiredSources` may contain only the four defined source bits.
`agreementSources` must be a subset of `requiredSources` with at least two
bits when disagreement comparison is enabled.

Each selected agreement source supplies one boolean presence claim: PIR
`Motion`/`StuckMotion`, beam interrupted, finish guard active, or range
approach valid. When all selected sources are present, fresh, valid, and not
all claims match, the disagreement candidate begins. Continuous mismatch
reaching `agreementWindow` publishes `Disagreement`; matching evidence clears
the candidate. Missing or unhealthy required evidence remains `Unqualified`,
`Stale`, or `SourceFault` and takes precedence rather than being mislabeled
disagreement. Optional absent sources never join the comparison.

`passageEvent` is emitted only on a valid beam restoration event that closes a
previously observed interruption after a positive duration no greater than
`beamPassageWindow`. The complete frame must otherwise be `Valid`; required
context must be present/fresh/valid and no disagreement may be active.
Optional absent context neither permits nor suppresses the event. A too-long,
stale, faulted, or disagreement frame clears the passage candidate without
emitting. Thus the beam owns the event transition while PIR/range/finish
evidence remains explicit context rather than a hidden vote.

The aggregate retains rather than replaces the source-specific evidence. Optical
calibration revision and quality, PIR phase and epoch, and the complete timed
range evidence remain available. A stale valid value remains stale, not false.
`Timeout` and `OutOfRange` remain distinct range states.
Disagreement is observable policy evidence and does not silently choose a
winner. Timestamp/enum malformed evidence has precedence over source fault,
then stale evidence, then disagreement, then ordinary presence/passage
transition. All input source epochs remain visible.

For every present source, `observedAt` or `completedAt` must be at or before the
`PresenceInput::observedAt` frame time under modular subtraction, and the
resulting age must be strictly below half-range. It need not equal frame time;
that difference is the source age. A future source epoch, exact half-range, or
larger apparent age is a timing fault before mutation.

The range adapter documents two domains rather than pretending to convert one
clock into the other. `startedAt` is the course-clock stamp at measurement
start. `completedAt` is captured in the exact adapter update that first
observes a terminal range outcome and is never restamped on later copies; no
queue lies between outcome observation and stamping. In the microsecond
acquisition domain,
`measurementStartedAt + measurementLatency` identifies the completed
measurement interval under that type's half-range rules. `completedAt` is the
course-clock stamp captured when that outcome is copied. Completion stamping
does not move the measurement epoch or erase latency. Both course-clock stamps
must be ordered within half-range, and the configured maximum range age also
bounds adapter-update delay.

For a present completed range with Ok status, legal tuples are:

| State | `distanceMm` | `echoDuration` | `valid` |
|---|---:|---:|:---:|
| `Valid` | greater than zero | greater than zero and no greater than `measurementLatency` | true |
| `Timeout` | zero | zero | false; `measurementLatency` is nonzero |
| `OutOfRange` | copied exactly, including a permitted zero/zero edge | no greater than `measurementLatency`; latency may be zero only for the zero/zero edge | false |

`Idle`, `AwaitingEcho`, and `Measuring` are not completed evidence. A non-Ok
source status uses canonical `Idle`, zero distance/echo/latency, and
`valid=false`, with `completedAt` identifying the fault-copy epoch.
`RangeState::Timeout` with Ok status is semantic no-echo evidence; a non-Ok
timeout status is a source failure. The model validates the entire input frame
in this order before any mutation: frame/source course time, enum and tuple
shape, microsecond interval, source status, freshness, disagreement, then
ordinary transition. A malformed frame leaves the prior stable state intact
and publishes one canonical fault snapshot with no partial event.

An optical event is forwarded once with its original provenance. The model
deduplicates it by source ID, calibration revision, event kind, and source
timestamp; a repeated identical frame is idempotent, while different evidence
with that identity faults. Same-time identity compares every public input
field, including otherwise inactive booleans and statuses. Same-time,
half-range, rollover, and mutation rules otherwise match Lesson 040. Reset
during warm-up or passage returns to an explicitly unqualified state and
creates no event.

The bounded remediation preserves copied configuration but removes duplicate
stored input-plus-snapshot evidence. `PresenceModel` should own one exact
evidence cache plus compact transition state and synthesize its public snapshot
from that single cache. It must not retain both a full `PresenceInput` and a
full `PresenceSnapshot`. If the exact AVR measurement still exceeds 128 bytes
after that redesign, evidence storage moves to explicit caller-owned fixed
storage with a documented lifetime contract, without allocation, hidden
aliasing, or borrowed temporary values. API fixation requires measured
evidence that every resulting type and object is at or below 128 bytes and
that aggregate composition SRAM remains within budget.

### Deterministic matrix

- PIR-policy warm-up, retrigger, held motion, stuck threshold, clear/recovery, and
  boundaries one tick before/at/after.
- Beam entry/exit, bounce, stuck states, reflective finish-guard transitions,
  and no duplicated optical dwell policy.
- Range minimum/maximum, no echo, timeout, out of range, acquisition latency,
  completion timestamp, and conversion boundaries.
- Freshness one tick before/at/after for every source; old-but-valid values;
  timestamp mismatch; every meaningful validity/age/disagreement combination.
- Simultaneous source changes in every input order, disagreement-window
  boundaries, source failure collisions, reset/restart, and field-stable replay.
- Invalid configurations/enums, idempotent same-time frames, changed same-time
  faults, rollover, half-range, and backward time.
- Every range enum and legal tuple, one-field-invalid tuple crosses,
  terminal/nonterminal collisions, semantic timeout versus timeout status,
  latency/echo boundaries, malformed range colliding with optical/PIR events,
  and no partial mutation.

### Narrative and resources

Synthetic traces precede hardware. Each exact source is then observed alone:
PIR warm-up/ready/motion, beam interrupted/restored, HC-SR04
start/valid/timeout/out-of-range, and finally copied multi-source frames.
Learners predict which source is old or disagrees before seeing the aggregate.

A provisional full Lesson 041 fixture reserves PIR D23, beam D22, optional
reflective guard A0, HC-SR04 trigger D24/echo D25, and D30--D32 evidence LEDs.
It owns no bus, timer, interrupt, storage, or actuator. Trigger/echo pins are
named test points. Exact current and module variants remain gated.

Pencil visuals cover source placement, warm-up, source-age lanes, disagreement,
range timing, state flow, and staged experiments. Each separately qualified
electrical circuit receives its own authoritative formal schematic; a
multi-source block diagram is pencil, not a schematic exception.

## 042 -- Tabletop course marshal

Energy class: E0 for the project engine and replay. Eventual E1 uses only
qualified low-voltage sensors/modules whose PX-to-P1/P2 electrical gates have
closed, resistor-limited indicators, an existing qualified display, and a
hand-moved card or unpowered model. There is no motion or gate actuator.

Architecture status: **API fixation blocked by measured SRAM buckling.** The
provisional AVR measurements are 653 bytes for `CourseMarshal` and 432 bytes
for `CourseMarshalPresenter`, each far above the 128-byte largest-object
ceiling. The declarations below are semantic design material, not a final
storage layout or public API.

Provisional files:

```text
src/course_marshal.h
src/course_marshal.cpp
tests/test_course_marshal.cpp
docs/design/stress-passes/course-marshal.md
```

### Public policy

The fixed capacity is four ordered intermediate checkpoints followed by a
separate reflective finish guard: five optical course boundaries total.
Capacity is part of configuration and tests, not pin identity. A configuration
maps one to four unique `CheckpointId` values into slots and supplies
run/finish/agreement timeouts. Zero checkpoints are invalid.

```cpp
struct CheckpointId
{
    uint8_t value;
};

struct CheckpointBinding
{
    CheckpointId      checkpointId;
    OpticalSourceKind sourceKind;
    uint8_t           sourceId;
    uint16_t          calibrationRevision;
};

enum struct CourseStartSource : uint8_t
{
    None,
    ExplicitButtonWithPirEligibility
};

struct CourseMarshalConfig
{
    CheckpointBinding orderedCheckpoints[4];
    uint8_t           checkpointCount;
    Duration     checkpointEventMaximumAge;
    Duration     checkpointSimultaneityWindow;
    Duration     finishAgreementWindow;
    Duration     maximumRunDuration;
};

enum struct MarshalPhase : uint8_t
{
    Disarmed,
    Arming,
    Ready,
    Running,
    Finished,
    Rejected,
    Fault
};

enum struct RunDisposition : uint8_t
{
    None,
    Accepted,
    SkippedCheckpoint,
    ReversedCheckpoint,
    DuplicateCheckpoint,
    SimultaneousCheckpoints,
    FinishTooEarly,
    TimedOut,
    EvidenceFault
};

struct CheckpointEvent
{
    CheckpointId       checkpointId;
    OpticalSourceKind  sourceKind;
    OpticalProvenance  provenance;
    OpticalQuality     quality;
    Status             status;
};

struct CourseStartInput
{
    TimePoint        observedAt;
    uint8_t          buttonSourceId;
    bool             buttonPressEvent;
    PirPresenceState pir;
};

struct CourseStartEvent
{
    bool              present;
    CourseStartSource source;
    uint8_t           buttonSourceId;
    TimePoint         observedAt;
    PirPresenceState  pir;
    Status            status;
};

struct CourseStartPolicy
{
    explicit CourseStartPolicy (uint8_t buttonSourceId) noexcept;

    CourseStartPolicy (const CourseStartPolicy&)            = delete;
    CourseStartPolicy& operator= (const CourseStartPolicy&) = delete;
    CourseStartPolicy (CourseStartPolicy&&)                 = delete;
    CourseStartPolicy& operator= (CourseStartPolicy&&)      = delete;

    Status           initialize  () noexcept;
    void             reset       () noexcept;
    Status           update      (const CourseStartInput& input) noexcept;
    CourseStartEvent snapshot    () const noexcept;
    bool             initialized () const noexcept;
};

struct RecordedCheckpoint
{
    CheckpointId      checkpointId;
    OpticalSourceKind sourceKind;
    OpticalProvenance provenance;
    OpticalQuality    quality;
    Status            status;
};

enum struct RunTriggerKind : uint8_t
{
    None,
    Checkpoint,
    FinishGuard,
    Range,
    Start,
    RunTimeout,
    PresenceFault
};

struct RunTriggerEvidence
{
    RunTriggerKind        kind;
    TimePoint             observedAt;
    uint8_t               checkpointCount;
    RecordedCheckpoint    checkpoints[4];
    OpticalPresenceState  finishGuard;
    RangePresenceState    range;
    CourseStartEvent      start;
    PresenceSnapshot      presence;
    PresenceQuality       presenceQuality;
    Status                status;
};

struct CourseMarshalInput
{
    TimePoint             observedAt;
    CourseStartEvent start;
    PresenceSnapshot      presence;
    uint8_t               eventCount;
    CheckpointEvent       events[4];
};

struct CourseRunRecord
{
    uint32_t       sequence;
    TimePoint      startedAt;
    TimePoint      finishedAt;
    Duration       elapsed;
    uint8_t            acceptedCheckpointCount;
    RecordedCheckpoint acceptedCheckpoints[4];
    RunTriggerEvidence trigger;
    CourseStartEvent      start;
    OpticalPresenceState finishGuard;
    RangePresenceState   finishRange;
    RunDisposition       disposition;
    bool                 sequenceExhausted;
    Status               status;
};

enum struct CoursePresentationPhase : uint8_t
{
    Starting,
    Ready,
    Running,
    Finished,
    Rejected,
    Fault
};

struct CoursePresentationIntent
{
    TimePoint               observedAt;
    CoursePresentationPhase phase;
    uint8_t                 acceptedMask;
    uint8_t                 expectedSlot;
    uint8_t                 displayCell;
    uint8_t                 displayValue;
    bool                    allRed;
    bool                    heartbeat;
    bool                    hasRecord;
    uint32_t                recordSequence;
    Duration                elapsed;
    Status                  status;
};

struct CourseMarshalSnapshot
{
    MarshalPhase    phase;
    uint8_t         expectedSlot;
    CheckpointId    expectedCheckpointId;
    uint8_t         acceptedCheckpointCount;
    Duration        elapsed;
    bool            hasRecord;
    CourseRunRecord record;
    Status          status;
};

struct CourseMarshal
{
    explicit CourseMarshal (const CourseMarshalConfig& config) noexcept;

    Status                initialize () noexcept;
    void                  reset      () noexcept;
    void                  acknowledgeRecord() noexcept;
    Status                update     (const CourseMarshalInput& input) noexcept;
    CourseMarshalSnapshot snapshot   () const noexcept;
    bool                  initialized() const noexcept;
};

struct CourseMarshalPresenter
{
    CourseMarshalPresenter (Duration displayQuantum,
                            Duration heartbeatInterval) noexcept;

    Status                   initialize () noexcept;
    void                     reset      () noexcept;
    Status                   update     (TimePoint now,
                                         const CourseMarshalSnapshot& snapshot) noexcept;
    CoursePresentationIntent intent     () const noexcept;
    bool                     initialized() const noexcept;
};
```

The bounded Lesson 042 remediation replaces the monolithic value graph with
small, read-only input views and explicit caller-owned fixed storage for
accepted evidence and replay records. Every view and storage handle needs a
documented lifetime; no engine may retain a view to a temporary or conceal
allocation. The trigger becomes a compact tagged payload that stores only the
selected cause, while inactive evidence is absent rather than duplicated
inside the engine. `CourseMarshalSnapshot` becomes a small operational summary,
and the presenter consumes that summary to produce a small presentation frame
instead of retaining complete run evidence. The caller-owned evidence and
replay buffers remain bounded, separately measured, and included in aggregate
SRAM accounting. Exact names, field partitions, capacities, and ownership
signatures remain open until AVR measurement proves that every public type and
object, including each view, engine, storage object, summary, and presenter
frame, is at or below 128 bytes.

`CourseStartPolicy` is the sole start-authority seam. Its composition owner
updates the existing debounced `Button`, constructs one `CourseStartInput` in
that same course-clock frame, and sets `buttonPressEvent` only from
`Button::pressEvent()`. The configured `buttonSourceId` must match the input.
The policy emits a `CourseStartEvent` only when that same frame contains the
press event and copied PIR evidence that is available, valid, not stale, Ok,
and in `PirPhase::Motion`. `ReadyClear`, `Warming`, `StuckMotion`, `Fault`,
missing evidence, and stale evidence are ineligible. A PIR level, phase
transition, or motion event can never synthesize the button press.

An absent start event has `present=false`, `CourseStartSource::None`, and
canonical-zero remaining fields with Ok status. A present event always has
`ExplicitButtonWithPirEligibility`, the configured button source ID, the
policy frame timestamp, and the complete copied PIR state. It lasts for one
snapshot and clears on the next later update. The marshal accepts it only in a
`CourseMarshalInput` with the same `observedAt`; it is never queued or accepted
as an aged authorization. It is deduplicated by source, timestamp, copied PIR
evidence, and status. Identical same-time inputs are idempotent; changed same-time,
backward, or half-range-invalid inputs publish a timing fault without partial
mutation. Button acquisition failure is handled by the composing endpoint
lifecycle: existing `Button::update()` has no runtime failure result, so the
policy does not invent one. The policy validates that the copied PIR epoch and
age agree with the policy frame under the modular half-range rule before
testing eligibility. A non-Ok start status can arise only from malformed time
or evidence the policy can actually report.

After initialization the marshal is `Disarmed`. A structurally valid presence
frame begins `Arming`; fresh valid Motion eligibility makes it `Ready`.
Ineligible healthy PIR evidence returns a non-running marshal to `Arming`;
malformed or faulted required evidence follows the documented fault
precedence. Only a present valid `CourseStartEvent` may transition `Ready` to
`Running`. A press before or after eligibility emits no start and is not
latched for later use. PIR changes after a run starts remain evidence and can
fault the run where required, but never retroactively grant or revoke start
authority.

`hasRecord` and the immutable terminal record remain stable until
`acknowledgeRecord()` or `reset()` clears the record; a new start is rejected
while an unacknowledged record is present.
Acknowledgment clears only the retained terminal record and returns the pure
engine to its no-run phase; it does not choose or synthesize a new start.
The sequence never silently reuses: reaching `UINT32_MAX` sets
`sequenceExhausted` and rejects every later start for that object lifetime.
`reset()` does not clear the sequence or exhaustion flag; reconstruction is
the explicit new volatile identity epoch. Every configured duration
is nonzero and strictly below unsigned half-range. Configuration
requires a maximum run duration inside that bound;
elapsed is modular subtraction only inside that valid window, and an overlong
run rejects as `TimedOut` rather than saturating time. One frame contains at
most four checkpoint events; below, at, and above capacity are tested.

Pins never define checkpoint order. The engine validates all events before
mutation and orders equal-time events by semantic slot. Two distinct legal
slots inside the simultaneity window reject as ambiguous rather than using
array or pin order. Timing/invalid enum precedes source/evidence fault,
simultaneous ambiguity, timeout, order violation, and legal acceptance.
Identical frames are idempotent; changed same-time frames fault without a
partial checkpoint.

The composition owner builds each `CheckpointBinding` once from a qualified
adapter's optical source kind/ID and active calibration revision.
Initialization rejects zero or duplicate checkpoint IDs, duplicate source
identities, unknown kinds, zero count, and noncanonical unused bindings.
Every event must match all four fields of its configured binding; a caller
cannot relabel one source as another checkpoint. A calibration-revision change
requires reset and a newly validated configuration rather than silent
remapping.

`checkpointSimultaneityWindow` is nonzero and strictly below half-range.
Distinct checkpoint events whose wrap-safe timestamp separation is less than
or equal to that window are simultaneous; one tick beyond is ordered normally.

Each event timestamp must be at or before the frame timestamp and younger than
the configured checkpoint-event maximum age, with both intervals below
half-range. Old events are rejected rather than silently treated as current.
Every supplied checkpoint entry is already a qualified activation event;
non-event raw levels never enter this array. Event identity includes checkpoint
ID, optical source kind, source ID, calibration revision, source timestamp,
quality, and status. A
repeated identity is consumed at most once. `eventCount` is validated before
reading the fixed array. Every unused array element has the canonical zero
checkpoint/source/provenance/quality with Ok status and participates in
field-stable replay identity. Replay equality compares public fields rather
than object padding.

After whole-frame structural/time/source validation, exact identities already
consumed are removed idempotently before simultaneity or order evaluation. Two
distinct identities for an already accepted checkpoint produce
`DuplicateCheckpoint`. Remaining distinct events are evaluated in this order:
simultaneity, timeout, skipped/reversed order, then legal acceptance. Array
order cannot change the result.

Finish acceptance requires every ordered checkpoint, a fresh valid
`finishGuard.activationEvent`, and fresh valid
`presence.range.approachValid` evidence inside the configured agreement
window. At the common frame time, compute each source age by wrap-safe
subtraction; both ages must be below half-range. Their ordinary absolute
difference is the source separation. Separation less than or equal to
`finishAgreementWindow` agrees, and one tick beyond does not. The record
freezes the ordered accepted checkpoint tuples, start tuple, complete
finish-guard and range states, and one discriminated `RunTriggerEvidence`.
Its active members are fixed:

| Disposition/cause | `kind` | Required active evidence |
|---|---|---|
| no terminal disposition | `None` | none; all payload members canonical zero |
| `Accepted` | `FinishGuard` | finish-guard and range states at the accepted finish epoch |
| skipped, reversed, or duplicate checkpoint | `Checkpoint` | the offending checkpoint tuple |
| simultaneous checkpoints | `Checkpoint` | every colliding tuple in semantic-slot order |
| `FinishTooEarly` | `FinishGuard` | early finish-guard event and contemporaneous range state |
| `TimedOut` | `RunTimeout` | frame epoch only |
| malformed/source-fault checkpoint | `Checkpoint` | offending checkpoint tuples after structural ordering |
| malformed/faulted start | `Start` | start tuple |
| range-only evidence fault | `Range` | range state |
| any other presence fault | `PresenceFault` | complete presence snapshot and aggregate quality/status |

For colliding `EvidenceFault` causes, trigger selection follows whole-frame
validation precedence: checkpoint structure/time/source, start
structure/time/source, then presence sources in PIR, beam, finish-guard, range
order. `RunTriggerEvidence` is a discriminated record, not a C++ union:
exactly the selected row is semantically active, and every other payload field
is canonical zero. Unused accepted slots and every inactive trigger payload
use canonical zeros. Rejected
records preserve the accepted prefix, start, rejection cause, and triggering
epoch; finish-only fields remain canonical absent values when finish was never
evaluated. Those fields preserve evidence without becoming occupancy or
security claims. A presentation failure never changes phase, order, elapsed
time, or the frozen record.

`CourseMarshalPresenter` is a pure copied-snapshot presentation policy; a thin
exact adapter renders its `CoursePresentationIntent` through existing
LED/display endpoints. Before initialization, intent is canonical `Starting`
with zero mask/slot/cell/value/sequence/elapsed, all booleans false, and
`NotInitialized`. Each valid update copies phase, accepted mask, expected slot,
record presence/sequence, and elapsed from one immutable marshal snapshot,
advances at most one display cell, and derives all-red/heartbeat without
changing marshal state.

Marshal `Disarmed`/`Arming` map to presentation `Starting`; the remaining
Ready/Running/Finished/Rejected/Fault phases map one-for-one. `acceptedMask`
sets exactly the low `acceptedCheckpointCount` bits. A running intent copies
snapshot elapsed; a terminal intent copies record elapsed; every other elapsed
value is zero. Without a record, record sequence is canonical zero.

Both presenter durations are nonzero and below half-range. The first valid
update after initialize/reset establishes `presenterEpoch=now` and emits cell
zero. Each later valid update at a later timestamp advances exactly one cell
modulo four, independent of the elapsed jump; identical same-time input emits
the same cell. `displayValue` is:

| Presentation phase | Value for emitted `displayCell` |
|---|---|
| `Starting` | 0 |
| `Ready` | `expectedSlot + 1` |
| `Running`, `Finished` | decimal digit `displayCell` (least significant first) of `elapsed / displayQuantum` |
| `Rejected` | `0x0E` |
| `Fault` | `0x0F` |

The elapsed quotient saturates at 9999 before digit selection. Heartbeat is
`((now - presenterEpoch) / heartbeatInterval) % 2 != 0` under modular
half-range rules; phase changes do not reset it. Reset clears the epoch, cell,
and heartbeat, so the next valid update deterministically re-establishes cell
zero and heartbeat false.

Invalid/backward/half-range time or malformed snapshots win before mapping,
leave prior copied value fields stable, and publish presenter fault status.
Identical same-time input is idempotent; changed same-time input faults. Reset
restores the just-initialized canonical intent. Endpoint-render failure remains
adapter status and cannot feed back into presenter or marshal. The renderer
shows display self-test before ready, local accepted-event LEDs, all-red for
rejected/fault, and a supplied-time heartbeat that distinguishes quiet from
stalled. Serial is optional.

### Deterministic project matrix

- Zero (invalid), one-, and four-checkpoint configurations; duplicate/unknown IDs;
  capacity below, at, and above four.
- Valid run; every checkpoint alone; skip, reverse, duplicate, late, and
  simultaneous events; input and pin-mapping permutations.
- Explicit button press before, during, and after Motion eligibility; bounce,
  repeated and held input; PIR warm-up, `ReadyClear`, `Motion`, stuck motion,
  fault, or loss during every phase; proof that PIR alone never starts a run.
- Finish early; guard absent/stale/faulted; approach absent/stale/timeout/out of
  range; guard/range agreement one tick before/at/after.
- Run timeout and maximum elapsed; rollover, half-range, backward time;
  repeated terminal frames and exact reset/rearm behavior.
- Source failure plus ordering fault plus presentation failure; reset and
  shutdown from every phase; record latch/acknowledgment and sequence
  exhaustion; field-stable golden replay including every accepted ID, epoch,
  provenance tuple, display intent, and canonical unused record field.
- Presenter phase mapping, every accepted mask, record/no-record, one-cell
  progression, heartbeat boundaries, identical/changed same-time frames,
  rollover/half-range/backward time, malformed snapshot, reset, renderer
  failure isolation, and field-stable intent replay.
- Mapping permutations proving checkpoint identity, not pins or array order,
  controls the run.

### Project narrative and publication

```text
setup: acquire sensors -> acquire indicators/display -> self-test -> show ready
loop:  observe all sources -> freeze one evidence frame
       decide course state -> present one bounded frame
```

Build stages are replay; source-by-source qualification; checkpoint LEDs;
ordered two-gate course; finish guard plus approach range; complete four-slot
run; injected faults; shutdown and power removal. Learners predict, observe,
and interpret every stage without Serial.

Pencil visuals cover the course layout, checkpoint identity mapping, evidence
frame, run timeline, state graph, display/LED meanings, fault diagnosis, and
staged build. Formal schematics are restricted to exact qualified conventional
electrical circuits and use the required marker. The course map, pin locator,
waveform, timing chart, and state graph remain pencil visuals.

HTML holds the searchable API, status and disposition tables, fixture traces,
links, errata, exact wiring tables after qualification, and canonical sketch.
PDF holds pencil orientation, predictions, worksheets, timing and
troubleshooting, adjacent pin-by-pin prose, and blank bench evidence. Neither
format alone owns safety, pins, limits, prerequisites, or acceptance.

## Aggregate resource and timing budget

The maximum provisional composition is four intermediate checkpoint optical
inputs plus one separate reflective finish guard (five optical boundaries),
PIR, HC-SR04 trigger/echo, four local checkpoint LEDs,
all-red/ready/heartbeat presentation, one existing display, and one debounced
existing button input as the sole start-event producer.

| Pressure | Provisional bound and promotion evidence |
|---|---|
| Pins/claims | About 18--20 GPIO/analog claims including the mandatory button input, depending on display transport and exact AO/DO specimens. This is an unverified planning estimate, not evidence. Enumerate the final Mega map, aliases, button source, claim-registry occupancy, and rollback order before E1 composition freeze |
| ADC | Up to five analog channels only if exact specimens expose qualified AO; use one reference, explicit channel order and settling evidence; do not label mux carryover as optical crosstalk |
| Time | Source acquisition must finish into an immutable frame before policy; bound HC-SR04 echo polling/latency, optical dwell, PIR cadence, start-event qualification, ADC settling, and one-cell display work so no source is starved |
| Timers/interrupts | None are assumed; a specimen or display requiring one reopens the budget and stress pass |
| Current | Direct LEDs planned below 5 mA each; module, emitter, display, per-pin, port, and aggregate USB current remain blank until exact qualification |
| SRAM/flash/stack | The provisional `PresenceModel`/marshal/presenter layouts measured 233/653/432 bytes and failed the 128-byte largest-object ceiling. Redesign around one exact 041 evidence cache, small 042 input views and summaries, compact tagged trigger evidence, and explicit caller-owned fixed replay/evidence storage. Measure every type/object at no more than 128 bytes plus total caller storage, aggregate SRAM, stack peak, example size, and below/at/above capacity |
| Diagnostics | Local LEDs, all-red, heartbeat, display, and optional Serial are included in pin/current/time/size budgets; fill/failure/disable cannot change primary policy |
| Persistence | Not applicable: run state and records are intentionally volatile and no storage/RTC owner exists |
| Motion/energy | Not applicable: no actuation path, motor, gate, load supply, or stored-energy command exists |
| Bus | Applicable only to the selected existing display; name its owner/borrower, bounded update, failure, and rollback, or select a direct display with the corresponding pin budget |

The mandatory maximum-collision replay combines a busy/failed resource during
partial acquisition, ADC/channel-order pressure, optical saturation, PIR
warm-up or stuck motion, HC-SR04 no echo, simultaneous checkpoint edges,
button bounce/stuck input, button acquisition failure, display failure, reset, and faults still
present at restart. Failure precedence,
attribution, retained evidence, rollback, and safe presentation must remain
deterministic.

## Architecture stress disposition

Reconciled disposition: **Lesson 040 E0 implementation authorized; Lessons
041--042 require bounded SRAM remediation before API fixation; powered
promotion remains gated**.

The seven pure copied-evidence and presentation policies
(`ReflectiveObservationPolicy`, `BeamObservationPolicy`,
`PirObservationPolicy`, `PresenceModel`, `CourseStartPolicy`,
`CourseMarshal`, and `CourseMarshalPresenter`) fit the intended downward
dependency, explicit-time, fixed-storage, `Status`, and
presentation-separation contracts semantically. Their provisional storage
design does not fit: AVR measured `PresenceModel` at 233 bytes,
`CourseMarshal` at 653 bytes, and `CourseMarshalPresenter` at 432 bytes,
violating the 128-byte largest-object ceiling.
The local `TimedRangeEvidence` bridge addresses missing range provenance
without changing prior consumers. Separate analog and beam policies avoid a
false universal sensor abstraction.

Reconciliation closed these former blockers:

1. The final Lessons 037--039 interfaces were reviewed. `ContactDynamics`,
   `AcousticEnvelope`, and `PercussionSequencer` remain unchanged and are not
   reused as generic optical or course owners. Shared `Status`, `Level`,
   `TimePoint`, `MicrosecondTimePoint`, `RangeReading`, and `Button` vocabulary
   is preserved.
2. The start authority is fixed: one explicit debounced button press with
   same-frame qualified PIR Motion eligibility. PIR alone never starts a run.

Open E0 implementation gates:

1. Lesson 040 may proceed through implementation, deterministic tests, its
   stress records, and exact object measurement.
2. Lesson 041 must eliminate duplicate stored input/snapshot evidence and
   measure its one-cache design at no more than 128 bytes, or move evidence to
   explicit caller-owned fixed storage before its API freezes.
3. Lesson 042 must replace full copied graphs with small input views, explicit
   caller-owned evidence/replay storage, a compact tagged trigger, and small
   summary/presenter values. Every type and object must measure no more than
   128 bytes, and the caller storage plus aggregate composition SRAM and stack
   must be recorded before its API freezes.
4. After each redesign, prove deterministic lifecycle, status precedence,
   modular timing, replay, capacity, collision behavior, and lifetime safety.

Open powered-composition and publication gates:

1. Qualify each selected exact specimen and every powered adapter, including
   button identity, markings, pin order, supply, rails, polarity, pull-ups,
   emitter behavior, and current where applicable.
2. Prove the aggregate cadence, ADC settling, pin/claim/current, memory, stack,
   display, diagnostic, and collision budgets;
3. Repeat the stress passes before powered promotion and run every component,
   Arduino, lesson, site, packaging, and publication gate.

If reconciliation calls for changing `RangeReading`, a generic observation
retrofit, universal optical owner, magnetic passage/storage reuse, hidden
emitter scheduler, persistence, or a changed shared timing/status convention,
the disposition becomes **architectural remediation required**. Stop
promotion, identify affected earlier consumers and migration costs, discuss
bounded alternatives with the user, and record the consequential decision
before implementation.

## Reconciliation and promotion boundary

The 037--039 dependency and the deliberate-start decision are complete. Lesson
040 implementation proceeds. Lesson 041 and 042 remain at redesign and
measurement depth: neither provisional declaration block may be treated as a
fixed API until its bounded remediation satisfies the 128-byte ceiling and
aggregate SRAM/lifetime gates. After Lesson 041 clears those gates, presence
and start composition may freeze; only then may the remediated marshal and
presenter contract freeze.

This authorization does not turn provisional pins, current estimates, retail
family names, or synthetic traces into electrical evidence. No powered
adapter, Mega wiring table, formal schematic, E1 example, or hardware support
claim becomes a canonical publication candidate until its exact specimen and
maximum composition close the applicable gates above. No physical
verification may be claimed without a recorded bench acceptance result.
