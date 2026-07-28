# Lessons 037–039 percussion laboratory plan

Status: active integration, 2026-07-28. Lessons 037--039 E0 cores, tests, and
hardware-independent drafts are complete. The linked exact-specimen Mega
example and authoritative schematic may now proceed against the documented
reference fixtures below. Incoming conformance and E1 physical acceptance
remain open. No
powered specimen, sound-pressure measurement, or physical acceptance is
claimed.

## Scope and specimen boundary

This block teaches the difference between raw mechanical contact, a qualified
contact event, an analog acoustic envelope, and a replayed musical cue. The
authorized Elegoo union lists `Tilt-Switch`, `Tap Module`, `Shock`,
`Big Sound`, `Small Sound`, active buzzer, and passive buzzer. Those labels do
not establish circuit topology, output type, polarity, pin order, voltage
range, comparator pull-up rail, sensitivity, or acoustic level.

Only an identified sealed ball contact or dry spring/contact output may enter
the Lesson 037 bench circuit. Mercury, cracked glass, an unknown capsule, a
bare piezo connected without a reviewed protection network, and an output that
cannot be proven to stay within the Mega input rails remain quarantined. A
microphone board enters Lesson 038 only after its analog envelope output and,
when used, distinct threshold output are traced and qualified. “Big” and
“small” remain inventory labels, not interchangeable revisions.

The
[sensor evidence report](../research/LESSONS_037_039_SENSOR_EVIDENCE_REPORT.md)
records the official-source research, including the `HDX HDX`/Light Country
source contradiction and the unresolved kit sound-board circuitry. It
also selects exact documented reference fixtures so canonical work no longer
depends on misidentifying an unknown kit specimen.

The canonical contact reference is C&K/Littelfuse `RB-220-07A R`. Use an
external 10 kΩ pull-up from Mega 5 V to `TP-C` and the switch from `TP-C` to
ground; the nominal closed current is 0.5 mA, between the manufacturer's
10 µA minimum rated-load point and 1 mA maximum. Configure `DigitalInput` with
`Pull::None` and `activeLevel = Level::Low`. The terminals are electrically
nonpolar, but installed orientation and closure still require mapping. E1
measures the actual rail/resistor and proves worst-case current at or below
1 mA. Lesson 037 uses one;
Lesson 039's four-lane fixture uses four identical parts.

The canonical acoustic reference is SparkFun `SEN-12642`, visibly identified
by silkscreen `V10` (hardware V1.0) and matched to hardware commit
`a456d9e5e916aa3cfe656a48f5670cb73323f696`. Power it from the common 5 V
domain, route `ENVELOPE` to the assigned analog input and active-high `GATE`
to the optional threshold input, and leave `AUDIO` disconnected. This maps
directly to `AcousticEnvelope` without changing its API. Configure no input
pull-up for the driven push-pull `GATE`, sample `ENVELOPE` against AVcc, and
measure both output ranges at E1.
The module and Mega must share the same powered/unpowered state; never leave a
module output driven while the Mega is unpowered.

These are documented design references, not physical-verification claims.
Incoming revision inspection, unpowered conformance, powered characterization,
and E1 bench acceptance remain open. A kit contact or sound board is a
substitution and must satisfy its preserved specimen requirements before use.
The references are external, non-Elegoo fixtures; they do not silently
satisfy coverage of any authorized Elegoo kit family.

Use these terms throughout:

```text
raw level -> qualified contact -> attack event -> acoustic event window
          -> quantized hit -> recorded pattern -> playback frame
```

Authorized-family labels map only to planned roles:

| Listing label | Permitted planned role | Admission condition |
|---|---|---|
| Tilt-Switch, Tap Module, Shock | copied contact sample | Exact specimen proves a safe dry-contact or bounded conditioned output |
| Big Sound, Small Sound, sound module | copied ADC envelope plus optional threshold | Exact AO/DO roles, amplifier/bias, rails, polarity, and pull-up are proven |
| Passive buzzer | pitched playback through existing `PiezoSounder` | Exact passive part, D6 path, Timer2, current, and exposure are qualified |
| Active buzzer | none in 037--039 | Future semantic adapter; fixed internal tone cannot replay pitches |

A qualified contact is not a force, acceleration, or damage measurement. An
acoustic envelope is not sound pressure level, loudness, speech, or frequency
content. The project records relative intensity only. It performs no speech
recognition, surveillance, medical inference, impact grading, or safety-alarm
function.

## 037 — Contact dynamics

Files: `src/contact_dynamics.h` and `src/contact_dynamics.cpp`.
Energy class: E0. This is a pure behavior over copied digital observations;
the lesson adapter owns the `DigitalInput`. An eventual qualified passive,
current-limited contact fixture is E1.

### Public contract

```cpp
enum struct ContactQuality : uint8_t
{
    Unqualified,
    Valid,
    StuckActive,
    SourceFault,
    TimingFault
};

enum struct ContactDisposition : uint8_t
{
    None,
    Accepted,
    SuppressedDuringRefractory
};

struct ContactDynamicsConfig
{
    ContactDynamicsConfig (Level    activeLevel,
                           Duration qualify,
                           Duration release,
                           Duration refractory,
                           Duration stuckActive) noexcept;

    Level    activeLevel;
    Duration qualify;
    Duration release;
    Duration refractory;
    Duration stuckActive;
};

struct ContactObservation
{
    TimePoint          observedAt;
    Level              rawLevel;
    bool               rawActive;
    bool               qualifiedActive;
    bool               attackEvent;
    bool               releaseEvent;
    Duration           qualifiedPulseWidth;
    Duration           refractoryRemaining;
    uint32_t           acceptedCount;
    uint32_t           suppressedCount;
    ContactDisposition disposition;
    ContactQuality     quality;
    Status             status;
};

struct ContactSample
{
    TimePoint observedAt;
    Level     rawLevel;
    Status    status;
};

struct ContactDynamics
{
    explicit ContactDynamics (const ContactDynamicsConfig& config) noexcept;

    ContactDynamics            (const ContactDynamics&) = delete;
    ContactDynamics& operator= (const ContactDynamics&) = delete;
    ContactDynamics            (ContactDynamics&&)      = delete;
    ContactDynamics& operator= (ContactDynamics&&)      = delete;

    Status             initialize () noexcept;
    void               reset      () noexcept;
    Status             update     (const ContactSample& sample) noexcept;
    bool               initialized() const noexcept;
    ContactObservation snapshot   () const noexcept;
};
```

