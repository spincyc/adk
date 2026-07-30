# Lessons 070--072 module characterization plan

Status: controlling implementation-depth E0 plan. Exact specimens, powered
acquisition, physical stimulus, presentation endpoints, persistence, and bench
acceptance remain open.

This arc declares one fixture contract, streams copied characterization
evidence, and prepares one inert volatile record. E0 owns no pin, ADC,
comparator input, timer, interrupt, power switch, module, stimulus, display,
control, clock, or storage endpoint. A declared fixture is not an identified
module. Descriptor millivolts and topology are declarations to validate copied
data, never authority to apply power.

## Evidence levels and exclusions

| Level | Authorized work |
|---|---|
| E0 | Stateless descriptor/frame validation, copied attributed points, streaming bounded legs, immutable envelope consumption, inert script/presentation intent, and one caller-owned volatile image |
| E1a | One exact low-voltage specimen and acquisition fixture: markings, primary device documentation plus an authoritative module schematic, or the documentation plus a reviewed trace of the populated module when no schematic exists; pins, voltages, topology, pull, stimulus, current, test points, power removal, and acceptance |
| E1b | Exact display/indicator/control endpoints with ownership, current, self-test, fault dominance, rollback, and independent acceptance |

Durable storage is outside this arc; there is no E1c persistence card here.
`recordPrepared` means only that bytes exist in caller memory.

Excluded throughout: register devices; unknown emitters; flame or combustion;
flammable, toxic, or unknown gas exposure; medical or physiological claims;
body contact; food safety; capacitive-touch generalization; immersion; mains;
high voltage; and unattended stimulus. Generic analog-temperature boards and
unknown three-pin modules are not admitted. Lessons 070--072 do not become a
generic qualification framework for the loads and indicators reserved to
Lessons 079--081.

## Dependency and implementation order

| Lesson | Public boundary | E0 ownership |
|---:|---|---|
| 070 | `ModuleThresholdDescriptor`, `ModuleThresholdFrame`, stateless validation | Declared fixture schema and attributable copied channels |
| 071 | `ModuleCharacterizationPoint`, `ModuleCharacterizationPolicy` | Ascending, descending, then verification legs; brackets, intervals, witnesses |
| 072 | `InertModuleCharacterizationBench`, `ModuleCharacterizationEnvelope`, codec | One descriptor/session, inert script/presentation, one 192-byte volatile record |

Implement and terminally stress Lesson 070, measure it, and perform a design
pass before Lesson 071. Repeat at the 071/072 boundary. A new component that
does not fit naturally stops integration for remediation discussion; do not
add flags until the design buckles.

## Shared ordering and failure precedence

Every accepted value binds descriptor schema/identity/revision, declared
specimen reference/revision, electrical-evidence revision, source/configuration
identity, session/run/leg identity, control ordinal, sequence, observation
time, and channel statuses. Supplied `TimePoint` is the only clock and is not
UTC. Durations are below modular half range; zero means explicitly “none
required.” Unknown duration is represented by
`ModuleDurationDeclaration::Unknown` with canonical numeric zero.

Equal sequence/time/ordinal is idempotent only for field-wise identical
evidence. Changed duplicates, regression, gaps where contiguity is required,
future/backward time, half-range ambiguity, identity drift, and capacity
exhaustion reject atomically.

| Priority | Condition | API result | Domain result |
|---:|---|---|---|
| 1 | invalid encoding/configuration | `InvalidConfiguration` or `InvalidArgument` | previous evidence unchanged |
| 2 | lifecycle/session/run/leg/correlation mismatch | `InvalidArgument` | previous evidence unchanged |
| 3 | producer/channel failure | `Ok` for attributable terminal evidence | `ProducerFault` |
| 4 | time/sequence discontinuity | `Ok` for attributable terminal evidence | named discontinuity |
| 5 | unsettled or unwarmed evidence | `Ok` for attributable terminal evidence | named machine observation |
| 6 | transition/chatter/relation outcome | `Ok` | named terminal result |
| 7 | complete verified run | `Ok` | complete evidence |

Invalid input is not a domain observation. Domain rejection is valid,
attributable evidence. All mutation uses staged candidates; failed API calls
leave object, outputs, and caller images unchanged.

## Lesson 070 -- declared threshold fixture and attributed frame

### Public values

All public names are module-prefixed.

