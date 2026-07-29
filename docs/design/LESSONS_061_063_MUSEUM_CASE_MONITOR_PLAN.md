# Lessons 061--063 museum-case monitor plan

Status: implementation-depth E0 plan; exact powered specimens, electrical
adapters, persistence, and bench acceptance remain open.

This arc qualifies copied resistive-probe, thermal, and radiant observations
and composes them with copied reed-contact evidence into an inert museum-case
monitor. E0 owns no ADC channel, GPIO pin, clock, bus, sensor, excitation
supply, relay, lamp, display, RTC, SD card, or persistent medium. It proves
bounded policy, provenance, fault precedence, alarm intent, and deterministic
audit-record intent from synthetic fixtures.

The authorized inventory lists `Water Level`, `Thermistor`, `Digital
Temperature`, `Flame`, and `Magnetic Spring` families. Listing authorization
is not electrical identity. In particular, `Digital Temperature` is not an
18B20 and must not be assigned a protocol, units, pinout, or powered adapter
until an exact specimen proves those facts. “Flame” means passive radiant
observation only. No flame, heater, hot wire, combustible target, ignition,
pyrotechnic device, or unknown IR replay is authorized.

## Evidence levels

| Level | Authorized work |
|---|---|
| E0 | Pure policy over copied synthetic samples, supplied time, fixed calibration/configuration identity, inert presentation/relay intent, and caller-recorded audit receipts |
| E1a | Exact conductive water-probe fixture, switched excitation, ADC path, corrosion/current/duty evidence, spill containment, drying, and acceptance |
| E1b | Each exact thermistor, distinct Digital Temperature, and radiant module qualified independently before its adapter, wiring, units, or threshold polarity is claimed |
| E1c | Exact reed, RGB/LCD/status presentation, and their non-Serial observation paths qualified independently |
| E1d | Combined passive monitor with simultaneous current, rail, age, disagreement, shutdown, and physical observation evidence |
| E2 | Exact extra-low-voltage inert relay/lamp fixture with independent power removal, contact/coil evidence, and restrained acceptance; no mains or safety function |

RTC and removable-media hardware remain separately deferred under Lessons 022
and 024. E0 emits copied audit-record intent and accepts copied recorder
receipts; it does not claim durable storage, wall-clock accuracy, atomic media
commit, wear behavior, or recovery after power loss.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Owns at E0 |
|---:|---|---|---|
| 061 | `ResistiveProbeObservationPolicy` | `Status`, `TimePoint`, copied excitation/sample evidence | Fixed calibration, freshness/order state, one qualified observation, and corrosion-duty accounting |
| 062 | `ThermalRadiantObservationPolicy` | `Status`, `TimePoint`, copied already-converted thermal and raw threshold/radiant evidence | Independent source qualification, freshness, uncertainty, threshold state, disagreement, and saturation |
| 063 | `MuseumCaseMonitor` | Lessons 061--062, copied reed evidence, supplied time, copied audit receipts | Alarm/cooldown state, acknowledgement policy, inert presentation/relay intent, and a bounded audit transaction |

Implementation order is strict:

1. review this E0 plan and the three initial stress passes;
2. implement and exhaustively test Lesson 061;
3. reassess the Lesson 061 stress pass against exact size evidence;
4. implement and exhaustively test Lesson 062;
5. reassess the Lesson 062 stress pass against exact size evidence;
6. implement Lesson 063 only from the two promoted observations and copied
   reed evidence;
7. add compile-only Mega replays, exact size/resource evidence, HTML, and
   pencil-drawing PDFs;
8. run terminal stress passes and the full non-hardware publication gates;
9. leave every powered-specimen and physical-acceptance card explicitly open.

No E0 sketch is permission to wire or energize a sensor. A powered Mega,
indicator, display, probe, module, relay, or debug transport is E1/E2 work.

## Shared observation rules

Every copied input names a nonzero source ID, nonzero configuration revision,
nonzero calibration revision when conversion or thresholds depend on
calibration, producer sequence, observation time, and producer `Status`.
Policies copy complete inputs and retain no caller pointer.

Supplied `TimePoint now` is the only policy clock. All maximum ages, duty
windows, confirmation intervals, cooldowns, and receipt deadlines are
strictly below the modular half range. Equal timestamps are idempotent only
for byte-identical duplicates. Changed duplicates, backward time, exact
half-range ambiguity, sequence regression, and source/configuration changes
reject atomically. Sequence exhaustion faults before wrapping to zero.

