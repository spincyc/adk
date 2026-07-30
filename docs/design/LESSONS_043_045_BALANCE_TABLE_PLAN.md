# Lessons 043--045 balance-table instrument plan

Status: implementation-depth E0 plan, 2026-07-28.

This block front-loads a responsive motion-sensing result while preserving the
inventory, electrical, and physical-evidence gates. The implementation
authorized by this plan is an E0 replay-first boundary: copied inertial values,
pure validation and orientation policies, deterministic project composition,
and existing light/sound presentation. It does not authorize a powered
inertial module, an I2C register transaction, a wiring table, a formal
schematic, or an E1 acceptance claim.

The likely kit alternatives are an MPU-6050 on a GY-521-style carrier and a
QMI8658-based module. A retail kit name is not an electrical identity. The two
devices have different register maps, identity mechanisms, scale encodings,
data-ready behavior, and conversion rules. They therefore require separate
future adapters. Neither adapter is hidden behind a speculative common driver.
Their eventual common output is the copied value described here.

This plan resolves the cadence's phrase “add separately identified adapters”
conservatively:

1. E0 adds the revision-neutral copied value and pure policies now.
2. An MPU-specific adapter may be added only after an exact MPU specimen and
   primary-source record close its electrical and register-contract gates.
3. A QMI-specific adapter may be added only after an exact QMI specimen and
   primary-source record close its independent gates.
4. Only the adapter matching an inventoried specimen may enter an E1 example.
5. The E0 lessons remain valid and replayable when neither adapter is built.

Canonical E0 execution and evidence run only in the host harness/simulator.
The Arduino sketches are compile-only packaging artifacts under the E0 claim:
they must not be run on a powered Mega, and observing MCU memory through a
hardware debugger is not E0 evidence. Any powered board, live control,
indicator, sounder, sensor, or debug transport is separately gated E1 work.

Lessons 067--069 retain record/schema normalization, calibration provenance,
source qualification, cross-device recorded comparison, and a motion recorder.
This block does not absorb that later scope. It accepts one already-converted,
explicitly identified sample at a time and makes no claim that two device
families, ranges, calibration revisions, or mounting systems are equivalent.
The 043 value preserves, rather than normalizes away, model, source,
configuration revision, calibration revision, declared ranges, data-ready
state, saturation, producer status, timestamp, and sequence so the later
record boundary can qualify and normalize them explicitly.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Does not add |
|---:|---|---|---|
| 043 | Copied inertial sample validation and freshness | `Status`, fixed-width time, recorded fixtures | I2C ownership, register codecs, physical conversion, filtering, calibration persistence |
| 044 | Board-frame orientation and light/tone intent | Lesson 043 copied observations | Trigonometric navigation, heading, position, vibration analysis, display driver |
| 045 | Stationary tabletop balance instrument | Lessons 043--044 and project-local copied joystick/button observations | Physical endpoints, handheld or moving use, safety/navigation claims, storage, later inertial normalization |

Implementation order is strict:

1. integrate this reviewed plan before code: link it from
   `docs/projects/component_project_cadence.md`, `docs/PROJECTS.md`, and
   `docs/WORK_QUEUE.md`; reconcile their 043--045 scope and status without
   claiming implementation;
2. record the pre-implementation stress pass for the Lesson 043 copied-value
   boundary;
3. add Lesson 043 public values and pure policy with tests;
4. record the Lesson 044 pre-implementation stress pass;
5. add orientation and presentation intent with tests;
6. record the Lesson 045 pre-implementation project stress pass;
7. add the replay-only project coordinator and deterministic fixtures;
8. add one Mega replay example per lesson, measured size evidence, HTML, PDF,
   indexes, and generated documents;
9. run post-implementation stress passes against the measured aggregate;
10. promote only after every non-hardware gate passes, leaving the exact
   specimen, powered adapter, schematic, and E1 cards explicitly open.

## Shared vocabulary and fixed-point units

All public numeric units are encoded in their names. There are no bare
“acceleration”, “rate”, “angle”, or “sensitivity” integers.

| Value | Representation | Meaning |
|---|---:|---|
| `AccelerationMicroG` | signed 32-bit | acceleration in 1/1,000,000 standard gravity |
| `AngularRateMilliDegreesPerSecond` | signed 32-bit | angular rate in 1/1000 degree per second |
| `AngleMilliDegrees` | signed 32-bit | signed board-frame angle in 1/1000 degree |
| `SensitivityPermille` | unsigned 16-bit | presentation gain in 1/1000 of configured full scale |
| `InertialSampleSequence` | unsigned 32-bit | producer-assigned sample identity |
| `CalibrationRevision` | unsigned 16-bit | conversion/calibration contract named by the producer |

The E0 fixtures supply already-converted fixed-point values. That is honest
only because each fixture explicitly names `SyntheticFixture` provenance,
conversion revision, range, data-ready state, saturation evidence, sequence,
and timestamp. E0 does not imply that
the Mega measured these units. A future exact adapter owns the device-specific
register decode and exact rational conversion into these units and must prove
that numerator, denominator, rounding, and overflow behavior against its
primary datasheet. Floating-point scale constants are not the adapter contract.

No floating point is required. Orientation uses bounded integer vector
projection. The E0 policy may use a documented fixed-point approximation to
`atan2`; its error bound must be measured across the complete accepted vector
domain and published. Acceleration squares, sums, products, and absolute-value
checks use signed/unsigned 64-bit intermediates so the micro-g representation
does not overflow. It must not label a linear ratio as degrees without that
proof.

## Lesson 043 — copied inertial observations

### Responsibility

Lesson 043 validates one copied six-axis sample without owning a transport or
sensor. It preserves source identity, sample identity, time, conversion
revision, configured range, producer status, saturation, and age. Validation
does not filter, rotate, calibrate, integrate, or combine samples.

### Proposed public surface

The exact spelling may change during the pre-implementation stress pass, but
the information and behavior may not be weakened silently.

```cpp
enum struct InertialSourceKind : uint8_t
{
    SyntheticFixture,
    Mpu6050Adapter,
    Qmi8658Adapter
};

enum struct InertialModel : uint8_t
{
    Synthetic,
    Mpu6050,
    Qmi8658UnknownRevision
};

enum struct InertialSaturation : uint8_t
{
    None        = 0,
    Acceleration = 1,
    AngularRate  = 2,
    Both         = 3
};

enum struct InertialSampleQuality : uint8_t
{
    Invalid,
    Current,
    Stale,
    Saturated
};

struct InertialSource
{
    InertialSourceKind kind;
    InertialModel      model;
    uint8_t            sourceId;
    uint16_t           configurationRevision;
    uint16_t           calibrationRevision;
    uint32_t           accelerationRangeMicroG;
    uint32_t           angularRateRangeMilliDegreesPerSecond;
};

struct InertialVector
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct InertialSample
{
    InertialSource source;
    InertialVector accelerationMicroG;
    InertialVector angularRateMilliDegreesPerSecond;
    TimePoint      observedAt;
    uint32_t       sequence;
    bool           dataReady;
    InertialSaturation saturation;
    Status         status;
};

struct InertialObservationConfig
{
    Duration maximumAge;
    uint16_t freshnessContractRevision;
};

struct InertialObservation
{
    InertialSample       sample;
    bool                 latestDataReady;
    InertialSampleQuality quality;
    Duration             age;
    Duration             maximumAge;
    uint16_t             freshnessContractRevision;
    uint32_t             sequenceGap;
    Status               status;
};

struct InertialObservationPolicy
{
    explicit InertialObservationPolicy (
        const InertialObservationConfig& config) noexcept;

    Status               initialize () noexcept;
    void                 reset      () noexcept;
    Status               update     (TimePoint now,
                                     const InertialSample& sample) noexcept;
    InertialObservation  snapshot   () const noexcept;
    bool                 initialized() const noexcept;
};
```

`InertialVector` gains meaning only from the field that contains it. It is not
a public unit-free vector operation library. If review finds that repeated
field misuse is too easy, use two strongly named structs instead; do not add
templates or runtime polymorphism merely for dimensional typing.

### Configuration and validation

`initialize()` rejects:

- zero `maximumAge`;
- zero `freshnessContractRevision`;
- a duration outside the repository's wrap-safe comparison interval; and
- any representation that cannot be evaluated with existing `TimePoint`
  rollover semantics.

`update(now, sample)` applies this order:

1. reject use before initialization with `NotInitialized`;
2. reject `sample.observedAt` in the future relative to policy time `now`, or
   policy-time regression outside the documented wrap-safe progression, with
   `InvalidArgument`;
3. copy the complete input before deriving output;
4. preserve a non-OK producer status as the observation status;
5. reject `sourceId == 0`, zero configuration/calibration revisions, zero
   declared ranges, either range above `INT32_MAX`, inconsistent kind/model,
   or an unrecognized value as `InvalidArgument`; widened comparisons never
   negate or take the signed absolute value of `INT32_MIN`;
6. treat kind, model, source ID, both declared ranges, configuration revision,
   and calibration revision as the sequence-domain identity; changing any
   member starts a new domain;
7. before rejecting a zero sequence delta, compare the full sample
   semantically (every named field, with no padding-byte or `memcmp`
   dependency); an exact repeated sample is idempotent sample evidence. A
   same-domain, same-sequence record whose only change is
   `dataReady: true -> false` is a readiness poll, not a replacement payload:
   retain the last accepted sample byte-for-byte and publish
   `latestDataReady == false`. Any other changed field at delta zero is
   invalid;