```cpp
enum struct ModuleChannelTopology : uint8_t
{
    AnalogOnly,
    ComparatorOnly,
    AnalogAndComparator
};

enum struct ModuleComparatorOutputStage : uint8_t
{
    Unspecified,
    PushPull,
    OpenDrain,
    OpenCollector
};

enum struct ModulePullRequirement : uint8_t
{
    Unspecified,
    None,
    PullUp,
    PullDown
};

enum struct ModuleDeclaredRail : uint8_t
{
    Unspecified,
    Ground,
    LogicSupply,
    ModuleSupply
};

enum struct ModuleComparatorPolarity : uint8_t
{
    Unspecified,
    ActiveHigh,
    ActiveLow
};

enum struct ModuleThresholdControlKind : uint8_t
{
    Unspecified,
    Fixed,
    Potentiometer
};

enum struct ModuleThresholdDirection : uint8_t
{
    Unspecified,
    IncreasingClockwise,
    IncreasingCounterclockwise
};

enum struct ModuleDurationDeclaration : uint8_t
{
    Known,
    Unknown
};

struct ModuleDeclaredDuration
{
    ModuleDurationDeclaration declaration;
    Duration value;
};

struct ModuleMillivoltRange
{
    uint16_t minimum;
    uint16_t maximum;
};

struct ModuleRawDomain
{
    uint16_t minimum;
    uint16_t maximum;
};

struct ModuleThresholdDescriptor
{
    uint16_t schemaRevision;
    uint32_t descriptorId;
    uint16_t descriptorRevision;
    uint32_t declaredSpecimenReference;
    uint16_t declaredSpecimenRevision;
    uint16_t declaredElectricalEvidenceRevision;
    ModuleChannelTopology channelTopology;
    ModuleComparatorOutputStage comparatorOutputStage;
    ModulePullRequirement pullRequirement;
    ModuleDeclaredRail declaredPullRail;
    ModuleMillivoltRange declaredSupplyMillivolts;
    ModuleMillivoltRange declaredSignalMillivolts;
    ModuleRawDomain rawDomain;
    ModuleComparatorPolarity comparatorPolarity;
    ModuleThresholdControlKind thresholdControlKind;
    ModuleThresholdDirection thresholdDirection;
    ModuleDeclaredDuration warmup;
    ModuleDeclaredDuration settling;
};

enum struct ModuleChannelStatus : uint8_t
{
    NotPresent,
    Current,
    Stale,
    ProducerFault
};

struct ModuleFrameProvenance
{
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sequence;
    TimePoint observedAt;
};

struct ModuleThresholdFrame
{
    uint16_t schemaRevision;
    uint32_t descriptorId;
    uint16_t descriptorRevision;
    uint32_t declaredSpecimenReference;
    uint16_t declaredSpecimenRevision;
    uint16_t declaredElectricalEvidenceRevision;
    ModuleFrameProvenance provenance;
    uint16_t analogRaw;
    ModuleChannelStatus analogStatus;
    bool comparatorLevelHigh;
    ModuleChannelStatus comparatorStatus;
    bool comparatorPresent;
    bool comparatorAsserted;
    bool declaredWarmupSatisfied;
    bool declaredSettlingSatisfied;
    Status analogProducerStatus;
    Status comparatorProducerStatus;
};

Status validateModuleThresholdDescriptor (
    const ModuleThresholdDescriptor& descriptor) noexcept;
Status validateModuleThresholdFrame (
    const ModuleThresholdDescriptor& descriptor,
    const ModuleThresholdFrame& frame) noexcept;
Result<bool> moduleComparatorAsserted (
    const ModuleThresholdDescriptor& descriptor,
    bool comparatorLevelHigh) noexcept;
Result<bool> moduleDescriptorDeclarationsComplete (
    const ModuleThresholdDescriptor& descriptor) noexcept;
```

`Unspecified` is a valid E0 declaration, not an invalid enum. It makes the
descriptor E1-inadmissible. Invalid underlying encodings always reject.
Nonzero schema, descriptor, specimen, and revision fields are required.
Declared ranges are ordered; declarations do not prove ratings. Pull rail must
be `Unspecified` unless a pull is declared, and must be specified when a pull
is declared. Comparator-stage, polarity, threshold kind, and direction
cross-fields are canonical for the topology. Comparator-only frames use the
canonical analog-absent representation; analog-only frames use the canonical
comparator-absent representation. Presence, statuses, values, and producer
statuses must agree.

Known zero warm-up/settling means none required. Unknown uses
`ModuleDurationDeclaration::Unknown`; its numeric field is canonically zero and
requires the corresponding frame satisfaction flag to be false.
`declaredWarmupSatisfied` and `declaredSettlingSatisfied` are copied
declarations bound to frame provenance/configuration. The policy does not wait
internally. `moduleDescriptorDeclarationsComplete()` reports only whether every
required declaration is specified; it cannot identify a module, close an E1
gate, or authorize power.

Rails are raw endpoint codes, not `OpenLike` or `ShortLike` diagnoses. Lesson
070 has no generic quality enum and no stateful `Policy` class.

### Exhaustive matrix and files

Files: `src/module_threshold_descriptor.h/.cpp`,
`tests/module_threshold_descriptor_test.cpp`, Mega
`examples/Lesson070ThresholdDescriptor/`, HTML API/lesson 070, and PDF 070.

- every valid and invalid enum encoding;
- every topology/output/pull/rail/polarity/control/direction cross-product;
- `Unspecified` accepted at E0 and reported incomplete by the structural
  declaration-completeness helper;