Domain quality is a value, not a replacement status convention. `Status`
reports malformed arguments, lifecycle misuse, or producer failure.
`ProbeQuality`, `ThermalQuality`, `RadiantQuality`, and `MuseumCaseHealth`
report valid but unhealthy domain outcomes such as dry, wet, stale,
saturated, disagreement, or faulted. Invalid or stale sensing can never
produce `Healthy`.

## Lesson 061 -- resistive-probe observations

### Responsibility

`ResistiveProbeObservationPolicy` qualifies one copied conductive-probe sample
against fixed dry/wet references, explicit excitation evidence, freshness,
and a bounded corrosion-duty contract. It does not switch power, read an ADC,
infer liquid identity, estimate depth, compensate contamination, or certify
the absence of a leak.

The producer supplies both the energized reading and the discharged/off
reading from one named acquisition cycle. The off reading makes a stuck-high,
backfeed, or incomplete-discharge condition visible. E0 synthetic fixtures
model that evidence; only E1a may establish that a physical circuit actually
removed excitation.

### Proposed public surface

```cpp
enum struct ProbeQuality : uint8_t
{
    Unqualified,
    Dry,
    Damp,
    Wet,
    Saturated,
    Disconnected,
    ExcitationFault,
    Stale,
    ProducerFault
};

struct ResistiveProbeSample
{
    uint8_t   sourceId;
    uint16_t  configurationRevision;
    uint16_t  calibrationRevision;
    uint32_t  sequence;
    TimePoint observedAt;
    uint16_t  energizedRaw;
    uint16_t  dischargedRaw;
    Duration  excitationOnTime;
    Duration  cycleTime;
    bool      excitationObservedOffAfterSample;
    Status    status;
};

struct ResistiveProbeConfig
{
    uint16_t adcMaximum;
    uint16_t dryReference;
    uint16_t wetReference;
    uint16_t disconnectedMaximum;
    uint16_t dischargedMaximum;
    uint16_t dampThresholdPermille;
    uint16_t wetThresholdPermille;
    Duration maximumAge;
    Duration maximumExcitationOnTime;
    uint16_t maximumDutyPermille;
};

struct ResistiveProbeObservation
{
    ResistiveProbeSample sample;
    uint16_t              normalizedPermille;
    uint16_t              observedCycleDutyPermille;
    ProbeQuality          quality;
    Duration              age;
    Status                status;
};

struct ResistiveProbeObservationPolicy
{
    explicit ResistiveProbeObservationPolicy (
        const ResistiveProbeConfig& config) noexcept;

    Status                    initialize  () noexcept;
    void                      reset       () noexcept;
    Status                    update      (
        TimePoint now, const ResistiveProbeSample& sample) noexcept;
    ResistiveProbeObservation snapshot    () const noexcept;
    bool                      initialized () const noexcept;
};
```

The exact spelling may change during implementation review, but the
information boundary may not be weakened silently. `dryReference` and
`wetReference` must be distinct; either slope is permitted and normalization
is monotonic between them. Thresholds are ordered in `0..1000`.
`adcMaximum` is the producer's declared ADC full scale and must be nonzero;
all raw values and calibration points must be at or below it.
`dischargedMaximum` is evaluated independently from liquid classification.
`cycleTime` is nonzero, excitation time cannot exceed it, and the widened
`onTime * 1000 / cycleTime` calculation cannot overflow.

The E0 disconnected predicate is exact and deliberately narrow:
`energizedRaw <= disconnectedMaximum` and
`dischargedRaw <= disconnectedMaximum`. Full-scale energized input is
`Saturated`, never disconnected or wet. Values between those predicates are
classified only by the calibrated slope.

`observedCycleDutyPermille` describes only the supplied acquisition cycle.
The policy retains no rolling energy history and cannot prove long-term
corrosion duty, cumulative energized time, or compliance between samples.
E1a must measure the physical excitation waveform over the complete campaign.

A valid update applies this precedence:

1. structural identity, time, sequence, and excitation-cycle validation;
2. producer failure;
3. missing off evidence, excessive off reading, excessive on-time, or
   excessive duty;
4. exact ADC full-scale saturation or the exact two-low-values disconnected
   predicate;
5. freshness;
6. normalized dry/damp/wet classification.