8. for an exact repeated sample or a readiness poll, recompute `age` and
   quality from the new policy time `now` without emitting a new sample event
   or sequence gap; this is how a previously `Current` sample becomes `Stale`;
9. otherwise evaluate sequence deltas modularly: `1..INT32_MAX` is forward,
   `0x80000000` is ambiguous and invalid, and larger deltas are regression;
   record `delta - 1` as the explicit sequence gap;
10. reject any absolute axis value above its declared range as
   `InvalidArgument`;
11. require the producer's explicit saturation field to agree with the axes at
    their declared bounds; disagreement is invalid;
12. `dataReady == false` cannot produce a new sequence or replace the accepted
    payload; it updates only `latestDataReady` and is stale evidence, not a
    fabricated current sample;
13. classify an axis exactly at either declared range as `Saturated`;
14. otherwise classify age greater than `maximumAge` as `Stale`; age equal to
    `maximumAge` remains current;
15. otherwise classify the sample as `Current`.

Producer failure dominates range, saturation, and age classification because
its numeric payload is not trustworthy. Structural invalidity dominates
staleness. Saturation dominates staleness when the copied payload is
structurally valid, because the value has already lost bounded measurement
meaning. These precedence rules must be table-tested.

`Stale` and `Saturated` are explicit quality states with `StatusCode::Ok`;
they are not transport failures. The application decides whether those
qualities permit presentation. Invalid structure or producer failure yields
`Invalid` quality and a non-OK status.

In the promoted E0 implementation, only the exact
`SyntheticFixture`/`Synthetic` kind-model pair is a valid producer.
`Mpu6050Adapter`, `Qmi8658Adapter`, and their model values exist only so
negative tests prove an unqualified physical claim cannot enter accidentally;
they return `Unsupported`. A later qualified adapter changes this allow-list
through its own reviewed boundary, not by relaxing E0 validation.

Every snapshot copies `maximumAge` and `freshnessContractRevision` from the
initialized policy config. These fields are evidence about how freshness was
derived, not a second source-controlled timestamp.

`sample.dataReady` belongs to the retained accepted payload.
`latestDataReady` reports the newest valid readiness poll for that same source
domain and sequence. Keeping both is intentional: a not-ready poll must remain
observable without rewriting the sample that produced the axes, provenance,
timestamp, or sequence. A newly accepted ready sample sets
`latestDataReady == true`; reset clears it to false.

The snapshot is stable until the next `update()` or `reset()`. Observation
does not consume an event. `reset()` clears sequence history and returns the
canonical invalid/not-initialized observation without changing configuration.

### Future exact-adapter seams

The following are separate deferred boundaries, not Lesson 043 E0 classes:

```text
Mpu6050Adapter(I2cDevice&, Mpu6050Config)
    -> exact WHO_AM_I and register contract
    -> device-specific range and signed-register decode
    -> InertialSample with Mpu6050Adapter provenance

Qmi8658Adapter(I2cDevice&, Qmi8658Config)
    -> exact identity and register contract
    -> device-specific range, byte order, and data-ready decode
    -> InertialSample with Qmi8658Adapter provenance
```

QMI8658 A- and C-family parts both report `WHO_AM_I == 0x05`; software identity
cannot distinguish them. Their gyro configuration and interrupt contracts
differ. A future adapter therefore requires exact chip/carrier evidence and an
explicit configured variant backed by primary sources. It must not infer the
variant from `WHO_AM_I`, probe guessed registers, or silently fall back. Until
that gate closes, `Qmi8658UnknownRevision` remains provenance for synthetic
records only and no physical QMI adapter is authorized.

Each owns no bus: it borrows one initialized `I2cDevice`, obeys the bus
lifetime, and uses bounded transactions. The adapters may share small
unit-tested arithmetic helpers only when their datasheets prove identical
arithmetic. They do not share a base class, guessed register codec, fallback
identity, or “try both devices” startup path.

Before either seam is implemented, its inventory record must identify:

- chip marking and carrier/module marking;
- carrier schematic or traced regulator, level, and pull-up topology;
- supply and logic limits;
- I2C address and identity value;
- reset/default state and configuration sequence;
- acceleration and angular-rate range encodings;
- output byte order, two's-complement representation, and data-ready rules;
- sampling cadence and startup delay;
- saturation and transport-failure interpretation; and
- primary manufacturer sources for every electrical and register claim.

### Lesson 043 deterministic test matrix

| Group | Required fixtures and assertions |
|---|---|
| Lifecycle | inert construction, valid/invalid initialize including zero freshness revision, repeated initialize, reset, destruction-safe pure object |
| Source identity | synthetic accepted; MPU/QMI structural negative cases return `Unsupported`; zero/duplicate source IDs where composed; zero revisions; each kind/model/source/range/config/calibration change starts a new sequence domain |
| Values | zero, +1/-1, each exact range, one beyond each positive/negative range, `INT32_MIN` absolute-value safety |
| Status | every `StatusCode` producer value, non-OK dominance, no numeric classification from invalid payload |
| Quality | current, exact age boundary, one tick stale, exact-axis saturation, saturation plus stale, latest not-ready poll retains accepted payload and publishes stale readiness |
| Sequence | first, semantic delta-zero replay before modular rejection, changed delta-zero sample rejection, forward delta 1, explicit gap, `INT32_MAX`, ambiguous `0x80000000`, regression, wrap, and new source/config/calibration domains |
| Time | zero, exact sample replay at later policy time transitions Current to Stale without a new event, conflicting same-identity sample, future sample time, maximum age inclusive, one over, `TimePoint` wrap, illegal regression |
| Snapshot | byte-stable copied output, non-consuming repeated reads, reset canonicalization |
| Replay | identical configuration and input bytes produce byte-identical observations |
| Capacity | object-size and stack probe on AVR; no heap, RTTI, exceptions, virtual dispatch, or hidden clock |

No test calls a physical adapter in E0.

### Lesson 043 compile-only Mega example and host evidence

`Lesson043InertialObservation.ino` contains a constant bounded fixture array
with level, stationary gravity, hand-tilt, saturated, stale, and producer-fault
frames. The narrative remains:

1. acquire no physical endpoint and initialize the pure policy;
2. configure one synthetic source and replay cadence;
3. start with a copied host-result self-test pattern;
4. observe one copied frame;
5. decide quality and status;
6. copy the complete health result into named result cells.

The example must print `E0 SYNTHETIC REPLAY` only as optional context. Its
primary evidence is a fixed host-harness/simulator result record containing
`quality`, `status`, `age`, and `sequenceGap`. Named numeric diagnostic codes distinguish
current, stale, saturated, invalid, and producer-failure cases without Serial.

The sketch contains no `Wire`, no I2C register address, no module pin table,
no GPIO/ADC/timer endpoint, and no powered sensor wording. It is compiled but
not executed on a powered Mega under E0. Physical LEDs, hardware debugging,
and test points are separately gated E1 work.

## Lesson 044 — board-frame orientation and presentation intent

### Responsibility

Lesson 044 maps one current, nonsaturated copied inertial observation into a
bounded board-frame pitch/roll estimate and presentation intent. It owns no
LED, sounder, endpoint, bus, or clock. Board mounting is explicit
configuration. The policy is for a stationary, hand-tilted tabletop device;
it is not an attitude and heading reference system.

Acceleration gives a gravity-relative estimate only when the device is
stationary enough for the configured experiment. Angular-rate values are
retained for provenance and a stationarity guard; E0 does not integrate or
fuse them. Pitch and roll are derived from gravity acceleration only. There is
no yaw, heading, displacement, velocity, fall, gesture, or navigation claim.

### Board-frame mapping

```cpp
enum struct SignedAxis : uint8_t
{
    PositiveX,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ
};

struct BoardFrame
{
    SignedAxis right;
    SignedAxis forward;
    SignedAxis up;
};
```

The three axes must be distinct and right-handed. Initialization rejects
duplicates, opposites that reuse one sensor axis, or a left-handed frame.
This makes carrier placement explicit without teaching the application a
device register convention.

### Proposed public surface