- zero/one/max revisions, ranges equal/inverted/exact, raw lower/upper rail;
- known-zero, known-nonzero, and unknown canonical durations;
- analog-only, comparator-only, dual-channel presence/status canonicality;
- active-high/low assertion and `Unspecified` assertion failure;
- separate channel producer faults and collision precedence;
- warmed/settled provenance and output non-mutation.

## Lesson 071 -- streaming three-leg characterization

### Public values and lifecycle

```cpp
enum struct ModuleCharacterizationLeg : uint8_t
{
    Ascending,
    Descending,
    Verification
};

enum struct ModuleSweepDirection : uint8_t
{
    Increasing,
    Decreasing,
    Unordered
};

enum struct ModuleCharacterizationState : uint8_t
{
    Idle,
    Collecting,
    Complete,
    Rejected,
    Shutdown
};

enum struct ModuleCharacterizationReason : uint8_t
{
    None,
    WarmupUnsatisfied,
    SettlingUnsatisfied,
    ProducerFault,
    Stale,
    SequenceDiscontinuity,
    TimestampDiscontinuity,
    DirectionViolation,
    Chatter,
    NoObservedTransitionActive,
    NoObservedTransitionInactive,
    AtLowerRail,
    AtUpperRail,
    TransitionOrientationMismatch,
    AnalogComparatorDisagreement
};

struct ModuleCompactWitness
{
    bool present;
    uint16_t controlOrdinal;
    uint16_t analogRaw;
    bool comparatorAsserted;
    uint32_t sequence;
    TimePoint observedAt;
};

enum struct ModuleComparatorRelation : uint8_t
{
    Unverified,
    Consistent,
    Ambiguous,
    Disagrees
};

struct ModuleCharacterizationPoint
{
    uint32_t sessionId;
    uint32_t runId;
    uint32_t legId;
    uint16_t controlOrdinal;
    ModuleCharacterizationLeg leg;
    ModuleSweepDirection direction;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    ModuleThresholdFrame frame;
};

struct ModuleTransitionBracket
{
    bool present;
    ModuleCharacterizationPoint before;
    ModuleCharacterizationPoint after;
};

struct ModuleAnalogInterval
{
    bool present;
    uint16_t lower;
    uint16_t upper;
};

struct ModuleCharacterizationConfig
{
    uint16_t characterizationRevision;
    ModuleThresholdDescriptor descriptor;
    uint8_t requiredPointsPerLeg;
    Duration maximumAge;
    Duration maximumGap;
};

struct ModuleCharacterizationEvidence
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t runId;
    uint32_t legId;
    uint16_t characterizationRevision;
    ModuleThresholdDescriptor descriptor;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    ModuleCharacterizationState state;
    ModuleCharacterizationReason reason;
    ModuleCharacterizationLeg terminalLeg;
    uint8_t ascendingCount;
    uint8_t descendingCount;
    uint8_t verificationCount;
    ModuleTransitionBracket ascendingBracket;
    ModuleTransitionBracket descendingBracket;
    ModuleAnalogInterval guaranteedInactiveInterval;
    ModuleAnalogInterval guaranteedActiveInterval;
    ModuleAnalogInterval ambiguityInterval;
    ModuleComparatorRelation relation;
    ModuleCompactWitness firstWitness;
    ModuleCompactWitness lastWitness;
    ModuleCompactWitness offendingBefore;
    ModuleCompactWitness offendingAfter;
    Status status;
};

struct ModuleCharacterizationPolicy
{
    explicit ModuleCharacterizationPolicy (
        const ModuleCharacterizationConfig& config) noexcept;

    Status initialize   (TimePoint now) noexcept;
    Status beginSession (TimePoint now, uint32_t sessionId,
                         uint32_t runId) noexcept;
    Status beginLeg     (TimePoint now, uint32_t legId,
                         ModuleCharacterizationLeg leg,
                         ModuleSweepDirection direction) noexcept;
    Status observe      (TimePoint now,
                         const ModuleCharacterizationPoint& point) noexcept;
    Status finalizeLeg  (TimePoint now) noexcept;
    Status reset        (TimePoint now) noexcept;
    Status shutdown     (TimePoint now) noexcept;
    Status evidence     (ModuleCharacterizationEvidence& output) const noexcept;
};
```

`requiredPointsPerLeg` is `[2, 16]`; every leg finishes at exactly N accepted
points and no point array is retained. `maximumAge` and `maximumGap` are each
nonzero and strictly below the modular half range; zero does not disable
freshness or continuity checks. Control ordinals are contiguous `1..N`
per leg. Source sequence is strictly contiguous forward; a field-wise
identical duplicate is idempotent and consumes neither an ordinal nor a point.
Any point beyond N, including a seventeenth, returns API `CapacityExceeded`
atomically and is not a terminal domain reason. Ascending must
be `Increasing`, descending `Decreasing`, verification `Unordered`. Legs occur
in that order, with distinct forward nonzero leg IDs. Session/run IDs are
forward nonzero. Initialize/reset increment generation and reject at
`UINT32_MAX`.