`ContactDynamics` is a non-owning pure policy. It owns only copied state,
claims no resource, performs no hardware I/O, has no `shutdown()`, and has a
trivial destructor. `reset()` is policy-state reset, not electrical cleanup;
the sketch must shut down its endpoint owners separately.

Construction is inert. Configuration requires nonzero `qualify`, `release`,
and `refractory`, all durations below unsigned half-range, plus nonzero
`stuckActive` below unsigned half-range. No relationship with the independent
release interval is imposed.
`initialize()` validates pure timing and enum policy and establishes no event.
Repeated initialization is inert. `reset()` returns to the just-initialized
state and clears time history and counters. The adapter validates capability,
initializes one `DigitalInput`, and passes a complete `ContactSample`.
The existing endpoint performs one acquisition sample during `initialize()`;
the adapter deliberately ignores that sample and does not submit it to the
pure behavior. Each loop then performs exactly one endpoint update before
copying its level. A non-Ok sample status propagates without manufacturing an
event and reports `SourceFault`.

`ContactSample::rawLevel` must be exactly `Level::Low` or `Level::High`.
Validation order is time first, then raw-level enum, then source status, then
raw candidate processing. An invalid raw level therefore wins over a
simultaneous non-Ok source status, latches `SourceFault` with
`InvalidArgument`, clears one-update events, preserves counts and last
qualified state, and requires `reset()` before qualification resumes.

In E0 tests, a non-Ok `ContactSample::status` is injected evidence at the pure
policy seam. The current `DigitalInput::update()` returns `void`, so a sketch
using only that endpoint cannot infer a runtime read failure and must not claim
one. An electrically specific future adapter must either use a qualified
status-producing input owner or narrow runtime-failure claims to the evidence
its exact specimen and endpoint can actually expose.

An attack candidate must remain continuously active for `qualify`. Its event
timestamp is the update that completes qualification. A qualified release
must remain continuously inactive for `release`; its pulse width is measured
from accepted attack to qualified release. Pulse width remains zero until a
release is accepted. Raw transitions remain visible even while qualification
or refractory policy rejects them.

Refractory time begins at accepted attack. Another completed attack before
the exact refractory boundary increments the saturating suppressed count and
reports `SuppressedDuringRefractory`; it does not increment accepted count.
An attack completing exactly at the boundary is accepted. A held contact does
not manufacture repeated attacks. Accepted and suppressed counts saturate at
`UINT32_MAX`.

The stuck clock begins at accepted attack, not at the first noisy raw edge.
Continuous qualified activity reaching `stuckActive` reports `StuckActive`
without creating another event. A later qualified release restores `Valid`;
the saturating counts and external trace retain that history, but the
snapshot has no separate latched-stuck flag. Unqualified means no
successful sample yet. An apparent time jump at or beyond unsigned half-range,
or a changed frame at the same time, reports `TimingFault` and
`InvalidArgument` without partial state mutation. An identical same-time
sample is idempotent. Natural `uint32_t` rollover remains valid.

Contact transition precedence is: uninitialized/invalid configuration;
timestamp mismatch or invalid time; invalid raw-level enum; non-Ok source
sample; raw candidate update; qualified release; stuck-active boundary;
refractory suppression; accepted attack. A non-Ok sample publishes
`SourceFault`, preserves counts and last qualified state, clears one-update
events, and requires `reset()` before qualification resumes. Collision
fixtures exercise every adjacent pair.

`attackEvent`, `releaseEvent`, and `disposition` describe exactly the current
update and clear on the next later update. Every field has a canonical value:
before qualification, pulse width and refractory remaining are zero; outside
refractory, remaining is zero.

### Deterministic fixture and failure matrix

Host fixtures cover:

- active-low and active-high configurations, every supported pull, and all
  starting raw levels;
- qualification and release one tick before, exactly at, and one tick after
  each boundary;
- single tap, double tap, long vibration train, shock-like short pulse,
  orientation hold, chatter, bounce, and a pulse too short to qualify;
- refractory completion before, at, and after its boundary, including a held
  contact and repeated suppressed attacks;
- pulse-width evidence, stuck-active entry and recovery, and both saturating
  counters;
- repeated time, changed same-time evidence, natural rollover, exact
  half-range, and backward apparent time;
- invalid raw-level enum alone and colliding with invalid time/source status,
  including latched fault and reset recovery;
- invalid enum values, zero/overflowing durations, unsupported, duplicate, and
  busy pins before hardware access;
- adapter endpoint initialization failure, repeated initialization/shutdown,
  destruction while active, claim reuse, and input mode after shutdown;
- one input sample per update and byte-identical snapshots/operation traces
  for identical timestamped raw-level fixtures; and
- non-copyable and non-movable traits.

No host trace labels a waveform as a particular retail module. Fixtures are
named for waveform shape (`shortPulse`, `chatterTrain`, `heldContact`) rather
than guessed mechanics.

### Circuit, example, and staged experiment

Provisional reference resources:

| Resource | Role |
|---|---|
| D22 `TP-C` | qualified dry-contact input, pull-up and active-low |
| D30 | raw-level LED through 1 kΩ |
| D31 | accepted-event LED through 1 kΩ |
| D32 | ready/fault LED through 1 kΩ |

No interrupt, PWM, bus, or timer is claimed. Direct indicators are bounded to
5 mA each. The contact input and interpreted LEDs are separate evidence.

`examples/Lesson037ContactDynamics/Lesson037ContactDynamics.ino` reads:

```text
setup: acquire contact -> acquire indicators -> show ready
loop:  observe contact -> qualify dynamics -> present evidence
```

The high-level helpers are `observeContact()`, `qualifyDynamics()`, and
`presentContactEvidence()`. Staged experiments are:

1. with USB removed, identify the exact specimen and prove continuity/open
   behavior without the Mega;