```cpp
enum struct OrientationQuality : uint8_t
{
    Invalid,
    Unsteady,
    Level,
    Tilted,
    BeyondPresentationRange
};

enum struct BalanceDirection : uint8_t
{
    None,
    Forward,
    Backward,
    Left,
    Right
};

struct OrientationConfig
{
    BoardFrame boardFrame;
    int32_t    minimumGravityMicroG;
    int32_t    maximumGravityMicroG;
    int32_t    maximumStationaryRateMilliDegreesPerSecond;
    int32_t    levelThresholdMilliDegrees;
    int32_t    maximumPresentationAngleMilliDegrees;
};

struct OrientationEstimate
{
    int32_t            pitchMilliDegrees;
    int32_t            rollMilliDegrees;
    OrientationQuality quality;
    Status             status;
};

Status validateOrientationConfig (const OrientationConfig& config) noexcept;

struct OrientationPolicy;

struct PreparedOrientationEstimate
{
    PreparedOrientationEstimate () noexcept;

    const OrientationEstimate& result () const noexcept;

  private:
    friend struct OrientationPolicy;
    OrientationEstimate       result_;
    const OrientationPolicy*  owner_;
    uint32_t                  generation_;
};

struct OrientationPolicy
{
    explicit OrientationPolicy (const OrientationConfig& config) noexcept;

    Status              initialize () noexcept;
    void                reset      () noexcept;
    Status              preview    (const InertialObservation& input,
                                    PreparedOrientationEstimate& prepared) const
        noexcept;
    bool                canCommit  (
                      const PreparedOrientationEstimate& prepared) const noexcept;
    Status              commit     (
                            const PreparedOrientationEstimate& prepared) noexcept;
    Status              update     (const InertialObservation& input) noexcept;
    OrientationEstimate snapshot   () const noexcept;
    bool                initialized() const noexcept;
};

struct BalanceLightIntent
{
    uint16_t redPermille;
    uint16_t greenPermille;
    uint16_t bluePermille;
    bool     fault;
};

struct BalanceToneIntent
{
    bool     enabled;
    uint16_t frequencyHertz;
    uint16_t durationMilliseconds;
};

struct BalancePresentationConfig
{
    BalanceLightIntent level;
    BalanceLightIntent forward;
    BalanceLightIntent backward;
    BalanceLightIntent left;
    BalanceLightIntent right;
    BalanceLightIntent unsteadyPhaseA;
    BalanceLightIntent unsteadyPhaseB;
    BalanceLightIntent beyondRange;
    BalanceLightIntent invalid;
    int32_t fullScaleAngleMilliDegrees;
    uint16_t minimumTiltIntensityPermille;
    uint16_t maximumTiltIntensityPermille;
    uint16_t directionChangeFrequencyHertz;
    uint16_t directionChangeDurationMilliseconds;
};

struct BalancePresentation
{
    BalanceDirection  direction;
    BalanceLightIntent light;
    BalanceToneIntent tone;
    Status            status;
};

Status validateBalancePresentationConfig (
    const BalancePresentationConfig& config) noexcept;

struct BalancePresentationPolicy;

struct PreparedBalancePresentation
{
    PreparedBalancePresentation () noexcept;

    const BalancePresentation& result () const noexcept;

  private:
    friend struct BalancePresentationPolicy;
    BalancePresentation              result_;
    const BalancePresentationPolicy* owner_;
    uint32_t                         generation_;
};

struct BalancePresentationPolicy
{
    explicit BalancePresentationPolicy (
        const BalancePresentationConfig& config) noexcept;

    Status              initialize () noexcept;
    void                reset      () noexcept;
    Status              preview    (const OrientationEstimate& estimate,
                                    uint16_t sensitivityPermille,
                                    bool diagnosticPhase,
                                    PreparedBalancePresentation& prepared) const
        noexcept;
    bool                canCommit  (
                     const PreparedBalancePresentation& prepared) const noexcept;
    Status              commit     (
                           const PreparedBalancePresentation& prepared) noexcept;
    Status              update     (const OrientationEstimate& estimate,
                                    uint16_t sensitivityPermille,
                                    bool diagnosticPhase) noexcept;
    BalancePresentation snapshot   () const noexcept;
    bool                initialized() const noexcept;
};
```

The intent values describe bounded desired presentation. They are not hardware
commands. Existing RGB/mono LED and sounder components remain the only owners
of pins and timers.

Both Lesson 044 policies expose nonmutating `preview()`, `canCommit()`, and
transactional `commit()` seams. A prepared candidate is opaque except for its
const-reference `result()` accessor. It binds the candidate to the producing
policy instance and that policy's committed generation.
`preview()` validates and derives the complete candidate from the policy's
current committed snapshot but changes no state. `canCommit()` returns true
only for a valid candidate produced by that same policy at its still-current
generation. Calling `commit()` without first obtaining true from `canCommit()`
is a caller-contract violation and returns `InvalidArgument`; with that
precondition, commit copies the candidate snapshot, advances the generation,
and returns `Ok` without a remaining failure path. Any
initialize, reset, update, or commit invalidates every older candidate.

Candidate binding is independent of the returned classification status.
Structurally valid, initialized input always remains bound and committable even
when its safe classification returns non-OK: this includes stale or saturated
inertial evidence, an originating producer fault, invalid presentation derived
from that fault, `Unsteady`, and `BeyondPresentationRange`. A malformed value
or a call before policy initialization returns the diagnostic result but
clears the owner binding, so `canCommit()` is false. This distinction lets a
caller commit safe-state history deliberately without treating a producer
fault as a transaction failure.

The existing `update()` remains the convenient standalone operation. It runs
`preview()` and publishes that result internally: a committable result follows
the same checked commit path, while malformed input publishes the canonical
diagnostic snapshot and advances the generation without making the malformed
candidate externally committable. The free functions
`validateOrientationConfig()` and `validateBalancePresentationConfig()`
perform the complete configuration checks without constructing, initializing,
or mutating a policy. These seams let Lesson 045 derive every dependent value
before one atomic project commit without copying a noncopyable policy or
attempting rollback.

### Orientation rules

1. Non-OK or non-current Lesson 043 input produces `Invalid` and preserves the
   originating status where non-OK.
2. `Saturated` and `Stale` never produce angles for presentation.
3. Remap sensor axes into the configured right/forward/up board frame.
4. Compute acceleration-vector magnitude with overflow-safe widened
   intermediates.
5. Outside the configured stationary gravity band produces `Unsteady`.
6. Any angular-rate axis above the stationary threshold produces `Unsteady`.
7. Compute pitch and roll with the fixed equations and approximation below.
8. Both absolute angles within the level threshold produce `Level`.
9. An angle beyond the maximum presentation range produces
   `BeyondPresentationRange`.
10. Otherwise produce `Tilted`.

At a diagonal tie, the dominant direction order is fixed:

1. larger absolute angle wins;
2. an exact pitch/roll tie resolves to pitch;
3. positive pitch is `Forward`, negative pitch `Backward`;
4. positive roll is `Right`, negative roll `Left`.

No insertion order, pin number, RGB channel order, or enum numeric value may
affect direction.

For remapped components `R` (right), `F` (forward), and `U` (up), the exact
definitions are:

```text
horizontal = isqrt(R*R + U*U)
pitch      = atan2MilliDegrees(F, horizontal)
roll       = atan2MilliDegrees(R, U)
```

Products and sums use 64-bit intermediates and `isqrt` is the floor of the
nonnegative integer square root. Positive `F` is positive pitch/Forward;
positive `R` is positive roll/Right. `(R,F,U)=(0,0,positive)` is exactly
zero pitch and roll. `(0,0,0)` fails the gravity guard and never calls the
angle function. `atan2(0,0)` is invalid; all other axis and quadrant cases use
the mathematical signs above.

`atan2MilliDegrees(y,x)` is a specified 16-step integer CORDIC vectoring
routine. Inputs are bounded to the largest widened component produced above
(`sqrt(2) * INT32_MAX`) and are signed 64-bit before sign changes. `(0,0)` is
invalid.

Before quadrant normalization, scale every nonzero vector to a fixed
high-resolution magnitude:

```text
scale = 1 << 30
m     = max(abs64(x), abs64(y))
x     = (x * scale) / m
y     = (y * scale) / m
```

Multiplication occurs in signed 64-bit and division truncates toward zero.
With the stated input bound, the largest product is below
`sqrt(2) * INT32_MAX * 2^30`, safely below `INT64_MAX`. At least one normalized
component has magnitude exactly `2^30`, preventing small vectors such as
`(-3,-4)` from losing angular precision in the recurrence. CORDIC
intermediates remain signed 64-bit.

After scaling, `(positive-or-negative y,0)` returns exactly positive or
negative 90,000 millidegrees; `(0,positive x)` returns zero; and
`(0,negative x)` returns positive 180,000 millidegrees. For `x < 0`, negate
the widened vector and set the initial angle to positive 180 degrees when
original `y >= 0`, otherwise negative 180 degrees. This leaves `x >= 0` for
vectoring without ever negating `INT32_MIN` in its narrow type.

For iterations `i = 0..15`, define `divPow2(value,i)` as C++ signed division
by `2^i` (truncation toward zero, never a signed right shift). Capture old
`x_i` and `y_i`, then apply exactly:

```text
if y_i > 0:
    x_(i+1) = x_i + divPow2(y_i, i)
    y_(i+1) = y_i - divPow2(x_i, i)
    angle   += table[i]
else if y_i < 0:
    x_(i+1) = x_i - divPow2(y_i, i)
    y_(i+1) = y_i + divPow2(x_i, i)
    angle   -= table[i]
else:
    stop
```

The immutable microdegree table is:

```text
45000000, 26565051, 14036243, 7125016,
3576334, 1789911, 895174, 447614,
223811, 111906, 55953, 27976,
13988, 6994, 3497, 1749
```

The accumulated microdegrees are normalized to `[-180000000, 180000000]` and
rounded half away from zero to millidegrees. The production and host
implementation use the same recurrence. An exhaustive accepted-domain sweep
at threshold-adjacent values plus boundary and randomized vectors compares it
with a high-precision `atan2` oracle; maximum error must be at most 4
millidegrees. Failure to meet that bound is a promotion blocker, not permission
to relabel a coarser ratio as degrees.

Classification consumes that error conservatively. Define `E = 4`
millidegrees. A pose is `Level` only when both
`abs(angle) + E <= levelThreshold`; every threshold error-band case is
`Tilted`. A pose remains inside the presentation range only when both
`abs(angle) + E <= maximumPresentationAngle`; every maximum-range error-band
case is `BeyondPresentationRange`. For direction, magnitudes differing by at
most `2*E` use the documented pitch-first tie. These rules are applied after
fixed-point rounding and are table-tested at `threshold-E-1` through
`threshold+E+1`.

### Presentation rules