Successful lifecycle calls retain supplied `now`; later session, leg,
finalization, reset, and shutdown calls require forward-or-equal modular time.
Backward and exact-half-range ambiguous values reject atomically.
Initialization after shutdown returns `NotInitialized`; a new constructed policy
object is required.

Each learning leg permits exactly one clean comparator transition. Its adjacent
points form the bracket. A second reversal is `Chatter`; a toggle at equal raw
code is also `Chatter`. No transition yields
`NoObservedTransitionActive` or `NoObservedTransitionInactive`, according to
the observed canonical comparator state. An endpoint-only run reports
`AtLowerRail` or `AtUpperRail`; it does not invent a threshold.

The ascending leg must begin with an accepted point at the descriptor raw
lower bound and end at its raw upper bound. The descending leg must begin at
the raw upper bound and end at its raw lower bound. Missing either exact
endpoint is `DirectionViolation`, except when every accepted point is the same
exact declared endpoint. That endpoint-only special case terminalizes as
`AtLowerRail` or `AtUpperRail` before the coverage check and cannot advance to
another leg or produce intervals. Thus each successful learning leg observes
the full declared raw domain; the policy never extrapolates a state beyond the
sampled sweep endpoints.

The two learned brackets must have complementary orientation: the ascending
bracket's before state equals the descending bracket's after state, the other
two states equal each other, and the two state groups differ. Otherwise
descending finalization records `TransitionOrientationMismatch`.

Let `lowState` be the common ascending-before/descending-after assertion state.
Let `lowProved` be the lesser raw code of those two points, and let
`highProved` be the greater raw code of the ascending-after and
descending-before points. The raw domain is partitioned inclusively:

- `[raw.lower, lowProved]` is guaranteed to have `lowState`;
- `[highProved, raw.upper]` is guaranteed to have the opposite state; and
- when `lowProved + 1 <= highProved - 1`, that closed middle interval is the
  ambiguity interval; otherwise ambiguity is absent.

`guaranteedInactiveInterval` and `guaranteedActiveInterval` receive the first
two intervals according to `lowState`. Arithmetic is widened before the
`+1`/`-1` tests. These formulas publish neither an exact threshold nor a
signed hysteresis scalar. The third verification leg streams up to 16
points after both brackets are frozen. Each verification point is classified
against only guaranteed regions. Evidence becomes `Disagrees` only when a
point inside a guaranteed region contradicts the comparator; points in the
ambiguity interval remain `Ambiguous`. This makes disagreement computable
without retained arrays or circular use of the point being learned.

Evidence owns the full descriptor and correlation fields and exactly four full
bracket points (two per bracket). First/last/offending fields are compact
witnesses, not duplicate full points. The first and last witnesses are the
single canonical owners of first/last sequence and observation time; evidence
does not duplicate those four values at top level.
Intervals and relation remain explicit. Terminal evidence is immutable.

Lesson 071 configuration accepts only `AnalogAndComparator` and explicit
active-high or active-low polarity. Every accepted point requires both channels
present, `analogStatus == Current`, `comparatorStatus == Current`, both producer
statuses OK, and canonical assertion. Analog-only, comparator-only, or
`Unspecified` polarity fails configuration validation. A point with
noncanonical presence rejects; attributable stale/faulted channels terminalize
under the precedence table before transition learning.

### Exhaustive matrix and files

Files: `src/module_characterization.h/.cpp`,
`tests/module_characterization_test.cpp`, Mega
`examples/Lesson071Characterization/`, HTML/API lesson 071, and PDF 071.

- 2 and 16 points per leg, attempted 17th, finish at `N-1`;
- active-high/low clean transition at every adjacent location;
- equal-code toggle, second/third reversal, no-transition active/inactive;
- lower/upper rails; increasing/decreasing/equal/direction violations;
- all bracket orderings and guaranteed/ambiguity interval boundaries;
- verification consistent, ambiguous, and disagreeing points;
- identity/session/run/leg/ordinal/source/config drift;
- changed/identical duplicate, gap/regression/wrap/half-range sequence and time;
- separate producer, stale, unwarmed, unsettled, and precedence collisions;
- lifecycle/session/run/leg exhaustion, reset/shutdown, terminal immutability;
- object-layout proof of no retained point array.

## Lesson 072 -- inert one-envelope bench

### Envelope, commands, and result

The bench owns no Lesson 071 child and retains no frames. It atomically consumes
one immutable envelope.

```cpp
struct ModuleCharacterizationEnvelope
{
    uint16_t envelopeRevision;
    ModuleCharacterizationEvidence evidence;
    uint32_t descriptorDigest;
    uint32_t evidenceDigest;
};
```