2. predict and observe idle and active voltage at `TP-C`;
3. compare the raw LED with accepted-event LED during a deliberate slow
   contact and a short pulse;
4. produce two bounded hand taps to observe refractory suppression;
5. hold the safe passive specimen to observe stuck-active presentation; and
6. call shutdown, then separately measure input high impedance and all
   semantic LEDs inactive.

Serial may print timestamps and counters but is not acceptance evidence.

## 038 — Acoustic envelope

Files: `src/acoustic_envelope.h` and `src/acoustic_envelope.cpp`.
Energy class: E0. This is a pure behavior over copied ADC and optional
threshold observations; the lesson adapter owns the endpoints. An eventual E1
fixture is allowed only after one exact amplified microphone module is
electrically qualified.

### Public contract

```cpp
enum struct AcousticPhase : uint8_t
{
    Calibrating,
    Quiet,
    EventOpen,
    Refractory,
    Fault
};

enum struct AcousticQuality : uint8_t
{
    Unqualified,
    ValidQuiet,
    ValidEvent,
    ClippedLow,
    ClippedHigh,
    ThresholdDisagreement,
    SourceFault,
    TimingFault
};

struct AcousticEnvelopeConfig
{
    AcousticEnvelopeConfig (bool     hasThreshold,
                            Level    thresholdActiveLevel,
                            uint16_t railMargin,
                            uint16_t attackAboveBaseline,
                            uint16_t releaseAboveBaseline,
                            uint8_t  baselineShift,
                            Duration calibration,
                            Duration eventWindow,
                            Duration quietToClose,
                            Duration refractory) noexcept;

    bool     hasThreshold;
    Level    thresholdActiveLevel;
    uint16_t railMargin;
    uint16_t attackAboveBaseline;
    uint16_t releaseAboveBaseline;
    uint8_t  baselineShift;
    Duration calibration;
    Duration eventWindow;
    Duration quietToClose;
    Duration refractory;
};

struct AcousticObservation
{
    TimePoint       observedAt;
    uint16_t        raw;
    uint16_t        baseline;
    uint16_t        amplitude;
    uint16_t        peakAmplitude;
    uint16_t        relativeIntensity;
    bool            rawThresholdActive;
    bool            eventStarted;
    bool            eventCompleted;
    TimePoint       eventStartedAt;
    Duration        eventDuration;
    AcousticPhase   phase;
    AcousticQuality quality;
    Status          status;
};

struct AcousticSample
{
    TimePoint observedAt;
    uint16_t  raw;
    bool      hasThreshold;
    Level     thresholdLevel;
    Status    analogStatus;
    Status    thresholdStatus;
};

struct AcousticEnvelope
{
    explicit AcousticEnvelope (
        const AcousticEnvelopeConfig& config) noexcept;

    AcousticEnvelope            (const AcousticEnvelope&) = delete;
    AcousticEnvelope& operator= (const AcousticEnvelope&) = delete;
    AcousticEnvelope            (AcousticEnvelope&&)      = delete;
    AcousticEnvelope& operator= (AcousticEnvelope&&)      = delete;

    Status              initialize () noexcept;
    void                reset      () noexcept;
    Status              update     (const AcousticSample& sample) noexcept;
    bool                initialized() const noexcept;
    AcousticObservation snapshot   () const noexcept;
};
```

`hasThreshold=false` makes `thresholdActiveLevel` canonical as `Level::Low`.
Configuration is valid exactly when:

- `railMargin` is `1..511`;
- `releaseAboveBaseline < attackAboveBaseline`;
- `attackAboveBaseline` is nonzero and less than
  `1023 - 2 * railMargin`;
- `baselineShift` is `1..8`;
- `calibration`, `eventWindow`, `quietToClose`, and `refractory` are nonzero
  and strictly below unsigned half-range;
- `quietToClose <= eventWindow`; and
- `thresholdActiveLevel` is a defined `Level`, with `Level::Low` required when
  `hasThreshold=false`.

Every other configuration returns `InvalidArgument` without state mutation.
Every supplied `raw` must be `0..1023`; an out-of-range sample is
`InvalidArgument` before interpretation.

The adapter
separately validates pins before claims, acquires
analog then optional threshold transactionally, samples analog then threshold
exactly once per loop, and releases threshold then analog. A non-Ok analog
status faults without interpreting `raw`; a configured threshold requires a
healthy threshold status. No ADC-reference change, interrupt, sampling timer,
FFT, heap, or hidden clock is introduced.
The existing `AnalogInput` and optional `DigitalInput` each take one
acquisition sample during their own `initialize()`; these samples are ignored
and never submitted to `AcousticEnvelope`. Only the one copied sample from
each loop update enters the pure behavior.

When `sample.hasThreshold=true`, `thresholdLevel` must be exactly
`Level::Low` or `Level::High`. Runtime validation order is time first, then
threshold-level enum, then analog/threshold status, then raw-range and signal
processing. An invalid threshold level therefore wins over simultaneous
source-status or raw faults, latches `Fault + SourceFault +
InvalidArgument`, preserves the previously published raw-threshold and
baseline evidence, performs no calibration/event transition, clears
event/intensity fields to canonical zero, and requires `reset()` before
sampling resumes.

The first healthy non-clipped sample sets `baseline=raw`, starts calibration,
and has zero amplitude. A rail sample before that point reports clipping and
does not start calibration. Any clipped sample during calibration enters
`Fault`; reset and a new healthy first sample are required. Calibration lasts
through the first sample at or beyond `calibration`.
Baseline is an integer exponential estimate:

```text
baseline += (raw - baseline) / 2^baselineShift
```

using widened signed arithmetic and truncation toward zero.
`baselineShift` is `1..8`. Baseline adapts only during calibration and quiet;
it freezes while an event or refractory interval is active.
`amplitude = abs(raw - baseline)`. Attack requires
`amplitude >= attackAboveBaseline`; release requires
`amplitude <= releaseAboveBaseline`, where release is strictly below attack.

