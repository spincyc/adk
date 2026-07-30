# Lessons 067--069 interchangeable motion recorder plan

Status: implementation-depth E0 plan; exact powered inertial specimens, bus
behavior, physical orientation, live presentation, persistence, and bench
acceptance remain open.

This arc turns copied inertial samples into a stable normalized record,
qualifies exactly one configured source in a declared qualification frame, and
records a bounded hand-motion script through a source-independent project
contract. E0 owns no pin, bus, interrupt, timer, sensor, display, LED, button,
RTC, SD card, filesystem, or physical clock. It consumes copied values and
supplied time and emits inert presentation and record intent.

The earlier cadence phrase “Lesson 043 adapters” is not an implementation
dependency: Lesson 043 deliberately publishes copied `InertialSample` values,
not physical adapters. Lessons 067--069 use synthetic, revision-specific
record fixtures at E0. `Mpu6050` and `Qmi8658` are provenance tags and negative
qualification seams, not claims that either physical family has been
identified or supported.

## Evidence levels

| Level | Authorized work |
|---|---|
| E0 | Pure policies over copied `InertialSample` values, explicit configuration, supplied time, fixed-capacity caller-owned storage, and inert presentation/export intent |
| E1a | One exact MPU6050 or QMI8658 chip and carrier revision, primary register map, identity/reset/configuration sequence, address strap, byte order, ranges, scaling, data-ready behavior, interrupt, mounting, regulator, pull-ups, logic levels, current, and I2C acceptance |
| E1b | Exact character display, RGB LED, and button endpoints, including ownership, polarity, current limits, startup, self-test, fault dominance, rollback, and observation |
| E1c | Exact RTC and SD/media adapters, including power-loss behavior, timestamp domain, filesystem and durability contract, corruption recovery, capacity, removal, and independent acceptance |

No runtime probe may write configuration to an unidentified address. No E0
test establishes physical axis direction, scale accuracy, calibration,
electrical acquisition, display behavior, clock accuracy, media durability, or
interchangeability of real devices. An unidentified revision remains
unpowered.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Owns at E0 |
|---:|---|---|---|
| 067 | `InertialRecordNormalizer` and `InertialRecordCodec` | Lesson 043 `InertialSample`, fixed-width values | Canonical normalized record and fixed 64-byte image preserving identity, ranges, readiness, saturation, calibration, time, sequence, and producer status |
| 068 | `InertialRecordQualificationPolicy` | Lesson 067 records, `SignedAxis`, supplied time | One configured source, one proper axis mapping, stationary-window checks, and immutable mapped qualification evidence |
| 069 | `QualifiedMotionRecorder` | Lessons 067--068, copied controls | One-source-per-session scripted recording, fixed-capacity caller-owned records, inert presentation/export intent, and fault-dominant health |

Implementation order is strict:

1. review this plan and the three stress dispositions;
2. implement and exhaustively test Lesson 067;
3. measure and reassess the record and stack budgets before freezing Lesson
   068;
4. implement and exhaustively test Lesson 068;
5. measure and reassess the qualification envelope before freezing Lesson 069;
6. implement Lesson 069 only from one promoted, immutable qualification
   envelope;
7. add synthetic Mega replays, exact resource evidence, HTML, and
   pencil-drawing PDFs;
8. run terminal stress passes and all non-hardware publication gates;
9. leave every powered, mounting, presentation, clock, and media acceptance
   card explicitly open.

The Lesson 043--045 public API remains unchanged. Lesson 044 retains exclusive
ownership of pitch/roll presentation. Lesson 067 does not map axes or qualify a
source; Lesson 068 does not select, vote, or fail over between sources; Lesson
069 does not read independent mutable snapshots from its children.

## Shared identity, ordering, and status rules

Every accepted value binds a nonzero schema revision,
normalization-contract revision, complete existing `InertialSource`,
sequence, observation time, and producer `Status`. E0 treats `TimePoint` as
supplied modular time and never calls it UTC. Revision zero and an
unrecognized closed-enum value are malformed.

All durations are nonzero and below the modular half range. Equal sequence and
time are idempotent only for byte-identical evidence. Changed duplicates,
future observations, backward time, exact half-range ambiguity, sequence
regression, identity or revision drift, and exhausted capacity reject
atomically. Sequence arithmetic uses the repository modular-ordering helper;
implementations never use signed casts as wraparound shortcuts.