Descriptor, evidence, and compact-witness digests use CRC-32/ISO-HDLC over
canonical little-endian, field-by-field encoding with domain tags
`ADK-MOD-DESC-1`, `ADK-MOD-EVID-1`, and `ADK-MOD-WIT-1`. Parameters are
polynomial `0x04c11db7` (reflected `0xedb88320`), initial `0xffffffff`, reflected
input/output, final XOR `0xffffffff`. Digest zero is valid and never a sentinel.
Raw struct bytes, padding, and addresses are never hashed.

Canonical digest encoding uses one byte for enums, Boolean values, and
`StatusCode`; two little-endian bytes for `uint16_t`; and four little-endian
bytes for `uint32_t`, `Duration::milliseconds()`, and
`TimePoint::milliseconds()`. A false `present` flag requires every following
field in that optional value to encode as zero.

- Descriptor order is its public field declaration order, recursively using
  range and duration declaration order.
- Compact-witness order is `present`, `controlOrdinal`, `analogRaw`,
  `comparatorAsserted`, `sequence`, and `observedAt`.
- Evidence order is its public field declaration order. The embedded
  descriptor uses descriptor encoding; bracket points use complete
  `ModuleCharacterizationPoint` declaration order; compact witnesses use the
  witness encoding above; `Status` contributes its one-byte `StatusCode`.

The domain tag bytes begin each digest and are included in the CRC. Algorithm
seed vectors are fixed: the tag alone hashes to `0xcb58c79d` for
`ADK-MOD-DESC-1`, `0x1daf5814` for `ADK-MOD-EVID-1`, and `0xb1b83fae` for
`ADK-MOD-WIT-1`. Tests additionally publish full valid descriptor, evidence,
and present/absent-witness golden byte vectors before any implementation is
promoted; changing field order, widths, tags, or canonical absence is a schema
change.

```cpp

enum struct ModuleBenchState : uint8_t
{
    Inert,
    Ready,
    ScriptActive,
    RecordPrepared,
    Fault,
    Shutdown
};

enum struct ModuleBenchScriptStep : uint8_t
{
    InspectDeclaration,
    ReviewAscending,
    ReviewDescending,
    ReviewVerification,
    PrepareRecord
};

enum struct ModuleBenchCommand : uint8_t
{
    None,
    Advance
};

struct ModuleBenchControl
{
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint32_t sessionId;
    uint32_t sequence;
    TimePoint observedAt;
    ModuleBenchCommand command;
    Status producerStatus;
};

struct ModuleBenchPresentationIntent
{
    ModuleBenchScriptStep step;
    ModuleBenchState state;
    bool faultDominant;
    ModuleComparatorRelation relation;
};

struct ModuleCharacterizationRecordImage
{
    static constexpr uint8_t version = 1;
    static constexpr uint16_t size = 192;
    uint8_t bytes[size];
};

enum struct ModuleCharacterizationRecordValidity : uint8_t
{
    Valid,
    BadLength,
    BadFraming,
    BadIntegrity,
    BadSemanticValue
};

struct ModuleBenchConfig
{
    uint16_t benchRevision;
    uint16_t envelopeRevision;
    uint16_t recordSchemaRevision;
    uint32_t expectedDescriptorId;
    uint16_t expectedDescriptorRevision;
    uint16_t expectedDescriptorSchemaRevision;
    uint16_t expectedDeclaredSpecimenRevision;
    uint16_t expectedDeclaredElectricalEvidenceRevision;
    uint32_t expectedDescriptorDigest;
    uint8_t expectedControlSourceId;
    uint16_t expectedControlSourceConfigurationRevision;
    Duration maximumControlAge;
};

struct ModuleBenchResult
{
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    ModuleBenchState state;
    ModuleBenchScriptStep step;
    uint32_t runId;
    uint32_t descriptorDigest;
    uint32_t evidenceDigest;
    ModuleComparatorRelation relation;
    ModuleBenchPresentationIntent presentation;
    bool recordPrepared;
    Status status;
};

struct ModuleCompactBracket
{
    bool present;
    uint16_t beforeRaw;
    uint16_t afterRaw;
    bool beforeAsserted;
    bool afterAsserted;
    uint32_t beforeSequence;
    uint32_t afterSequence;
};

struct ModuleCharacterizationRecord
{
    uint16_t recordSchemaRevision;
    uint16_t benchRevision;
    uint16_t descriptorSchemaRevision;
    uint16_t envelopeRevision;
    uint32_t lifecycleGeneration;
    uint32_t sessionId;
    uint32_t descriptorId;
    uint16_t descriptorRevision;
    uint32_t declaredSpecimenReference;
    uint16_t declaredSpecimenRevision;
    uint16_t declaredElectricalEvidenceRevision;
    ModuleChannelTopology channelTopology;
    ModuleComparatorOutputStage comparatorOutputStage;
    ModulePullRequirement pullRequirement;
    ModuleDeclaredRail declaredPullRail;
    ModuleComparatorPolarity comparatorPolarity;
    ModuleThresholdControlKind thresholdControlKind;
    ModuleThresholdDirection thresholdDirection;
    ModuleMillivoltRange declaredSupplyMillivolts;
    ModuleMillivoltRange declaredSignalMillivolts;
    ModuleRawDomain rawDomain;
    ModuleDeclaredDuration warmup;
    ModuleDeclaredDuration settling;
    uint32_t runId;
    uint32_t characterizationLifecycleGeneration;
    uint16_t characterizationRevision;
    uint8_t sourceId;
    uint16_t sourceConfigurationRevision;
    uint8_t ascendingCount;
    uint8_t descendingCount;
    uint8_t verificationCount;
    ModuleCompactBracket ascendingBracket;
    ModuleCompactBracket descendingBracket;
    ModuleAnalogInterval guaranteedInactiveInterval;
    ModuleAnalogInterval guaranteedActiveInterval;
    ModuleAnalogInterval ambiguityInterval;
    ModuleComparatorRelation relation;
    uint32_t firstWitnessDigest;
    uint32_t lastWitnessDigest;
    uint32_t offendingBeforeDigest;
    uint32_t offendingAfterDigest;
    uint32_t firstSequence;
    uint32_t lastSequence;
    uint32_t descriptorDigest;
    uint32_t evidenceDigest;
    ModuleCharacterizationState terminalState;
    ModuleCharacterizationReason terminalReason;
    Status terminalStatus;
    ModuleBenchScriptStep scriptStep;
};

struct ModuleCharacterizationRecordCodec
{
    Result<uint16_t> encode (
        const ModuleCharacterizationRecord& record,
        MutableByteSpan output) const noexcept;
    ModuleCharacterizationRecordValidity decode (
        ByteSpan image, ModuleCharacterizationRecord& output) const noexcept;
};

struct InertModuleCharacterizationBench
{
    explicit InertModuleCharacterizationBench (
        const ModuleBenchConfig& config) noexcept;

    Status initialize     (TimePoint now) noexcept;
    Status beginSession   (TimePoint now, uint32_t sessionId,
                           const ModuleCharacterizationEnvelope& envelope) noexcept;
    Status applyCommand   (TimePoint now,
                           const ModuleBenchControl& control) noexcept;
    Status prepareRecord  (TimePoint now,
                           ModuleCharacterizationRecordImage& output) noexcept;
    Status reset          (TimePoint now) noexcept;
    Status shutdown       (TimePoint now) noexcept;
    Status result         (ModuleBenchResult& output) const noexcept;
};
```

