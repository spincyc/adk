# Lessons 046--048 kinetic-light-sculpture plan

Status: implementation-depth E0 plan, 2026-07-28.

This block front-loads an interactive kinetic result without weakening the
inventory, motion, inductive-load, or physical-evidence gates. The authorized
implementation is E0 first: copied tactile and directional evidence, pure
bounded sequencing, transactional project composition, and project-local
semantic light intent. It does not authorize a powered input adapter, a GPIO
read, a coil driver, a motor supply, moving hardware, wiring table, formal
schematic, or physical acceptance claim.

The likely later actuator is a 28BYJ-48 stepper plus ULN2003 carrier. A retail
kit label is not its electrical identity. E2 work requires the exact motor
voltage and winding, driver identity and topology, ULN2003 clamp connection,
separate current-limited supply, measured coil current, thermal and sequence
limits, mechanical restraint, independent power removal, and a reviewed
acceptance card. No E0 object writes a pin or claims a resource.

The likely later touch source is the listing-authorized Metal Touch family.
Its name does not establish capacitive sensing, active polarity, supply,
output level, debounce needs, or electrical safety. Lesson 046 therefore
publishes no generic capacitive sensor and no heartbeat, pulse, gesture,
proximity, or physiological interpretation. E1 adapters may be added only
after exact specimens and primary-source electrical records qualify them.

The evidence levels are:

| Level | Authorized work |
|---|---|
| E0 | Host replay and pure policy; compile-only Mega sketches using copied fixtures and inert presentation intent |
| E1 | Separately qualified low-energy tactile/directional inputs and existing indicators; no stepper power |
| E2 | Exact qualified stepper, driver, separate supply, restraint, independent stop/power removal, and measured motion acceptance |

Running an E0 sketch on a powered Mega, using live input GPIO, energizing an
indicator, or observing through a hardware debugger is not E0 evidence.
Arduino builds are packaging and size evidence only. E1 does not authorize a
coil or moving load. The E2 gate cannot be inferred from a passing host trace,
visible intent mirror, or compiler result.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Does not add |
|---:|---|---|---|
| 046 | Narrow tactile/directional facade over copied evidence | Lesson 037 `ContactDynamics`; copied Lesson 031 joystick vocabulary | GPIO ownership, capacitive sensing, gesture decoding, heartbeat semantics, specimen equivalence |
| 047 | Pure bounded logical step sequencer | `Status`, explicit `TimePoint`, copied commands | Pins, timers, interrupts, driver topology, power ownership, physical position, homing |
| 048 | Transactional kinetic-light-sculpture coordinator | Lessons 046--047 and project-local semantic light intent | Powered input/stepper endpoints, hidden stop path, position sensing, persistence, autonomous motion |

Implementation order is strict:

1. integrate this reviewed plan into the cadence, project catalog, work queue,
   and curriculum without claiming implementation;
2. complete the Lesson 046 pre-implementation architecture stress pass;
3. add the Lesson 046 facade and exhaustive host fixtures;
4. complete the Lesson 047 pre-implementation architecture stress pass;
5. add the pure sequencer and exhaustive host fixtures;
6. complete the Lesson 048 pre-implementation composition stress pass;
7. add the transactional coordinator and complete replay matrix;
8. add one compile-only Mega replay example per lesson, measured size
   evidence, HTML, pencil-drawing PDF, indexes, and generated documents;
9. run post-implementation stress passes against the measured aggregate; and
10. promote only when every non-hardware gate passes, retaining E1 and E2 as
    explicit open work.

The plan intentionally introduces no speculative shared “input source,” motor
driver, motion endpoint, or generic transaction framework. A second concrete
consumer must justify any shared abstraction.

## Shared copied-evidence vocabulary

All copied inputs carry identity, time, sequence, validity, and status. The
project must not combine observations from different epochs as if they were
simultaneous.

```cpp
enum struct InteractionSourceKind : uint8_t
{
    SyntheticFixture,
    CopiedContact,
    CopiedJoystick
};

struct InteractionSource
{
    InteractionSourceKind kind;
    uint8_t                sourceId;
    uint16_t               configurationRevision;
};

struct DirectionalEvidence
{
    InteractionSource source;
    TimePoint         observedAt;
    uint32_t          sequence;
    int16_t           xPermille;
    int16_t           yPermille;
    bool              saturated;
    Status            status;
};
```

`xPermille` and `yPermille` are each in `[-1000, 1000]`. They are copied
directional evidence, not raw ADC values and not an `AnalogJoystick` owner.
The source revision names the already-applied calibration contract. Lesson
046 does not reproduce axis calibration or button debounce from Lesson 031.

Sequence comparison uses the repository modular rule: deltas
`1..INT32_MAX` move forward, `0x80000000` is ambiguous, and larger deltas
regress. Exact repeats may age but do not emit a new event. Changed payload at
the same source-domain sequence is invalid. A source domain comprises kind,
source ID, and configuration revision.

## Lesson 046 — tactile and directional intent

### Responsibility

`InteractionIntentPolicy` is a narrow facade for one accepted tactile action
and one copied directional observation. It privately composes the existing
`ContactDynamics`; it does not duplicate qualification, release, refractory,
stuck-active, or count policy. Structural chronology faults reject before
commit rather than publishing a timing-fault quality. It derives a bounded directional
sector and magnitude from copied joystick evidence and publishes one coherent
interaction snapshot.