The policy must not silently reinterpret rail saturation as “very wet.”
Disconnected detection is limited to the fixed synthetic/E1-qualified
contract; the lesson states that arbitrary contamination, ionic content,
geometry, and corrosion prevent absolute liquid-depth claims.

### Deterministic proof

Tests cover both calibration slopes; every ADC endpoint; references and
thresholds immediately below/at/above; dry-to-wet and contamination-drift
ramps; open, rail, stuck, incomplete-discharge, backfeed, duty, stale,
producer-fault, duplicate, regression, rollover, reset, and replay traces.
The Mega replay uses supplied copied samples and a named result cell; it does
not energize a probe.

## Lesson 062 -- thermal and radiant observations

### Responsibility

`ThermalRadiantObservationPolicy` preserves three independent copied sources:
an already-converted thermistor temperature with uncertainty, a distinct
Digital Temperature threshold observation, and a raw radiant threshold
observation. It validates each source independently and reports agreement,
freshness, saturation, and hazard state without pretending the three devices
share a transport, unit, calibration, or electrical adapter.

Only the thermistor path carries temperature units at E0, and only because its
synthetic producer explicitly supplies milli-degrees Celsius plus a
calibration revision and uncertainty. A future exact thermistor adapter owns
divider conversion and its proved fixed-point curve. The unidentified Digital
Temperature source remains categorical `BelowThreshold`/`AtOrAboveThreshold`
evidence. Radiant evidence remains unitless raw/threshold state; it is not
temperature, flame presence, irradiance, or a life-safety alarm.

### Proposed public surface

```cpp
enum struct ThresholdState : uint8_t
{
    Below,
    AtOrAbove
};

enum struct ThermalQuality : uint8_t
{
    Unqualified,
    Normal,
    Warning,
    Alarm,
    Disagreement,
    Saturated,
    Stale,
    ProducerFault
};

enum struct RadiantQuality : uint8_t
{
    Unqualified,
    Quiet,
    AbruptChange,
    Sustained,
    SaturatedAmbient,
    Stale,
    ProducerFault
};

struct ConvertedThermalSample
{
    uint8_t   sourceId;
    uint16_t  configurationRevision;
    uint16_t  calibrationRevision;
    uint32_t  sequence;
    TimePoint observedAt;
    int32_t   milliCelsius;
    uint32_t  uncertaintyMilliCelsius;
    bool      saturated;
    Status    status;
};

struct CategoricalThresholdSample
{
    uint8_t        sourceId;
    uint16_t       configurationRevision;
    uint16_t       calibrationRevision;
    uint32_t       sequence;
    TimePoint      observedAt;
    uint16_t       raw;
    ThresholdState state;
    bool           saturated;
    Status         status;
};

struct ThermalRadiantEnvelope
{
    ConvertedThermalSample   thermistor;
    CategoricalThresholdSample digitalTemperature;
    CategoricalThresholdSample radiant;
};

struct ThermalRadiantConfig
{
    int32_t  warningMilliCelsius;
    int32_t  alarmMilliCelsius;
    Duration maximumAge;
    Duration radiantPulseMaximum;
    Duration radiantSustainedMinimum;
};

struct ThermalRadiantObservation
{
    ThermalRadiantEnvelope envelope;
    ThermalQuality         thermalQuality;
    RadiantQuality         radiantQuality;
    Duration               thermistorAge;
    Duration               digitalTemperatureAge;
    Duration               radiantAge;
    bool                   thermalHazard;
    bool                   radiantHazard;
    Status                 status;
};

struct ThermalRadiantObservationPolicy
{
    explicit ThermalRadiantObservationPolicy (
        const ThermalRadiantConfig& config) noexcept;

    Status                     initialize  () noexcept;
    void                       reset       () noexcept;
    Status                     update      (
        TimePoint now, const ThermalRadiantEnvelope& envelope) noexcept;
    ThermalRadiantObservation snapshot    () const noexcept;
    bool                       initialized () const noexcept;
};
```

Source identities for the three roles must be distinct. The thermistor warning
is below the alarm threshold. The copied value and uncertainty define widened
explanatory endpoints `lower = value - uncertainty` and
`upper = value + uncertainty`. The policy computes only the upper endpoint
because it alone controls conservative hazard classification. `upper <
warning` is `Normal`; `upper >= alarm` is `Alarm`; otherwise `upper >=
warning` is `Warning`. Equality and any uncertainty interval that reaches
alarm are therefore conservatively alarm.