`beginSession()` validates the evidence-owned descriptor against the expected
identity/revisions/digest, envelope/evidence terminal
state, session/run/source correlation, authoritative fields, and both
domain-separated digests before committing. It does not replay Lesson 071.
Script advancement and record preparation are separate explicit methods;
there are no duplicated begin/finish-leg commands and no export/acknowledge
state.

Only `prepareRecord()` writes caller storage, through the one
`ModuleCharacterizationRecordCodec::encode()` path. The bench derives one
compact `ModuleCharacterizationRecord` from its saved envelope/result, stages
192 bytes, and copies them atomically. The coordinator does not hand-encode a
second format.
`ModuleBenchCommand` has only `None` and `Advance`. `Advance` reaches the final
`PrepareRecord` script step but never writes bytes. The direct `reset()` method
is the sole reset mutation path; it discards the saved compact snapshot,
increments generation, and returns Ready. Session IDs are nonzero forward
modular and never reused. Every control must exactly match
`expectedControlSourceId` and
`expectedControlSourceConfigurationRevision`; neither value is inferred or
latched from the first control.

### Exact record layout

All multibyte fields are little-endian. Reserved bytes are zero. CRC-16/
CCITT-FALSE uses polynomial `0x1021`, initial `0xffff`, no reflection, final
XOR `0x0000`, covers bytes 0--189, and occupies 190--191.
The four magic bytes are ASCII `ADMC`: `0x41 0x44 0x4d 0x43`.