- `Level`: green intensity at configured minimum or greater, no tone.
- `Tilted`: scale the dominant absolute angle against the configured
  `fullScaleAngleMilliDegrees` and the sensitivity supplied to that `update()`.
  The unbounded intensity in permille is
  `abs(dominantAngle) * sensitivityPermille / fullScaleAngleMilliDegrees`,
  using a 64-bit numerator and integer division toward zero. Clamp that result
  to the configured minimum and maximum intensity rather than wrapping it,
  map forward/backward/left/right to documented color patterns, and emit a
  bounded short tone intent only on a direction change.
- `BeyondPresentationRange`: amber/fault-safe light intent, no tone.
- `Unsteady`: blue alternating intent expressed as a frame value, no tone.
- `Invalid` or non-OK: red fault intent, no tone.

The pure policy does not toggle LEDs over time. The caller supplies the
diagnostic phase explicitly and the configuration contains both complete
frames. No hidden clock enters this class.

Initialization rejects:

- invalid board frame;
- nonpositive or inverted gravity band;
- nonpositive stationarity threshold;
- negative level threshold;
- presentation angle not greater than the level threshold;
- presentation range above the approximation's proven domain;
- any RGB intensity above 1000 permille;
- a nonpositive full-scale presentation angle or one above 180,000
  millidegrees;
- inverted or zero tilt-intensity range;
- zero tone frequency with nonzero duration, or the reverse; or
- a duration or frequency outside the existing sounder contract.

Each update rejects sensitivity outside 1--1000 permille. Sensitivity is not
constructor state, so Lesson 045 can render a frozen measurement at a newly
selected sensitivity without mutating or reconstructing the presentation
policy. A `Tilted` estimate whose dominant absolute angle exceeds 180,000
millidegrees is malformed and is rejected before scaling; the widened
numerator therefore cannot overflow.

### Lesson 044 deterministic test matrix

| Group | Required fixtures and assertions |
|---|---|
| Frame mapping | all 24 valid right-handed orientations; duplicate, opposite-reuse, and left-handed rejection |
| Canonical poses | level, ±pitch, ±roll, four diagonals, exact dominant-axis tie |
| Fixed point | every quadrant, zero branches, symmetric ± values, `(-3,-4)`, minimum nonzero component beside maximum component, normalization overflow bounds, rounding boundaries, and <=4 md oracle sweep |
| Gravity guard | exact minimum/maximum magnitude, one outside each edge, widened-square overflow proof |
| Rate guard | exact threshold and one over on every positive/negative axis |
| Input quality | current, stale, saturated, invalid, producer failure; invalid values never leak angles |
| Thresholds | exact level threshold, one beyond, exact max presentation angle, one beyond |
| Intent | all directions; full-scale angle at 1 and 180,000 millidegrees; one below/at/above the configured full scale; 1 and 1000 sensitivity; 64-bit product boundary; minimum/maximum intensity clamps; malformed angle rejection; direction-change tone; repeated-direction silence |
| Permutations | sensor-axis mapping changes geometry only as configured; presentation is independent of pins and source kind |
| Replay | byte-identical estimates and intents for byte-identical copied input |
| Transaction seam | every valid and malformed orientation/presentation candidate through `preview()`; snapshot unchanged before commit; malformed and preinitialize previews are unbound; classified stale, saturated, unsteady, beyond-range, and producer-fault safe states remain bound even when preview returns non-OK; wrong-owner and stale-generation candidates fail `canCommit()`; both candidates pass `canCommit()` before either commit; successful commit exactly matches `result()`; standalone `update()` publishes the preview result and advances generation for both valid and malformed diagnostics |
| Lifecycle/size | invalid config, repeated initialize/reset, canonical safe snapshot, AVR object and stack measurements |

The fixed-point angle sweep compares the integer implementation with an
offline high-precision oracle in tests. The published maximum error must be
small enough that threshold cases are deterministic; samples inside the error
band around a threshold use an explicitly documented tie rule rather than an
accuracy claim.

### Lesson 044 compile-only Mega example and host evidence

`Lesson044OrientationPresentation.ino` replays stationary board-frame poses
and copies complete presentation intents into named result cells.
It first publishes a fixed intent self-test, then
level, four cardinal tilts, diagonal dominance, unsteady, beyond-range, and
invalid frames.

E0 has no physical circuit. Its non-Serial inspection path is the stable
light/tone intent record in the host harness/simulator. Mega compilation is
packaging evidence only; the sketch is not run on powered hardware. The PDF
explains that both inertial input and presentation output are synthetic.
Physical RGB, mono-LED, sounder, resistor, pin, and test-point work is
separately gated E1.

## Lesson 045 — stationary balance-table instrument

### Project scope

The project is a stationary hand-tilted tabletop instrument: the learner
rests or gently tilts a small inert platform by hand and observes direction
and sensitivity. It is not handheld while walking, attached to a person,
mounted on a vehicle, used to balance a load, or used as a safety instrument.
It has no actuator, external-load, or stored-energy path. Any eventual physical
platform is lightweight and stable, rests fully on the table, carries no loose
load, and has no sharp edge or pinch point.

The joystick adjusts presentation sensitivity. A debounced explicit button
freezes the last eligible estimate for comparison and unfreezes it. The
button, never tilt or a joystick threshold crossing, is the sole freeze
authority. The project consumes copied observations and emits intent; existing
components perform physical presentation.

### Proposed project API

```cpp
enum struct BalanceInstrumentMode : uint8_t
{
    AwaitingFrame,
    Live,
    Frozen,
    Recovering,
    Fault
};

enum struct SensitivityEvent : uint8_t
{
    None,
    Increase,
    Decrease,
    Contradictory
};

struct BalanceJoystickObservation
{
    int16_t            xPermille;
    int16_t            yPermille;
    SensitivityEvent   event;
    TimePoint          observedAt;
    uint32_t           sequence;
    Status             status;
};

struct BalanceButtonObservation
{
    bool      pressed;
    bool      pressEvent;
    bool      releaseEvent;
    TimePoint observedAt;
    uint32_t  sequence;
    Status    status;
};

struct BalanceInstrumentConfig
{
    uint16_t minimumSensitivityPermille;
    uint16_t maximumSensitivityPermille;
    uint16_t sensitivityStepPermille;
    Duration inertialMaximumAge;
    uint16_t inertialFreshnessContractRevision;
    Duration maximumInputSkew;
    Duration diagnosticPhase;
    BalancePresentation awaitingFramePresentation;
    BalancePresentation recoveringPresentation;
    BalancePresentation faultPresentation;
    BalancePresentation shutdownPresentation;
};

struct BalanceInstrumentInput
{
    InertialObservation        inertial;
    BalanceJoystickObservation joystick;
    BalanceButtonObservation   freezeButton;
    TimePoint                  frameAt;
    uint32_t                   frameSequence;
};

struct CompactInertialEvidence
{
    InertialSource        source;
    TimePoint             observedAt;
    uint32_t              sequence;
    InertialSampleQuality quality;
    Duration              maximumAge;
    uint16_t              freshnessContractRevision;
    InertialSaturation    saturation;
    bool                  acceptedDataReady;
    bool                  latestDataReady;
    Status                status;
};

struct BalanceMeasurementEvidence
{
    CompactInertialEvidence provenance;
    OrientationEstimate    estimate;
    bool                   available;
};

struct BalanceFrameStorage
{
    BalanceInstrumentInput previous;
    bool                   available;
};

struct BalanceInstrumentOutput
{
    BalanceInstrumentMode     mode;
    BalanceMeasurementEvidence liveEvidence;
    BalanceMeasurementEvidence frozenEvidence;
    BalancePresentation       presentation;
    uint16_t                  sensitivityPermille;
    uint32_t                  acceptedFrameSequence;
    TimePoint                 acceptedFrameAt;
    Status                    inertialStatus;
    Status                    joystickStatus;
    Status                    buttonStatus;
    Status                    status;
};

struct BalanceInstrument
{
    BalanceInstrument (const BalanceInstrumentConfig& config,
                       const OrientationConfig& orientationConfig,
                       const BalancePresentationConfig& presentationConfig,
                       BalanceFrameStorage& replayStorage) noexcept;

    Status                   initialize () noexcept;
    void                     shutdown   () noexcept;
    Status                   acknowledgeFault () noexcept;
    Status                   update     (const BalanceInstrumentInput& input) noexcept;
    BalanceInstrumentOutput  snapshot   () const noexcept;
    bool                     initialized() const noexcept;
};
```

The output deliberately does not duplicate either six-axis payload.
`CompactInertialEvidence` retains every fact needed to identify and interpret
the live or frozen derived measurement: kind, model, source ID, both ranges,
configuration and calibration revisions, timestamp, sequence, quality,
saturation, accepted-payload readiness, latest readiness, freshness
maximum/revision, and status. The full axes exist only in the
current input and one caller-owned `BalanceFrameStorage` used to prove exact
idempotent replay.

The caller owns `replayStorage`, initializes it unavailable, and keeps it alive
and unmodified for the instrument's entire initialized lifetime. The
instrument copies each newly accepted full frame there only after atomic
validation succeeds. It compares a delta-zero candidate field by field against
that storage. Failed frames never mutate it. `shutdown()` marks it unavailable.
The project object stores only a non-owning pointer; it never aliases the
caller's transient candidate input as retained state.

The project owns its two pure Lesson 044 policies by value. The constructor
accepts their immutable configs and constructs the policies internally; it
does not borrow mutable policy state that another caller could change between
project preview and commit. It still owns no endpoint and performs no callback.
There is no heap allocation, runtime polymorphism, repository-wide service
locator, copied rollback object, or externally shared mutable policy.

