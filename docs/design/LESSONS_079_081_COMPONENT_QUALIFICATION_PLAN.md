# Lessons 079--081 component qualification plan

Status: controlling implementation-depth E0 plan. The public E0 boundary is
pure copied-evidence policy: `BoundedLowSideDriverPolicy`,
`SmallIndicatorSemanticsPolicy`, and `InertComponentQualificationBench`.
It owns no GPIO endpoint. Exact powered specimens, electrical adapters,
presentation, persistence, and physical acceptance remain separately gated.

This plan resolves the older cadence sketch in
`docs/projects/component_project_cadence.md`. That sketch proposed a
low-side-driver “intent and endpoint,” fake endpoint tests, and first powered
loads within Lesson 079. The chosen boundary supersedes that proposal:
Lesson 079 publishes policy and logical intent only; Lesson 080 publishes
semantic policy over copied declarations and observations; Lesson 081 is an
inert replay bench. A future exact endpoint is E2 work and cannot be inferred
from an E0 type name, a passing replay, or a record image.

## Evidence levels, exclusions, and source authority

| Level | Authorized work |
|---|---|
| E0 | Immutable declarations, copied attributed evidence, fixed-point budget arithmetic, bounded logical drive/indicator intent, deterministic qualification replay, semantic presentation intent, and one caller-owned volatile 256-byte record image |
| E1 | Strictly unpowered identity/source/trace work: both-face photographs, markings, primary sources, traced pin order and populated topology, passive continuity/resistance/diode-mode observations, energy classification, and a reviewed specimen descriptor; E1 owns no energized endpoint and records no powered observation |
| E2a | Direct, low-energy, independently current-limited powered fixtures only: exact indicator identity, polarity, populated resistor/driver, supply/signal limits, autonomous behavior, warm-up, current, safe state, non-Serial observation, rollback, power removal, and bench acceptance |
| E2b | Transistor-switched, externally powered, or inductive fixtures only: exact low-side fixture and GPIO owner, resource and rail map, base limiting, load/device limits, diode identity/orientation/return/rating, independent current limiting and power removal, test points, rollback, stored-energy disposition, and bench acceptance |
| E2c | Exact presentation/control endpoints and, only if separately selected, a persistent store with ownership, failure, recovery, and independent acceptance |

E0 consumes declarations; it does not create electrical identity. E1 may make
one descriptor eligible for a later powered review, but cannot authorize
power. E2 requires the exact populated specimen, exact fixture, authoritative
schematic or a reviewed trace tied to primary component documentation, and a
recorded bench result. PN2222 and S8050 are separate variants; their names
never select a pinout, rating, or substitutable device.

Excluded throughout: mains; relay contacts; motors, fans, solenoids, heaters,
lasers, ignition, launchers, pyrotechnics, combustion, gas exposure, medical
or physiological use, body contact, food use, unknown emitters, unattended
operation, and arbitrary “three-pin module” support. E0 never applies voltage
or current. The future E2 fixture is limited to one identified low-energy,
current-limited indicator or restrained inert inductive specimen whose
specific card permits it; an E2 card may narrow this list further.

The specimen record, primary-source packet, and exact fixture card are
promotion inputs for E1/E2, not generated output of these policies. Missing,
conflicting, or stale source evidence assigns the specimen an ineligible
source disposition and prohibits powered work. Retail-kit listing establishes
curriculum scope only.

## Dependency, cadence, and promotion order

| Lesson | Public boundary | E0 ownership |
|---:|---|---|
| 079 | `BoundedLowSideDriverPolicy` | One declared low-side configuration, exact checked current-budget arithmetic, and bounded all-off/logical-drive intent |
| 080 | `SmallIndicatorSemanticsPolicy` | One declared low-energy indicator, copied drive and observation evidence, polarity/autonomy/safe-state semantics |
| 081 | `InertComponentQualificationBench` and record codec | One specimen/session, a fixed review/stimulus script, semantic presentation intent, and one canonical 256-byte volatile record |

Implement and terminally stress Lesson 079 before beginning Lesson 080.
Re-read this plan, the Lesson 079 stress result, and `docs/WORK_QUEUE.md` at
that boundary. Repeat after Lesson 080 and after Lesson 081 integration.
Each pass exercises the maximum authorized composition below. A new component
must add naturally: if it requires hidden ownership, a GPIO exception,
duplicate identity rules, relaxed arithmetic, new lifecycle conventions, or
changes to a published consumer, stop promotion and discuss a bounded
remediation. Do not repair architectural strain with optional flags.

The cadence is component, component, project-bearing. “Project-bearing” does
not waive the project gates: Lesson 081 must cover replayable success, every
state transition, timeout and modular rollover, invalid input, copied producer
failure, restart, and shutdown from every active state. Open E1/E2 cards do not
pause later safe host work. A failed source, correctness, architecture,
packaging, resource, lesson, or publication gate does.

## Shared representation, identity, ordering, and arithmetic

All externally encoded fields use fixed-width unsigned integers, explicit
enums, and little-endian byte order. `TimePoint` is supplied monotonic time,
not UTC. Durations and time deltas must be strictly below modular half range.
Zero duration means explicitly no wait is required; an unknown duration uses
an explicit declaration enum and canonical numeric zero.

For format version 1, every enum code is its zero-based declaration order in
this plan and is frozen even if the C++ declarations are later rearranged.
Encoded `StatusCode` values are likewise frozen as `Ok` through
`HardwareFailure` = 0 through 10 in the order declared by `src/status.h`;
values 11--255 are invalid. A later code requires a new record format rather
than silently changing version 1.

Every accepted value binds:

- schema revision;
- specimen family reference, specimen reference, and specimen revision;
- electrical-evidence revision and source-packet digest;
- policy configuration identity and revision;
- source identity and source-configuration revision;
- session, run, step, and request identity;
- sequence and supplied observation time; and
- complete producer status.

Zero is reserved for “not assigned” for every identity and revision. A valid
active transaction requires nonzero values. Reusing an identity with changed
bound fields is a conflict, never a new observation. Equal sequence, time,
and request identity are idempotent only for a field-wise identical value.
Changed duplicates, regressions, required gaps, future/backward time,
half-range ambiguity, identity drift, and capacity exhaustion reject
atomically.

All current quantities are integer microamps. Voltage quantities are integer
millivolts. Resistance quantities are integer ohms. Time is repository
`Duration`/`TimePoint`. There is no floating point. The following checked
operations are normative:

```text
ceilDiv(n, d) = n / d + (n % d != 0), only when d != 0
availableSupplyUa = supplyLimitUa - reservedSupplyUa
availableDeviceUa = min(deviceContinuousUa, fixtureLoadCeilingUa)
requestedBaseUa = ceilDiv(requestedLoadUa * forcedGainDenominator,
                          forcedGainNumerator)
lowerResistanceOhms =
    floor(baseResistanceOhms * (1000 - tolerancePermille) / 1000)
upperResistanceOhms =
    ceilDiv(baseResistanceOhms * (1000 + tolerancePermille), 1000)
maximumPossibleBaseUa =
    ceilDiv(logicHighMaximumMv * 1000, lowerResistanceOhms)
minimumBaseDropMv =
    max(0, logicHighMinimumMv - baseEmitterMaximumMv)
minimumAvailableBaseUa =
    floor(minimumBaseDropMv * 1000 / upperResistanceOhms)
basePathSafe = maximumPossibleBaseUa <= gpioSourceCeilingUa
admittedBaseUa = min(minimumAvailableBaseUa, gpioSourceCeilingUa)
admittedLoadUa = min(requestedLoadUa, availableSupplyUa,
                     availableDeviceUa,
                     floor(admittedBaseUa * forcedGainNumerator /
                           forcedGainDenominator))
```