The facade exists because Lesson 048 needs atomic, age-qualified intent rather
than direct access to two unrelated policy snapshots. It is not a universal
sensor interface.

### Proposed public surface

The following public names, public fields, and semantics are the implementation
boundary. Opaque preview private storage is deliberately not frozen; its
representation may change within the stated object budgets without changing
public behavior or ABI. Any other change before code requires an explicit plan
amendment and review.

```cpp
enum struct InteractionDirection : uint8_t
{
    Neutral,
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};

enum struct InteractionQuality : uint8_t
{
    Invalid,
    Current,
    Stale,
    SourceFault,
    TimingFault,
    StuckActive
};

// TimingFault is reserved; structural chronology faults reject at preview.

struct InteractionIntentConfig
{
    InteractionIntentConfig (
        const ContactDynamicsConfig& contact,
        Duration                     maximumContactAge,
        Duration                     maximumDirectionalAge,
        uint16_t                     engageMagnitudePermille,
        uint16_t                     releaseMagnitudePermille) noexcept;

    ContactDynamicsConfig contact;
    Duration              maximumContactAge;
    Duration              maximumDirectionalAge;
    uint16_t              engageMagnitudePermille;
    uint16_t              releaseMagnitudePermille;
};

struct InteractionIntent
{
    InteractionSource    contactSource;
    InteractionSource    directionalSource;
    TimePoint            observedAt;
    uint32_t             contactSequence;
    uint32_t             directionalSequence;
    InteractionDirection direction;
    uint16_t             magnitudePermille;
    bool                 touchActive;
    bool                 touchEvent;
    bool                 touchReleaseEvent;
    bool                 directionEvent;
    InteractionQuality   quality;
    Duration             contactAge;
    Duration             directionalAge;
    bool                 directionalSaturated;
    ContactQuality       contactQuality;
    Status               contactStatus;
    Status               directionalStatus;
    Status               status;
};

struct InteractionIntentPreview
{
  private:
    const void* owner;
    uint32_t    generation;
    // Validated copied inputs and derived directional candidate.
    friend struct InteractionIntentPolicy;
};

struct InteractionIntentPolicy
{
    explicit InteractionIntentPolicy (
        const InteractionIntentConfig& config) noexcept;

    InteractionIntentPolicy (const InteractionIntentPolicy&) = delete;
    InteractionIntentPolicy& operator= (
        const InteractionIntentPolicy&) = delete;
    InteractionIntentPolicy (InteractionIntentPolicy&&) = delete;
    InteractionIntentPolicy& operator= (
        InteractionIntentPolicy&&) = delete;

    Status initialize () noexcept;
    void   reset      () noexcept;
    Status preview    (TimePoint                   now,
                       const InteractionSource&    contactSource,
                       uint32_t                    contactSequence,
                       const ContactSample&        contact,
                       const DirectionalEvidence&  directional,
                       InteractionIntentPreview&   candidate) const noexcept;
    bool   canCommit  (const InteractionIntentPreview& candidate) const noexcept;
    Status commit     (const InteractionIntentPreview& candidate) noexcept;

    bool              initialized () const noexcept;
    InteractionIntent snapshot    () const noexcept;

  private:
    ContactDynamics contact_;
    // Remaining fields are copied evidence and derived policy state only.
};
```

`InteractionQuality::TimingFault` is reserved for ABI stability and is not
published by this boundary. Future-time, regression, rollover-ambiguity, and
other structural chronology faults reject atomically during `preview()` and
leave the committed interaction snapshot unchanged.

`reset()` returns the pure policy to its initialized, event-free baseline. It
is not endpoint shutdown. The opaque preview contains validated copied contact
input and derived directional state; owner/generation binding rejects foreign,
stale, or reused candidates. Preview is side-effect free. The public surface
deliberately does not expose the private `ContactDynamics` snapshot; every
needed fact is copied into `InteractionIntent`.

### Configuration and update semantics

Initialization rejects either zero or wrap-unsafe age, thresholds above 1000, an
engage threshold of zero, or `releaseMagnitudePermille` greater than
`engageMagnitudePermille`. It initializes the private contact policy
atomically. Failure leaves the facade uninitialized and event-free.

The canonical Lesson 046 configuration is active-high contact,
`qualify = 8 ms`, `release = 8 ms`, `refractory = 80 ms`,
`stuckActive = 2000 ms`, `maximumContactAge = 100 ms`,
`maximumDirectionalAge = 100 ms`, `engageMagnitudePermille = 300`, and
`releaseMagnitudePermille = 200`.

`preview()` applies this order:

1. reject pre-initialization use;
2. validate both source identities, nonzero revisions/IDs, source-kind
   consistency, status enumerations, ranges, sequences, timestamps, and
   wrap-safe time progression;
3. reject a future observation, source-domain regression, ambiguous sequence,
   same-sequence changed payload, or axis outside `[-1000, 1000]`;
4. compute contact and directional ages independently and publish `Stale` when
   either exceeds its configured maximum; exact repeats age both sources;
5. treat `directional.saturated` as valid extreme evidence: it remains usable,
   retains the flag through provenance, and does not itself cause fault or
   staleness; out-of-range axes or invalid source status remain faults;