`Status` reports malformed input, lifecycle misuse, producer failure, and
capacity failure. Domain enums preserve valid but unhealthy states. Failure
precedence is structural/configuration error, lifecycle/correlation error,
producer fault, source-quality fault, age/order fault, qualification failure,
then healthy operation. Lower-priority evidence remains inspectable where the
public value provides fields for it. No invalid status can become a healthy
sample through averaging, mapping, retry, or presentation.

Public large results are caller-owned.
Every mutating operation stages a complete candidate and commits only after all
validation succeeds. On failure, the previously published value and caller
buffer remain byte-for-byte unchanged.

## Lesson 067 -- inertial record normalization

### Responsibility

`InertialRecordNormalizer` translates one copied Lesson 043
`InertialSample` into an explicit stable record. “Normalization” means
stable field representation and canonical encoding; it does not mean axis
rotation, bias removal, unit conversion beyond the units already established
by Lesson 043, physical calibration, or source qualification.

```cpp
enum struct InertialRecordState : uint8_t
{
    Recorded,
    NotReady,
    SourceFault
};

struct InertialRecordConfig
{
    uint16_t schemaRevision;
    uint16_t normalizationRevision;
};

struct InertialRecord
{
    uint16_t            schemaRevision;
    uint16_t            normalizationRevision;
    InertialSource      source;
    InertialVector      accelerationMicroG;
    InertialVector      angularRateMilliDegreesPerSecond;
    TimePoint           observedAt;
    uint32_t            sequence;
    bool                dataReady;
    InertialSaturation  saturation;
    Status              producerStatus;
    InertialRecordState state;
};

struct InertialRecordNormalizer
{
    explicit InertialRecordNormalizer (
        const InertialRecordConfig& config) noexcept;

    Status normalize (const InertialSample& sample,
                      InertialRecord&       output) const noexcept;

  private:
    InertialRecordConfig config_;
};

enum struct InertialRecordValidity : uint8_t
{
    Valid,
    BadLength,
    BadFraming,
    BadIntegrity,
    BadSemanticValue
};

struct InertialRecordCodec
{
    static constexpr uint8_t version = 1;
    static constexpr uint8_t size    = 64;

    Result<uint16_t> encode (const InertialRecord& record,
                             MutableByteSpan       output) const noexcept;
    InertialRecordValidity decode (ByteSpan        image,
                                   InertialRecord& output) const noexcept;
};
```

The implementation may align names with existing Lesson 043 type spellings,
but it must not weaken the information boundary. `InertialRecordState` is
derived without destroying producer evidence: ready and healthy becomes
`Recorded`; a healthy non-ready value becomes `NotReady`; any non-OK producer
status becomes `SourceFault`. Saturation remains independently visible.
Canonical fault and not-ready records preserve provenance, ranges,
calibration, time, sequence, and producer status. They set readiness false,
saturation to `None`, and both vectors to zero so stale or untrusted payload
cannot masquerade as a current sample.

The canonical image is exactly 64 bytes, little-endian, and never a raw object
dump. Its locked offsets are:

| Bytes | Field |
|---:|---|
| 0--3 | magic |
| 4 | image version |
| 5 | encoded length |
| 6--7 | schema revision |
| 8--9 | normalization revision |
| 10 | record state |
| 11 | source kind |
| 12 | model |
| 13 | source ID |
| 14--15 | source configuration revision |
| 16--17 | calibration revision |
| 18--19 | reserved, both zero |
| 20--23 | acceleration range |
| 24--27 | angular-rate range |
| 28--39 | acceleration vector, x/y/z |
| 40--51 | angular-rate vector, x/y/z |
| 52--55 | observation time |
| 56--59 | sequence |
| 60 | flags: bit 0 data-ready, bits 1--2 saturation |
| 61 | producer status |
| 62--63 | CRC-16 |

All multibyte fields, including the CRC, use little-endian encoding. The CRC
uses polynomial `0x1021`, initial value `0xffff`, final XOR `0x0000`, and
covers bytes 0--61. Unknown flag bits, nonzero reserved bytes, invalid closed
enums, bad framing, bad integrity, or inconsistent semantic combinations are
rejected before publishing. `encode()` returns the written length through
`Result<uint16_t>` and reports a short `MutableByteSpan` as capacity failure.
`decode()` accepts a `ByteSpan`, requires exactly 64 bytes, returns typed
`InertialRecordValidity`, and leaves output unchanged on failure. Field-wise
comparison, not `memcmp`, defines semantic equality.