`tolerancePermille` is `baseResistanceTolerancePermille` and must be
0--999. The lower bound deliberately rounds down; the upper bound deliberately
rounds up. Because the descriptor has no authenticated minimum base-emitter
drop, maximum possible base current conservatively uses zero millivolts for
that drop. The lower-resistance result must be nonzero and its maximum current
must not exceed the declared GPIO/base-path ceiling. The upper-resistance
result determines minimum guaranteed drive; `requestedBaseUa` must not exceed
`minimumAvailableBaseUa`, and only that minimum may support the forced-gain
load calculation.

Every multiplication, including `baseResistanceOhms * (1000 +/- tolerance)`
and each millivolt-to-microamp product, is promoted to `uint64_t`, checked
before use, and the
final value must fit `uint32_t`; subtraction checks its minuend first.
Divisors, forced-gain numerator/denominator, resistance, and required ceilings
must be nonzero. `logicHighMaximumMv` must be at least
`logicHighMinimumMv`; a zero minimum drop cannot admit an active request.
Rounding is deliberately conservative: required/maximum base current and upper
resistance round up; lower resistance, minimum available base current, and
supported load current round down. Saturation is not a
substitute for an overflow result. Overflow, underflow, divide by zero, an
unrepresentable result, or a requested load above any binding ceiling rejects
the request without mutation.

An all-off request has canonical requested load and base current zero.
`logicalActive == false` always produces all-off intent. No policy output
claims that a transistor switched, a load drew current, or a flyback device
clamped voltage.

## Shared failure precedence and atomicity

The first matching row controls. API-invalid input never becomes a domain
observation. An attributable producer or fixture failure is valid terminal
domain evidence and returns `Status::Ok`.

| Priority | Condition | API result | Domain disposition |
|---:|---|---|---|
| 1 | invalid enum encoding, malformed configuration, arithmetic error, or invalid image | `InvalidConfiguration` or `InvalidArgument` | no mutation |
| 2 | lifecycle, identity, revision, session, run, step, request, or correlation mismatch | `InvalidArgument` | no mutation |
| 3 | independent stop/cancel or shutdown | `Ok` | canonical all-off/cancelled result |
| 4 | copied producer/endpoint failure | `Ok` | `ProducerFault` |
| 5 | missing flyback declaration for a declared inductive load, unsafe budget, or ineligible source | `Ok` | named rejected result |
| 6 | sequence/time discontinuity or stale evidence | `Ok` | named discontinuity result |
| 7 | warm-up/settling or required observation incomplete | `Ok` | named incomplete result |
| 8 | polarity, safe-state, autonomous-waveform, or expected/observed disagreement | `Ok` | named semantic failure |
| 9 | complete matching evidence | `Ok` | accepted result |

Cancel and stop always remove drive authority and force the all-off path. If
that path is confirmed, cancellation is the terminal disposition and copied
producer status remains attributable. If an attributable producer or endpoint
failure prevents confirming off, or reports stuck/open/failure-to-return-safe,
terminal state is `Fault`; cancellation is retained as the causal reason and
cannot hide the failed safe return. Shutdown is idempotent and leaves all
published intent off. Every mutating operation stages a complete candidate;
failure leaves the object, output arguments, and caller-owned images
byte-for-byte unchanged.

## Lesson 079 -- bounded low-side driver policy

### Public values and API

```cpp
enum struct LowSideLoadEnergy : uint8_t
{
    ResistiveIndicator,
    InductiveInert
};

enum struct LowSideFlybackRequirement : uint8_t
{
    NotRequired,
    Required
};

enum struct LowSideFlybackDeclaration : uint8_t
{
    Absent,
    Present
};

enum struct LowSideDriveState : uint8_t
{
    Off,
    Requested,
    Rejected,
    Cancelled,
    Fault,
    Shutdown
};

enum struct LowSideDriveReason : uint8_t
{
    None,
    SourceIneligible,
    BudgetExceeded,
    BaseBudgetInsufficient,
    FlybackMissing,
    ProducerFault,
    SequenceDiscontinuity,
    TimestampDiscontinuity,
    Expired,
    CapacityExceeded,
    Cancelled
};

struct LowSideCurrentBudget
{
    uint32_t supplyLimitUa;
    uint32_t reservedSupplyUa;
    uint32_t fixtureLoadCeilingUa;
    uint32_t deviceContinuousUa;
    uint32_t gpioSourceCeilingUa;
    uint32_t forcedGainNumerator;
    uint32_t forcedGainDenominator;
    uint32_t baseResistanceOhms;
    uint16_t baseResistanceTolerancePermille;
    uint16_t logicHighMinimumMv;
    uint16_t logicHighMaximumMv;
    uint16_t baseEmitterMaximumMv;
    uint16_t collectorEmitterOperatingMaximumMv;
    Duration maximumActiveDuration;
    Duration dutyWindow;
    uint16_t maximumDutyPermille;
};

struct LowSideDriverDescriptor
{
    uint16_t schemaRevision;
    uint32_t specimenFamilyReference;
    uint32_t specimenReference;
    uint16_t specimenRevision;
    uint16_t electricalEvidenceRevision;
    uint32_t sourcePacketDigest;
    uint32_t configurationId;
    uint16_t configurationRevision;
    LowSideLoadEnergy loadEnergy;
    LowSideFlybackRequirement flybackRequirement;
    LowSideFlybackDeclaration flybackDeclaration;
    bool sourceEligible;
    uint32_t flybackDiodeIdentity;
    uint16_t flybackDiodeRevision;
    uint8_t flybackOrientationCode;
    uint8_t flybackReturnCode;
    uint32_t flybackRepetitiveReverseMv;
    uint32_t flybackForwardCurrentUa;
    LowSideCurrentBudget budget;
};

struct LowSideDriveRequest
{
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t requestId;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
    uint32_t lifecycleGeneration;
    bool logicalActive;
    uint32_t requestedLoadUa;
    Duration requestedActiveDuration;
    Status producerStatus;
};

struct LowSideControl
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t controlId;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
    bool offConfirmed;
    Status producerStatus;
};

struct LowSideDriveIntent
{
    uint32_t lifecycleGeneration;
    uint32_t driverDescriptorIdentityDigest;
    uint32_t specimenReference;
    uint16_t specimenRevision;
    uint16_t electricalEvidenceRevision;
    uint32_t policyConfigurationId;
    uint16_t policyConfigurationRevision;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t requestId;
    LowSideDriveState state;
    LowSideDriveReason reason;
    bool logicalActive;
    bool outputLevelHigh;
    uint32_t requiredBaseUa;
    uint32_t admittedBaseUa;
    uint32_t admittedLoadUa;
    TimePoint expiresAt;
    Status producerStatus;
};

struct BoundedLowSideDriverPolicy
{
    explicit BoundedLowSideDriverPolicy (
        const LowSideDriverDescriptor& descriptor) noexcept;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    bool   initialized () const noexcept;

    Status beginSession (uint32_t sessionId, uint32_t runId) noexcept;
    Status apply        (const LowSideDriveRequest& request,
                         LowSideDriveIntent& intent) noexcept;
    Status update       (TimePoint now,
                         LowSideDriveIntent& intent) noexcept;
    Status cancel       (const LowSideControl& control,
                         LowSideDriveIntent& intent) noexcept;
    Status reset        () noexcept;

    LowSideDriveIntent snapshot () const noexcept;
};

Status validateLowSideDriverDescriptor (
    const LowSideDriverDescriptor& descriptor) noexcept;
uint32_t lowSideDriverDescriptorIdentityDigest (
    const LowSideDriverDescriptor& descriptor) noexcept;
```