`initialize()` first calls all three complete configuration validators:
the project validator, `validateOrientationConfig()`, and
`validateBalancePresentationConfig()`. Only after every preflight
succeeds does it initialize the owned policies and publish the canonical
`AwaitingFrame` snapshot. Thus a configuration failure cannot leave one owned
policy initialized and the other uninitialized. `shutdown()` resets both owned
policies after publishing the configured shutdown presentation.

The four project presentations are complete canonical intent records rather
than values synthesized from inaccessible Lesson 044 configuration. Project
config validation requires recognized enum/status values, every light channel
within 0--1000 permille, and tone disabled with zero frequency and duration.
The awaiting, recovery, and shutdown records use `BalanceDirection::None`,
carry their documented noncurrent status, and have no fault light bit except
where explicitly documented; the fault record uses `BalanceDirection::None`,
sets the fault light bit, carries the selected fault status at publication
time, and is always tone-off. The project copies the configured light and
direction values, then sets the status required by the transition; it never
reaches into a presentation policy to recover an `invalid` light frame.

### Input identity and admission

These project-local copied joystick and button values deliberately do not
pretend that repository-wide `JoystickObservation` or `ButtonObservation`
types exist. A caller may translate an existing `AnalogJoystickSnapshot` and
`Button` accessors into them, but that adapter is outside the pure project
policy. Each copied value carries its own time, sequence, and status.

The copied controls are self-consistent records, not unverified event labels.
Both producer sequences are nonzero and every enum and `Status` value is
recognized. Joystick axes are each in `[-1000, 1000]`. `+X` is right, `-X` is
left, `+Y` is up, and `-Y` is down. The axes corroborate the already-qualified
event; they do not define another dead zone:

```text
positive       = xPermille > 0 || yPermille > 0
negative       = xPermille < 0 || yPermille < 0
canonicalEvent = None          when !positive && !negative
                 Increase      when  positive && !negative
                 Decrease      when !positive &&  negative
                 Contradictory when  positive &&  negative
```

The supplied event must equal that canonical event. `Contradictory` is an
admitted no-op, but the complete frame is not fully healthy. For the button,
`pressEvent` requires `pressed && !releaseEvent`, `releaseEvent` requires
`!pressed && !pressEvent`, and when neither event is set, `pressed` may
represent either held state. The other four Boolean tuples are malformed.

One `BalanceInstrumentInput` is an atomic application frame. `frameAt` is the
coordinator's frame time; it is distinct from each producer's `observedAt`.
The coordinator
does not fetch components itself. The caller snapshots each producer once,
then submits the complete frame.

Admission order:

1. require initialization;
2. if prior replay storage exists and frame identity has delta zero, compare
   every named field semantically first; return the stable prior output for an
   exact replay and reject any changed field;
3. then validate `frameAt` and `frameSequence` with modular forward deltas
   `1..INT32_MAX`; reject the ambiguous half-range and regressions;
4. require each observation's timestamp not to be in the future relative to
   `frameAt`;
5. require inertial, joystick, and button ages to be within
   `maximumInputSkew`;
6. validate the copied-control ranges, enums, statuses, and event/axis or
   event/level consistency described above;
7. validate each producer sequence by the same replay-first modular rule; the
   first joystick and button sequences may be any nonzero value, and their
   accepted baselines remain independent. A change to any member of the
   complete inertial source domain starts a fresh inertial sequence baseline:
   the first nonzero sequence in the new domain is accepted without computing
   a cross-domain gap or regression. Subsequent samples compare only with that
   domain's accepted baseline. A delta-zero control record must be
   semantically identical and is replay evidence that never reapplies its
   event, even inside a later forward complete frame. A changed field at delta
   zero is malformed. A forward producer sequence with an otherwise identical
   payload is new producer identity and its event applies. For
   inertial delta zero, equality covers only every immutable named field of
   the underlying `InertialSample` (complete source domain, both vectors,
   sample timestamp, sequence, accepted-payload data-ready, saturation, and
   producer status), not the observation's explicit latest readiness,
   not the observation's derived age, quality, gap, or status;
8. independently recompute canonical inertial `age`, `quality`,
   `sequenceGap`, and observation status from the immutable sample,
   explicit `latestDataReady`, `frameAt`, prior accepted sequence, and
   configured `inertialMaximumAge`;
   require the supplied derived fields to equal those canonical values, so a
   repeated sample may correctly age from Current to Stale while arbitrary
   derived-field mutation is rejected;
9. require the observation's `maximumAge` and
   `freshnessContractRevision` to equal `inertialMaximumAge` and
   `inertialFreshnessContractRevision`; mismatch is
   `InvalidConfiguration`;
10. preserve compact input evidence in the output even when presentation
    faults;
11. apply fault precedence;
12. derive the current live orientation into a nonmutating prepared candidate;
13. evaluate explicit freeze/unfreeze against that current candidate and select
    live or frozen evidence;
14. apply sensitivity events to a candidate sensitivity;
15. select the estimate rendered by the candidate mode;
16. derive one complete presentation intent into a nonmutating prepared
    candidate;
17. require both policies' `canCommit()` checks to pass, then infallibly commit
    both owned policy candidates, output, evidence, control state,
    diagnostic epoch,
    and frame/producer baselines together, then copy the accepted full frame
    to caller-owned replay storage and set its `available` flag last.

An exactly repeated complete frame is idempotent only when `frameAt`, frame
sequence, and every named field of all copied observations are semantically
equal.
There is no `memcmp` or padding-byte identity. Any changed frame with equal
`frameAt` is invalid. Producer observations may be older than `frameAt` only
within `maximumInputSkew`; no producer `observedAt` may be in the future.

That complete-frame rule is distinct from a producer sample reused in a later
frame. For later `frameAt`, the immutable `InertialSample` may retain its
sequence while L045 requires newly recomputed derived fields. Specifically:

```text
age          = frameAt.elapsedSince(sample.observedAt)
sequenceGap  = 0 for unchanged sample identity, otherwise forwardDelta - 1
quality      = Invalid when sample/status structure is invalid
               Saturated when canonical saturation applies
               Stale when !latestDataReady or age > inertialMaximumAge
               Current otherwise
status       = preserved non-OK producer/validation status, otherwise Ok
```

Freshness remains inclusive at `age == inertialMaximumAge`. L045
initialization rejects zero or wrap-unsafe `inertialMaximumAge` and zero
`inertialFreshnessContractRevision`. The carried observation fields make the
required equality with Lesson 043 observable and testable; a mismatch is
invalid configuration, not a second freshness policy. `maximumInputSkew` must
be greater than `inertialMaximumAge`; otherwise
the promised visible stale state would be unreachable because skew would fault
first.

The project must not combine a new joystick event with an old inertial sample
as though they were simultaneous. A skew fault produces no tone and the
canonical red fault intent. Duplicate button snapshots do not retrigger
freeze because the project tracks button event identity according to the
project-local button sequence.

Admission and derivation are transactional. Before mutating any project,
owned policy, baseline, epoch, or replay state, `update()` performs the full
replay-first check; validates all values, times, skew, freshness metadata, and
sequence deltas; canonicalizes the inertial evidence; and computes fault
precedence, health, controls, diagnostic phase, evidence, estimate,
sensitivity, and mode into temporaries. It calls the owned Lesson 044 policies'
const `preview()` methods only after input prevalidation. A non-OK preview
status is not itself rejection when it is the classified safe-state result of
an admitted stale, saturated, unsteady, beyond-range, or producer-fault frame.
The project calls `canCommit()` on both opaque, generation-bound candidates
before committing either. Only after both checks pass does it call both
`commit()` methods; under those checked preconditions each returns `Ok`
without a remaining failure path. No project-level failure-injection hook is
added for `canCommit()`: Lesson 044 exhaustively tests malformed, wrong-owner,
and stale-generation candidates through the opaque seam, while Lesson 045
owns both policies and performs no callback or other state-changing operation
between its two preflight checks and the commits. Project tests instead prove
that both preflights precede either commit and that every admitted frame
reaches the infallible checked commit path. There is no ad-hoc rollback. A
malformed or rejected frame
leaves the output, owned-policy snapshots, baselines, epoch, and replay storage
byte-identical. A structurally valid producer-fault frame is instead admitted
and atomically commits the corresponding configured `Fault` presentation.

Every admitted frame commits both candidates, including fault and ineligible
safe-state frames. This resets presentation direction history consistently, so
the first later eligible direction cannot emit a tone by comparing against a
direction hidden before the interruption. The project may override the
candidate's public output presentation with its configured fault, recovery, or
other canonical project intent, but it still commits the prepared Lesson 044
candidate that records the admitted frame's safe-state history.

### Freeze semantics

- Only a qualified button activation event toggles `Live`/`Frozen`.
- A freeze event is accepted only when the current orientation estimate is
  `Level` or `Tilted`.
- The frozen estimate includes the accepted inertial source, sequence,
  timestamp, and calibration revision through the associated output evidence.
- While frozen, new valid samples still update `liveEvidence` and health but
  do not replace `frozenEvidence`.
- Sensitivity changes affect presentation of the frozen angle immediately;
  they do not rewrite the frozen measurement.
- Unfreeze returns to the newest eligible live estimate in the same atomic
  update.
- A stale, saturated, unsteady, invalid, or fault frame never becomes the
  frozen value.
- Any dominating fault enters a latched `Fault` presentation but retains the
  last frozen value for diagnosis. Healthy later frames update retained
  evidence but do not auto-clear the latch or apply controls.