The Digital Temperature mapping is fixed: `AtOrAbove` means categorical
`Alarm`; `Below` means categorical normal. `AtOrAbove` makes the combined
thermal result `Alarm`. `Below` paired with thermistor `Warning` or `Alarm`
is `Disagreement`; `thermalHazard` remains true when the thermistor interval
reaches alarm, so disagreement cannot suppress a possible alarm. Stale,
saturated, or producer-fault evidence is never used to vote.

Radiant state owns `activeSince`, last accepted sequence/time, and whether a
candidate is active. An inactive-to-active edge starts it. While active,
elapsed `< radiantSustainedMinimum` is `AbruptChange`; equality and later are
`Sustained`. An active-to-inactive edge with total elapsed
`<= radiantPulseMaximum` reports `AbruptChange`; a longer completed candidate
is conservatively `Sustained`. Configuration requires nonzero
`radiantPulseMaximum < radiantSustainedMinimum`, both wrap-safe. A
byte-identical duplicate is idempotent and cannot extend duration; a changed
duplicate rejects atomically. `reset()` clears candidate duration and accepted
identity and publishes `Unqualified`. Ambient saturation clears the candidate.

No API accepts wavelength, protocol bytes, flame labels, or an arbitrary
captured IR command. Lasers are prohibited. E1 radiant stimulus is an owned,
documented harmless low-energy IR target/source pair or an owned TV remote
used only as a stimulus under a configured maximum exposure of 5 seconds and
at least 30 seconds inactive before another exposure. It is never replayed,
aimed at an eye, used continuously/unattended, or used with an unknown target.

### Deterministic proof

Tests cover signed temperature extrema without overflow; uncertainty intervals
immediately below/at/across both thresholds; every thermal/categorical
agreement combination; radiant pulse and sustained boundaries; ambient
saturation; source collision; stale/future/regressing observations; changed
duplicates; producer faults; rollover; reset; and byte-stable replay. The
Mega replay is synthetic and unpowered.

## Lesson 063 -- museum-case monitor

### Responsibility

`MuseumCaseMonitor` consumes one complete copied hazard frame: the latest
Lesson 061 observation, latest Lesson 062 observation, copied reed-contact
evidence, operator acknowledgement, and a copied audit receipt. It owns the
alarm state machine and inert output intent. It does not resample sources,
drive a relay, write storage, read an RTC, or infer that delayed observations
were simultaneous.

### Proposed public surface