### Deterministic matrix

- every source enum, record state, supported range pair, readiness value,
  saturation value, and producer-status class;
- exact preservation of negative, zero, and positive vector components,
  including extrema accepted by Lesson 043;
- healthy ready, healthy not-ready, producer-fault, and saturated records;
- every invalid zero revision, invalid enum, and inconsistent state;
- byte-exact golden images and round trips;
- corruption of magic, version, length, enums, flags, reserved bytes, payload,
  or CRC;
- short and long spans, wrong version/encoded length, unknown flag bits, and
  every typed decode-validity result;
- output non-mutation on every failure;
- proof that records do not depend on later library compile inventory.

The Mega replay uses only synthetic copied samples. Its non-Serial observation
path is a caller-owned record plus fixed image inspected by deterministic
tests.

## Lesson 068 -- inertial source qualification

### Responsibility

`InertialRecordQualificationPolicy` qualifies exactly one explicitly
configured source from a bounded stationary record window. It verifies exact
identity and revisions, maps the source frame into one declared qualification
frame, rejects stale, sequence- or timestamp-discontinuous, faulted,
saturated, or moving evidence, and
publishes a terminal evidence envelope. It performs no discovery, probing,
driver configuration, source comparison, voting, failover, or orientation
presentation.

The only positive E0 source configuration is
`SyntheticFixture`/`Synthetic`. A structurally valid configuration naming an
MPU6050 or QMI8658 source returns `StatusCode::Unsupported` from
`initialize()` and does not initialize or publish evidence. Those tags remain
negative seams until their separate E1 adapter and specimen gates close.

`SourceAxisMapping` is a foundational proper rotation shared by both
acceleration and angular-rate vectors. It consists of three distinct
`SignedAxis` selections and admits exactly the 24 right-handed signed
permutation matrices. The validator exhaustively classifies all 216 triples.
A left-handed sensor-native transform belongs in a future physical adapter,
not in qualification. Negating `INT32_MIN` returns an error without invoking
undefined behavior.

```cpp
struct SourceAxisMapping
{
    SignedAxis outputX;
    SignedAxis outputY;
    SignedAxis outputZ;
};

enum struct InertialQualificationState : uint8_t
{
    Idle,
    Collecting,
    Qualified,
    Rejected
};

enum struct InertialQualificationReason : uint8_t
{
    None,
    ConfigurationMismatch,
    ProducerFault,
    NotReady,
    Saturated,
    Stale,
    SequenceDiscontinuity,
    TimestampDiscontinuity,
    AccelerationOutsideWindow,
    AngularRateOutsideWindow,
    ArithmeticOverflow,
    Disordered  = SequenceDiscontinuity,
    SequenceGap = SequenceDiscontinuity
};

struct InertialRecordQualificationConfig
{
    uint16_t                 qualificationRevision;
    uint16_t                 expectedSchemaRevision;
    uint16_t                 expectedNormalizationRevision;
    InertialSource           expectedSource;
    SourceAxisMapping        sourceToQualificationFrame;
    uint8_t                  requiredSampleCount;
    Duration                 maximumAge;
    Duration                 maximumGap;
    InertialVector          expectedStationaryAccelerationMicroG;
    InertialVector          maximumAccelerationDeviationMicroG;
    InertialVector          maximumAngularRateMilliDegreesPerSecond;
};

struct InertialWideVector
{
    int64_t x;
    int64_t y;
    int64_t z;
};

struct InertialQualificationEvidence
{
    uint32_t                    attemptId;
    uint32_t                    lifecycleGeneration;
    uint16_t                    qualificationRevision;
    SourceAxisMapping           sourceToQualificationFrame;
    InertialQualificationState  state;
    InertialQualificationReason reason;
    uint8_t                     acceptedSampleCount;
    uint32_t                    firstSequence;
    uint32_t                    lastSequence;
    TimePoint                   firstObservedAt;
    TimePoint                   lastObservedAt;
    Duration                    maximumObservedAge;
    Duration                    maximumObservedGap;
    InertialVector              meanAccelerationMicroG;
    InertialVector              meanAngularRateMilliDegreesPerSecond;
    InertialVector              minimumAccelerationMicroG;
    InertialVector              maximumAccelerationMicroG;
    InertialVector              minimumAngularRateMilliDegreesPerSecond;
    InertialVector              maximumAngularRateMilliDegreesPerSecond;
    InertialWideVector          accelerationSumsMicroG;
    InertialWideVector          angularRateSumsMilliDegreesPerSecond;
    InertialRecord              terminalRecord;
    InertialRecord              mappedRecord;
    Status                      status;
};

struct InertialRecordQualificationPolicy
{
    explicit InertialRecordQualificationPolicy (
        const InertialRecordQualificationConfig& config) noexcept;

    Status initialize (TimePoint now) noexcept;
    Status begin      (TimePoint now, uint32_t attemptId) noexcept;
    Status observe    (TimePoint             now,
                       const InertialRecord& record) noexcept;
    Status reset      (TimePoint now) noexcept;
    Status shutdown   (TimePoint now) noexcept;
    Status evidence   (InertialQualificationEvidence& evidence) const noexcept;

    bool initialized () const noexcept;
};
```