| Bytes | Content |
|---:|---|
| 0--3 | magic |
| 4 | version |
| 5--6 | length = 192 |
| 7--8 | record schema revision |
| 9--10 | bench revision |
| 11--14 | lifecycle generation |
| 15--18 | session ID |
| 19--22 | descriptor ID |
| 23--24 | descriptor revision |
| 25--28 | specimen reference |
| 29--30 | specimen revision |
| 31--32 | declared electrical evidence revision |
| 33 | channel topology |
| 34 | comparator output stage |
| 35 | pull requirement |
| 36 | declared pull rail |
| 37 | comparator polarity |
| 38 | threshold-control kind |
| 39 | threshold direction |
| 40--41 | declared supply minimum mV |
| 42--43 | declared supply maximum mV |
| 44--45 | declared signal minimum mV |
| 46--47 | declared signal maximum mV |
| 48--49 | raw-domain minimum |
| 50--51 | raw-domain maximum |
| 52 | warm-up declaration |
| 53--56 | warm-up milliseconds |
| 57 | settling declaration |
| 58--61 | settling milliseconds |
| 62--65 | run ID |
| 66--69 | characterization lifecycle generation |
| 70--71 | characterization revision |
| 72 | ascending count |
| 73 | descending count |
| 74 | verification count |
| 75--89 | ascending bracket: presence, raw pair, assertion flags, sequence pair |
| 90--104 | descending bracket: presence, raw pair, assertion flags, sequence pair |
| 105--109 | guaranteed-inactive interval: presence/lower/upper |
| 110--114 | guaranteed-active interval: presence/lower/upper |
| 115--119 | ambiguity interval: presence/lower/upper |
| 120 | comparator relation |
| 121--124 | first-witness digest |
| 125--128 | last-witness digest |
| 129--132 | offending-before digest |
| 133--136 | offending-after digest |
| 137--140 | first sequence |
| 141--144 | last sequence |
| 145--148 | descriptor digest |
| 149--152 | evidence digest |
| 153 | terminal characterization state |
| 154 | terminal reason |
| 155 | terminal status code |
| 156 | script step |
| 157--158 | descriptor schema revision |
| 159--160 | envelope revision |
| 161 | source ID |
| 162--163 | source-configuration revision |
| 164--189 | reserved zero |
| 190--191 | CRC-16 |

This table is the frozen full layout. The compact record intentionally cannot
reconstruct the full envelope or its full points; its domain-tagged digests
bind the authoritative envelope while compact brackets and summaries support
record review. No raw struct, padding, address, or pointer enters the image.
Decode returns typed validity and does not mutate output on failure.

Digest value zero is valid and never a sentinel, including for
`expectedDescriptorDigest`.

Decode order is exact span size, framing, CRC, semantic/canonical fields, then
staged assignment. `BadLength` is only a non-192-byte span; `BadFraming` is
wrong magic/version/encoded length; `BadIntegrity` is CRC mismatch; and
`BadSemanticValue` is a CRC-valid semantic defect. Encode validates semantics
before capacity, stages all bytes, zeros reserved bytes, calculates CRC last,
and copies only on success.

Semantic checks cover declared enum/status encodings and required nonzero
revisions/identities; reconstructed descriptor validity; counts in 0--16;
canonical absent brackets/intervals; bracket domain, assertion, and sequence
rules; ordered in-domain intervals with nonoverlapping guaranteed regions;
first/last sequence summaries; terminal `Complete`/`Rejected`
state-reason-status relationships; exact `PrepareRecord` step; and zero
reserved bytes. Digests are opaque and are not recomputed by the compact
codec.

`beginSession()` precedence is lifecycle/configuration, structural envelope
and descriptor/evidence validation, exact correlation, digest equality, then
terminal admissibility. Structural/correlation/digest failures atomically
reject even when fields resemble a producer fault; a fully correlated,
digest-correct attributable rejection is admitted as fault-dominant evidence.

The image omits `terminalLeg`, current `legId`, observation times, control
ordinals, directions, complete points, and full producer statuses. Those
remain authoritative in Lesson 071 evidence and are bound by
`evidenceDigest`; compact summaries cannot reconstruct a Lesson 071 result.

### State/command table

| State | command `None` | command `Advance` | direct `prepareRecord()` | direct `reset()` |
|---|---|---|---|---|
| Ready | idempotent | invalid | invalid | Ready, generation + 1 |
| ScriptActive before final | idempotent | next script step | invalid | Ready, generation + 1 |
| ScriptActive at final | idempotent | remains final | prepare canonical record | Ready, generation + 1 |
| RecordPrepared | idempotent | invalid | idempotent, byte-identical | Ready, generation + 1 |
| Fault | invalid | invalid | invalid | Ready, generation + 1 |
| Shutdown | not initialized | not initialized | not initialized | not initialized |

Fault presentation always dominates a plausible relation or raw observation.
Malformed controls reject without changing state.

### Test-to-outcome matrix and files

Files: `src/inert_module_characterization_bench.h/.cpp`,
`src/module_characterization_record.h/.cpp`,
`tests/inert_module_characterization_bench_test.cpp`,
`tests/module_characterization_record_test.cpp`, Mega
`examples/Lesson072ModuleCharacterizationBench/`, HTML/API lesson 072, PDF 072.

| Test | Required outcome |
|---|---|
| descriptor/envelope mismatch | API rejection, no state mutation |
| digest mismatch | API rejection, no state mutation |
| nonterminal Lesson 071 evidence | API rejection |
| wrong source/session/run | API rejection |
| valid envelope | `ScriptActive`, copied immutable envelope |
| command collision/invalid enum | API rejection |
| fault envelope | fault-dominant presentation |
| early prepare | API rejection, image unchanged |
| final prepare | exactly one canonical 192-byte image |
| repeated prepare | byte-identical image, no new state |
| framing/reserved/CRC/semantic corruption | corresponding typed validity |
| reset/session reuse/generation exhaustion | documented modular result |
| endpoint/persistence symbol scan | none present |