An event window opens at attack, retains its peak, and closes at the earlier
of continuous quiet for `quietToClose` or exact `eventWindow` expiry.
Define `headroom = max(baseline - railMargin,
(1023 - railMargin) - baseline)`. Runtime state must have
`headroom > attackAboveBaseline`. The exact completed-event mapping is
`1000 * clamp(peakAmplitude - attackAboveBaseline, 0,
headroom - attackAboveBaseline) / (headroom - attackAboveBaseline)`, using
widened unsigned arithmetic and integer truncation. It is a unitless
within-configuration value, not comparable SPL.
If baseline placement leaves insufficient headroom, the update enters `Fault`
with `InvalidArgument`, publishes canonical zero event/intensity fields, and
requires reset/recalibration.
Refractory begins on completion and prevents a new event until its exact
boundary.

`raw <= railMargin` is `ClippedLow`; `raw >= 1023 - railMargin` is
`ClippedHigh`. Clipping cannot open or extend an event. After calibration,
threshold disagreement means
`rawThresholdActive != (amplitude >= attackAboveBaseline)`. Continuous
disagreement for `quietToClose` reports `ThresholdDisagreement`; agreement
resets its candidate. Clipping and source failure take precedence and neither
starts nor extends disagreement. A single mismatch remains visible but is not
a fault. The analog envelope remains the authority for event
timing and intensity. A threshold-only specimen is not supported by this
component.

Backward/half-range time and changed same-time input follow the Lesson 037
time rule and enter `Fault`. Recovery from clipping, disagreement, source
failure, or timing fault requires `reset()` and a new calibration interval;
the behavior does not silently recalibrate. Event flags are one-update
snapshots. Before a
completed event, intensity, event start, and event duration use zero canonical
values.

Acoustic transition precedence is: uninitialized/invalid configuration;
timestamp mismatch or invalid time; invalid runtime threshold-level enum;
analog source failure; configured threshold source failure; raw-range error;
clipping; calibration; sustained threshold disagreement; event close; event
open; baseline update. A winning fault makes
no lower transition. `TimingFault` preserves the last completed event evidence;
clipping, source failure, threshold disagreement, and invalid runtime
headroom clear event/intensity fields to their canonical zero values.

The only legal nested snapshot combinations are:

| Phase | Quality | Status and event meaning |
|---|---|---|
| `Calibrating` | `Unqualified` | Ok; no event fields are valid |
| `Quiet` | `ValidQuiet` | Ok; no event fields are valid |
| `EventOpen` | `ValidEvent` | Ok; `eventCompleted=false`, live start/peak only |
| `Refractory` | `ValidEvent` | Ok only on the completion update; `eventCompleted=true` |
| `Refractory` | `ValidQuiet` | Ok after the completion snapshot has cleared |
| `Fault` | `ClippedLow` or `ClippedHigh` | `InvalidArgument`; event/intensity fields canonical zero |
| `Fault` | `ThresholdDisagreement` | `HardwareFailure`; event/intensity fields canonical zero |
| `Fault` | `SourceFault` | propagated non-Ok endpoint status or `InvalidArgument` for an invalid runtime level; event/intensity fields canonical zero |
| `Fault` | `TimingFault` | `InvalidArgument`; last completed evidence retained |
| `Fault` | `Unqualified` | `InvalidArgument` for an out-of-range raw ADC sample or insufficient runtime headroom |

No other phase/quality/status tuple is valid. Sequencer association eligibility
is exactly `Refractory + ValidEvent + Ok + eventCompleted=true`, with nonzero
event duration and the published start/end interval. It is eligible for that
one update only.

Invalid configuration fails `initialize()` with `InvalidArgument` and leaves
the policy uninitialized; it does not publish or mutate a `Fault` snapshot.

### Deterministic fixture and failure matrix

Host fixtures cover:

- constant midrail, asymmetric quiet noise, slow baseline drift, exact integer
  baseline steps, and calibration boundary samples;
- attack/release thresholds ±1/exact, early quiet close, maximum-window close,
  exact refractory expiry, overlapping impulses, and uneven update intervals;
- positive and negative excursions with equal amplitude, peak retention, and
  intensity mapping at zero, interior, and clamp boundaries;
- rail margin ±1/exact, startup at each rail, injected source-unavailable,
  rail/stuck-style supplied traces without claiming electrical diagnosis, and
  explicit fault recovery;
- invalid runtime threshold-level enum alone and colliding with invalid time,
  source status, and out-of-range raw input, proving precedence, preserved
  raw-threshold/baseline evidence, no partial transition, and reset recovery;
- optional threshold absent, agreement, one-sample mismatch, sustained
  disagreement, reversed active level, and comparator chatter;
- repeated timestamps, same-time mismatch, rollover, half-range, and backward
  apparent time;
- invalid thresholds, margins, shifts, duration relationships, duplicate or
  unsupported pins, busy pins, and every partial-acquisition rollback;
- idempotent lifecycle, destruction, claim reuse, exact analog-then-digital
  sample order, and non-copyable/non-movable traits; and
- byte-identical observations and operation traces on repeated seed-free
  sample/time fixtures.

### Circuit, example, and staged experiment

Provisional reference resources:

| Resource | Role |
|---|---|
| A0/D54 `TP-E` | qualified analog envelope output |
| D22 `TP-T` | optional qualified comparator output |
| D30 | one-update `eventCompleted` LED through 1 kΩ |
| D31 | copied active-high `GATE` LED through 1 kΩ |
| D32 | initialized-and-healthy LED through 1 kΩ |

`examples/Lesson038AcousticEnvelope/Lesson038AcousticEnvelope.ino` reads:

```text
setup: acquire evidence panel -> configure envelope -> start lesson
loop:  observe acoustic evidence -> decide envelope -> actuate evidence
```

Helpers are `acquireEvidencePanel()`, `configureEnvelope()`,
`startLesson()`, `observeAcousticEvidence()`, `decideEnvelope()`,
`actuateEvidence()`, and `stopSafely()`. `AcousticEnvelope` owns copied policy
state only. The sketch owns the three `MonoLed` endpoints; the optional
reference-fixture path performs the explicit A0 and D22 board reads and
submits their copied values and statuses to the policy. It does not imply that
the pure behavior owns those pins or can diagnose a failed board read.