6. retain the contact input for the private policy only after all independently
   detectable facade validation succeeds;
7. derive magnitude as `max(abs(x), abs(y))`, using widened arithmetic so
   signed minima are never negated;
8. apply engage/release hysteresis and deterministic octant boundaries;
9. publish the two source identities and sequences beside the derived intent;
10. clear one-update events on exact repeated evidence while recomputing age;
   and
11. bind one complete candidate to the current owner/generation.

`commit()` calls the private contact policy once, before changing facade state,
only while `canCommit()` remains true. The existing contact operation's
failed-update atomicity is a required oracle test. On success, commit publishes
the complete facade snapshot and advances generation. No public operation
exposes a partially advanced facade.

Octants use widened unsigned magnitudes `ax` and `ay`. Neutral is selected
before sector classification. For a non-neutral value, let `major` be the
larger magnitude and `minor` the smaller. If
`minor * 1000 <= major * 414`, the sign of the major axis selects the principal
axis; exact equality therefore belongs to that axis. Otherwise the two signs
select the diagonal. When `ax == ay`, the result is diagonal. No division or
floating point is used. Direction changes emit `directionEvent`
only while engaged. Returning below the release threshold publishes
`Neutral` and one direction event. A source-domain change begins a fresh
direction baseline and cannot fabricate an event from the prior domain.

Status precedence among admitted snapshots is source fault, stuck-active,
stale, then current. Structural invalidity, including chronology faults,
rejects before publication. A contact source fault cannot be hidden by
a current joystick, and a directional fault cannot be hidden by a touch
event. No faulted update emits a usable touch or direction event.

### Deterministic test matrix

- every invalid configuration and initialization retry;
- all contact qualification/release/refractory/stuck cases inherited through
  the facade, checked against direct `ContactDynamics` oracle traces;
- source IDs/revisions/kinds at zero, valid boundaries, and unknown values;
- axis `-1000`, `1000`, neutral, engage/release edges, every octant boundary,
  exact ties, saturation, and invalid out-of-range values;
- exact repeats, changed payload at equal sequence, gaps, wrap, ambiguous
  half-range, regression, domain change, future time, age boundary, and
  `TimePoint` rollover;
- simultaneous touch/direction events and deterministic snapshot ordering;
- independent contact fault, directional fault, stale, and colliding faults;
- fault recovery only after valid fresh evidence establishes a new baseline;
- reset and failed-update atomicity; and
- byte-identical output replay without comparing padding bytes.

### E0 example, publication, and resource gates

The Mega sketch replays named copied fixtures. `setup()` initializes policy
and validates the fixture table. `loop()` advances explicit fixture time,
observes copied contact/directional evidence, decides, then writes only an
in-memory intent mirror. It performs no GPIO or ADC access.

The HTML is a concise API reference. The PDF teaches why a touch-module label
does not establish capacitive behavior, how copied evidence retains noise and
validity, contact qualification reuse, directional hysteresis, provenance,
prediction/observation/interpretation, and the E1 specimen card. Every
non-schematic visual is a pencil drawing. E0 has no electrical schematic.

Initial promotion targets, subject to measured stress review:

- object size target/hard ceiling: 192/256 bytes;
- complete Lesson 046 sketch: at most 16 KiB flash and 2 KiB static SRAM;
- no heap, pins, timers, interrupts, buses, ADC, or powered observation path;
- standalone header, umbrella export, native/package build, exception-unwind
  traits, sanitizer, and host-size gates; and
- E1 remains open for exact touch/contact/joystick adapter, test points,
  electrical schematic, and bench evidence.

## Lesson 047 — bounded logical step sequencing

### Responsibility

`BoundedStepperSequence` converts explicit logical motion commands and time
into one of four coil-intent bits. It is a volatile pure policy. It never owns
GPIO, a timer, interrupt, driver, supply, motor, stop switch, or position
sensor. Its “position” is a bounded logical count since initialization, not
physical position and not homing evidence.

The policy guarantees command expiry, rate and travel limits, exactly one due
transition per call, cancellation, stop dominance, and an all-off intent on
initialization failure, stop, expiry, fault, and reset. Endpoint shutdown is
future E2 work.

### Proposed public surface