Construction is inert and stores a descriptor copy. `initialize()` validates
it and publishes canonical off intent; repeated initialization is idempotent.
`beginSession()` requires initialized/off state and fresh nonzero identities.
`apply()` requires exact descriptor/source/session correlation and contiguous
sequence after the first accepted request. Off requests remain valid with a
producer fault only when their canonical request fields are otherwise valid;
they publish fault attribution and off intent.

The sole topology is active-high bare NPN low-side drive; there is no polarity
Boolean and no active-low variant. An inductive declaration requires
`flybackRequirement == Required`, `flybackDeclaration == Present`, and
nonzero diode identity/revision, orientation, return, repetitive-reverse
voltage, and forward-current rating fields. A resistive indicator requires
`NotRequired`; every flyback field is canonical zero/`Absent`. E0 checks
declaration completeness and conservative rating arithmetic only; it does not
authenticate the diode or observe its placement or behavior.

`outputLevelHigh` is the logical level a future endpoint would be asked to
produce: active is high and off is low. This is intent,
not an observed pin state. Rejected, cancelled, fault, reset, and shutdown
always publish off. A new active request cannot replace an outstanding active
request; it must first receive explicit cancellation or a canonical off
request. Lesson 079 owns no pin, claim, endpoint, driver callback, clock,
supply, transistor, diode, or load.

An active request has `requestedActiveDuration` in
`[1, maximumActiveDuration]`; an off request uses canonical zero. Admission
sets `expiresAt = observedAt + requestedActiveDuration` after rejecting
half-range ambiguity or wrap-unsafe duration. `update(now)` takes supplied
time, rejects backward/half-range chronology atomically, and publishes
canonical off with `Expired` reason at equality or later; replaying a request
never extends its deadline.

Duty enforcement uses a fixed eight-entry ring of admitted active reservations
`[start, expiresAt)`. Before admission or update, entries whose end is at or
before `now - dutyWindow` are pruned. For each remaining entry, the overlap
with `[now - dutyWindow, now + requestedActiveDuration)` is summed in widened
ticks; the candidate is admitted only when
`reservedTicks * 1000 <= dutyWindow.ticks() * maximumDutyPermille`.
Equality passes and one tick/permille over rejects. Cancellation or an early
off request closes the current reservation at its supplied observation time;
the shortened interval remains in history. A ninth still-overlapping interval
is well-formed admitted domain evidence: the eight-entry reservation history
remains byte-for-byte unchanged, while policy snapshot and caller output are
atomically replaced by canonical off `Rejected/CapacityExceeded` intent for
that request. This is domain-result mutation, not API-rejection nonmutation.
`dutyWindow`, every duration, and every compared delta are nonzero
and strictly below modular half range; `maximumDutyPermille` is 1--1000.

The reservation ring and its last accepted supplied-time chronology belong to
the lifetime of the policy object, not to a session. Construction starts with
an empty ring and no chronology floor. `reset()` and `shutdown()` publish off
and invalidate session/request authority, but they neither clear nor shorten
reservations, reset the chronology floor, nor prune an entry.
`shutdown()` followed by `initialize()` on the same object preserves both
ring and floor. A new session may restart its session-local sequence at the
documented first value, but its first supplied time must still be forward or
equal under the preserved modular chronology. Because reset/shutdown have no
supplied time, an outstanding interval remains conservatively reserved through
its original `expiresAt`.

Pruning occurs only while processing a valid supplied-time `apply`, `update`,
or `cancel`, after chronology validation proves that time is forward and an
entry has expired relative to the duty window. API-invalid input, reset,
shutdown, initialization, and session start never prune. Destroying and
reconstructing the C++ object necessarily loses volatile E0 history; that is
not a reset mechanism, an authorization to drive, or evidence that a physical
cooldown elapsed. E0 history is deterministic policy accounting, not a
physical interlock. Any E2 endpoint must independently own and prove its
duration/duty/thermal limits, retained or fail-closed restart state, supplied
time authority, power-removal path, and exact-fixture evidence; it may never
treat a new E0 object or empty ring as hardware authorization.

### Deterministic matrix and files

Files: `src/bounded_low_side_driver_policy.h/.cpp`,
`tests/bounded_low_side_driver_policy_test.cpp`, Mega
`examples/Lesson079BoundedLowSideDriver/`, HTML API and lesson 079, and
PDF 079.

Tests exhaust:

- every enum encoding and every load/flyback cross-product;
- zero/max identities and revisions, identity drift, and changed duplicates;
- independent canonical descriptor-digest goldens plus one-field mutation of
  every descriptor field, explicitly specimen-family reference and
  source-packet digest; the independent test encoder starts with literal bytes
  `41 44 4B 37 39 44 53 43` and tests omitted/extra-NUL/wrong-tag variants;
- each zero divisor/ceiling, equal/one-below/one-above budgets, and every
  intermediate `uint32_t`/`uint64_t` overflow edge;
- tolerance 0/1/999 permille; nominal resistance 1 and `UINT32_MAX`; exact
  lower-bound floor and upper-bound ceil vectors; lower bound zero rejection;
  `R*(1000-t)` and `R*(1000+t)` widened boundaries; maximum-current ceil and
  minimum-drive floor division remainders; GPIO/base ceiling one-below/equal/
  one-above; minimum drive versus requested base one-below/equal/one-above;
  voltage product overflow guards and zero minimum-drop active rejection;
- exact conservative rounding vectors and independently computed goldens;
- requested duration zero/equality/one-over, deadline equality/one-before,
  supplied-time `update`, replay-without-extension, early off/cancel interval
  shortening, eight-entry duty history pruning, ninth-entry exhaustion, and
  widened duty equality/one-over at ordinary rollover;
- reset with a partially elapsed reservation, shutdown from active,
  reinitialize/new-session with preserved ring and time floor, backdated first
  evidence rejection, pruning only through supplied-time apply/update/cancel,
  and explicit construction-empty versus reconstructed-no-authority fixtures;
- active-high bare-NPN off-low and active-high intent, plus rejection of every
  alternate-polarity/topology encoding;
- resistive and declared-inductive configurations without implying hardware;
- source ineligibility, budget rejection, missing flyback, producer fault,
  stop/fault collision, cancellation, restart, shutdown, and idempotence;
- sequence/time first/equal/gap/regression/wrap/half-range cases; and
- output canaries and complete state non-mutation on every API rejection.

## Lesson 080 -- small indicator semantics policy

### Public values and API