Required sample count is in `[2, 32]`. Every sample, not merely the final
average, must meet acceleration and angular-rate bounds. After applying the
proper axis mapping, each acceleration axis is compared in micro-g directly
against the corresponding
`expectedStationaryAccelerationMicroG` component and its configured
`maximumAccelerationDeviationMicroG`; each angular-rate axis is compared
against its corresponding
`maximumAngularRateMilliDegreesPerSecond` component. There is no conversion
to milli-g and no scalar vector-magnitude comparison. This preserves the exact
Lesson 067 units, keeps the stationary pose explicit, and avoids lossy
rounding. All sums and differences use explicit checked widening. Every limit
component is nonnegative. Exact configured limits pass; one unit beyond fails.

An attempt becomes terminal on its first rejection or after its required
sample count qualifies. Additional observations cannot change terminal
evidence; `reset()` and a new nonzero attempt ID are required. A semantic
rejection is a valid result with `Status::Ok` and a non-`None` reason.
Malformed input and arithmetic failure use non-OK `Status`. The terminal
record and extrema make the rejection attributable. For a successful attempt,
`mappedRecord` is an immutable qualification-frame record derived from the
terminal accepted record with the configured proper rotation applied to both
vectors. The envelope binds both records, source, mapping, attempt, revision,
aggregates, and outcome as one copied value. Lesson 069 consumes that envelope
instead of remapping a source record or combining values from different
attempts, so normalized comparisons remain reproducible across separately
configured source sessions.

The lifecycle generation starts at zero and increments on each successful
`initialize()` and `reset()`. Both operations reject atomically with
`StatusCode::CapacityExceeded` rather than wrapping `UINT32_MAX`;
`initialize()` also preserves the uninitialized state on unsupported physical
sources. Evidence carries the generation and the exact
`sourceToQualificationFrame`, so a consumer can reject an envelope from an
older lifecycle or a different mapping. It also carries the checked
`InertialWideVector` acceleration and angular-rate sums used to derive the
published means; the sums are evidence, not hidden recomputation.

Sequence discontinuity is distinct from timestamp discontinuity. Duplicate
sequence with changed content, gaps, regression, and half-range ambiguity
produce `SequenceDiscontinuity`. Future/backward observation time,
zero/nonforward inter-record time, excessive gap, and timestamp half-range
ambiguity produce `TimestampDiscontinuity`. A byte-identical duplicate is
idempotent.

### Deterministic matrix

- all 216 signed-axis triples, exactly 24 accepted proper rotations, vector
  mapping for each accepted rotation, and explicit `INT32_MIN` rejection;
- every configuration field at zero, exact minimum/maximum, and one beyond;
- identity, schema, normalization, source configuration, calibration, and
  range mismatch independently;
- producer fault, not-ready, saturation, stale, future, duplicate, changed
  duplicate, sequence gap, wrap, regression, and exact half-range ambiguity;
- sample windows at `N-1` and `N`, each axis at its exact bound and one unit
  over, asymmetric per-axis configurations, cancellation by averages, and
  widened-sum overflow probes;