D30 is true only for the update that completes a bounded event. D31 mirrors
the copied raw `GATE` level and can therefore precede, outlast, or disagree
with the qualified analog event. D32 is lit after the evidence panel and
policy initialize and remains lit only while the policy is not in `Fault`;
it is ready/healthy evidence, not a code for the specific fault source.
Initialization acquires D30, D31, then D32 and configures the pure policy.
Any failed acquisition rolls back already acquired LEDs. `stopSafely()` turns
the lesson inert in reverse presentation order by shutting down D32, D31, and
D30, then resetting the pure policy. The resulting high-impedance LED pins
are the shutdown evidence; policy reset is not electrical cleanup.

The checked-in replay advances its copied evidence timestamps by 20 ms even
though presentation is intentionally paced at 250 ms of wall time. This makes
the lesson deterministic and visible but does not model real-time capture,
bandwidth, or every transient between polls. In reference-fixture mode the
timestamp and sampling cadence come from the running sketch, so a pulse
between samples can still be missed.

Staged experiments are:

1. inventory both PCB faces, pin labels, supply, output topology, microphone
   bias/amplifier, comparator rail, and potentiometer direction before power;
2. place probes with USB removed, then predict and observe quiet `TP-E` and
   `TP-T` values without touching the powered breadboard;
3. observe baseline settling with no deliberate sound;
4. make one gentle nearby hand tap and compare the raw `GATE` LED, the
   one-update completed-event LED, and completed relative intensity;
5. adjust only an identified comparator control with USB removed between
   trials, then compare threshold agreement; and
6. prove clipping/fault presentation with host fixtures, not a live short,
   then measure shutdown high impedance and all indicators inactive.

Stop for an out-of-rail output, unexpected reset, rail instability, heat,
odor, excessive current, unidentified part, or schematic/specimen
disagreement. The lesson never asks for a loud or startling stimulus.

This polled demonstration records no waveform. An always-visible observation
LED shows when envelope sampling is active. The documented update interval is
part of acceptance; a pulse wholly between updates may be missed. No bandwidth,
frequency-response, or complete-capture claim follows from this behavior.

## 039 — Project: percussion sequencer

Files: `src/percussion_sequencer.h` and `src/percussion_sequencer.cpp`.
Energy class: E0 for the project engine and four-lane composition fixture. One
qualified passive contact is sufficient for the initial E1 acceptance route.
A four-surface E1 build is permitted only after four exact contacts or
documented safe substitutions are separately inventoried and qualified,
alongside one qualified microphone module, resistor-limited LEDs, display, and
an identified passive piezo.

The project engine is hardware-neutral. It owns no pins, timer, endpoint,
clock, callback, or storage. The Mega example is the adapter that observes
four `ContactDynamics` components, one `AcousticEnvelope`, and a tempo
`AnalogInput`; decides through the engine; and actuates existing LEDs,
`SevenSegmentDisplay`, and `PiezoSounder`.

### Public contract

```cpp
enum struct PercussionMode : uint8_t
{
    Recording,
    Playing,
    Full,
    Fault
};

enum struct PercussionFaultSource : uint8_t
{
    None, Surface0, Surface1, Surface2, Surface3,
    Acoustic, Timing, Tempo, Input
};

enum struct PercussionAssociation : uint8_t
{
    None,
    AcousticCompletion,
    AssociationTimeout
};

struct PercussionHit
{
    uint8_t               surface;
    uint8_t               step;
    uint16_t              intensity;
    uint32_t              ordinal;
    PercussionAssociation association;
};

struct PercussionSequencerConfig
{
    PercussionSequencerConfig (uint8_t  steps,
                               uint16_t minimumTempoBpm,
                               uint16_t maximumTempoBpm,
                               Duration simultaneousWindow,
                               Duration acousticAssociationTimeout) noexcept;

    uint8_t  steps;
    uint16_t minimumTempoBpm;
    uint16_t maximumTempoBpm;
    Duration simultaneousWindow;
    Duration acousticAssociationTimeout;
};

struct PercussionAcousticCompletion
{
    PercussionAcousticCompletion () noexcept;

    bool      present;
    TimePoint eventStartedAt;
    Duration  eventDuration;
    uint16_t  intensity;
};

struct PercussionSequencerInput
{
    PercussionSequencerInput () noexcept;

    TimePoint                    observedAt;
    uint8_t                      attackMask;
    Status                       surfaceStatus[4];
    Status                       acousticStatus;
    PercussionAcousticCompletion acousticCompletion;
    uint16_t                     tempoPosition;
    bool                         playEvent;
    bool                         clearEvent;
};

struct PercussionFrame
{
    uint8_t  step;
    uint8_t  surfaceMask;
    uint16_t intensity[4];
    uint16_t frequencyHz;
    Duration toneDuration;
    bool     heartbeat;
};

struct PercussionSequencerSnapshot
{
    PercussionMode        mode;
    uint16_t              tempoBpm;
    uint8_t               currentStep;
    uint8_t               hitCount;
    uint32_t              nextOrdinal;
    bool                  hitAccepted;
    bool                  hitSuppressed;
    bool                  patternFull;
    bool                  frameValid;
    PercussionFaultSource faultSource;
    PercussionAssociation lastAssociation;
    PercussionHit         lastHit;
    PercussionFrame       frame;
    Status                status;
};

struct PercussionSequencer
{
    static constexpr uint8_t maximumHits = 32;
    static constexpr uint8_t maximumSteps = 16;

    explicit PercussionSequencer (
        const PercussionSequencerConfig& config) noexcept;

    PercussionSequencer            (const PercussionSequencer&) = delete;
    PercussionSequencer& operator= (const PercussionSequencer&) = delete;
    PercussionSequencer            (PercussionSequencer&&)      = delete;
    PercussionSequencer& operator= (PercussionSequencer&&)      = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (const PercussionSequencerInput& input) noexcept;
    void   clear      () noexcept;

    bool                        initialized () const noexcept;
    PercussionSequencerSnapshot snapshot    () const noexcept;
    Result<PercussionHit>       hit         (uint8_t index) const noexcept;
};
```