```cpp
enum struct StepDirection : uint8_t
{
    Stopped,
    Forward,
    Reverse
};

enum struct StepSequencePhase : uint8_t
{
    Inactive,
    Holding,
    Moving,
    Complete,
    Cancelled,
    Fault
};

enum struct StepSequenceDisposition : uint8_t
{
    None,
    Accepted,
    Replaced,
    Cancelled,
    Rejected
};

struct StepperSequenceConfig
{
    StepperSequenceConfig (Duration minimumStepInterval,
                           Duration maximumStepInterval,
                           Duration maximumCommandAge,
                           int32_t  minimumLogicalPosition,
                           int32_t  maximumLogicalPosition,
                           bool     holdAtRest) noexcept;
    // named fields correspond to constructor arguments
};

struct StepperCommand
{
    uint32_t      commandId;
    TimePoint     issuedAt;
    StepDirection direction;
    uint32_t      stepCount;
    Duration      stepInterval;
    bool          cancel;
    Status        status;
};

struct StepperSequencePreview
{
  private:
    const void* owner;
    uint32_t    generation;
    // Candidate next state; callers cannot construct or inspect it.
    friend struct BoundedStepperSequence;
};

struct StepperSequenceSnapshot
{
    uint32_t                   commandId;
    StepSequencePhase          phase;
    StepSequenceDisposition    disposition;
    StepDirection              direction;
    int32_t                    logicalPosition;
    uint32_t                   requestedSteps;
    uint32_t                   completedSteps;
    uint8_t                    coilIntent;
    TimePoint                  phaseSince;
    TimePoint                  nextStepAt;
    bool                       hasDeadline;
    Status                     status;
};

struct BoundedStepperSequence
{
    explicit BoundedStepperSequence (
        const StepperSequenceConfig& config) noexcept;

    Status initialize () noexcept;
    void   reset      () noexcept;
    Status preview    (TimePoint now, const StepperCommand& command,
                       StepperSequencePreview& candidate) const noexcept;
    bool   canCommit  (const StepperSequencePreview& candidate) const noexcept;
    Status commit     (const StepperSequencePreview& candidate) noexcept;
    Status stop       (TimePoint now) noexcept;

    bool                    initialized () const noexcept;
    StepperSequenceSnapshot snapshot    () const noexcept;
};
```

The opaque preview seam is required by Lesson 048: its coordinator must prove
that input and motion candidates are both admissible before either commits.
Owner and generation binding reject foreign, stale, replayed, or already-used
candidates. Preview is side-effect free and allocation free.

### Command and timing semantics

Configuration rejects zero/wrap-unsafe durations, reversed interval bounds,
invalid logical bounds, and a range that cannot be updated safely with widened
arithmetic.

Commands require a nonzero ID, valid status, non-future `issuedAt`, interval
inside configured bounds, and a step count whose intended endpoint remains
inside logical bounds. `Stopped` requires zero steps. Moving requires nonzero
steps. `cancel` dominates all other fields except structural invalidity and
may cancel only a currently live command. Exact replay of the current command
is idempotent. A new forward modular command ID replaces a live command only
if its complete endpoint is admissible; regression, half-range ambiguity, or
same-ID changed payload is rejected without mutation.

At most one due step is prepared per call. If multiple deadlines elapsed, the
policy advances one frame and retains the next logical deadline; repeated
calls catch up without stretching the requested interval or silently skipping
coil frames. The scheduler/latency gate must show this bounded work cannot
starve the Lesson 048 composition.

The four-bit half-step sequence is fixed and published:
`0001, 0011, 0010, 0110, 0100, 1100, 1000, 1001`; reverse traverses it in the
opposite order. The bits are logical coil intents only, not pin numbers or an
electrical drive contract. `holdAtRest == false` clears all bits at completion.
When true, completion may retain the final logical frame at E0/E1, but E2
promotion must independently bound hold time and temperature; shutdown,
fault, expiry, cancellation, and independent stop always clear all bits.

Command age is checked before stepping. Expiry enters `Fault`, clears coil
intent, retains attribution, and requires `reset()` before new motion.
`stop()` is idempotent, clears intent immediately, records cancellation, and
does not claim physical power removal.

### Deterministic test matrix

- every configuration and command validity boundary;
- all eight frames forward/reverse, reversals, one-step and maximum-travel
  commands, logical endpoint exact bounds, and overflow probes;
- deadlines immediately before/at/after, multiple overdue steps, repeated
  one-step catch-up calls, rollover, and time regression;
- exact command replay, replacement, same-ID mutation, ID wrap, ambiguity,
  regression, cancellation, expiry, source status failure, and stop dominance;
- preview foreign owner, stale generation, double commit, preview after reset,
  and failed-commit atomicity;
- hold/no-hold completion; all-off on every fault/cancel/stop/reset path;
- restart from every phase with no retained physical-position claim; and
- byte-identical traces for identical commands and timestamps.

### E0 example, publication, and resource gates

The sketch replays commands and mirrors the four intent bits into a plain
byte. It names the prediction and resulting bit pattern but owns no output.
The PDF uses pencil drawings for sequence, timing, cancellation, and the
separation between logical position and physical position. It includes an E2
formal-schematic placeholder only as a visibly gated acceptance item; no
authoritative circuit is drawn before specimen qualification.

Initial promotion targets:

- object size target/hard ceiling: 128/176 bytes;
- complete Lesson 047 sketch: at most 16 KiB flash and 2 KiB static SRAM;
- bounded update work proven for exactly one transition per call;
- exact all-off intent tests independent of resource-acquisition evidence; and
- E2 driver, supply, clamp, coil, thermal, restraint, stop, power-removal, and
  physical-motion cards remain open.

## Lesson 048 — kinetic light sculpture

### Responsibility

`KineticLightSculpture` transactionally combines one Lesson 046 interaction
intent with one Lesson 047 logical sequence and project-local semantic light
intent. Touch authorizes one bounded motif; direction selects one of eight
motifs and its logical direction. Existing shift-register/light intent mirrors
the prepared coil frame before any future E2 motor output. A stop input is
independent, dominant, and cannot be represented only through the tactile
facade.

At E0 the “sculpture” is a complete replayable instrument model. It produces
light and coil intent values but actuates nothing.

### Proposed project surface