- rejection-precedence collision pairs;
- physical-family configurations returning `Unsupported` without mutation and
  synthetic fixture/model as the sole positive E0 pairing;
- distinct sequence/timestamp discontinuity reasons and their collision
  precedence;
- exact lifecycle-generation increments, initialize/reset exhaustion at
  `UINT32_MAX`, mapping and wide-sum evidence, terminal immutability,
  attempt-ID correlation, shutdown, snapshot non-mutation, and replay
  equivalence after encode/decode.

The Mega replay supplies a stationary synthetic trace and one independently
faulted trace. Its non-Serial observation path is the copied qualification
envelope.

## Lesson 069 -- interchangeable motion recorder

### Responsibility

`QualifiedMotionRecorder` consumes one complete Lesson 068 qualification
envelope and subsequent copied Lesson 067 records from that same source. One source is fixed
for the entire session. “Interchangeable” means that separately configured
sessions expose the same consumer contract; it never means runtime detection,
hot switching, voting, fallback, or splicing records from different sources.

The project records the current script step before advancing to the next step.
The single closed `MotionRecorderCommand` value prevents contradictory
simultaneous button states; `Reset` dominates because it is handled before
recording-mode and trace validation. Fault dominates valid orientation
for presentation. E0 emits semantic LCD/RGB intent and a caller-owned record
image; it does not drive presentation or promise storage.

```cpp
enum struct MotionRecorderMode : uint8_t
{
    Inert,
    AwaitingQualification,
    Ready,
    Recording,
    Complete,
    Fault,
    Shutdown
};

enum struct MotionScriptStep : uint8_t
{
    Rest,
    TiltForward,
    TiltBack,
    TiltLeft,
    TiltRight,
    ReturnToRest
};

enum struct MotionRecorderHealth : uint8_t
{
    Unknown,
    Ready,
    Recording,
    Complete,
    SourceFault,
    Stale,
    Saturated,
    CapacityExhausted
};

enum struct MotionDisplayToken : uint8_t
{
    QualifySource,
    ReadyToRecord,
    HoldRest,
    TiltForward,
    TiltBack,
    TiltLeft,
    TiltRight,
    ReturnToRest,
    RecordingComplete,
    Fault
};

struct MotionRecorderConfig
{
    uint16_t          recordSchemaRevision;
    uint16_t          normalizationRevision;
    uint16_t          qualificationRevision;
    uint16_t          recorderRevision;
    uint16_t          maximumRecordCount;
    uint32_t          traceToken;
    Duration          maximumRecordAge;
    Duration          minimumStepDuration;
    InertialSource    expectedSource;
    OrientationConfig orientation;
};

enum struct MotionRecorderCommand : uint8_t
{
    None,
    Advance,
    Reset,
    RequestExport,
    AcknowledgeExport
};

struct MotionRecorderControl
{
    uint8_t               sourceId;
    uint32_t              sequence;
    TimePoint             observedAt;
    uint16_t              qualificationRevision;
    uint32_t              qualificationLifecycleGeneration;
    uint32_t              qualificationAttemptId;
    uint32_t              qualificationDigest;
    uint32_t              traceToken;
    MotionRecorderCommand command;
    Status                status;
};

struct MotionPresentationIntent
{
    MotionDisplayToken token;
    MotionRecorderHealth health;
    uint8_t             rgbRed;
    uint8_t             rgbGreen;
    uint8_t             rgbBlue;
    bool                orientationValid;
    int16_t             pitchTenthsDegree;
    int16_t             rollTenthsDegree;
};

struct MotionRecordImage
{
    static constexpr uint16_t capacity = 128;

    uint8_t bytes[capacity];
};

enum struct MotionRecordValidity : uint8_t
{
    Valid,
    BadLength,
    BadFraming,
    BadIntegrity,
    BadSemanticValue
};

struct DecodedMotionRecord
{
    uint16_t             recorderRevision;
    uint32_t             lifecycleGeneration;
    uint32_t             sessionId;
    uint16_t             ordinal;
    MotionScriptStep     scriptStep;
    MotionRecorderHealth health;
    uint16_t             qualificationRevision;
    uint32_t             qualificationLifecycleGeneration;
    uint32_t             qualificationAttemptId;
    uint32_t             qualificationDigest;
    uint32_t             recordDigest;
    uint32_t             traceToken;
    SourceAxisMapping    sourceToQualificationFrame;
    InertialRecord       mappedRecord;
    OrientationEstimate orientation;
};

struct MotionRecordCodec
{
    static constexpr uint8_t version = 1;

    MotionRecordValidity decode (
        const MotionRecordImage& image,
        DecodedMotionRecord&     output) const noexcept;
};

uint32_t motionQualificationDigest (
    const InertialQualificationEvidence& evidence) noexcept;
uint32_t motionRecordDigest (const InertialRecord& record) noexcept;

struct MotionRecorderResult
{
    uint32_t                      sessionId;
    uint32_t                      lifecycleGeneration;
    MotionRecorderMode            mode;
    MotionRecorderHealth          health;
    MotionScriptStep              scriptStep;
    uint16_t                      recordCount;
    uint16_t                      recordCapacity;
    InertialRecord                latestRecord;
    InertialQualificationEvidence qualification;
    MotionPresentationIntent      presentation;
    bool                          exportRequested;
    Status                        status;
};

struct QualifiedMotionRecorder
{
    explicit QualifiedMotionRecorder (
        const MotionRecorderConfig& config) noexcept;

    QualifiedMotionRecorder (const QualifiedMotionRecorder&) = delete;
    QualifiedMotionRecorder&
    operator= (const QualifiedMotionRecorder&) = delete;
    QualifiedMotionRecorder (QualifiedMotionRecorder&&) = delete;
    QualifiedMotionRecorder&
    operator= (QualifiedMotionRecorder&&) = delete;

    Status initialize (TimePoint now, uint16_t recordCapacity) noexcept;
    Status qualify    (TimePoint                            now,
                       const InertialQualificationEvidence& evidence) noexcept;
    Status begin      (TimePoint now, uint32_t sessionId) noexcept;
    Status update     (TimePoint                    now,
                       const InertialRecord&         record,
                       const MotionRecorderControl& control,
                       MotionRecordImage*           records,
                       uint16_t                     recordCapacity) noexcept;
    Status acknowledgeExport (TimePoint now) noexcept;
    Status reset      (TimePoint now) noexcept;
    Status shutdown   (TimePoint now) noexcept;
    Status result     (MotionRecorderResult& result) const noexcept;

    bool initialized () const noexcept;
};
```