These are project-owned narrow DTOs. The Lesson 039 adapter translates contact
and acoustic policies into `attackMask`, four source statuses, one acoustic
status, and an optional canonical completion. The engine neither accepts nor
reconstructs nested `ContactObservation`/`AcousticObservation` tuples and has
no `inputValid` escape hatch. `PercussionFaultSource` attributes the winning
status without importing source-specific quality enums.

When `acousticCompletion.present=false`, its start, duration, and intensity
are canonical zero. When present, duration is nonzero, its interval is below
unsigned half-range, and it cannot begin in the apparent future. Invalid
completion fields fault atomically as
`Acoustic`; no pending group or pattern state changes.

`steps` is `4..16`; tempo is bounded within `30..240 BPM`;
the configured range is ordered; simultaneous window is nonzero and below
half-range. `acousticAssociationTimeout` is strictly below half-range and
longer than the simultaneous window. The adapter separately ensures it can
cover the selected acoustic event policy. `tempoPosition` is `0..1000` and maps linearly to the configured
tempo using widened integer arithmetic. The step period is
`60000 / tempoBpm / 4` milliseconds: every stored step is a sixteenth-note
grid position. Remainder is deliberately truncated and documented.

Recording begins at the first accepted contact attack. The tempo mapped on
that update freezes the recording grid until clear; later potentiometer
changes affect playback only. Attack time is
quantized to the nearest step relative to that epoch; exact half-step ties
round forward. Up to four attacks enter one bounded pending simultaneous
group. It closes on the first later update at or beyond
`firstAttack + simultaneousWindow`; an attack exactly at that boundary belongs
to the group. Each surface can contribute once, so no fifth slot exists.

Exactly one four-lane group may be pending. After closure it remains pending
until the first completed acoustic
window whose inclusive
`[eventStartedAt, eventStartedAt + eventDuration]` contains the first attack.
That completion intensity applies to every hit in the group. At
`firstAttack + acousticAssociationTimeout`, timeout wins over a newly supplied
window and finalizes the group with intensity zero. A completed acoustic
snapshot is eligible only on its one update; each completion can satisfy at
most the oldest pending group. Thus later acoustic completion can qualify
earlier contact without rewriting an already published hit. Each stored hit
retains `AcousticCompletion` or `AssociationTimeout` provenance; zero
intensity alone never implies which path finalized it.
New attacks while that closed group awaits association set
`hitSuppressed=true` and are not queued. Pending timers use unsigned elapsed
subtraction, never ordering of wrapped endpoint timestamps.

The adapter alone decides when a contact policy yields an attack bit and when
an acoustic policy yields a completion DTO. The engine validates only the
four `surfaceStatus` values, `acousticStatus`, mask bounds, and canonical
completion fields. Undefined status codes are `InternalInvariant`; the first
non-Ok surface in index order wins before acoustic status.

Surface order `0..3` is semantic and independent of pins. Attacks inside
`simultaneousWindow` quantize against one shared timestamp and are stored in
ascending surface order, regardless of input array evaluation order. A second
attack from the same surface at the same quantized step is suppressed.
Different surfaces may share a step. Hits are stored by step, then surface,
then bounded ordinal. The one pending group does not reserve finalized
capacity. Group admission is atomic: after duplicate removal, every accepted
surface must fit. If the whole group does not fit, none of it is stored,
`hitSuppressed=true`, and all existing hits/ordinals/provenance remain
unchanged. A group that exactly fills capacity commits in surface order and
enters `Full`.

`playEvent` toggles Recording/Playing. Starting playback with no hits is an
idempotent no-op. `clearEvent` dominates play and all attacks at the same
timestamp; it clears hits, epoch, frame, and ordinals and returns to Recording.
Playing ignores contact attacks but retains invalid-input fault precedence.
Tempo is sampled continuously; a tempo change takes effect only at the next
step boundary, preserving the current step deadline.

`Full` means finalized capacity is exhausted while stopped. Play from `Full`
enters `Playing`; stopping returns to `Full`, not Recording. Further attacks
remain observable as full/suppressed evidence and never replace a hit. Clear
from any non-fault mode returns to empty Recording. A fault silences frame and
tone intents immediately, retains finalized hits, and accepts no control until
shutdown/reinitialize; reinitialize returns to `Full` when 32 hits remain and
otherwise Recording.
Starting playback creates a new playback epoch at that update and publishes
step zero immediately. Stopping discards that epoch. Each later start begins
again at step zero; the recording epoch is only for hit quantization.

Playback uses supplied time and may cross multiple elapsed step boundaries in
one update. It skips directly to the frame current at `observedAt` and never
emits retroactive light or tone cues for missed deadlines. Host tests obtain
every frame by calling at computed boundaries; sparse-update tests prove
skipped cues remain absent. Pattern position is calculated from epoch and step
duration rather than update count, so uneven calls cannot drift. Natural
rollover is valid; same-time identical input is idempotent; changed same-time
input, half-range, or backward apparent time faults without partial mutation.

Each frame ORs the four surface bits for its step, retains one intensity per
surface, chooses the highest-intensity hit for the tone, and breaks equal
intensity by lowest surface. Fixed surface tones are 262, 330, 392, and
523 Hz. Tone duration is the smaller of 60 ms and half the step duration.
A frame with no hit has frequency and duration zero. Heartbeat toggles once per
beat and remains independent of whether the pattern is silent.

Update precedence is: uninitialized/configuration fault; observed-time
validation; attack-mask/status/completion DTO validation; invalid tempo;
`clearEvent`; pending-group timeout; eligible acoustic completion; play
toggle; contact attacks (including an attack exactly at the simultaneous
boundary); pending-group closure; atomic capacity/full transition; tempo
application; playback boundary. A higher result prevents partial
mutation by lower rules. Clear therefore dominates valid controls, but never
hides invalid evidence.

Same-time identity is semantic and fieldwise across the project DTO:
`observedAt`, mask, four statuses, acoustic status, canonical completion
fields, tempo, play, and clear. It never compares padding or source-policy
objects. An identical semantic input is idempotent; any changed semantic field
is a timing fault.