```cpp
enum struct SculpturePhase : uint8_t
{
    Inactive,
    Ready,
    Preview,
    Running,
    Complete,
    Stopped,
    Fault
};

enum struct AuthorizationDisposition : uint8_t
{
    None,
    Pending,
    Accepted,
    Inhibited,
    BoundRejected
};

struct AuthorizationRecord
{
    uint32_t                 originatingFrameId;
    InteractionSource        contactSource;
    InteractionSource        directionalSource;
    uint32_t                 contactSequence;
    uint32_t                 directionalSequence;
    InteractionDirection     direction;
    AuthorizationDisposition disposition;
    Status                   status;
};

struct SculptureInput
{
    TimePoint                  observedAt;
    uint32_t                   frameId;
    InteractionSource         stopSource;
    TimePoint                 stopObservedAt;
    uint32_t                  stopSequence;
    bool                      stopActive;
    ContactQuality            stopQuality;
    Status                    stopStatus;
    InteractionSource         touchSource;
    uint32_t                  touchSequence;
    ContactSample             touchSample;
    DirectionalEvidence       directional;
    Status                    status;
};

struct SculptureLightIntent
{
    uint8_t shiftRegisterBits;
    bool    ready;
    bool    running;
    bool    stopped;
    bool    fault;
    bool    travelLimit;
};

struct SculptureSnapshot
{
    uint32_t                frameId;
    SculpturePhase          phase;
    InteractionIntent       interaction;
    StepperSequenceSnapshot motion;
    SculptureLightIntent    lights;
    InteractionSource       stopSource;
    TimePoint               stopObservedAt;
    uint32_t                stopSequence;
    bool                    stopActive;
    bool                    hasStopIdentity;
    bool                    travelLimit;
    bool                    hasPendingAuthorization;
    AuthorizationRecord     pendingAuthorization;
    bool                    hasLastTerminalAuthorization;
    AuthorizationRecord     lastTerminalAuthorization;
    ContactQuality          stopQuality;
    Status                  stopStatus;
    Status                  interactionStatus;
    Status                  motionStatus;
    uint32_t                acceptedMotifCount;
    Status                  status;
};

struct KineticLightSculpture
{
    KineticLightSculpture (
        const InteractionIntentConfig& interactionConfig,
        const StepperSequenceConfig&   sequenceConfig,
        Duration                       maximumFrameAge,
        Duration                       maximumSourceSkew) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (const SculptureInput& input) noexcept;

    bool              initialized () const noexcept;
    SculptureSnapshot snapshot    () const noexcept;

  private:
    InteractionIntentPolicy interaction_;
    BoundedStepperSequence  sequence_;
};
```

`SculptureLightIntent` is the exact project-local presentation value. It does
not reuse, mutate, or overload an endpoint-owning presentation component. The
project owns its two pure policies by value and exposes no borrowed lifetime. Independent stop
evidence is already-qualified copied evidence, not a third debounce/contact
policy. A future E1 stop adapter owns electrical sampling and qualification
and must preserve source, time, sequence, active state, and status.

### Transaction and state semantics

`maximumFrameAge` is fixed to 250 ms in the canonical example and may be
configured only in `[1, 1000]` ms. `maximumSourceSkew` is fixed to 40 ms and
may be configured only in `[0, maximumFrameAge]`. Initialization validates all
configurations before initializing either child,
then initializes children in dependency order. Failure resets both children and
publishes `Inactive` with cleared light and coil intent. Repeated
initialization is idempotent. `shutdown()` is idempotent, clears every intent,
calls `sequence_.stop()` using the last valid project time, then resets the
interaction and sequence policies, clears project intent, and marks the project
uninitialized. It does not claim endpoint shutdown.

Each update:

1. validates the independent stop channel's identity, time, sequence, quality,
   and status before examining unrelated fields;
2. commits an active valid stop immediately; stop dominates malformed
   touch/direction evidence, command replacement, completion, and source fault,
   while the snapshot retains any structurally readable collided statuses;
3. when stop is inactive, validates frame identity, age, source skew, and all
   touch/direction identities, sequences, statuses, and structural shape
   before child mutation;
4. previews interaction without exposing partial project state;
5. derives any stepper command solely from a pending authorization captured
   from the last committed `InteractionIntent`; the opaque current preview is
   validation-only and is never inspected for `touchEvent`;
6. previews the complete sequence transition from that prior authorization;
7. completes every structural check and calls both `canCommit()` operations
   before any commit;
8. commits interaction first and inspects its resulting private snapshot;
9. when that snapshot is healthy and current, commits the already-preflighted
   sequence candidate; project-exclusive ownership and no intervening sequence
   mutation guarantee this commit cannot fail or become stale;
10. when interaction instead admits `Stale`, `StuckActive`, or
    source fault, does not commit the motion candidate; it calls the sequence's
    idempotent stop/cancel at the already validated project time, which is
    contractually infallible after preflight, consumes zero steps and zero
    logical travel, and does not increment `acceptedMotifCount`; and
11. when no authorization was pending at frame start and the frame is healthy,
    records a usable freshly committed `touchEvent` and its committed direction
    as authorization eligible only on the next strictly-forward frame, then
    publishes the sole resulting project snapshot; a frame that resolves an
    older pending authorization cannot enqueue a replacement, and an
    admitted-fault frame publishes all-off with pending authorization
    inhibited.