## Authoritative provisional resource gates

Resource arithmetic is conservative:

`residual = 8192 - measured static SRAM - conservative synchronous stack - 128`

The 128-byte term is reserved interrupt margin. Targets are not measurements.

| Lesson/resource | Target | Hard limit |
|---|---:|---:|
| 070 flash | 10 KiB | 14 KiB |
| 070 static SRAM | 768 B | 1,024 B |
| 070 stack | 320 B | 448 B |
| 070 object composition | 192 B | 256 B |
| 070 descriptor | 64 B | 96 B |
| 070 residual SRAM | 4,096 B | 3,072 B |
| 071 flash | 12 KiB | 16 KiB |
| 071 static SRAM | 1,024 B | 1,536 B |
| 071 stack | 448 B | 640 B |
| 071 policy | 512 B | 768 B |
| 071 evidence | 320 B | 384 B |
| 071 caller phase-local point | 96 B | 128 B |
| 071 residual SRAM | 4,096 B | 3,072 B |
| 072 flash | 24 KiB | 32 KiB |
| 072 static SRAM | 2,048 B | 3,072 B |
| 072 stack | 768 B | 1,024 B |
| 072 coordinator | 512 B | 768 B |
| 072 record image | exactly 192 B | exactly 192 B |
| 072 simultaneous staged + destination images | 384 B | 384 B |
| 072 residual SRAM | 4,096 B | 3,072 B |

Provisional AVR ABI estimates, to be replaced by measured `sizeof` evidence,
show the spine under its gates:

| Value/object | AVR estimate | Controlling gate |
|---|---:|---:|
| `ModuleThresholdDescriptor` | 56 B | 96 B |
| descriptor + one frame composition | 104 B | 256 B |
| `ModuleCharacterizationPoint` caller-local | 72 B | 128 B |
| `ModuleCharacterizationEvidence` | 376 B | 384 B |
| `ModuleCharacterizationPolicy` | 736 B | 768 B |
| `InertModuleCharacterizationBench` | 720 B | 768 B |
| staged + destination record images | 384 B | 384 B |

Implementation adds AVR-target layout probes and fails the resource review if
compiler ABI measurement exceeds any hard gate; host `sizeof` is supporting,
not AVR evidence.

Any target miss triggers a design pass. Any hard-limit miss blocks publication.
Other triggers: retained point arrays, dynamic allocation, floating threshold
math, exact threshold scalar, generic value bags, per-family subclasses,
duplicate codecs, retained caller pointers, endpoint ownership, persistence
language, or duplicated descriptor/evidence snapshots.

## Narrative, HTML, and PDF

Examples follow acquire/configure/start, then observe/decide/actuate. Lesson 070
validates a declaration and frame. Lesson 071 streams ascending, descending,
and verification points and exposes interval/witness evidence. Lesson 072
first executes the real Lesson 070 validators and complete Lesson 071
three-leg policy to construct its immutable envelope, then consumes that
envelope, advances the review script, and prepares one caller image.
Preconstructed envelopes remain import-test fixtures, never maximum-composition
proof. Typed values/images are the non-Serial observation paths.

HTML is the primary accessible route and publishes declarations, canonicality
tables, state/command/precedence tables, synthetic fixtures, bracket and
verification examples, record layout, resource evidence, and open gates. It
states that brackets are not exact thresholds, declarations are not power
authority, and records are not acceptance or durability.

PDFs contain prediction/observation/interpretation, an explicit “no E0 circuit”
section, transition and ambiguity exercises, troubleshooting, and open bench
records. Every non-schematic visual is visibly pencil-drawn and immediately
preceded by `% ADK visual: pencil`. E0 has no formal schematic.

## E1 acceptance

### E1a specimen and acquisition

Record exact markings/photos and primary device documentation; when no
authoritative module schematic exists, also record a reviewed trace of the
populated circuit. Record pinout, rated supply/signal levels,
regulator/protection, output topology, pull and rail, threshold control,
warm-up/settling, current, ADC reference/impedance, inactive default, and
analog/digital/power/ground test points. Optional switched
power uses an exactly rated high-side switch; never GPIO-power a module. If no
qualified switch is needed, use a qualified board supply and physical power
removal.

Stimulus must be family-specific and harmless: visible ambient/covered light;
quiet hand clap or speaker at conservative level; small permanent magnet with
pinch/media precautions; room-temperature/hand warmth only; noncombustive
visible-light radiant source; dry insulated Metal Touch surrogate only after
circuit acceptance; gentle tabletop vibration; or inert opaque obstacle.
No family borrows another family’s acceptance.

### E1b presentation

Qualify exact display/indicator/controls separately. Present raw value beside
comparator state, distinguish faults without color, provide self-test
independent of stimulus, bound current, record resources, and prove startup
and rollback. This does not close acquisition, characterization accuracy,
specimen identity, storage, or durability.