Storage is fixed-capacity and caller-owned. `initialize()` receives only the
capacity and retains no trace pointer. Every `update()` receives the trace
pointer and capacity synchronously, validates that capacity against the saved
contract, builds one candidate locally, and copies it into the next cell only
after all record, control, mapping, orientation, and codec checks pass. The
pointer is never retained. `maximumRecordCount` cannot exceed supplied
capacity. A null/short span rejects without mutation; a full trace publishes
`CapacityExhausted` without overwriting an old record or incrementing the
count. There is no hidden ring buffer.

Each canonical project record is exactly 128 bytes and binds
magic/version/length, recorder revision,
session ID, record ordinal, script step, trace token, the complete
source/schema/normalization/calibration/range identity, qualification attempt
and revision, the immutable mapped record, mapped orientation snapshot,
health, qualification-evidence digest, the canonical Lesson 067 record digest,
reserved zeroes, and CRC.
Explicit endian encoding is mandatory. Raw structs and `memcmp` are not
persistence formats. `MotionRecordCodec::decode()` returns
`MotionRecordValidity::{Valid, BadLength, BadFraming, BadIntegrity,
BadSemanticValue}`, stages `DecodedMotionRecord`, and leaves output unchanged
on failure.

`qualify()` accepts only terminal `Qualified` evidence matching every
configured revision, expected source, mapping, and record contract.
The recorder saves that immutable qualification envelope. Every update control
must repeat its qualification revision, lifecycle generation, attempt ID,
qualification digest, and trace token, and must match the record's source ID,
sequence, and observation time. This is the stream-domain correlation
contract: a record cannot be attached to a different accepted qualification
or caller-owned trace merely because its numeric vectors look plausible.