- `acknowledgeFault()` succeeds only while the latest complete frame is fully
  healthy. “Fully healthy” means a newly accepted complete frame passed every
  structural, time, sequence, freshness, and skew check; every producer status
  is `Ok`; canonical inertial quality is `Current`, latest readiness is true,
  and saturation is `None`; orientation is `Ok` and `Level` or `Tilted`; the
  joystick event is not `Contradictory`; the button tuple is consistent; and
  initialized presentation derivation succeeded. Stale, saturated, unsteady,
  beyond-range, invalid, not-ready, contradictory, or failed evidence is not
  fully healthy. The method returns `NotInitialized` before initialization and
  `InvalidArgument` unless mode is `Fault` and that predicate is true.
  It is an out-of-band operation, never a bit in an input frame. Success
  changes `Fault` to `Recovering`, preserves live/frozen evidence, frozen
  measurement, sensitivity, and diagnostic epoch, emits the canonical no-tone
  recovery intent, and does not change accepted-frame identity or replay
  storage. It waits for a later forward, fully healthy complete frame; an exact
  replay of the pre-ack frame cannot complete recovery. A producer/skew fault
  while recovering relatches `Fault`; a nonfault ineligible frame remains
  `Recovering`, updates evidence, and applies no controls or tone. The first
  forward fully healthy frame re-primes every frame, producer, and event
  baseline, enters `Live`, and suppresses both freeze and sensitivity events
  carried by that frame. Before the first complete frame after initialization,
  mode is `AwaitingFrame`.
  Shutdown/reinitialize returns to `AwaitingFrame`. None of these transitions
  manufactures a freeze or sensitivity event.
- `shutdown()` clears frozen state and returns a canonical no-tone/invalid
  copied intent. Restart begins `AwaitingFrame`.

### Sensitivity semantics

The joystick's already-qualified directional event changes sensitivity by one
configured step. `Increase` and `Decrease` follow the canonical axis/event
contract above; `Contradictory` is an admitted no-op and invalid joystick
evidence is rejected or faulted according to the precedence rules. Clamp at
configured minimum and maximum. Sensitivity is volatile and resets to the
midpoint rounded toward the minimum. Persistence belongs to an explicitly
planned later configuration boundary, not this project.

The potentiometer listed in the cadence is not required by the canonical E0
project because the joystick already supplies bounded adjustment. It may not
be added as a second authority without a new decision defining precedence and
learner value.

### Fault and simultaneous-event precedence

The project uses this descending precedence:

1. not initialized or invalid project configuration;
2. malformed or future-dated atomic frame;
3. non-OK producer status, preserving the first fault in fixed field order
   and latching `Fault`: inertial, freeze button, joystick;
4. excessive input skew, which latches `Fault`;
5. an already-latched project fault;
6. stale, saturated, unsteady, or beyond-range evidence, which is visibly
   ineligible and forces tone off but does not erase evidence or itself latch
   a producer fault;
7. freeze/unfreeze event;
8. sensitivity event;
9. ordinary live presentation.

`acknowledgeFault()` is outside `update()` and therefore is not ordered among
same-frame controls. While fault-latched, all frame controls are ignored. The
method can enter `Recovering` only after a healthy complete frame has been
observed.

This order determines control and presentation, not destruction of evidence.
The output retains independent status/quality fields so a joystick fault does
not relabel a valid inertial sample and an inertial fault does not disappear
because the fault LED also failed.

When freeze and sensitivity events share one admitted frame, the policy first
derives the current frame's prepared orientation, then freeze selection uses
that current estimate rather than the prior committed snapshot, and sensitivity
changes the rendering of the selected estimate. The deterministic order is
tested. Reordering input members, source IDs, or physical pins cannot change
it.

### Timing

- All time enters in the copied frame.
- There is no delay, busy wait, internal sampling cadence, or hidden retry.
- `maximumInputSkew` is a configuration limit, not a scheduler.
- Diagnostic alternation uses
  `(frameAt.elapsedSince(epoch) / diagnosticPhase) & 1` with a recorded
  epoch and wrap-safe arithmetic.
- `diagnosticPhase` is nonzero and no greater than `INT32_MAX`.
  `AwaitingFrame` has no epoch and uses canonical phase A/false. The first
  newly accepted complete frame establishes `epoch = frameAt`, so it renders
  phase A; the exact one-phase boundary renders B and the two-phase boundary
  renders A. The epoch persists through Live, Frozen, Fault,
  `acknowledgeFault()`, Recovering, and exact replay. Shutdown clears it and
  reinitialization again has no epoch; acknowledgement never re-phases.
- Equal `frameAt` values are legal only for a semantically identical complete,
  idempotent frame. A changed frame requires a forward frame time and frame
  sequence. Producer `observedAt` values retain their independent freshness
  meaning.
- A time regression that cannot be represented by ordinary `TimePoint`
  rollover fails the frame without changing the last good control state.
- No output tone duration authorizes blocking; E0 retains it as copied intent.
  A future E1 caller may pass it to the existing nonblocking sound component.

### Lesson 045 deterministic test matrix

| Group | Required fixtures and assertions |
|---|---|
| Lifecycle | every invalid project/orientation/presentation config fails preflight before either owned policy mutates; repeated initialize; `AwaitingFrame`; shutdown from Live/Frozen/Recovering/Fault; restart |
| Canonical project intents | awaiting, recovery, fault, and shutdown records are independently configured and validated; each is tone-off with canonical direction/light/status; no transition depends on private presentation-policy config |
| Happy path | level, each direction, diagonal, sensitivity up/down, freeze, changed live tilt, frozen comparison, unfreeze |
| Freeze authority | tilt alone cannot freeze; joystick cannot freeze; press event uses the current frame's prepared orientation rather than the prior snapshot; held/replayed event cannot; invalid current estimate cannot become frozen |
| Control consistency | joystick zero, every sign quadrant and corner, exact +/-1000, invalid +/-1001, and every supplied/canonical event mismatch; all eight button Boolean tuples; recognized enums/statuses; zero sequence |
| Sensitivity | each direction mapping, simultaneous opposites as admitted contradictory no-op, exact min/max, one step into each clamp, frozen rerender |
| Producer replay | zero/forward/half-range/regressed frame, inertial, joystick, and button sequences; every inertial source-domain member changed independently starts a fresh nonzero baseline with no cross-domain gap or regression; identical producer record in a later frame never retriggers; changed delta-zero rejected; forward identical-payload event applies |
| Atomicity | each field future dated, exact skew, one tick over, freshness maximum/revision match and each mismatch, full-frame semantic replay before delta-zero rejection, changed delta-zero frame, immutable sample reused in a later frame with Current→Stale recomputation, latest not-ready evidence recomputed without replacing the accepted payload, arbitrary latest-readiness/derived age/quality/gap/status mutation, repeated snapshots, and each ordinary prevalidation rejection leave project/replay/owned-policy snapshots unchanged; Lesson 044 separately exhausts malformed/unbound preview, wrong-owner candidate, stale generation, and `canCommit()` rejection; classified non-OK safe states remain bound; both project preflights are observed true and precede either commit; every admitted frame commits both histories before project intent override and sets replay availability last; no artificial production failure hook is required |
| 043→045 integration | feed one Lesson 043 Current observation into L045; advance policy/frame time without a new sample sequence and require matching age-based Stale derivation; separately publish a latest not-ready poll and require matching readiness-based Stale derivation while both layers retain the same accepted payload, report zero sequence gap, emit visible ineligible/no-tone intent, and fabricate no producer or control event |
| Failure precedence | all pairwise and credible triple collisions across inertial/button/joystick status, stale, saturation, unsteady, skew, freeze, sensitivity |
| Recovery | healthy frame does not auto-clear latch; exclude each health factor independently; valid acknowledgement preserves evidence/sensitivity/epoch and does not mutate replay identity; exact pre-ack replay cannot recover; producer fault relatches while ineligible evidence remains Recovering; forward healthy recovery suppresses simultaneous freeze and sensitivity; frozen evidence retained during fault; shutdown clears retention |
| Time | zero, semantically identical equal-time replay, changed equal-time frame rejection, rollover, diagnostic epoch establishment, exact phase and two-phase edges, exact replay, acknowledgement without re-phase, illegal regression |
| Permutations | source ID, axis mapping, input member construction order, and copied presentation-channel order do not alter semantic order |
| Replay | complete golden trace byte-identical on repeated run; malformed traces fail at the same record |
| Capacity | below/at/above fixture capacity where a replay runner is used; every AVR `sizeof`; compact live/frozen evidence; replay-storage lifetime; candidate + resident composition + returned-snapshot stack peak; flash and update-work bounds |
| Output safety | every fault and ineligible quality disables tone intent; shutdown publishes canonical invalid/no-tone intent; invalid frame cannot label a prior nonfault intent current |

The golden trace includes stationary startup, self-test completion, live
level, all four tilts, sensitivity clamps, freeze, changed live evidence,
unfreeze, stale input, producer failure, recovery, saturation, timestamp wrap,
and shutdown.

### Lesson 045 compile-only Mega example and host replay evidence

`Lesson045BalanceTableInstrument.ino` is the canonical E0 project sketch.
It uses:

- constant inertial, joystick, and button copied-fixture traces;
- copied RGB, mono-diagnostic, and tone intents held in named result cells; and
- no character display, because the lesson explicitly promises presentation
  without a later display driver.

The canonical E0 path executes only in the host harness/simulator, never reads
a live control, and never actuates a physical
endpoint. Live joystick/button inputs, RGB/mono LEDs, sounder, schematics,
resistors, pins, powered-debug transport, and test points belong to a
separately qualified E1 example. Mega compilation is packaging evidence only;
the sketch must not run on a powered board under the E0 claim.
The inertial source remains synthetic in E0.