Step 10 is a controlled asymmetric admitted-fault protocol, not symmetric
child-state atomicity. It is bounded because interaction fault evidence must
be retained while motion must fail off. Both children are private, and no
project snapshot, light intent, coil intent, callback, or other output is
published between interaction commit and sequence stop; the intermediate
child state is unobservable. Structural rejection never enters this protocol
and mutates neither child nor project state.

Authorization is a one-entry project-owned queue with two explicit public
records. When `hasPendingAuthorization`, `pendingAuthorization` retains the
full originating frame, contact/directional sources and sequences, direction,
exact status, and disposition `Pending`. When
`hasLastTerminalAuthorization`, `lastTerminalAuthorization` retains the same
full provenance with exactly one terminal disposition: `Accepted`,
`Inhibited`, or `BoundRejected`.
The dual-record surface preserves distinct live and historical evidence; it
does not assert that consumption and a replacement touch are reachable in one
frame.
An exact repeated frame cannot enqueue again. A strictly-forward frame consumes
the entry exactly once only when both child commits succeed. Stop, source fault,
shutdown, reset, or command expiry consumes it with an explicit inhibited/fault
disposition and cannot replay it. A travel-bound rejection consumes it with
`travelLimit == true`; recovery requires a new opposite-direction touch event.
If no eligible authorization exists, the sequence preview advances an existing
command or remains stopped. On the next strictly-forward frame, an older
pending authorization reaches exactly one terminal disposition. That
resolving frame is ineligible to enqueue another authorization. Later
strictly-forward frames must first carry the contact release and subsequent
requalification; only the later frame that commits the new `touchEvent` may
publish a new pending authorization. Reusing the same frame/sequence with a
changed contact payload remains a structural rejection, never a shortcut to a
second event.
Thus an event is either pending, accepted once, or recorded with one terminal
disposition; it is never repeated or silently lost.

`Accepted` is recorded only on the frame that admits motion. `Inhibited`
records stop, admitted fault, expiry, shutdown, or reset. `BoundRejected`
records complete-endpoint rejection. Initialization and power-loss restart
canonically clear both `has` flags and zero both records with disposition
`None` and OK status. Stop or fault terminalizes a pending record as
`Inhibited` with the exact cause, copies it to `lastTerminalAuthorization`,
and clears the pending flag; without a pending entry it preserves the previous
terminal record. Project shutdown terminalizes pending in the same way before
clearing live intent; the terminal record remains inspectable until the next
initialization. Internal child reset cannot create or erase an authorization
record. A terminal record is observable on the frame that resolves it. The
next accepted strictly-forward frame clears it to the canonical `None` record
unless that frame replaces it with a newer terminal outcome.

The staged one-frame authorization makes the Lesson 046 preview genuinely
opaque: Lesson 048 never needs a prepared-intent accessor and never predicts a
private `ContactDynamics` event. Its validated-input candidate supplies
structural preflight without changing that existing public contract. Commit
must prove every structurally failing private contact update is non-mutating,
every semantic fault is admitted and classifiable only after commit, and the
preflighted sequence commit or fault-stop cannot fail absent intervening
sequence mutation. If an oracle fails, promotion stops for a bounded
remediation decision. Adding a public preview seam to `ContactDynamics`
challenges an existing public contract and requires explicit architectural
remediation and user discussion.

Malformed non-stop input causes no child or project mutation when stop is
inactive. A malformed stop channel is itself a fail-closed project fault:
without trusting its timestamp, active bit, quality, or status, the project
uses the last valid project time to call `sequence_.stop()`, clears project
coil/light intent, publishes `Fault`, attributes the failure to the stop
channel with `InvalidArgument`, and retains the last valid stop identity as
historical evidence. If no valid project time or stop identity exists, it uses
`sequence_.reset()` rather than a timed stop and publishes the canonical
no-evidence tuple: `stopSource = {SyntheticFixture, 0, 0}`,
`stopObservedAt = TimePoint(0)`, `stopSequence = 0`,
`stopQuality = ContactQuality::Unqualified`,
`stopStatus = Status(StatusCode::InvalidArgument)`, and
`hasStopIdentity == false`. No initialization time is invented. Unrelated
malformed evidence cannot suppress a valid active stop. A valid source fault
enters project `Fault`, clears motion and all non-fault presentation, retains
the original status and source attribution, and requires reinitialization.
Stop is not a fault: it enters `Stopped`, clears motion immediately, displays
the independent stop indication, and requires a released, newly qualified
stop plus a fresh touch event before returning to `Ready`. Power-loss replay
starts `Inactive`; volatile logical position and motif progress are not
restored. Simultaneous valid stop and source fault publishes `Stopped`, retains
the exact collided source status in the snapshot, and permits no motion.

Simultaneous precedence is:

1. valid independent stop active/event;
2. malformed stop evidence (fail-closed fault);
3. unrelated structural invalidity (atomic rejection while stop is inactive);
4. admitted child/source semantic fault;
5. command expiry;
6. completion;
7. freshly committed touch queued for next-frame authorization;
8. direction change; and
9. ordinary progress.

The light mirror displays the committed coil intent, never a speculative
preview. It is evidence of software intent, not proof that a coil is powered
or moving. In E2, independent physical power removal remains authoritative
over firmware and over this mirror.

### Fixed motifs