```cpp
enum struct MuseumCaseHealth : uint8_t
{
    Qualifying,
    Healthy,
    Warning,
    Alarm,
    Fault,
    Cooldown
};

enum struct MuseumHazard : uint8_t
{
    None       = 0,
    Liquid     = 1,
    Thermal    = 2,
    Radiant    = 4,
    Opening    = 8,
    Sensing    = 16,
    Recording  = 32
};

struct MuseumReedEvidence
{
    uint8_t   sourceId;
    uint16_t  configurationRevision;
    uint32_t  sequence;
    MagneticObservation observation;
};

struct MuseumAcknowledgeEvidence
{
    uint8_t   sourceId;
    uint16_t  configurationRevision;
    uint32_t  sequence;
    TimePoint observedAt;
    bool      pressed;
    Status    status;
};

struct MuseumAuditIntent
{
    uint32_t       ownerToken;
    uint32_t       lifecycleGeneration;
    uint16_t       configurationRevision;
    uint32_t       recordSequence;
    TimePoint      observedAt;
    MuseumCaseHealth health;
    uint8_t        hazardMask;
    uint32_t       liquidSequence;
    uint32_t       thermistorSequence;
    uint32_t       digitalTemperatureSequence;
    uint32_t       radiantSequence;
    uint32_t       reedSequence;
    uint32_t       acknowledgeSequence;
    uint32_t       witnessDigest;
    TimePoint      issuedAt;
    uint8_t        attempt;
};

struct MuseumAuditReceipt
{
    uint32_t ownerToken;
    uint32_t lifecycleGeneration;
    uint16_t configurationRevision;
    uint32_t recordSequence;
    TimePoint observedAt;
    MuseumCaseHealth health;
    uint8_t  hazardMask;
    uint32_t liquidSequence;
    uint32_t thermistorSequence;
    uint32_t digitalTemperatureSequence;
    uint32_t radiantSequence;
    uint32_t reedSequence;
    uint32_t acknowledgeSequence;
    uint32_t witnessDigest;
    uint8_t  attempt;
    bool     accepted;
    Status   status;
};

struct MuseumCaseConfig
{
    uint32_t ownerToken;
    uint16_t configurationRevision;
    uint8_t  expectedLiquidSourceId;
    uint16_t expectedLiquidConfigurationRevision;
    uint16_t expectedLiquidCalibrationRevision;
    uint8_t  expectedThermistorSourceId;
    uint16_t expectedThermistorConfigurationRevision;
    uint16_t expectedThermistorCalibrationRevision;
    uint8_t  expectedDigitalTemperatureSourceId;
    uint16_t expectedDigitalTemperatureConfigurationRevision;
    uint16_t expectedDigitalTemperatureCalibrationRevision;
    uint8_t  expectedRadiantSourceId;
    uint16_t expectedRadiantConfigurationRevision;
    uint16_t expectedRadiantCalibrationRevision;
    uint8_t  expectedReedSourceId;
    uint16_t expectedReedConfigurationRevision;
    uint8_t  expectedAcknowledgeSourceId;
    uint16_t expectedAcknowledgeConfigurationRevision;
    Duration maximumReedAge;
    Duration maximumAcknowledgeAge;
    Duration healthyCooldown;
    Duration auditReceiptDeadline;
    uint8_t  maximumAuditAttempts;
};

struct MuseumCaseEnvelope
{
    TimePoint                    now;
    ResistiveProbeObservation    liquid;
    ThermalRadiantObservation    environment;
    MuseumReedEvidence           reed;
    MuseumAcknowledgeEvidence    acknowledge;
    bool                         hasAuditReceipt;
    MuseumAuditReceipt           auditReceipt;
};

struct MuseumCaseIntent
{
    uint32_t         ownerToken;
    uint32_t         lifecycleGeneration;
    uint16_t         configurationRevision;
    MuseumCaseHealth health;
    uint8_t          hazardMask;
    uint8_t          rgbBlinkCode;
    bool             lcdShowsAgeOrFault;
    bool             alarmSoundIntent;
    bool             inertRelayLampIntent;
    bool             alarmOutputInactive;
};

struct MuseumCaseResult
{
    MuseumCaseIntent intent;
    bool              hasAuditIntent;
    MuseumAuditIntent auditIntent;
    Status            status;
};

struct MuseumCaseMonitor
{
    explicit MuseumCaseMonitor (const MuseumCaseConfig& config) noexcept;

    Status           initialize (TimePoint now) noexcept;
    Status           reset      (TimePoint now) noexcept;
    MuseumCaseResult update     (const MuseumCaseEnvelope& envelope) noexcept;
    Status           shutdown   () noexcept;
    MuseumCaseIntent snapshot   () const noexcept;
    bool             initialized() const noexcept;
};
```

The implementation may replace the bit mask with explicit fields if that is
safer under the repository style, but it must retain simultaneous hazards and
source attribution. A single “highest alarm” enum is insufficient evidence.

The reed wrapper preserves the complete qualified `MagneticObservation`:
`source == ContactDigital`, raw value, raw level, observation time, polarity,
activation/deactivation edges, active state, `stableFor`, quality, and status.
It adds only identity, configuration, and sequence. The monitor never reduces
this to a bare Boolean. A qualified active contact means case closed; a
qualified inactive contact means **reed-open / case-open** and raises
`Opening`. Producer tests cover both electrical polarities.

### State and precedence

Structural invalidity rejects atomically before semantic precedence. For a
valid frame:

1. shutdown/lifecycle state prevents any active output intent;
2. sensing invalidity or staleness produces `Fault`, never `Healthy`;
3. liquid wet, thermal alarm, radiant hazard, or reed-open/case-open evidence latches
   `Alarm`;
4. lesser damp/thermal uncertainty/disagreement conditions produce `Warning`;
5. acknowledgement records operator intent but cannot clear an active hazard;
6. after every hazard is absent and every source is current, acknowledgement
   starts a bounded cooldown;
7. any hazard or sensing fault during cooldown relatches alarm/fault;
8. only a completed healthy cooldown returns to `Healthy`.