```cpp
enum struct SmallIndicatorKind : uint8_t
{
    ActiveBuzzer,
    TrafficLight,
    DualColorLed,
    AutoFlashLed,
    VoltageIndicator
};

enum struct SmallIndicatorAutonomy : uint8_t
{
    FollowsDrive,
    AutonomousWhileEnabled,
    ObservationOnly
};

enum struct SmallIndicatorSafeState : uint8_t
{
    DriveInactive,
    HighImpedanceRequired,
    UnpoweredRequired
};

enum struct SmallIndicatorObservationState : uint8_t
{
    NotObserved,
    Inactive,
    Active,
    Alternating,
    Fault
};

struct SmallIndicatorChannels
{
    static constexpr uint8_t Red     = 0x01;
    static constexpr uint8_t Amber   = 0x02;
    static constexpr uint8_t Green   = 0x04;
    static constexpr uint8_t Blue    = 0x08;
    static constexpr uint8_t Sound   = 0x10;
    static constexpr uint8_t Voltage = 0x20;
};

enum struct SmallIndicatorDisposition : uint8_t
{
    Idle,
    Eligible,
    Incomplete,
    Accepted,
    Rejected,
    ProducerFault,
    Cancelled,
    Shutdown
};

enum struct SmallIndicatorReason : uint8_t
{
    None,
    SourceIneligible,
    DriveNotAdmitted,
    ProducerFault,
    Stale,
    WarmupUnsatisfied,
    SettlingUnsatisfied,
    PolarityMismatch,
    SafeStateMismatch,
    AutonomousWaveformMissing,
    UnexpectedAutonomy,
    ObservationMismatch,
    Cancelled
};

struct SmallIndicatorDescriptor
{
    uint16_t schemaRevision;
    uint32_t specimenFamilyReference;
    uint32_t specimenReference;
    uint16_t specimenRevision;
    uint16_t electricalEvidenceRevision;
    uint32_t sourcePacketDigest;
    uint32_t configurationId;
    uint16_t configurationRevision;
    uint32_t driverSpecimenReference;
    uint16_t driverSpecimenRevision;
    uint16_t driverElectricalEvidenceRevision;
    uint32_t driverPolicyConfigurationId;
    uint16_t driverPolicyConfigurationRevision;
    uint32_t expectedDriverDescriptorIdentityDigest;
    SmallIndicatorKind kind;
    SmallIndicatorAutonomy autonomy;
    SmallIndicatorSafeState safeState;
    bool sourceEligible;
    bool activeHigh;
    bool populatedResistorDeclared;
    bool populatedDriverDeclared;
    uint8_t declaredChannelMask;
    Duration warmup;
    Duration settling;
    Duration maximumObservationAge;
};

struct SmallIndicatorObservation
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t observationId;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
    SmallIndicatorObservationState state;
    uint8_t expectedActiveMask;
    uint8_t observedActiveMask;
    bool driveIntentActive;
    bool copiedLevelHigh;
    bool autonomousTransitionObserved;
    bool safeStateObserved;
    bool warmupSatisfied;
    bool settlingSatisfied;
    Status producerStatus;
};

struct SmallIndicatorSemanticResult
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t observationId;
    SmallIndicatorDisposition disposition;
    SmallIndicatorReason reason;
    SmallIndicatorObservationState observationState;
    bool semanticActive;
    uint8_t semanticActiveMask;
    bool safeStateSatisfied;
    bool autonomousBehaviorObserved;
    Status producerStatus;
};

struct SmallIndicatorControl
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t controlId;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
    bool offConfirmed;
    Status producerStatus;
};

struct SmallIndicatorSemanticsPolicy
{
    explicit SmallIndicatorSemanticsPolicy (
        const SmallIndicatorDescriptor& descriptor) noexcept;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    bool   initialized () const noexcept;

    Status beginSession (uint32_t sessionId, uint32_t runId,
                         TimePoint startedAt) noexcept;
    Status apply        (const LowSideDriveIntent& drive,
                         const SmallIndicatorObservation& observation,
                         TimePoint now,
                         SmallIndicatorSemanticResult& result) noexcept;
    Status cancel       (const SmallIndicatorControl& control,
                         SmallIndicatorSemanticResult& result) noexcept;
    Status reset        () noexcept;

    SmallIndicatorSemanticResult snapshot () const noexcept;
};

Status validateSmallIndicatorDescriptor (
    const SmallIndicatorDescriptor& descriptor) noexcept;
```

The public product is stateful and closed to exactly the five concrete
`SmallIndicatorKind` values above. It has no generic role/medium payload and
no open-ended module tag. Indicator kinds are semantic families, never
universal electrical adapters.

The validity table is exhaustive; every unlisted combination rejects. Mask
constants are `Red=0x01`, `Amber=0x02`, `Green=0x04`, `Blue=0x08`,
`Sound=0x10`, and `Voltage=0x20`; bits 6--7 are reserved zero. Population
columns are exact Booleans, not minimum requirements.

| Kind | Autonomy | Safe state | Active high | Channel mask | Resistor | Driver |
|---|---|---|---:|---:|---:|---:|
| `ActiveBuzzer` | `FollowsDrive` | `DriveInactive` | true | `Sound` | false | true |
| `ActiveBuzzer` | `AutonomousWhileEnabled` | `UnpoweredRequired` | true | `Sound` | false | true |
| `TrafficLight` | `FollowsDrive` | `DriveInactive` | true | `Red\|Amber\|Green` | true | false |
| `DualColorLed` | `FollowsDrive` | `DriveInactive` | true | `Red\|Green` | true | false |
| `AutoFlashLed` | `AutonomousWhileEnabled` | `UnpoweredRequired` | true | `Red\|Green\|Blue` | true | false |
| `VoltageIndicator` | `ObservationOnly` | `HighImpedanceRequired` | true | `Voltage` | true | false |

`expectedActiveMask`, `observedActiveMask`, and `semanticActiveMask` must be
subsets of `declaredChannelMask`; inactive evidence uses zero. A follows-drive
observation must match the expected mask exactly. Autonomous evidence may
change among nonzero declared subsets but requires
`autonomousTransitionObserved`; observation-only evidence requires expected
mask zero and never gains drive authority. This bounded mask representation
preserves Traffic Light and Dual Color LED color semantics without allocating,
inventing per-channel endpoints, or generalizing the five kinds.

`NotObserved` and `Fault` observations use canonical zero expected and
observed masks. `Inactive` uses observed mask zero. `Active` uses exactly one
nonzero declared subset; `Alternating` uses a nonzero declared subset and is
valid only for autonomous kinds with transition evidence. `copiedLevelHigh`
is authoritative only for the single-channel `ActiveBuzzer` and
`VoltageIndicator` rows and must equal `(observedActiveMask != 0)` because all
six rows are active-high. For every multi-channel row it is canonical false;
the bounded masks are the sole color authority.
The descriptor does not infer pin count or topology. Traffic-light and
dual-color semantics require the exact specimen descriptor to say whether
resistors/drivers are populated, but E0 does not fill in missing values.
`VoltageIndicator` is an observation-only indicator; it cannot receive active
drive authority. `AutoFlashLed` must declare
`AutonomousWhileEnabled`. `ActiveBuzzer` may be `FollowsDrive` or
`AutonomousWhileEnabled` only in the two rows frozen above. The plan table is
the sole validity authority and is exhaustively tested; no implementation-only
row or permissive default exists.

The policy copies both the Lesson 079 intent and an observation. It never
calls Lesson 079, toggles a line, schedules a waveform, synthesizes a tone, or
claims optical/acoustic output. An autonomous transition is copied evidence:
its cadence, frequency, duty, loudness, color, and intensity remain outside
E0. `semanticActive` follows the descriptor polarity only for a
`FollowsDrive` specimen; for autonomous and observation-only specimens it is
derived from the explicit observation state. Missing evidence remains
incomplete, never inactive.

`lowSideDriverDescriptorIdentityDigest()` is CRC-32/ISO-HDLC over exactly the
eight ASCII domain-tag bytes `ADK79DSC` followed immediately by the canonical
little-endian encoding of every `LowSideDriverDescriptor` field in declaration
order. The tag length is exactly 8 and its bytes are
`41 44 4B 37 39 44 53 43` hexadecimal; there is no NUL terminator, encoded
length, or separator byte. CRC parameters are polynomial `0x04C11DB7`,
reflected input/output, initial/final XOR `0xFFFFFFFF` (reflected
implementation polynomial `0xEDB88320`). The descriptor encoding includes
schema, specimen-family/specimen
identity and revisions, electrical-evidence revision, source-packet digest,
policy configuration identity/revision, topology/protection fields, source
eligibility, every diode field, and every budget/voltage/tolerance/duration/
duty field. Zero is a valid digest. Construction computes it once from the
validated immutable descriptor and every `LowSideDriveIntent` copies it.