The canonical sequencer configuration uses a 60--250 ms interval range,
2000 ms maximum command age, logical bounds `[-256, 256]`, and no holding at
rest. The project publishes this fixed table:

| Direction | Logical direction | Steps | Interval |
|---|---|---:|---:|
| North | Forward | 8 | 120 ms |
| NorthEast | Forward | 12 | 100 ms |
| East | Forward | 16 | 80 ms |
| SouthEast | Forward | 12 | 100 ms |
| South | Reverse | 8 | 120 ms |
| SouthWest | Reverse | 12 | 100 ms |
| West | Reverse | 16 | 80 ms |
| NorthWest | Reverse | 12 | 100 ms |

The light byte is project-local mirror intent: its low four bits exactly equal
the committed `motion.coilIntent`, and its high four bits are zero. Stop,
fault, reset, and shutdown publish zero. It is not a `ShiftRegisterOutput`
frame or electrical contract. A future E1 adapter may present it through an
existing qualified output. Motifs contain no arbitrary script, heap storage,
or externally supplied coil frames.
The longest motif defines the maximum authorized composition pressure.
E2 may reduce these bounds after measured pull-in, thermal, fixture, and supply
evidence; it may not silently increase them.

The longest canonical motif lasts 1280 ms at its requested cadence, leaving
720 ms of aggregate scheduling lateness before the 2000 ms expiry. Command age
is checked before every transition; crossing 2000 ms faults all-off even when
steps remain. At a logical travel bound, a fresh motif whose complete endpoint
would exceed `[-256, 256]` is rejected without child mutation or project fault.
The project remains `Ready`, publishes a `travelLimit` indication, and accepts
a fresh opposite-direction touch motif that moves back into range. It never
wraps, clamps a partially executed motif, or calls the bound a physical stop.

### Deterministic project test matrix

- initialization validation, child rollback, repeated initialization,
  shutdown/destruction from every phase, and restart;
- all eight motifs, neutral rejection, exact one-frame authorization latency,
  pending/accepted/inhibited/bound-rejected dispositions, no repeat or loss,
  refractory touch, direction hysteresis, completion, and fresh
  reauthorization;
- structurally rejected frames, including every chronology fault, proving zero
  child/project mutation before preflight, plus admitted stale/stuck/source
  faults proving interaction
  evidence commits, the motion candidate does not commit, stop/cancel cannot
  fail, zero step/travel/count is consumed, output remains all-off, and no
  intermediate child state is observable;
- both authorization records' complete provenance/status for `None`,
  `Pending`, `Accepted`, `Inhibited`, and `BoundRejected`; proof that a
  resolving frame cannot enqueue; later release and requalification before a
  new pending frame; terminal visibility on its resolving frame and clearing
  on the next accepted forward frame unless replaced; canonical
  initialization/power-loss clearing; stop/fault/shutdown terminalization; and
  no event loss or repeat;
- frame/source sequence exact repeat, gap, wrap, ambiguity, regression, domain
  change, future time, rollover, and stale evidence;
- stop before start, during each logical frame, simultaneous with touch,
  simultaneous with completion/fault, held stop, release qualification, and
  attempted restart without a fresh touch;
- source fault in each input, sequence fault, command expiry, multiple-overdue
  one-step calls,
  and at least the maximum credible collision of stop plus source fault plus
  overdue steps;
- candidate validation failure after otherwise valid child preparation,
  proving no partial child commit;
- intent mirror equality to every committed coil frame and all-off equality on
  stop, fault, expiry, shutdown, and restart;
- simulated power loss from every phase, proving no restored position claim;
- complete state-transition coverage and byte-identical golden replay; and
- capacity immediately below, at, and above every finite bound.

### Maximum-composition stress scenario

The promotion stress pass uses one sculpture with the longest motif, minimum
allowed step interval, multiple overdue transitions, touch and direction arriving at
the same timestamp, a simultaneously asserted independent stop, a stale/faulted
directional source, and enabled intent-mirror diagnostics. It accounts for:

- both child objects, copied inputs/snapshots, preview candidates, motif
  table, fixture table, stack peak, flash, static SRAM, and return-address
  depth;
- worst-case bounded update work without a hidden scheduler, timer, interrupt,
  or heap;
- no bus, persistence, hardware resource claim, or power domain at E0;
- provenance for contact, direction, stop, command, light intent, and coil
  intent;
- diagnostic mirror failure or omission without changing primary state; and
- fault precedence, retained attribution, cleared intent, reset, and replay.

The complete Mega E0 sketch must remain at or below 28 KiB flash and 2 KiB
static SRAM, with at least 1 KiB measured conservative stack margin. Each
project object has an initial target/hard ceiling of 512/640 bytes. These are
promotion gates, not predictions; measured pressure may require local
compression that preserves behavior or architectural discussion.

### Narrative example and publication division

The compile-only example reads as:

1. acquire: validate the fixed fixture and motif tables;
2. configure: construct the two policy configurations;
3. start: atomically initialize the sculpture;
4. observe: select one copied input frame at explicit time;
5. decide: call one project update; and
6. actuate: copy committed light/coil intent to an in-memory evidence cell.