Narrative order:

1. acquire no endpoint and initialize pure policies;
2. configure board frame, validation, orientation, presentation, and project;
3. start copied-intent self-test and announce the E0 fixture source;
4. observe one atomic frame;
5. decide validation, orientation, freeze, and sensitivity;
6. copy the complete light/tone/diagnostic intent into host result cells;
7. verify every ineligible or fault record carries tone-off intent.

Non-Serial evidence:

| Evidence | E0 host-simulator path | Prediction and interpretation |
|---|---|---|
| Policy initialization | `mode` and `status` cells | `AwaitingFrame` precedes the first complete frame |
| Level | complete green light-intent cell | gravity/rate guard accepted and both angles are within threshold |
| Direction | distinct complete intent record per cardinal direction | copied tilt is valid and dominates on the named board axis |
| Frozen | mode plus complete frozen-evidence cell | retained provenance remains visible while live evidence updates |
| Stale/saturated/unsteady | complete ineligible diagnostic cell | evidence is visible and tone intent is off |
| Fault | fault intent plus three producer-status cells | project fault dominates controls without erasing attribution |
| Safe state | no-tone/invalid intent after shutdown | no physical command exists and no stale intent is labeled current |

Serial may print source, sequence, fixed-point angles, sensitivity, and status
for supporting diagnosis. It cannot be the only inspection path; named host
result cells remain stable between updates.

## Resource and size budgets

The pre-implementation budget is deliberately stricter than the board's raw
capacity. Measured values replace estimates before promotion.

### Pure E0 object budgets

| Object | Target `sizeof` on AVR | Hard review threshold |
|---|---:|---:|
| `InertialSample` | <= 52 B | 64 B |
| `InertialObservationPolicy` including snapshot/history | <= 96 B | 128 B |
| `OrientationPolicy` including config/snapshot | <= 80 B | 112 B |
| `BalancePresentationPolicy` | <= 64 B | 96 B |
| `BalanceInstrument` including owned Lesson 044 policies and compact output, excluding caller replay storage | <= 352 B | 384 B |
| `BalanceInstrumentInput` | <= 112 B | 128 B |
| `BalanceInstrumentOutput` with two compact evidence records | <= 144 B | 160 B |
| Caller-owned `BalanceFrameStorage` | <= 128 B | 144 B |
| Peak input + output + replay-storage representation | <= 388 B | 436 B |
| Resident owned-policy composition plus replay storage | <= 560 B | 688 B |
| Worst live composition + candidate input + returned snapshot copy | <= 816 B | 976 B |

Crossing a target requires explanation and a bounded reduction attempt.
Crossing a hard threshold blocks promotion pending a design stress decision.
Do not remove identity, status, range, timestamp, or calibration evidence
merely to meet a number.

The Lesson 044 AVR probe measures `BalancePresentationConfig` at 75 B and
`BalancePresentationPolicy` at 91 B after making the full-scale angle
explicit. The policy therefore exceeds its 64 B target but remains below the
96 B hard threshold. Nine complete, independently configurable light intents
account for 63 B of the configuration; retaining those semantic frames and the
4 B angle is preferred to a lossy encoding. The Lesson 045 aggregate and stack
gates remain controlling.

The final AVR GCC 7.3 measurement uses `-mmcu=atmega2560 -Os
-fno-exceptions -fno-rtti`. It measures 14 B copied joystick input, 12 B copied
button input, 76 B instrument config, 101 B project input, 34 B compact inertial
evidence, 45 B measurement evidence, 102 B replay storage, 119 B output, 23 B
orientation config, 16 B prepared orientation, 38 B orientation policy, 7 B
light intent, 5 B tone intent, 75 B presentation config, 14 B presentation
value, 20 B prepared presentation, 95 B presentation policy, 80 B inertial
observation policy, and 339 B instrument.

The instrument's revised 352 B target and 384 B hard threshold are an
owned-policy accounting decision, not permission to grow aggregate memory.
The measured 339 B includes both Lesson 044 policies and four complete 14 B
canonical project presentations. The earlier 192/208 B limits assumed a much
smaller config and did not account for that owned state. Borrowing the policies
would only move their memory outside the instrument while adding lifetime,
aliasing, and mutation coupling, so that alternative is rejected.

The measured representation total is 322 B for input, output, and replay
storage. The actual resident composition is 521 B:
`InertialObservationPolicy` 80 B + `BalanceInstrument` 339 B +
`BalanceFrameStorage` 102 B. Do not add the orientation and presentation
policies again because the instrument owns them. The worst live composition is
741 B after pessimistically adding the 101 B candidate input and 119 B returned
snapshot copy. These results pass the unchanged 560/688 B resident and
816/976 B worst-live target/hard gates.

AVR compiler stack reports for the isolated core are 399 B for `update`, 146 B
for `shutdown`, 33 B for `initialize`, 19 B for `acknowledgeFault`, 11 B for
construction, at most 7 B for helpers, and 3 B for `snapshot`. A minimal linked
core harness measures 17,156 B text, 64 B initialized data, and 1 B BSS:
17,220 B of flash including initialized data and 65 B static SRAM. It contains
no heap, RTTI, exceptions, vtable, Wire/TWI, or application timer symbols; the
ordinary AVR runtime vectors remain. These isolated-core results establish
feasibility but do not close the canonical-sketch gate.

The final canonical sketch was rebuilt with Arduino AVR core 1.8.8 and AVR
GCC 7.3.0, without link-time optimization. Compilation used `-Os`,
`-fno-exceptions`, `-fno-rtti`, `-fno-threadsafe-statics`,
`-ffunction-sections`, `-fdata-sections`, `-fstack-usage`, and
`-mmcu=atmega2560`; linking used `-Os`, `--gc-sections`, a cross-reference map,
and the same MCU. Its ELF measures 21,538 B `.text`, 238 B `.data`, and
1,660 B `.bss`: 21,776 B flash including initialized data and 1,898 B static
SRAM. Linker garbage collection was accounted for: unlinked archive stack
records were excluded from the reachable graph. The deepest direct reachable
foreground chain is `main` (3 B) to `setup` (261 B, dynamic but bounded), to
`replayFrame` (4 B), to `BalanceInstrument::update` (399 B), to
`OrientationPolicy::preview` (85 B), to `atan2MilliDegrees` (65 B), and then
the resolved libgcc 64-bit division helpers. Including three-byte Mega return
addresses, the 12-register `__divdi3` save, and the `__udivmod64` return and
one-byte save gives a conservative 851 B foreground bound. A timer-zero
overflow may preempt that path even though the application owns no timer or
interrupt; reserving its 12 B compiler frame plus the three-byte hardware
return address raises the bound to 866 B. No recursion or application indirect
call is present. The conservative remaining SRAM is therefore
`8192 - 1898 - 866 = 5428 B`, exceeding the 1,024 B gate by 4,404 B.
Startup's `init` path and global constructor are shorter and do not coexist
with the deepest foreground frames; startup enables the timer-zero interrupt,
whose possible preemption is included above.

This is a reviewed static call-chain bound for the exact final no-LTO ELF, not
a runtime canary, measured high-water mark, or physical observation. E0
prohibits the powered Mega execution required for a stack sentinel. Runtime
stack-high-water instrumentation is therefore an explicit E1 acceptance item,
not an E0 promotion gate.

### Mega aggregate budgets

| Resource | E0 budget |
|---|---|
| Static SRAM, complete Lesson 045 example | <= 2,048 B, leaving at least 6,000 B nominal headroom |
| Conservative static stack reserve | >= 1,024 B after complete-sketch static SRAM, deepest bounded no-LTO call chain, Mega return addresses, and startup/ISR preemption reserve |
| Flash | <= 28 KiB for the canonical project sketch |
| Heap | 0 B and no allocator calls |
| Application timers | zero in canonical E0; Arduino startup still links timer zero |
| Application interrupts | zero in canonical E0; the static proof reserves timer-zero ISR preemption |
| I2C claims | zero in E0 |
| ADC inputs | zero in canonical E0 |
| Digital pins | zero in canonical E0 |
| Update work | bounded O(1), no input-sized loop except a test/example replay cursor |

The exact pin table is selected only from existing qualified examples and
checked for conflicts. No inertial SDA/SCL points appear in E0. The PDF may
name the Mega's SDA/SCL locations only in a clearly deferred specimen worksheet
that contains no powered wiring instruction; the safer default is to omit
them until E1.

Future powered composition must add:

- one initialized `I2cBus`, one exact `I2cDevice`, and one exact adapter;
- bus owner and borrower lifetime;
- bounded transaction size and timeout;
- address collision analysis;
- bus pull-up and logic-level evidence;
- SDA/SCL and rail test points;
- startup, partial configuration rollback, NACK, short, stuck-bus, stale
  data-ready, saturation, shutdown, and restart evidence; and
- aggregate flash/SRAM/stack/current measurements with all diagnostics active.

## Architecture stress passes

Create three pre-implementation and three post-implementation records from
`docs/templates/component-design-stress-pass.md`:

1. `inertial-observation.md`;
2. `orientation-presentation.md`;
3. `balance-table-instrument.md`.

Each pass must assess all standard rows and the following maximum composition:

```text
one synthetic inertial producer
  -> one copied InertialObservationPolicy
  -> one OrientationPolicy
  -> one BalancePresentationPolicy
  -> one BalanceInstrument
plus copied joystick/button observations
plus copied RGB + diagnostic + tone intent records
at the fastest documented replay cadence
with stale + producer fault + button event + diagnostic failure colliding
```