`initialize()` and `reset()` increment a nonzero lifecycle generation and
return `CapacityExceeded` rather than wrapping `UINT32_MAX`. Reset discards the
qualification and returns to `AwaitingQualification`. `begin()` requires
`Ready`, a nonzero session ID, and a forward modular session ID distinct from
every earlier session retained by the object; duplicate, backward, or
half-range-ambiguous IDs reject. Each update validates the control and record,
records at most once, and only then applies an eligible `Advance`. A
byte-identical duplicate record is idempotent. Controls are copied command
evidence, not debounced electrical inputs. Correlation mismatches reject
without mutation. A producer fault, stale record, saturation, or capacity
exhaustion enters a visible fault without fabricating orientation.

Export is a volatile request/acknowledgement handshake over the caller-owned
images. “Requested” does not mean written, flushed, synchronized, durable, or
recoverable. A future E1c adapter must return separately correlated prepare,
write, flush, and verification receipts before any durability claim.

### Deterministic matrix

- every closed command, invalid command, reset dominance, advance before
  begin, repeated controls, minimum-step boundary, and current-step-before-
  advance ordering;
- qualification missing, rejected, stale, wrong attempt, wrong revisions, and
  source/range/calibration drift;
- every source health state and RGB/display fault-dominance collision;
- capacity zero, configured/supplied mismatch, null update trace, changing
  synchronous trace pointer, proof that no pointer is retained, exactly full,
  and one append beyond full with old images unchanged;
- record/control duplicates, changed duplicates, independent sequence wraps,
  backward/future time, maximum age/gap boundaries, and half-range ambiguity;
- byte-exact records, corruption of each field class, reserved bytes, CRC,
  record digest binding, and replay identity;
- lifecycle initialize/reset increments and exhaustion, session-ID duplicate/
  regression/half-range rejection, saved qualification revision/generation/
  attempt correlation, export request, acknowledgement misuse, typed 128-byte
  decode outcomes, and proof that no method invokes an endpoint.

The Mega replay uses one synthetic qualified source per run and an exact
six-cell `MotionRecordImage records[6]` caller-owned trace. Six updates record,
in order, `Rest`, `TiltForward`, `TiltBack`, `TiltLeft`, `TiltRight`, and
`ReturnToRest`; each uses `Advance`, so the sixth cell is committed before the
recorder becomes `Complete`. The six cells and presentation intent are the
non-Serial observation path. Separate source-session fixtures remain a
deterministic host-test seam rather than a second physical-source claim in the
canonical Mega replay.

## Resource and ownership budgets

These ceilings governed implementation. The canonical and exact no-LTO
measurements below are the promotion evidence.

| Resource | Target | Hard limit | Disposition |
|---|---:|---:|---|
| Lesson 067 normalizer object | 4 B | 4 B | immutable two-revision configuration |
| Canonical Lesson 067 image | 64 B | 64 B | locked little-endian image |
| Lesson 068 qualifier object | 384 B | 640 B | at most 32 samples, but aggregates rather than retaining the window |
| Qualification evidence | 224 B | 320 B | one immutable terminal envelope |
| Lesson 069 recorder object | 512 B | 768 B | excludes caller-owned record cells |
| One project record cell | 128 B | 128 B | fixed canonical image |
| Conservative synchronous stack | 768 B | 1,024 B | measured no-LTO call-chain estimate |
| Recurring composition SRAM | 2,048 B | 3,072 B | objects, envelopes, one working record, endpoint-free |
| Mega 2560 residual SRAM | 4,096 B | 3,072 B | after recurring composition and measured globals |
| Arc replay flash | 32 KiB | 40 KiB | honest linked 067--069 canonical build |

The promoted Lesson 069 Mega replay honestly runs Lessons 067, 068, and 069
and measures 39,428 B flash and 2,347 B static SRAM. Its exact no-LTO
composition measures 35,144 B flash, 2,347 B static SRAM, 861 B synchronous
stack, a 509 B recorder object, a 128 B record image, and 4,856 B residual
SRAM. Exact flash, static SRAM, and stack miss their targets by 2,376 B, 299 B,
and 93 B respectively; independent review accepted them after confirming the
full composition retains genuine qualification, provenance, correlation,
transactional encoding, and caller-owned trace semantics. All hard limits
pass. The acceptance is bound to boundary-scoped resource fingerprint
`7be0c300acc4f0e93d9cb5fa2e9b5a0ced5771458f9749e38eb8e507e61b30c6`
and becomes stale when any measured source, configuration, public layout,
toolchain, probe, fixture, or call graph changes.