A record-creating decision publishes one outstanding immutable audit intent. Its full
witness contains owner, lifecycle and configuration revisions, record
sequence, observation time, health/hazard mask, and all six contributing
producer sequences. `issuedAt` and `attempt` are delivery metadata and are not
part of the immutable event witness. `witnessDigest` is 32-bit FNV-1a (offset `0x811c9dc5`,
prime `0x01000193`) over ASCII `ADK.MUSEUM.AUDIT.V1` without NUL, then
`ownerToken`, `lifecycleGeneration`, `configurationRevision`,
`recordSequence`, `observedAt`, `health`, `hazardMask`, `liquidSequence`,
`thermistorSequence`, `digitalTemperatureSequence`, `radiantSequence`,
`reedSequence`, and `acknowledgeSequence`, in that order. Integers are
little-endian and enums/masks one byte. Tests freeze one literal field vector
and digest. Full fields are compared; the digest never substitutes for the
witness.

The canonical witness is owner `0x01020304`, lifecycle `0x11223344`,
configuration `0x5566`, record `0x778899aa`, observed milliseconds
`0x0a0b0c0d`, health `Alarm` (`0x03`), hazard mask `0x19`, and producer
sequences `{1,2,3,4,5,6}` in the order above. Its digest is `0x4086e509`.

Exactly one record may be outstanding. Each update returns at most that one
immutable intent and never calls storage. A receipt must match owner,
lifecycle, configuration revision, record sequence, every copied witness
field, digest, and current attempt. Acceptance retires the record. Failure or
deadline loss adds `Recording`, increments the bounded
attempt, and reissues the same witness on a later call; it never retries in a
loop or changes sequence/digest. Missing receipt through the inclusive
deadline remains pending; the next valid tick records loss. At
`maximumAuditAttempts`, failure is terminal until explicit `reset()`. Foreign,
stale, future, crossed, wrong-digest, or changed-duplicate receipts reject
atomically. An identical duplicate of a retired accepted receipt is
idempotent.

Record creation uses one exact semantic decision key:
`{health, hazardMask}`. Producer sequences and observation time are witness
fields, but a fresh sample with the same semantic key does not by itself
create another record. After each `initialize()` or `reset()`, the first valid
complete expected-source frame creates an initial record for its derived key,
including `Healthy`, `Warning`, `Alarm`, `Fault`, or `Cooldown`. The
construction-time and just-initialized `Qualifying` output is not a sensed
decision and creates no record; a later transition back to `Qualifying`
caused by valid policy state does create one.

After that initial record, every change of either key field creates a record
when no record is outstanding. This includes every transition into or out of
`Healthy`, `Warning`, `Alarm`, `Fault`, `Cooldown`, or evidence-driven
`Qualifying`; every recovery transition; and every addition, removal, or
combination change in `Liquid`, `Thermal`, `Radiant`, `Opening`, `Sensing`, or
`Recording`, even when `health` is unchanged. Acknowledgement creates a record
only when it changes health or hazard mask, such as entering cooldown; a
repeated/ineffective acknowledgement does not. Shutdown publishes inactive
intent and invalidates audit state but does not claim a record because E0 has
no storage authority during teardown. Reinitialize's first complete decision
starts the new lifecycle's audit sequence as specified above.

One additional fixed `dirtySuccessor` slot prevents decision changes during a
pending record from disappearing. On every valid update, the monitor derives
the complete current witness after receipt structural validation. If it
has a semantic decision key different from the latest qualifying key already
represented by the outstanding record or dirty slot, `dirty` becomes true and
the slot is replaced with the latest complete witness fields. Thus every
qualifying key change is noticed, while several changes may coalesce only to
the latest successor the one-slot contract can retain. A same-key fresh
sample merely refreshes live provenance and does not dirty the slot. The slot
has no record sequence, digest, delivery attempt, or deadline until promotion,
so it cannot masquerade as a submitted record.

When a matching accepted receipt and changed decision arrive in the same
update, order is fixed: validate the entire envelope and receipt; derive and
store the latest successor; retire the accepted record; increment
`recordSequence`; bind that sequence and the existing lifecycle/configuration
to the successor; compute its digest; and return the successor immediately as
attempt one. If `dirty` was already set, the current update replaces it before
promotion. No intermediate state is claimed durable, but the latest decision
can never be silently treated as covered by the retired witness.

Failed/lost receipt processing occurs before successor promotion and retains
both the immutable outstanding record and latest dirty slot. Terminal retry
exhaustion keeps both frozen, asserts `Recording` and `Fault`, emits no further
delivery attempt, and cannot become healthy. Only lifecycle-invalidating
`reset()`/`shutdown()` may discard them; that explicit discard is tested and
never reported as audit success. Record-sequence exhaustion during promotion
likewise faults inactive without overwriting either witness.