Standalone Lesson 080 requires
`drive.driverDescriptorIdentityDigest ==
descriptor.expectedDriverDescriptorIdentityDigest` before interpreting any
drive value. The explicit expected specimen/reference/electrical/configuration
fields remain useful inspectable correlation and must also match, but they
never substitute for the full digest. It then matches lifecycle, session, run, step, and
request/observation correlation; Lesson 081 adds pair/envelope correlation.
`now` must not precede `observedAt`; age equal to
`maximumObservationAge` is current and one tick greater is stale. Known-zero
warm-up/settling requires no delay. Safe state is a distinct observation from
resource/drive admission; one boolean never proves both.

### Deterministic matrix and files

Files: `src/small_indicator_semantics_policy.h/.cpp`,
`tests/small_indicator_semantics_policy_test.cpp`, Mega
`examples/Lesson080SmallIndicatorSemantics/`, HTML API and lesson 080, and PDF
080.

Tests exhaust:

- every kind/autonomy/safe-state/active-polarity/population/channel-mask
  cross-product, including every reserved mask bit and every color mismatch;
- invalid encodings and zero/max identities/revisions/durations;
- each one-field drive-intent specimen reference/revision,
  electrical-evidence, policy
  configuration identity/revision mismatch against the immutable L080
  expected-driver fields;
- actual/expected full driver-descriptor digest equality, zero-valid digest,
  and one-field family/source-packet drift that preserves the partial fields
  but must reject through the full digest;
- all five kinds with explicit, non-generalized fixtures;
- copied off/active/rejected/cancelled/fault drive intents;
- inactive/active/alternating/fault observations, canonical NotObserved/Fault
  zero masks, single-channel copied-level meaning, and multichannel canonical
  false copied level;
- missing/unexpected autonomous transitions;
- separate admission, warm-up, settling, safe-state, and semantic checks;
- exact freshness boundary, time wrap, half-range ambiguity, sequences,
  duplicates, gaps, and source/configuration drift;
- simultaneous stop, producer, stale, and semantic failures at each
  precedence boundary; and
- restart, shutdown from each state, canaries, and atomic non-mutation.

## Lesson 081 -- inert component qualification bench

### Script, envelope, presentation, and lifecycle

The project evaluates one descriptor pair for one entire session. It does not
select among specimens and cannot change identity mid-session.

```cpp
enum struct ComponentQualificationStep : uint8_t
{
    ReviewSource,
    ConfirmInactive,
    RequestBoundedIntent,
    ObserveResponse,
    ConfirmSafeState
};

enum struct ComponentQualificationState : uint8_t
{
    Idle,
    Running,
    Complete,
    Rejected,
    Cancelled,
    Fault,
    Shutdown
};

enum struct ComponentQualificationReason : uint8_t
{
    None,
    SourceIneligible,
    StepMismatch,
    DriverRejected,
    IndicatorRejected,
    ProducerFault,
    SequenceDiscontinuity,
    TimestampDiscontinuity,
    ObservationIncomplete,
    SafeStateUnproved,
    Cancelled
};

enum struct ComponentQualificationPresentationCell : uint8_t
{
    Off,
    Pending,
    Pass,
    Reject,
    Fault
};

enum struct ComponentQualificationControlAction : uint8_t
{
    Advance,
    Cancel
};

struct ComponentQualificationControl
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint16_t stepId;
    uint32_t controlId;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
    ComponentQualificationControlAction action;
    bool offConfirmed;
    Status producerStatus;
};

struct ComponentQualificationEnvelope
{
    LowSideDriverDescriptor driverDescriptor;
    SmallIndicatorDescriptor indicatorDescriptor;
    ComponentQualificationControl control;
    LowSideDriveRequest driveRequest;
    SmallIndicatorObservation indicatorObservation;
};

struct ComponentQualificationSnapshot
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint32_t qualificationSequence;
    ComponentQualificationState state;
    ComponentQualificationStep step;
    ComponentQualificationReason reason;
    LowSideDriveIntent drive;
    SmallIndicatorSemanticResult indicator;
    ComponentQualificationPresentationCell sourceCell;
    ComponentQualificationPresentationCell driveCell;
    ComponentQualificationPresentationCell observationCell;
    ComponentQualificationPresentationCell safeStateCell;
};

struct InertComponentQualificationBench
{
    InertComponentQualificationBench (
        const LowSideDriverDescriptor& driverDescriptor,
        const SmallIndicatorDescriptor& indicatorDescriptor) noexcept;

    Status initialize  () noexcept;
    void   shutdown    () noexcept;
    bool   initialized () const noexcept;

    Status beginSession (uint32_t sessionId, uint32_t runId,
                         TimePoint startedAt) noexcept;
    Status apply        (const ComponentQualificationEnvelope& envelope,
                         TimePoint now,
                         ComponentQualificationSnapshot& snapshot) noexcept;
    Status cancel       (const ComponentQualificationControl& control,
                         ComponentQualificationSnapshot& snapshot) noexcept;
    Status prepareRecord (uint8_t* image, size_t imageSize) const noexcept;
    Status reset         () noexcept;

    ComponentQualificationSnapshot snapshot () const noexcept;
};
```

The exact five-step order is fixed. Record preparation is outside the script,
is legal only after a terminal state, and never creates a sixth step. A
control advances at most one step.
Steps never share authority implicitly:

1. `ReviewSource` requires matching, source-eligible descriptors and immutable
   source-packet digests.
2. `ConfirmInactive` requires canonical off intent plus a separately copied
   inactive/safe-state observation.
3. `RequestBoundedIntent` evaluates one bounded Lesson 079 request; it does
   not assert physical application.
4. `ObserveResponse` evaluates the correlated Lesson 080 observation.
5. `ConfirmSafeState` requires cancellation/off intent and a new,
   later safe-state observation before terminal acceptance.

For an `Advance` control, `offConfirmed` is canonically false except at
`ConfirmInactive` and `ConfirmSafeState`, where it must equal the correlated
safe-state observation. For `Cancel`, it reports the attributable result of
the forced all-off path under the same lifecycle/session/source/sequence/time
correlation as the control. False plus a non-OK producer status is terminal
`Fault`; true is `Cancelled`; false with OK status is also `Fault` because an
unconfirmed off state is never treated as safe. The bench copies these fields
into the two child control values without inventing evidence.

A terminal rejection, cancellation, or fault cannot later become accepted.
`prepareRecord()` is permitted only for a terminal state, does not mutate the
bench, and writes exactly one 256-byte volatile image. It is not persistence
and has no interrupted-write guarantee. The method rejects a null pointer or
any span whose length is not exactly 256 and leaves the caller buffer
unchanged. The implementation first builds a local candidate, then copies it.

### Canonical 256-byte record