No policy stores the 32-sample qualification window. Widened accumulators,
extrema, first/last provenance, and terminal record are sufficient. If a
target is missed, perform a design pass before accepting it: distinguish
caller-owned/lifetime storage from recurring storage, remove duplicated
snapshots, and reconsider the boundary. Crossing a hard limit blocks
publication unless the plan is explicitly revised with independent review.

E0 pin, timer, interrupt, bus, and endpoint budgets are all zero. E1 planning
must account for the selected sensor address/interrupt, I2C coexistence with
the exact LCD backpack if used, RGB current and pins, button bias, RTC bus,
SD chip select/SPI ownership, voltage domains, pull-ups, and safe startup.

## Narrative examples

Each example is readable in dependency order and follows acquire, configure,
start in `setup()`, then observe, decide, actuate in `loop()`.

- Lesson 067 acquires a synthetic copied sample, configures the record schema,
  normalizes once, and exposes the typed record and image.
- Lesson 068 acquires a fixed stationary trace, configures one source and
  proper axis map, starts one attempt, observes records, and exposes terminal
  evidence.
- Lesson 069 acquires caller-owned cells and one promoted qualification
  envelope, configures a session, observes copied records and controls,
  decides the script transition, and actuates only by publishing presentation
  and export intent.

Examples use the same domain vocabulary as code. Serial is optional supporting
output and never the only proof. No example includes I2C addresses, register
writes, guessed wiring, physical calibration instructions, or media writes.

## HTML, PDF, and publication

HTML is the primary accessible route and includes the complete API, runnable
synthetic fixtures, state/reason tables, fixed-image field tables, failure
interpretation, resource evidence, and open E1 cards. Lesson 069 links back to
Lessons 043--045 and forward to the exact-specimen acceptance work; it does
not imply those earlier lessons contain physical drivers.

Each PDF is a rich printable companion with prediction, observation, and
interpretation prompts; a pin-by-pin section explicitly says “no circuit at
E0”; a troubleshooting matrix distinguishes producer fault, not-ready,
saturation, stale data, correlation mismatch, and capacity exhaustion; and an
open hardware-acceptance record names the evidence still required.

All non-schematic PDF visuals use visibly pencil-drawn presentation and carry
`% ADK visual: pencil` immediately before their source construct. This includes
record-layout sketches, frame/axis plates, qualification flows, state diagrams,
trace charts, and recorder timelines. Only a genuinely electrically
authoritative future circuit may use `% ADK visual: schematic` and
`circuitikz`; E0 has no schematic. Every meaningful image has contextual
alternative text and adjacent prose.

Publication requires:

1. native and no-exception compile/test matrices;
2. exhaustive deterministic matrices above and terminal stress reviews;
3. Arduino library packaging, Mega example compile, and resource evidence;
4. API reference, site lesson pages, navigation/search/index coverage, and
   link checks;
5. deterministic PDF builds, metadata/font/text/monochrome/visual-language
   gates, and human page inspection;
6. clean full quality, lessons, site, release, and fresh-clone gates;
7. explicit deferral of all E1 acceptance in `docs/WORK_QUEUE.md`.

## E1 acceptance gates

Before any powered work, record exact chip top marking, carrier front/back
markings and photographs, schematic or traced circuit, address strap, regulator
and level-shifter population, pull-up values and destinations, logic voltage,
supply limits/current, primary datasheet and register-map revision, identity
register, reset state, configuration sequence, range/scaling, output byte
order, data-ready semantics, interrupt electrical behavior, sampling cadence,
mounting axes, calibration procedure, and rollback to a safe idle state.

MPU6050 evidence never closes a QMI8658 card, and QMI8658 evidence never closes
an MPU6050 card. Each supported carrier revision needs its own adapter,
golden trace, identity-mismatch, NACK, stale/data-ready disagreement,
saturation, axis, timing, and physical acceptance record. A marketing family
name, an I2C scan, or a plausible `WHO_AM_I` byte is insufficient identity.

Display self-test is separate from changing orientation. RGB fault indication
must dominate valid orientation on the exact hardware. SDA, SCL, interrupt,
and sensor-rail test points expose acquisition; presentation has its own named
observation path. RTC and SD evidence separately establishes timestamp and
media semantics, interrupted-write recovery, verification, and power-loss
behavior. Until those cards close, the released project remains a deterministic
volatile recorder policy over copied evidence.