Shutdown clears the pending group, timing epochs, transient association and
frame/output intent, sets `NotInitialized`, and retains finalized indexed hits,
hit count, next ordinal, and `lastHit`. Reinitialize retains that pattern,
returns to `Full` at capacity or `Recording` otherwise, and starts fresh time
epochs. Only `clear()` erases retained hits. `hit(index)` returns
`Result<PercussionHit>`: each in-range value includes association provenance;
an out-of-range index returns `InvalidArgument` plus the canonical zero/None
hit. No EEPROM, RTC, removable media, randomness, or dynamic allocation is
introduced.

`lastHit` changes only when an entire group finalizes successfully and is the
last surface-ordered hit admitted from that group. `lastAssociation` reports
that group provenance for the finalization update and otherwise returns to
`None`; indexed hits retain their own provenance permanently. Healthy
snapshots use `faultSource=None`, and absent frame/completion values use their
documented canonical zeros.

Measured layouts for the final declaration are:

| Type | x86-64 host | ATmega2560 AVR |
|---|---:|---:|
| `PercussionSequencerConfig` | 16 B | 13 B |
| `PercussionAcousticCompletion` | 16 B | 11 B |
| `PercussionSequencerInput` | 32 B | 25 B |
| `PercussionHit` | 12 B | 9 B |
| `PercussionFrame` | 20 B | 17 B |
| `PercussionSequencerSnapshot` | 56 B | 42 B |
| `PercussionSequencer` | 400 B | 371 B |

These measurements use the repository host compiler and Arduino AVR GCC
7.3.0-atmel3.6.1. They are layout evidence, not the still-open linked-example
flash/static-RAM baseline or full-circuit SRAM proof.

### Deterministic project matrix

Core and adapter fixtures cover:

- each surface alone, every surface pair and four-way simultaneous attacks,
  input-order permutations, same-surface duplicates, and exact simultaneous
  window edges;
- pending-group closure, completed acoustic windows before/at/after timeout,
  inclusive overlap edges, attacks suppressed behind the sole pending group,
  sequential windows, zero-intensity timeout, and clear/shutdown while pending;
- contact qualification/refractory evidence propagated from Lesson 037 and
  acoustic overlap/non-overlap from Lesson 038;
- quantization one tick before, exactly at, and after half-step; first and last
  steps; pattern wrap; and all supported tempo endpoints;
- capacity at 31, 32, and 33 hits including a group that partly fits,
  full-state stability, bounded ordinal,
  clear dominance, and clear/re-record;
- recording/play toggles, empty playback, attacks during playback, and
  start/stop at a step boundary;
- uneven updates with explicit skipped cues, repeated timestamps, natural rollover,
  half-range/backward time, and tempo changes immediately before/at/after a
  boundary;
- intensity zero, every clamp edge, highest-intensity selection, equal
  tie-break, silent frames, tone-duration clamp, and heartbeat cadence;
- invalid mask/status/completion DTO evidence, invalid tempo position,
  invalid configuration/enums, and shutdown/reinitialization from every mode;
- frame-for-frame adapter replay including LED, display, and piezo intents;
  D6 pin/Timer2 conflict before any cue plus initialization failure and reverse
  rollback in the example owner;
  and
- versioned fieldwise golden replay of every snapshot field, `lastHit`, and
  every indexed `hit(index)` value/provenance, without struct-memory or padding
  comparison.

### Narrative example and resource budget

`examples/Lesson039PercussionSequencer/Lesson039PercussionSequencer.ino` reads:

```text
setup: acquire surfaces -> acquire envelope and tempo -> acquire presentation
       -> initialize sequencer -> show ready
loop:  observe hits -> decide pattern -> present step -> sound bounded cue
```

Helpers are `observePercussionInputs()`, `decidePattern()`,
`presentPlaybackFrame()`, and `soundPlaybackFrame()`. They appear before
mechanism helpers. Code, HTML, PDF, and diagrams use the same surface numbers,
step vocabulary, and status meanings.

Provisional Mega budget:

| Resource | Role |
|---|---|
| D40–D43 `TP-C0..3` | four logical contact lanes; physical specimens separately gated |
| A8/D62 `TP-E`, D44 `TP-T` | microphone envelope and optional threshold |
| A9/D63 `TP-BPM` | 10 kΩ tempo potentiometer wiper |
| D45–D48 | accepted-surface LEDs, 1 kΩ each |
| D49/D50/D51 | 74HC595 data/clock/latch for one-digit step display |
| D52 | healthy ready/heartbeat LED, 1 kΩ |
| D53 | envelope-observation/capture LED, 1 kΩ |
| D6 and timer 2 | existing `PiezoSounder`, silent when inactive |
| D38/D39 | play and clear buttons |

There is no interrupt, bus, external supply, motor, relay, active buzzer, or
storage claim. The four lanes are injected logical inputs in the engine; this
does not claim that four physical contact specimens exist in the kit. Active
buzzer is excluded because a fixed internal oscillator cannot replay the four
project pitches. Known direct LED load is at most 30 mA. The one-digit display
retains Lesson 010's 8 mA per segment and 40 mA aggregate segment budget. The
combined provisional known indicator/segment load is therefore at most 70 mA,
excluding module current, 74HC595 quiescent/package current, pulls, passive
piezo current, and Mega baseline. Exact specimen qualification must add
per-pin, per-port, 74HC595 package, 5 V rail, and total current arithmetic
before power.

The four surface LEDs report accepted hits, not raw contact. The one-digit
display reports the active step. The heartbeat distinguishes a quiet pattern
from a stalled scheduler. The dedicated capture LED is on for every envelope
sampling interval, making microphone activity visible even though no waveform
is retained. `TP-C0..3`, `TP-E`, and `TP-T` preserve raw
electrical evidence. Resource acquisition is proved by the ready sequence;
safe state is proved separately by silent piezo, inactive LEDs/display, and
high-impedance inputs after shutdown.

The sketch is the explicit adapter owner; no hidden adapter class is planned.
It owns four `DigitalInput`s, the envelope `AnalogInput`, optional threshold
`DigitalInput`, tempo `AnalogInput`, two `Button`s, six `MonoLed`s,
`SevenSegmentDisplay`, and `PiezoSounder`. Setup initializes in that dependency
order and rolls back in strict reverse order. Shutdown stops/releases sounder,
display and LEDs, buttons, tempo, threshold/envelope, then contacts in
descending lane order. Behavior objects reset only after endpoint outputs are
inactive.