The image is a versioned compact summary, not a serialized C++ object. No
padding, ABI layout, `Status` representation, or enum underlying value is
copied implicitly. All reserved bytes are zero. Boolean bytes are exactly
zero or one. Enum codes are frozen by the codec and validated independently
of C++ declaration order.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 4 | magic ASCII `A`,`D`,`Q`,`C` |
| 4 | 2 | record format version, `1` |
| 6 | 2 | encoded length, `256` |
| 8 | 2 | driver schema revision |
| 10 | 2 | indicator schema revision |
| 12 | 4 | lifecycle generation |
| 16 | 4 | session identity |
| 20 | 4 | run identity |
| 24 | 4 | qualification sequence |
| 28 | 4 | terminal control identity |
| 32 | 4 | terminal control sequence |
| 36 | 4 | terminal control observation ticks |
| 40 | 1 | terminal control source identity |
| 41 | 2 | terminal control source-configuration revision |
| 43 | 1 | terminal control action in bit 7 and producer-status code in bits 0--3; bits 4--6 zero |
| 44 | 4 | driver specimen-family reference |
| 48 | 4 | driver specimen reference |
| 52 | 2 | driver specimen revision |
| 54 | 2 | driver electrical-evidence revision |
| 56 | 4 | driver source-packet digest |
| 60 | 4 | driver configuration identity |
| 64 | 2 | driver configuration revision |
| 66 | 4 | indicator specimen-family reference |
| 70 | 4 | indicator specimen reference |
| 74 | 2 | indicator specimen revision |
| 76 | 2 | indicator electrical-evidence revision |
| 78 | 4 | indicator source-packet digest |
| 82 | 4 | indicator configuration identity |
| 86 | 2 | indicator configuration revision |
| 88 | 1 | load-energy code |
| 89 | 1 | bit 0 flyback required, bit 1 flyback present; bits 2--7 reserved zero |
| 90 | 1 | indicator-kind code |
| 91 | 1 | bits 0--1 autonomy code, bits 2--3 safe-state code; bits 4--7 reserved zero |
| 92 | 4 | requested load, microamps |
| 96 | 4 | required base current, microamps |
| 100 | 4 | admitted base current, microamps |
| 104 | 4 | admitted load current, microamps |
| 108 | 4 | maximum active-duration ticks |
| 112 | 4 | duty-window ticks |
| 116 | 2 | maximum duty, permille |
| 118 | 2 | base-resistance tolerance, permille |
| 120 | 2 | logic-high minimum, millivolts |
| 122 | 2 | logic-high maximum, millivolts |
| 124 | 2 | base-emitter maximum, millivolts |
| 126 | 2 | collector-emitter operating maximum, millivolts |
| 128 | 4 | diode identity |
| 132 | 2 | diode revision |
| 134 | 1 | diode orientation code |
| 135 | 1 | diode return code |
| 136 | 4 | diode repetitive-reverse rating, millivolts |
| 140 | 4 | diode forward-current rating, microamps |
| 144 | 4 | terminal request identity |
| 148 | 4 | terminal request sequence |
| 152 | 4 | terminal request observation ticks |
| 156 | 4 | terminal observation identity |
| 160 | 4 | terminal observation sequence |
| 164 | 4 | terminal observation ticks |
| 168 | 1 | qualification-state code |
| 169 | 1 | qualification-step code |
| 170 | 1 | qualification-reason code |
| 171 | 1 | drive-state code |
| 172 | 1 | drive-reason code |
| 173 | 1 | drive flags: bit 0 logical active, bit 1 output high; bits 2--7 reserved zero |
| 174 | 1 | drive producer-status code |
| 175 | 1 | copied observation-state code |
| 176 | 1 | semantic-disposition code |
| 177 | 1 | semantic-reason code |
| 178 | 1 | bit 0 semantic active, bit 1 safe state satisfied, bit 2 autonomous transition observed; bits 3--7 reserved zero |
| 179 | 1 | observation producer-status code |
| 180 | 1 | source presentation-cell code |
| 181 | 1 | drive presentation-cell code |
| 182 | 1 | observation presentation-cell code |
| 183 | 1 | safe-state presentation-cell code |
| 184 | 4 | actual canonical full driver-descriptor identity digest |
| 188 | 4 | indicator descriptor digest |
| 192 | 4 | driver-evidence digest |
| 196 | 4 | indicator-evidence digest |
| 200 | 4 | complete-envelope evidence digest |
| 204 | 4 | fixed five-step script digest |
| 208 | 4 | source/configuration identity digest |
| 212 | 4 | electrical/energy declaration digest |
| 216 | 4 | record sequence, fixed `1` for format version 1 |
| 220 | 2 | terminal control step identity |
| 222 | 4 | requested active-duration ticks |
| 226 | 4 | terminal drive expiry ticks |
| 230 | 1 | declared indicator-channel mask |
| 231 | 1 | terminal expected-active mask |
| 232 | 1 | terminal observed-active mask |
| 233 | 1 | terminal semantic-active mask |
| 234 | 4 | expected driver-descriptor identity digest from indicator configuration |
| 238 | 14 | reserved, all zero |
| 252 | 4 | CRC-32/ISO-HDLC over bytes 0--251 |

All eight domain digests use CRC-32/ISO-HDLC (`poly 0x04C11DB7`, reflected input and
output, initial and final XOR `0xFFFFFFFF`; reflected implementation polynomial
`0xEDB88320`). Digest zero is valid and has no sentinel meaning. The record
CRC uses the same algorithm but is a distinct field and domain. Golden vectors
must be produced by an independent test-side implementation.
The expected digest at offset 234 is a copied L080 configuration value in the
same driver-descriptor digest domain, not a ninth independently recomputed
domain.

The compact image intentionally omits complete intermediate envelopes,
duplicate/failure candidates, intermediate snapshots, raw C++ `Status`
representations, full descriptor fields already bound by the descriptor
digests, and full evidence fields already bound by the domain digests. The
fixed topology derives active-high and off-low rather than encoding polarity.
Offsets 48, 52, 54, 60, and 64 carry the driver specimen/revision,
electrical-evidence revision, and policy-configuration identity/revision
copied into terminal drive intent; the indicator descriptor digest binds its
five expected-driver counterparts,
offset 184 carries the actual canonical full driver-descriptor identity
digest, and offset 234 carries the indicator configuration's expected digest.
The full replay verifier recomputes the actual digest from every descriptor
field and proves equality; the compact decoder treats both digests as opaque.
Cancel-time `offConfirmed` is derived only as true for a cancelled terminal
record whose drive is off and safe-state flag is true; a fault terminal record
retains cancellation cause without claiming confirmation.
Record existence and preparation state are external and are neither encoded
nor part of terminal semantic state. The compact decoder treats every
descriptor, source/configuration, electrical, script, and evidence digest as
opaque: it validates width only, and zero is valid. Compact fields are
insufficient to recompute any domain digest. A decoder without the full replay
must not fabricate verification, reconstruct omitted evidence, or claim
durable provenance.

Codec APIs are:

```cpp
enum struct ComponentQualificationRecordDecode : uint8_t
{
    Ok,
    BadLength,
    BadFraming,
    BadIntegrity,
    BadSemanticValue
};

struct ComponentQualificationRecord
{
    uint16_t driverSchemaRevision;
    uint16_t indicatorSchemaRevision;
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint32_t qualificationSequence;
    uint32_t terminalControlId;
    uint16_t terminalControlStepId;
    uint32_t terminalControlSequence;
    TimePoint terminalControlObservedAt;
    uint8_t terminalControlSourceId;
    uint16_t terminalControlSourceConfigurationRevision;
    ComponentQualificationControlAction terminalControlAction;
    Status terminalControlProducerStatus;
    uint32_t driverSpecimenFamilyReference;
    uint32_t driverSpecimenReference;
    uint16_t driverSpecimenRevision;
    uint16_t driverElectricalEvidenceRevision;
    uint32_t driverSourcePacketDigest;
    uint32_t driverConfigurationId;
    uint16_t driverConfigurationRevision;
    uint32_t indicatorSpecimenFamilyReference;
    uint32_t indicatorSpecimenReference;
    uint16_t indicatorSpecimenRevision;
    uint16_t indicatorElectricalEvidenceRevision;
    uint32_t indicatorSourcePacketDigest;
    uint32_t indicatorConfigurationId;
    uint16_t indicatorConfigurationRevision;
    LowSideLoadEnergy loadEnergy;
    LowSideFlybackRequirement flybackRequirement;
    LowSideFlybackDeclaration flybackDeclaration;
    SmallIndicatorKind indicatorKind;
    SmallIndicatorAutonomy indicatorAutonomy;
    SmallIndicatorSafeState indicatorSafeState;
    uint32_t requestedLoadUa;
    Duration requestedActiveDuration;
    uint32_t requiredBaseUa;
    uint32_t admittedBaseUa;
    uint32_t admittedLoadUa;
    TimePoint driveExpiresAt;
    Duration maximumActiveDuration;
    Duration dutyWindow;
    uint16_t maximumDutyPermille;
    uint16_t baseResistanceTolerancePermille;
    uint16_t logicHighMinimumMv;
    uint16_t logicHighMaximumMv;
    uint16_t baseEmitterMaximumMv;
    uint16_t collectorEmitterOperatingMaximumMv;
    uint32_t flybackDiodeIdentity;
    uint16_t flybackDiodeRevision;
    uint8_t flybackOrientationCode;
    uint8_t flybackReturnCode;
    uint32_t flybackRepetitiveReverseMv;
    uint32_t flybackForwardCurrentUa;
    uint32_t terminalRequestId;
    uint32_t terminalRequestSequence;
    TimePoint terminalRequestObservedAt;
    uint32_t terminalObservationId;
    uint32_t terminalObservationSequence;
    TimePoint terminalObservationObservedAt;
    ComponentQualificationState qualificationState;
    ComponentQualificationStep qualificationStep;
    ComponentQualificationReason qualificationReason;
    LowSideDriveState driveState;
    LowSideDriveReason driveReason;
    bool driveLogicalActive;
    bool driveOutputHigh;
    Status driveProducerStatus;
    SmallIndicatorObservationState observationState;
    SmallIndicatorDisposition semanticDisposition;
    SmallIndicatorReason semanticReason;
    bool semanticActive;
    bool safeStateSatisfied;
    bool autonomousTransitionObserved;
    uint8_t declaredChannelMask;
    uint8_t expectedActiveMask;
    uint8_t observedActiveMask;
    uint8_t semanticActiveMask;
    uint32_t expectedDriverDescriptorIdentityDigest;
    Status observationProducerStatus;
    ComponentQualificationPresentationCell sourceCell;
    ComponentQualificationPresentationCell driveCell;
    ComponentQualificationPresentationCell observationCell;
    ComponentQualificationPresentationCell safeStateCell;
    uint32_t driverDescriptorIdentityDigest;
    uint32_t indicatorDescriptorDigest;
    uint32_t driverEvidenceDigest;
    uint32_t indicatorEvidenceDigest;
    uint32_t envelopeEvidenceDigest;
    uint32_t scriptDigest;
    uint32_t sourceConfigurationDigest;
    uint32_t electricalDeclarationDigest;
    uint32_t recordSequence;
};

struct ComponentQualificationReplay
{
    uint32_t acceptedEnvelopeCount;
    ComponentQualificationEnvelope acceptedEnvelopes[5];
    ComponentQualificationSnapshot acceptedSnapshots[5];
};

Status encodeComponentQualificationRecord (
    const ComponentQualificationRecord& record,
    const ComponentQualificationReplay& replay,
    uint8_t* image,
    size_t imageSize) noexcept;
Result<ComponentQualificationRecordDecode>
decodeComponentQualificationRecord (
    const uint8_t* image,
    size_t imageSize,
    ComponentQualificationRecord& record) noexcept;
struct ComponentQualificationReplayVerifier
{
    static Status verify (
        const ComponentQualificationRecord& record,
        const ComponentQualificationReplay& replay) noexcept;
};
```

`ComponentQualificationReplayVerifier` validates the complete five-step
replay, all descriptor/source/electrical identities, Lesson 079 widened
arithmetic and duty reservations, Lesson 080 channel masks and validity table,
chronology, correlations, and terminal semantics; it then independently
recomputes all eight domain digests and compares them with the compact record.
It also compares recomputed actual driver-descriptor identity to the copied
expected value at offset 234.
Encoding always invokes this full verifier before staging any byte. Thus a
record cannot be encoded from compact fields alone, and neither encode nor
decode promises compact digest recomputation.

Decode precedence is exact: span not 256 is `BadLength`; bad magic, format
version, or encoded length is `BadFraming`; a CRC mismatch is `BadIntegrity`;
CRC-valid invalid enums, flags, reserved bytes, locally encoded cross-fields,
identities, or terminal semantics are `BadSemanticValue`; digest bytes remain
opaque and locally plausible arithmetic summaries are range-checked without
recomputing full declaration arithmetic; otherwise
`Ok`. Decode failure leaves the output unchanged.

### Deterministic matrix and files

Files: `src/inert_component_qualification_bench.h/.cpp`,
`src/component_qualification_record.h/.cpp`,
`src/component_qualification_digest.h/.cpp`,
`tests/inert_component_qualification_bench_test.cpp`, Mega
`examples/Lesson081InertComponentQualificationBench/`, HTML API and lesson
081, and PDF 081.

Tests include:

- the exact five-step happy path and byte-identical replay;
- every state transition, premature/duplicate/skipped control, every terminal
  state, cancel at every active step, restart, reset, and shutdown;
- child duty-history persistence across bench reset and
  shutdown/reinitialize, including rejection when the new session would exceed
  retained duty and pruning only after a valid supplied-time envelope/control;
- descriptor cross-identity, revision, source-digest, configuration, session,
  run, step, request, sequence, and timestamp mismatches;
- collisions among cancel, producer fault, budget, flyback, freshness,
  autonomy, polarity, observation, and safe-state failures;
- exact time boundaries, wrap, half-range ambiguity, and stale/future data;
- null/255/256/257-byte record spans, canaries, and output atomicity;
- one fixed 256-byte golden vector, decode/encode round trip, every-byte
  corruption, and CRC-repaired invalid magic, enums, flags, reserved bytes,
  identities, locally checkable arithmetic ranges, and semantic cross-fields;
- valid zero digests and domain separation among descriptor, evidence,
  expected-driver descriptor, script, and record CRC calculations; and
- arbitrary digest changes with a recomputed record CRC, which remain
  decode-valid opaque values, followed by
  `ComponentQualificationReplayVerifier` rejection against the full replay;
  no digest is fabricated or recomputed by the compact decoder.

## Maximum authorized composition and resource gates

The Lesson 081 maximum resource probe composes, simultaneously:

- one `InertComponentQualificationBench` object graph containing its one
  production `BoundedLowSideDriverPolicy` child and one production
  `SmallIndicatorSemanticsPolicy` child;
- two 256-byte record images (candidate plus published);
- one full input envelope and one output snapshot;
- one maximum Lesson 081 example presentation state;
- the ordinary Mega runtime/static baseline; and
- the repository ISR reserve and conservative synchronous stack chain.

Standalone Lesson 079 and 080 policy objects appear only in their isolated
lesson probes and `sizeof` reports; they are never live additions to the
Lesson 081 maximum graph. The exact no-LTO fingerprint binds compiler path/version,
flags, source closure and hashes, probe source, configuration, budget table,
and schema/layout versions. A stale fingerprint is a failed gate.

Initial targets and hard limits are:

| Boundary | Flash target / hard | Static SRAM target / hard | Stack target / hard | Object/value target / hard |
|---|---:|---:|---:|---:|
| 079 | 10 / 14 KiB | 768 / 1,024 B | 320 / 448 B | policy 192 / 256 B; descriptor 96 / 128 B |
| 080 | 14 / 20 KiB | 1,024 / 1,536 B | 448 / 640 B | policy 384 / 512 B; evidence 256 / 384 B |
| 081 maximum composition | 32 / 40 KiB | 3,072 / 4,096 B | 1,024 / 1,280 B | coordinator 768 / 1,024 B; one/two images exactly 256/512 B |

Residual SRAM target/hard for Lesson 081 is 4,096/3,072 B. Storage is counted
once by actual lifetime: measured static SRAM already includes globals;
coordinator and retained children are one object graph; caller envelope,
snapshot, and decode output count only while live; the private staged image
and caller destination overlap during encoding and therefore count as exactly
512 B; mutually exclusive phase locals may be compacted only with call-graph
evidence. Residual is `8192 - measured static - conservative synchronous
stack - 128-byte ISR reserve`; do not subtract object/image values again when
they are already in static or stack measurement. A target miss below every
hard limit requires an independent review record tied to the exact
fingerprint; a hard or residual-hard miss blocks promotion. Ordinary Arduino
size and exact no-LTO evidence are both recorded and labeled; neither
substitutes for the other.

The promoted Lesson 079 exact no-LTO boundary measures 12,974 bytes flash,
561 bytes static SRAM, 320 bytes conservative synchronous stack, a 240-byte
policy, and a 96-byte descriptor, leaving 7,183 bytes residual SRAM. Its
fingerprint is
`4972bd9733d608d52ac81bfb1320e61088b10c3e59910f3be8c439b5838d33e7`.
The flash and policy-size target misses have independent, fingerprint-bound
reviews below their hard limits; all other Lesson 079 targets pass.

## Teaching narrative and observation honesty

The examples are compile-only deterministic replays. They declare a fictional,
explicitly synthetic specimen; review its source eligibility; predict the
logical off, bounded intent, semantic observation, and restored-safe results;
then replay copied observations and inspect volatile presentation cells and
record bytes. `setup()` reads acquire policy, configure the replay, start the
session. `loop()` reads observe copied evidence, decide the next qualification
step, publish inert intent.

The examples never name Arduino pins, draw a wiring diagram, instantiate
`DigitalOutput`, or imply that a load was powered. Their non-Serial observation
path is semantic presentation intent with four named cells: Source, Drive,
Observation, and Safe State. At E0 these are caller-inspectable values and
volatile result cells, not physical lights. Serial may explain them but cannot
turn them into physical evidence.

Each lesson uses predict, observe, interpret:

- Lesson 079 predicts off/budget/rejection intent, observes the returned
  logical intent, and interprets only arithmetic and policy state.
- Lesson 080 predicts polarity/autonomy/safe-state semantics, observes copied
  evidence and the semantic result, and interprets only their consistency.
- Lesson 081 predicts the next fixed step and terminal record, observes the
  four semantic cells and image, and interprets replay determinism.

Future E2 lessons/cards must add a real non-Serial path and named electrical
test points for source power, base/gate, collector/load, rail, flyback/clamp,
and safe state. Resource-acquisition evidence and electrical safe-state
evidence remain separate.

## HTML, PDF, and publication package

Each lesson lands as one coherent dependency boundary with:

- declarative public header, out-of-line implementation, aggregate-header
  export, deterministic host target, and compile-alone header check;
- one Mega 2560 replay example and ordinary plus exact resource evidence;
- HTML API reference and lesson page linking source, implementation, example,
  tests, governing plan, stress pass, resource evidence, and open cards;
- a rich PDF with learner goal, vocabulary, synthetic fixture table,
  arithmetic or state-machine walkthrough, predict/observe/interpret
  experiment, diagnosis, exercises, and acceptance record; and
- shared curriculum, course, project, lesson, component, API, download, and
  work-queue indexes updated only at promotion.

Every PDF visual is a pencil drawing unless it is an explicitly identified,
electrically authoritative formal schematic. E0 has no circuit and therefore
must not include a breadboard layout or authoritative schematic. Appropriate
visuals are pencil-drawn arithmetic worksheets, evidence cards, state
machines, record-byte maps, and a clearly fictional inert desk. Each asset
uses the classification markers and gates required by `docs/PDF_POLICY.md`;
grayscale or a filename alone does not establish compliance.

HTML and PDF state prominently that E0 owns no GPIO, supply, transistor,
diode, load, indicator, sound, light, display, or persistent storage.
“Accepted” means the copied replay satisfies this policy, not that a specimen
is supported. Record existence is external caller state, not an encoded bench
semantic.

Promotion order is source and style, host correctness, architecture stress,
ordinary Arduino build and size, exact resource/fingerprint review, lesson and
PDF gates, site gates, shared indexes, then the consuming lesson. Generated
PDFs accompany their sources. No draft path, local PDF, passing replay, E1
inventory record, or E2 card alone grants publication or changes support
claims.

## Initial and terminal architecture stress requirements

Before public shape is fixed and again before each lesson is promoted, record
the repository stress-pass template and explicitly disposition:

- pure-policy layer placement and the absence of GPIO/resource ownership;
- descriptor authority, exact-specimen identity, and source revision drift;
- checked arithmetic, rounding, overflow, all-off rollback, and flyback
  declaration limits;
- lifecycle, ordering, time, duplicate, cancellation, terminal, and status
  precedence;
- Lesson 079/080 identity and correlation without a shared abstraction
  invented for only one composition;
- autonomous indicator observation without scheduler, waveform, acoustic, or
  optical claims;
- safe-state evidence distinct from admission and producer status;
- 256-byte codec canonicality, compact omissions, digest domains, corruption,
  caller-buffer atomicity, and non-persistence wording;
- maximum simultaneous Mega composition and exact fingerprint review;
- example narrative, vocabulary parity, circuit-native E0 semantic
  observability, HTML/PDF honesty, pencil-visual classification, and canonical
  publication isolation; and
- downstream E2 endpoint pressure without pre-authorizing its API.

A terminal pass must cite measured rather than proposed resources and the
final record vector/fingerprint. Any need to add a GPIO owner, alter existing
endpoint lifetimes, generalize three-pin modules, weaken exact identity,
reinterpret autonomous behavior, or revise several published consumers is
architectural strain and requires user discussion before remediation.

## Promotion checklist

Lesson 079 implementation and publication artifacts are assembled; its final
promotion repair and gate reconciliation remain in progress. Lessons 080--081
remain queued and unimplemented. A checked planning item records completed
design work, not implementation or physical acceptance:

- [x] initial Lesson 079 architecture stress pass;
- [ ] Lesson 079 code, exhaustive arithmetic/lifecycle tests, Mega replay,
      HTML/PDF, exact resources, and terminal stress pass;
- [ ] boundary design pass and `docs/WORK_QUEUE.md` re-read;
- [x] initial Lesson 080 architecture stress pass;
- [ ] Lesson 080 code, exhaustive semantics tests, Mega replay, HTML/PDF,
      exact resources, and terminal stress pass;
- [ ] boundary design pass and `docs/WORK_QUEUE.md` re-read;
- [x] initial Lesson 081 architecture stress pass;
- [ ] Lesson 081 composition, exact 256-byte codec/vector/corruption tests,
      Mega replay, HTML/PDF, exact resources, and terminal stress pass;
- [ ] maximum-composition and fingerprint review;
- [ ] source, style, headers, host, Arduino, lesson, PDF, site, packaging, and
      canonical-publication gates;
- [ ] shared indexes and work queue updated in the same promotion boundary;
- [ ] E1 and E2 cards remain visibly open and no physical claim is made.