The canonical collision trace publishes alarm record 41, changes liquid then
thermal evidence while attempt one is pending, fails attempt one, changes
radiant and reed-open evidence while attempt two is pending, and supplies the
matching accepted attempt-two receipt in the same update as acknowledgement
changes. The result retires 41 and immediately publishes record 42 containing
the latest six producer sequences and hazard state, not either intermediate
witness. A variant exhausts the final attempt, proves record 41 plus its latest
dirty successor remain frozen with no retry, then proves reset invalidates
both and advances lifecycle generation.

The record-predicate matrix additionally proves: first complete `Healthy`,
`Warning`, `Alarm`, `Fault`, and `Cooldown` decisions each create an initial
record in separate lifecycle fixtures; same-key sequence/age refresh creates
none; every health transition creates one; each individual hazard bit add and
remove creates one while health is held constant; combined-mask change creates
one; ineffective acknowledgement creates none; acknowledgement-to-cooldown
and cooldown-to-healthy recovery each create one; fault-to-warning and
fault-to-healthy recovery each create one; evidence-driven `Qualifying`
creates one; shutdown creates none; and reinitialize creates exactly one
record for its first complete decision.

`recordSequence` and nonzero `lifecycleGeneration` use modular half-range
ordering and fault before zero. Construction is inert. `initialize()` advances
lifecycle generation and starts qualifying with inactive outputs.
`reset(now)` advances lifecycle generation, invalidates old receipts, clears
audit retry/recording fault, alarm/cooldown and acknowledgement history, and
returns to qualifying without treating present hazards as cleared.
`shutdown()` advances lifecycle generation, invalidates receipts, clears the
pending intent, and publishes canonical inactive outputs; repeated shutdown
is idempotent. Reinitialize advances again and requires fresh expected
observations. Generation exhaustion faults inactive and reset cannot recover
it.

Configuration rejects zero owner/revisions/source IDs/ages/deadline/attempt
limit, duplicate role source IDs, non-wrap-safe durations, and any
`maximumAuditAttempts` outside `1..8`. An attempt starts at one.

While a receipt is outstanding, newer hazards still update the live alarm
mask but do not overwrite the frozen record. Recording failure never
suppresses alarm/presentation intent or claims durability. Every input is
checked against the exact source/configuration/calibration identities in
`MuseumCaseConfig`; zero, duplicate role identity, changed identity, or
revision mismatch rejects before semantic precedence.

RGB semantics are healthy/warning/alarm/fault with a grayscale-safe blink
code. LCD intent always requires age or fault visibility. Sound and relay-lamp
intents are semantic only at E0. Shutdown and every not-initialized result
publish `alarmSoundIntent == false`, `inertRelayLampIntent == false`, and
`alarmOutputInactive == true`. `Healthy` requires every source fresh,
qualified, expected and nonhazardous, no pending/terminal recording fault, and
completed qualification/cooldown. The future relay may drive only an
extra-low-voltage current-limited inert lamp; no mains, lock, access control,
confinement, egress, or safety load is permitted.

### Maximum composition proof

The maximum E0 fixture owns one instance of each policy, retains one copied
input/output frame per source, one outstanding audit intent/receipt, and fixed
result cells. It drives dry-to-wet contamination drift while temperature
uncertainty crosses both boundaries, a radiant pulse becomes ambient
saturation, the reed opens, acknowledgement arrives, the audit write is
interrupted, time crosses rollover, and shutdown/restart occurs with faults
still present.

One `update()` performs bounded work with no allocation, recursion, callbacks,
polling, retry loop, catch-up loop, transport call, or endpoint access. The
resource gate measures each object, caller-owned frames, aggregate static
SRAM, synchronous stack, flash, and residual Mega SRAM. Capacity is checked
below, at, and above the single outstanding record limit. Replay must be
byte-identical.

## Canonical E0 resource gates

These are the only planning numbers; the stress passes reference this table.

Resource-review: lesson=062 metric=ordinary_static_sram observed=1133
target=1024 hard=1536 disposition=accepted-target-miss