D52 is only ready/heartbeat evidence during healthy execution: ready before
playback and the bounded `frame.heartbeat` cadence while a frame is valid. It
must not be described as a generic runtime-fault indicator unless a separate,
bounded, documented diagnostic state machine assigns an unambiguous fault
pattern. Runtime source, timing, and input faults are instead proven by
deterministic host traces that retain `faultSource` provenance and show cue
intent clearing. Safe shutdown, including a fault-triggered stop, leaves D52,
the capture and surface LEDs, display, and piezo inactive; keeping D52 driven
after teardown would conflate resource survival with safe-state evidence.

Staged project experiments are:

1. replay checked-in contact and envelope fixtures with no hardware;
2. wire and prove one qualified passive surface and its raw/accepted evidence;
3. exercise all four lanes with injected traces; add physical surfaces one at
   a time only when each exact specimen or substitution has its own gate;
4. record four deliberate gentle taps with microphone intensity disabled,
   proving contact timing alone;
5. enable the qualified microphone and compare two gentle relative-intensity
   trials without making a calibrated or absolute claim;
6. set tempo at both bounded potentiometer positions and observe step display,
   heartbeat, LEDs, and piezo; and
7. clear, shutdown, reset, and remove USB, recording each distinct safe state.

## HTML/PDF split and visual policy

HTML is the normative searchable reference. For each lesson it contains the
public API, configuration constraints, state/quality tables, exact precedence,
resource and pin table, canonical example/download, deterministic fixture
links, status/support language, and correction history. It states that retail
labels do not establish a supported specimen and that intensity is relative.

PDFs are richer printable laboratory companions. They contain:

- a concise concept map and vocabulary chain;
- a separately identified, electrically authoritative formal schematic plus
  adjacent pin-by-pin wiring prose;
- pencil-drawn specimen orientation, staged build, timing/event-window,
  quantization/playback, and troubleshooting visuals;
- predict/observe/interpret worksheets for each dependent stage;
- raw-versus-qualified and analog-versus-threshold diagnosis;
- exercises that operate on supplied traces when hardware is absent; and
- a blank physical acceptance card with no prefilled observation or pass.

Every non-schematic PDF visual uses visibly pencil-drawn presentation and is
preceded by `% ADK visual: pencil`. Only a conventional component/net diagram
that is explicitly identified as the electrically authoritative formal
schematic may use `% ADK visual: schematic` and `circuitikz`. A waveform,
event window, state diagram, pin locator, breadboard view, chart, or playback
flow is not a schematic. Render review checks the appearance; filenames,
markers, grayscale, or rasterization alone do not prove compliance.

## Physical acceptance and stop conditions

All three bench cards remain open until a person records the exact Mega 2560,
specimen identity and PCB faces, primary sources, pinout, topology, pull-up
rail, supply/output ranges, measured 5 V reference, resistor values, module and
aggregate currents, tools, versions, date, predictions, observations,
interpretations, deviations, and reviewer.

The cards separately prove:

1. unpowered identity, continuity, polarity, and wiring against the formal
   schematic;
2. resource acquisition and complete rollback;
3. raw contact/envelope/threshold electrical evidence;
4. qualified events, relative intensity, quantized hits, and presentation;
5. reset, component shutdown, board reset, and USB power removal; and
6. passive piezo silence/high impedance as distinct safe-state evidence.

Place probes and change wiring only with USB removed. Live shorts and
out-of-range signals are simulated in host fixtures. Stop and remove USB for
heat, odor, startling output, unstable power, reset, out-of-rail voltage,
unexpected activity, current above any pin/port/package/rail/aggregate budget,
loose or cracked parts, mercury or unknown glass, unreviewed bare piezo,
unidentified microphone bias/amplifier, or disagreement between specimen,
source, schematic, and observed behavior.

Compilation, size evidence, host replay, generated PDFs, LEDs, or Serial output
cannot close a physical card.

## Delivery order and gates

Each boundary is independently buildable:

1. `contact: add qualified contact dynamics`
2. `lessons: teach contact timing evidence`
3. `audio: add acoustic envelope windows`
4. `lessons: teach relative acoustic evidence`
5. `project: add percussion sequencer`
6. `lessons: build percussion sequencer`
7. `docs: publish percussion laboratory`
8. `release: verify percussion laboratory`

Component commits include the public declaration, out-of-line implementation,
umbrella header, header-alone gate, exact lifecycle/rollback tests, deterministic
fixtures, and no lesson promotion. Lesson commits add canonical Mega examples,
measured size baselines, HTML, TeX/PDF, pencil assets, source downloads,
navigation, and open bench cards. The project core precedes its example and
publication integration.

Every code boundary runs formatting, focused host tests, strict headers,
ordinary/exception-enabled/sanitizer host gates where available, Mega compile,
size, and `git diff --check`. Lesson boundaries also run PDF build/check,
visual-classification and rendered-style review, site/link checks, and
canonical-sketch identity. The block closes only after full `make quality`,
all-example size checks, lessons/site checks, package and native consumers,
strict Arduino lint policy, clean-tree metadata reconciliation, independent
code/publication review, Pages deployment, and live newest-lesson verification.

## Explicit deferrals

- No guessed electrical compatibility among Tilt-Switch, Tap, Shock, Big
  Sound, Small Sound, knock, vibration, or microphone products.
- No mercury switch, loose impact projectile, hard strike, drop test, loud
  stimulus, calibrated force, acceleration, SPL, frequency, or damage claim.
- No speech recognition, recording, surveillance, medical, security, or
  safety-alarm interpretation.
- No threshold-only sound adapter, interrupt sampling, FFT, audio waveform
  storage, automatic gain control, or hidden ADC-reference change.
- No active-buzzer support claim; the project uses the existing passive
  `PiezoSounder` only after exact-part qualification.
- No EEPROM, RTC, removable storage, external load, motor, relay, radio,
  launcher, ignition, or pyrotechnic behavior.
- No hardware-verification claim without completed Mega 2560 bench cards.