Required stress evidence:

- fixed per-update operation bound and no starvation mechanism;
- aggregate object/global/stack/flash measurement;
- zero bus ownership in E0 and explicit future I2C seam;
- intentionally volatile freeze and sensitivity state;
- no actuator, external-load, or stored-energy path;
- source/sequence/time/calibration identity retained through freeze;
- diagnostics included in pin, timer, memory, and failure budgets;
- fault collision precedence and independent output-failure evidence; and
- impact review of Lessons 022, 031, 033, 040--042, and retained Lessons
  067--069.

Expected pre-pass disposition is `natural fit` or `bounded local remediation`.
Any need to change `Status`, `TimePoint`, `I2cBus`, joystick/button public
contracts, presentation ownership, or the 067--069 curriculum scope is
architectural remediation. Stop promotion, enumerate consumers, discuss the
alternatives, and record a consequential decision before changing a shared
contract.

The post-pass uses measured code and the most demanding canonical example.
Passing isolated unit tests is not sufficient.

## Publication plan

Each lesson ships one HTML reference and one complementary printable PDF.
HTML is concise API/reference material; PDF is the learner experiment and
diagnostic workbook. Both use the same fixed-point unit names, state names,
board directions, status precedence, and E0/E1 labels as code and tests.

### HTML

Lesson 043 HTML includes:

- copied sample and provenance contract;
- named fixed-point units;
- quality/status precedence;
- source/sequence/time/calibration identity;
- E0 fixture and test links;
- explicit MPU/QMI adapter deferral; and
- no wiring or powered-support claim.

Lesson 044 HTML includes:

- right-handed board-frame configuration;
- fixed-point angle method and measured error bound;
- stationarity limitations;
- orientation and intent tables;
- tie/threshold rules; and
- no navigation or motion-tracking claim.

Lesson 045 HTML includes:

- atomic frame, freeze authority, sensitivity, and fault precedence;
- stationary tabletop scope;
- replay trace and source links;
- existing indicator/test-point semantics;
- complete size evidence; and
- open exact-specimen, powered-adapter, and E1 cards.

### PDFs and visual classification

Every visual must carry the classification marker required by
`docs/PDF_POLICY.md`. Every Lesson 043--045 visual in E0 is a pencil drawing:

- hand-drawn axes and board-frame orientation;
- sample/quality timeline;
- gravity-vector and tilt explanation;
- level/cardinal/diagonal pose sequence;
- freeze/live comparison;
- project state flow;
- staged replay experiment layout;
- RGB/fault indication key; and
- diagnosis and acceptance worksheets.

No E0 visual is an electrically authoritative formal schematic. A block
diagram, pin-location sketch, breadboard orientation, timing trace, state
machine, and test-point illustration are pencil visuals even if geometrically
precise. Grayscale or a schematic-like filename does not change its class.

A future formal schematic is permitted only after the exact powered specimen
and adapter are qualified. It must be explicitly labeled electrically
authoritative, use conventional symbols and exact values/nets, pass the formal
schematic gate, and remain distinct from pencil orientation art.

Each PDF includes:

- a prominent `E0 SYNTHETIC REPLAY — NO POWERED INERTIAL MODULE CLAIM` notice;
- predict/observe/interpret steps for acquisition and safe state separately;
- fixed-point worksheets with units on every quantity;
- malformed, stale, saturation, unsteady, and fault diagnosis;
- exercises that do not require an unqualified sensor;
- exact source/example/test links;
- blank E1 acceptance fields; and
- no prefilled physical measurements.

### PDF and site gates

Run the canonical lesson build, deterministic rebuild/hash comparison,
monochrome inspection, visual classification audit, text extraction, page
count, margins, links, source/PDF divergence, site build, and internal-link
checks. Independently review that every non-schematic visual actually reads as
pencil drawing and that no prose implies an E1 measurement.

## Specimen and E1 gates

The E0 boundary may be promoted with the following work open and named:

| Gate | Closure evidence |
|---|---|
| Exact specimen identity | physical inventory photographs/markings and stable record for the actual module |
| Primary sources | manufacturer datasheet/register documentation matching that identity |
| Carrier qualification | traced or authoritative regulator, pull-up, address-select, and logic-level topology |
| Adapter implementation | one device-specific register codec, lifecycle, status, timing, and conversion contract |
| Host adapter proof | recording I2C fake covers identity mismatch, NACK, timeout, short read, bad data-ready, saturation, rollback, shutdown, restart |
| Powered schematic | exact authoritative circuit with values, rails, and test points |
| Mega measurement | aggregate flash/SRAM/stack and bus timing with the exact adapter |
| E1 bench acceptance | board/module revision, supply, instruments, tool versions, predictions, measured observations, interpretations, safe shutdown |

MPU closure does not close QMI gates, and QMI closure does not close MPU
gates. Publication may truthfully say “the E0 value can receive a future
qualified MPU6050 adapter” only while that adapter remains absent. It may not
say “supports MPU6050/QMI8658” until each named implementation passes its own
gates.

E1 must separately prove:

1. bus/resource acquisition and partial-failure rollback;
2. device identity and configured range;
3. stationary gravity and known hand-tilt response in documented units;
4. data-ready/freshness and saturation behavior;
5. SDA/SCL and rail observation at named points;
6. non-Serial direction, freeze, health, and fault indications;
7. output-failure behavior and tone silence;
8. shutdown/reset/power-removal safe state; and
9. byte-stable recorded trace replay back through the E0 policies.

Physical acceptance remains blank until measured. Host replay, successful
compilation, a datasheet, or a PDF does not substitute.

## Packaging and acceptance inventory

Expected first-class E0 inventory:

```text
src/inertial_observation.h
src/inertial_observation.cpp
src/orientation_presentation.h
src/orientation_presentation.cpp
src/balance_table_instrument.h
src/balance_table_instrument.cpp
tests/test_inertial_observation.cpp
tests/test_orientation_presentation.cpp
tests/test_balance_table_instrument.cpp
examples/Lesson043InertialObservation/Lesson043InertialObservation.ino
examples/Lesson044OrientationPresentation/Lesson044OrientationPresentation.ino
examples/Lesson045BalanceTableInstrument/Lesson045BalanceTableInstrument.ino
site/pages/lessons/043.md
site/pages/lessons/044.md
site/pages/lessons/045.md
docs/lessons/043/main.tex
docs/lessons/044/main.tex
docs/lessons/045/main.tex
docs/design/stress-passes/inertial-observation.md
docs/design/stress-passes/orientation-presentation.md
docs/design/stress-passes/balance-table-instrument.md
doc/lessons/043.pdf
doc/lessons/044.pdf
doc/lessons/045.pdf
```

Build inventories, umbrella header, archive manifest, lesson/site navigation,
component tables, curriculum/project/work-queue status, size baseline, and
post-deploy newest-lesson checks update in the same integration boundary.
Generated PDFs accompany their sources. Shared ledgers do not claim host
verification until every non-hardware gate passes.

Required final gates:

1. strict host tests with exceptions and RTTI disabled;
2. sanitizer tests and deterministic replay;
3. style and standalone-header compilation;
4. canonical Mega 2560 builds and reviewed size baseline;
5. complete lesson/PDF policy gates and deterministic PDF rebuild;
6. site, link, package-consumer, lint, and archive checks;
7. pre/post stress-pass review with exact aggregate measurements;
8. independent code, example, publication, safety, and adversarial reviews;
9. AIQ task-state reconciliation and coherent dependency-ordered commits; and
10. clean release/publication audit while preserving every open E1 card.

## Design risks and bounded responses

| Risk | Consequence | Bounded response |
|---|---|---|
| A “common adapter” hides incompatible chips | false electrical/register support | common copied value only; separate exact adapter seams |
| Fixed-point fields imply unproved physical accuracy | misleading units | synthetic provenance in E0; datasheet conversion and bench uncertainty required in E1 |
| Orientation consumes later normalization | duplicate or collapsed 067--069 scope | one-source board-frame policy only; retain calibration/provenance qualification and recorder later |
| Dynamic acceleration is read as tilt | incorrect learner conclusion | stationary gravity/rate guard, tabletop scope, `Unsteady` state |
| Freeze hides a current fault | stale good-looking indication | health always updates; fault presentation dominates retained frozen evidence |
| Diagnostics change behavior | timing/resource coupling | intent-only policies; include outputs in aggregate budget and collision tests |
| Approximation buckles near thresholds | nondeterministic direction/level | full-domain oracle sweep, error band, explicit ties |
| Copied frames inflate AVR SRAM | composition failure | early `sizeof`, stack, and aggregate measurements; redesign locally before promotion |
| Live joystick plus replay obscures lesson | narrative overload | use recorded joystick/button frames if existing live composition is not clean |
| Future powered work leaks into E0 prose | unsupported safety claim | explicit E0 banners, no wiring/schematic, independent publication review |

## Promotion definition

Lessons 043--045 are host verified only when the copied-value, orientation,
and project APIs pass their deterministic and aggregate gates; the examples
compile with measured budgets; HTML and pencil-visual PDFs agree; navigation,
packaging, ledgers, and live-publication checks agree; and independent review
finds no unsupported electrical, motion, navigation, or physical-evidence
claim.

Promotion wording remains:

> Lessons 043--045 are host verified as an E0 synthetic-replay balance-table
> instrument. Exact MPU6050/QMI8658 specimens, powered adapters, authoritative
> schematics, and E1 bench acceptance remain open independently.

Anything stronger waits for the corresponding specimen and E1 evidence.