The HTML is an API and state/reference page with source, tests, example, PDF,
evidence-level limits, and next-step links. The PDF is complementary: learner
story, pencil orientation drawing, tactile/directional provenance, logical
motion flipbook, atomic state flow, prediction/observation/interpretation,
fault diagnosis, exercises, size record, and blank E1/E2 acceptance cards.
Every visual is pencil presentation unless a future electrically authoritative
formal schematic is explicitly classified. E0 publishes no schematic.

The E0 non-Serial observation path is a named in-memory `intentMirror` evidence
cell inspected by the deterministic host/compile fixture. It must not be
described as a physical circuit observation. E1 adds an existing
shift-register/LED mirror and an independent stop indication only after its
endpoints are qualified. E2 separately proves the coil-frame test points,
measured current, restrained motion, de-energized startup/shutdown, fault
injection, reset, unplugged logic, stale command, thermal bound, and independent
power removal.

## Block-wide acceptance gates

### Source and API

- standalone headers compile; supported code contains no `legacy/`;
- warnings are errors; no heap, exceptions, RTTI, hidden clock, or global
  registration;
- copy/move traits match pure-policy ownership;
- private composition reuses `ContactDynamics` without exposing or duplicating
  its policy; and
- every enum and input is exhaustively validated.

### Determinism and failure

- fixtures provide explicit timestamps, source domains, sequences, statuses,
  axes, levels, frame IDs, command IDs, and expected complete snapshots;
- every transition, rollover, finite boundary, collision, recovery, restart,
  and shutdown is covered;
- preview/commit owner and generation binding prevents partial commits; and
- repeated runs produce byte-identical semantic records without padding-byte
  comparisons.

### Arduino, size, and packaging

- one canonical compile-only Mega sketch per lesson;
- measured flash, static SRAM, object, aggregate live-state, and conservative
  stack evidence;
- library archive, native consumer, umbrella header, source inventory, lesson
  inventory, downloads, and site staging include the boundary exactly once;
- no E0 sketch calls GPIO, ADC, timer, interrupt, bus, delay, or powered
  endpoint APIs; and
- a clean full quality run passes before promotion.

### Publication

- HTML and PDF are complementary and use identical vocabulary;
- all non-schematic PDF visuals pass the pencil-drawing classification gate;
- no formal schematic appears until it is electrically authoritative;
- every claim labels E0, E1-open, or E2-open honestly;
- the landing page remains scan-first and the newest-lesson verifier advances
  only with actual promotion; and
- the work queue, curriculum, projects, roadmap, components, size baseline,
  indexes, and release limitations agree.

### Physical gates retained

E1 requires exact input specimen identity, electrical source, supply/output
limits, polarity, calibration/noise evidence, GPIO ownership, named test
points, authoritative schematic, and separate resource/safe-state proof.

E2 additionally requires exact motor and ULN2003 identities, winding and
voltage, separate current-limited supply, clamp connection, coil order,
measured current, thermal limits, bounded rate/travel, restrained fixture,
independent stop and power removal, reset/unplug/stale/fault tests, and
power-removal verification. Nothing in E0 or E1 closes those requirements.

## Design-stress disposition and remediation triggers

Each lesson receives a completed pre-implementation and pre-promotion stress
record using `docs/templates/component-design-stress-pass.md`. The provisional
disposition is `natural fit` only if measurement confirms all stated bounds.

Bounded local remediation is permitted when confined to this unpromoted block
and behavior remains unchanged, for example:

- compacting private snapshots or candidates after exact size measurement;
- reducing a fixed motif count while retaining the published
  minimum useful experiment;
- changing private staging order to preserve atomicity; or
- splitting host tests into deterministic size-bounded shards.

Stop promotion and discuss architectural remediation with the user before:

- changing the public `ContactDynamics`, `AnalogJoystick`, resource, time,
  `Status`, endpoint, or presentation contracts;
- adding a public generic source, motor, transaction, preview, scheduler, or
  power-domain abstraction without a second concrete consumer;
- permitting partial child commit or inferring rollback after mutation;
- hiding pins, timers, interrupts, buses, endpoint ownership, or a clock in a
  policy;
- treating logical count as physical position or weakening homing scope
  reserved for Lessons 049--051;
- treating a light mirror as coil-current, motion, stop, or safe-state proof;
- increasing E0 limits to accommodate unmeasured E2 behavior;
- allowing powered or moving work without exact specimen and acceptance
  evidence;
- exceeding a hard memory, stack, flash, update-work, pin, timer, current,
  thermal, or travel bound;
- requiring a compatibility change in multiple existing consumers; or
- changing canonical publication, safety, inventory, or evidence-level policy.

For a trigger, record affected decisions and consumers, evidence, bounded
alternatives, migration cost, safety/resource consequences, and the smallest
next experiment. Promotion remains blocked until the user selects any
materially different outcome and the decision is durably recorded.

## Scope preserved for later lessons

Lessons 049--051 retain identity, physical position uncertainty, bounded
homing, and the parts carousel. Lesson 046 does not authenticate a person or
identify a gesture. Lesson 047 does not establish home or absolute position.
Lesson 048 does not persist position across reset.

Lessons 055--057 retain the generic constraint and fault-aware operator-panel
composition. Lessons 070--072 retain descriptor and copied-sweep policy, with
exact-specimen characterization separately gated. Lessons 079--081 retain
low-side-driver qualification and indicator semantics. This block may consume
existing presentation intent, but it does not preempt those later boundaries
or claim an unidentified pulse/gesture module.