| Lesson | Flash target/hard | Static SRAM target/hard | Stack target/hard | Object target/hard |
|---:|---:|---:|---:|---:|
| 061 | 8/12 KiB | 768/1,024 B | 320/448 B | 192/256 B |
| 062 | 12/16 KiB | 1,024/1,536 B | 384/576 B | 320/448 B |
| 063 aggregate | 24/32 KiB | 2,048/3,072 B | 640/896 B | 768/1,024 B |

Resource-review: lesson=062 metric=ordinary_static_sram observed=1133 target=1024 hard=1536 disposition=accepted-target-miss

Exact probes measure ordinary sketch flash/static SRAM, no-LTO flash,
synchronous stack, each child and complete coordinator object, every
caller-owned input/output/intent/receipt buffer, aggregate live storage, and
residual Mega SRAM. They assert every public enum's size, every public
structure's size/alignment/trivial-copy/destruction expectations, and each
owning policy/coordinator's noncopy/nonmove properties. Header review rejects
hidden large returns, accidental by-value aggregate copies on the maximum call
path, recursion, virtual dispatch, heap use, or compiler-hidden temporary
pressure omitted from the stack probe. Target misses require reviewed
rationale; hard or residual failures block promotion.

Residual SRAM is exactly
`8192 - measured_static_sram - measured_synchronous_stack - 128`, where 128 B
is the fixed interrupt/ambient reserve. The residual target is 3,072 B and the
non-reviewable hard floor is 2,048 B. Each individual caller-owned buffer has
a 256 B target and a non-reviewable 512 B hard limit. Caller-owned buffers in
the maximum-composition fixture are instantiated and measured exactly once;
aliases and parameter views are not counted as additional storage.

Every resource result records a machine-readable fingerprint of compiler
executable/version, Arduino AVR core package/version, board/F_CPU, linker,
LTO state, optimization, language standard, defines, include paths, and exact
compile/link flags. Synchronous stack evidence is generated from the compiler
callgraph and includes three bytes for every retained return-address edge.
Indirect/unknown edges, recursion, missing callgraph nodes, or a fingerprint
change fail the gate rather than receiving an estimate.

Reviewed target misses live in machine-readable markers naming lesson, metric,
observed value, target, hard limit, rationale, reviewer, and exact fingerprint.
The checker fails stale markers whenever source, probe, observed value,
threshold, or fingerprint changes. Hard-limit and residual-hard-floor failures
are never reviewable. The maximum-composition proof compiles and links
`examples/Lesson063MuseumCaseMonitor/Lesson063MuseumCaseMonitor.ino` with
Lessons 061/062, the monitor, full caller-owned fixture/result cells, audit
outstanding/dirty slots, and configured diagnostics; an isolated object probe
cannot substitute for that linked result.

## Publication and physical boundaries

Each lesson requires deterministic host tests, strict native warnings,
sanitizers, one compile-only Mega replay, exact ordinary and no-LTO resource
evidence, HTML reference, rich pencil-drawing PDF, and explicit open physical
cards. Non-schematic visuals use pencil presentation. A formal schematic is
not permitted until its exact specimen and electrical evidence exist.

Future E1 work must record exact markings, PCB revision, pin order, supply and
output rails, pull-ups, comparator topology/polarity, threshold direction,
ADC reference/source impedance, powered-off/backfeed behavior, current,
corrosion/drying procedure, safe stimulus, and non-Serial observation path.
Combined work additionally measures simultaneous current, rail droop,
diagnostic interference, stale/fault indication, startup/reset/shutdown, and
physical power removal. E2 relay work separately records coil/load supplies,
driver/flyback/opto topology, contact isolation, inactive level, stuck/open
faults, current, lamp behavior, and independent removal.

Host verification never establishes liquid detection accuracy, temperature
accuracy, irradiance, flame detection, security, preservation suitability,
storage durability, relay safety, or physical alarm behavior.

## Initial architecture disposition

The four-file pre-implementation review concludes `natural fit` for E0 after
fixing the ADC/disconnected and per-cycle-duty boundary, conservative thermal
and categorical mapping, complete radiant state machine, exact monitor source
identities, full copied magnetic evidence, bounded audit witness/recovery, and
lifecycle/output invariants. Implementation is permitted only inside these
fixed copied-value contracts and the canonical resource gates. Any powered
adapter, generalized switched-power endpoint, durable recorder, shared bus,
calibrated radiant unit, relay driver, or changed prior public interface
requires its separately gated evidence and, where cross-cutting, a new
architecture decision.
