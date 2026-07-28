# Lessons 049--051 identity-controlled parts-carousel plan

Status: implementation-depth E0 plan, 2026-07-28.

This block follows the kinetic-light-sculpture work with a tangible
identity-and-position project while preserving the repository's electrical,
motion, storage, and evidence gates. The authorized implementation is E0:
copied synthetic identity/key evidence, fixed caller-owned record images, a pure bounded
homing policy, transactional project composition, and inert actuator and
presentation intent. It does not authorize an RFID transaction, keypad GPIO,
nonvolatile write, Hall/reed input, powered indicator, servo pulse, stepper
coil, moving mechanism, wiring table, formal schematic, or physical acceptance
claim.

An identifier is a local lesson token, not a credential, proof of possession,
person, authorization system, or security boundary. The project dispenses
nothing and opens no powered gate at E0. Its “parts” are named paper-bin
records; its carousel position is a logical coordinate in a deterministic
fixture.

## Evidence levels and stale cadence correction

| Level | Authorized work |
|---|---|
| E0 | Pure host policies, fixed volatile caller-owned storage, synthetic copied evidence, compile-only Mega replay, and inert intent mirrors |
| E1 | Separately qualified RFID/keypad/home inputs, existing display and LEDs, and a qualified nonvolatile adapter; no motor or servo power |
| E2 | Exact qualified stepper/driver and servo, separately switchable current-limited actuator power, restrained lightweight mechanism, independent stop and power removal, and measured motion acceptance |

The older cadence prose says that the carousel “homes and moves,” the servo
“opens,” audit records “survive” an interrupted write, and position LEDs work
before motors are powered. Those sentences describe future E1/E2 experiments,
not the active implementation boundary. They are stale as authorization and
must be reconciled when this plan is integrated:

- E0 emits exact logical step, coil, and semantic gate intents but performs no homing motion,
  gate movement, endpoint write, or durable storage operation.
- E0 interruption recovery means reconstruction from an explicitly supplied
  fixed record image; it does not claim EEPROM/SD durability.
- E0 observation consists of retained result cells and fixture-owned mirrors.
  A powered Mega sketch, debugger, LED, LCD, RFID reader, keypad, or sensor is
  not E0 evidence.
- “Position” means logical position only. Physical position remains unknown
  until an E2 homing acceptance record succeeds.

This correction preserves the cadence subject and learner outcome. It changes
only the evidence pacing that predates the repository's explicit E0/E1/E2
model.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Does not add |
|---:|---|---|---|
| 049 | Fixed local identity records and bounded enrollment over copied evidence | `Status`, explicit time, caller-owned fixed storage concepts | RFID transport, keypad endpoint, cryptography, access control, person identity, persistent medium |
| 050 | Pure bounded homing and logical-position policy | copied home/stop evidence and semantic signed-step intent | Lesson 047 child, coil driver, timer, interrupt, sensor endpoint, physical-position claim, autonomous retry |
| 051 | Transactional inert parts-carousel coordinator | Lessons 049--050, one project-owned Lesson 047 sequencer, semantic gate/presentation and audit records | Servo child, powered motion, hidden endpoint ownership, automatic dispensing, physical-media durability claim |

Implementation order is strict:

1. integrate the reviewed plan into the cadence, curriculum, project catalog,
   work queue, and task state without claiming implementation;
2. complete a pre-implementation architecture stress pass for each lesson;
3. implement Lesson 049 and its exhaustive host fixtures;
4. implement Lesson 050 and its exhaustive host fixtures;
5. complete the Lesson 051 maximum-composition stress pass;
6. implement the transactional coordinator and complete replay matrix;
7. add one compile-only Mega replay per lesson, measured size evidence, HTML,
   pencil-drawing PDFs, indexes, and generated documents;
8. run post-implementation stress passes against measured aggregate budgets;
9. promote only after every non-hardware gate passes; and
10. retain E1 and E2 as independently open work.

No generic identity provider, motion controller, transaction framework, or
durability abstraction is introduced. Existing lower-layer contracts change
only after a separately reviewed architectural decision.

## Shared copied-evidence vocabulary

Every input belongs to an explicit source domain and retains its occurrence
time and sequence. Same-domain sequence deltas `1..INT32_MAX` move forward,
`0x80000000` is ambiguous, and larger deltas regress. Exact same-sequence
replay may age but cannot create an event. A changed payload at the same
sequence is malformed and must not mutate accepted history.

Lesson 050 takes the conservative bounded form: every repeated frame sequence
is rejected atomically, including an otherwise identical replay. Exact
idempotent acceptance would require retaining the complete prior frame and
command beyond the reviewed 128-byte policy ceiling; a fingerprint would
weaken exact identity. Rejection preserves the snapshot and creates no event.

Time admission is directional. Compare `now` to each occurrence time first:
a modular delta `0..INT32_MAX` is present/past, `0x80000000` is ambiguous, and
a larger delta is future/regressed. Only present/past evidence is tested for
maximum age and then pairwise/frame skew; skew never makes future evidence
fresh. The independently valid stop path applies this rule to stop alone
before considering malformed unrelated fields.

```cpp
enum struct CarouselSourceKind : uint8_t
{
    SyntheticIdentity,
    SyntheticKey,
    SyntheticHome,
    SyntheticStop
};

struct CarouselSource
{
    CarouselSourceKind kind;
    uint8_t            sourceId;
    uint16_t           configurationRevision;
};

struct CopiedKeyEvidence
{
    CarouselSource source;
    TimePoint      observedAt;
    uint32_t       sequence;
    uint8_t        key;
    bool           pressed;
    Status         status;
};

struct CopiedBinaryEvidence
{
    CarouselSource source;
    TimePoint      observedAt;
    uint32_t       sequence;
    bool           active;
    bool           qualified;
    uint32_t       qualificationEpoch;
    Status         status;
};
```

`key` uses only the project-configured digits plus confirm and cancel codes.
No ASCII parsing, PIN secrecy, long-press interpretation, or debounce is
performed. `qualified` and `qualificationEpoch` are copied adapter claims, not
raw-level inference: release and acquisition edges must use the same nonzero
epoch, source, and configuration revision. Future E1 adapters own electrical
polarity, sampling, debounce, rollover-safe event production, and exact source
qualification.

## Lesson 049 — local identity records

### Responsibility and public values

`LocalIdentityRegistry` validates copied UID-shaped observations, maps them to
one of a bounded set of local paper-bin IDs, and stages enrollment changes
transactionally. It does not read a card, authenticate a token, identify a
person, authorize a real resource, or own storage.

```cpp
static constexpr uint8_t maximumLocalIdentityBytes = 10;
static constexpr uint8_t maximumLocalIdentities    = 8;
static constexpr uint8_t maximumCarouselBins       = 8;

struct LocalIdentity
{
    uint8_t length;
    uint8_t bytes[maximumLocalIdentityBytes];
};

struct IdentityEvidence
{
    CarouselSource source;
    TimePoint      observedAt;
    uint32_t       sequence;
    LocalIdentity  identity;
    Status         status;
};

enum struct IdentityDisposition : uint8_t
{
    None = 0,
    Known = 1,
    Unknown = 2,
    Duplicate = 3,
    LockedOut = 4,
    DirectoryFull = 5,
    AuthorizationRequired = 6,
    EnrollmentPending = 7,
    Malformed = 8,
    ImageCorrupt = 9,
    ImageUnsupported = 10,
    CommitIndeterminate = 11,
    StorageFault = 12
};

struct IdentityBinding
{
    LocalIdentity identity;
    uint8_t       binId;
    uint16_t      revision;
    uint16_t      checksum;
};

static constexpr uint16_t localIdentityImageMagic      = 0x4944;
static constexpr uint8_t  localIdentityImageVersion    = 1;
static constexpr uint8_t  localIdentitySlotCount       = 2;
static constexpr uint16_t localIdentityImageBytes      = 160;

struct IdentityImageView
{
    const uint8_t* bytes;
    uint16_t       length;
    uint8_t        slot;
    uint32_t       generation;
};

struct LocalIdentityRegistryConfig
{
    uint32_t registryConfigurationId;
    uint8_t  binCount;
    uint8_t  maximumFailures;
    Duration lockoutDuration;
    Duration maximumEvidenceAge;
};

struct IdentityRegistrySnapshot
{
    IdentityDisposition disposition;
    uint8_t             selectedBin;
    uint8_t             bindingCount;
    uint8_t             failedAttempts;
    bool                enrollmentPending;
    bool                externalCommitPending;
    uint32_t            imageGeneration;
    uint16_t            matchedBindingRevision;
    uint32_t            acceptedSequence;
    Status              status;
};
```

The identity byte codec is normative. Multibyte integers are unsigned
little-endian; reserved bytes/bits are zero. An erased slot is all 160 bytes
`0xff` and is not checksum-valid.

| Offset | Bytes | Encoding |
|---:|---:|---|
| 0 | 2 | magic `0x4944` |
| 2 | 1 | schema version `1` |
| 3 | 1 | flags `0` |
| 4 | 2 | encoded length `160` |
| 6 | 4 | nonzero `registryConfigurationId` |
| 10 | 4 | image generation |
| 14 | 1 | binding count `0..8` |
| 15 | 1 | reserved zero |
| 16 | 128 | eight 16-byte binding entries |
| 144 | 14 | reserved zero |
| 158 | 2 | image checksum |

An entry encodes length at `+0`, ten identity bytes at `+1..+10` with zero
tail, bin at `+11`, nonzero binding revision at `+12..+13`, and entry checksum
at `+14..+15`; unused entries are all zero. Checksums are
CRC-16/CCITT-FALSE (polynomial `0x1021`, initial `0xffff`, no reflection,
final XOR zero). Entry coverage is `+0..+13`; image coverage is bytes
`0..157`, including unused and reserved bytes. Unknown flags/version, wrong
length/configuration ID, noncanonical bytes, malformed entries, and checksum
failure are rejected.

Files are `src/local_identity_registry.h` and
`src/local_identity_registry.cpp`. The intended surface is:

```cpp
struct EnrollmentCandidate
{
    uint32_t owner;
    uint32_t candidateGeneration;
    uint32_t baseImageGeneration;
    uint32_t operationId;
    uint8_t         scratchIndex;
    uint16_t        checksum;
    Status          status;
};

struct IdentityDurableCommitEvidence
{
    uint32_t          owner;
    uint32_t          candidateGeneration;
    uint32_t          operationId;
    uint8_t           slot;
    IdentityImageView reconciledImage;
    bool              synchronized;
    bool              rereadValidated;
    Status            durableStatus;
};

struct LocalIdentityRegistry
{
    LocalIdentityRegistry (const LocalIdentityRegistryConfig& config,
                           IdentityBinding*                   liveStorage,
                           uint8_t                            capacity,
                           uint8_t*                           imageSlotBytes,
                           uint16_t                           imageSlotByteExtent,
                           uint16_t                           imageSlotStride,
                           uint8_t                            imageSlotCount,
                           uint8_t*                           candidateImageBytes,
                           uint16_t                           candidateImageCapacity)
        noexcept;

    Status initialize () noexcept;
    Status reset      () noexcept;
    void   shutdown   () noexcept;

    Status observe (TimePoint now, const IdentityEvidence& evidence) noexcept;

    Result<EnrollmentCandidate> previewEnrollment (
        TimePoint now, const IdentityEvidence& evidence, uint8_t binId) noexcept;
    Result<IdentityImageView> previewExport (
        const EnrollmentCandidate& candidate) const noexcept;
    Status acknowledgeExternalCommit (
        const EnrollmentCandidate& candidate,
        const IdentityDurableCommitEvidence& evidence) noexcept;
    Status cancelEnrollment () noexcept;

    IdentityRegistrySnapshot snapshot () const noexcept;

    LocalIdentityRegistry (const LocalIdentityRegistry&) = delete;
    LocalIdentityRegistry& operator= (const LocalIdentityRegistry&) = delete;
    LocalIdentityRegistry (LocalIdentityRegistry&&) = delete;
    LocalIdentityRegistry& operator= (LocalIdentityRegistry&&) = delete;
};
```

Construction is inert and copies the small configuration value; it borrows
only the explicitly sized storage. Initialization validates nonzero
`registryConfigurationId`, configuration, non-null
storage, capacity `1..8`, and exactly two supplied canonical image slots.
Their extent must cover `2 * 160`, stride must equal `160`, and candidate
capacity must equal `160`; undersized, oversized/ambiguous, or overlapping
views are rejected.
Each slot has fixed magic, version, encoded length, generation, bounded entry
count, canonical unused bytes, valid entries, and checksum. Recovery chooses
the newer valid slot by the same half-range generation rule as source
sequences; equal generations must be byte-identical and half-range separation
is ambiguous. One valid slot dominates one corrupt, torn, erased, or
unsupported or foreign-configuration slot. Two invalid slots fail closed; corruption is never
interpreted as an empty directory. The chosen image is copied into the
caller-owned live table only after complete validation. The registry owns no
resource. Caller-owned arrays must outlive it and must not move. Failed
validation leaves the arrays and object uninitialized.

Identity lengths are `4..10`; zero padding beyond `length` is canonical and
participates in validation. All-zero and all-`0xff` identifiers are rejected
as fixture sentinels. Equality is length plus bytes, not checksum. Checksums
detect malformed fixed images but make no integrity or security claim.

Unknown evidence increments a saturating failure count once per new sequence.
At `maximumFailures`, lockout begins for `lockoutDuration`; exact boundary
expiry is allowed. Known evidence clears the failure count only after lockout
has expired. Structural, source, time, or storage-image failure dominates
lockout and never increments the count.

A `Known` snapshot exposes the exact nonzero `matchedBindingRevision` from the
matched committed image; every other disposition exposes zero. Lesson 051
copies that revision and the same snapshot's image generation into its
authorization and audit candidates, rather than re-reading mutable storage
after admission.

Enrollment is explicit and cannot be triggered by ordinary observation.
Preview rejects a known identity, occupied bin, stale evidence, full table, or
active lockout without mutation. The registry is the exclusive writer of one
caller-owned 160-byte canonical candidate buffer and privately owns its owner
token and monotonically advancing candidate generation; returned candidates are copies
bound to the registry owner, base-image generation, operation ID, scratch
index, and checksum. `previewExport()` returns a bounded view of that same
complete canonical byte image and is retryable without consuming the
candidate. The view remains valid only until cancel, reset, shutdown, commit,
or the next successful preview on that registry. An external
persistence coordinator writes the candidate to the inactive one of exactly
two durable slots, synchronizes it, reads and validates it, then reports the
same operation, generation, slot, reconciled canonical bytes, synchronization
result, validation result, and durable status through
`acknowledgeExternalCommit()`. A negative, indeterminate, foreign, stale,
byte-mismatched, or failed acknowledgement preserves the prior table and
retryable candidate and latches reconciliation-required. No observation,
preview, cancel, reset, or new work advances while that latch is set.
`acknowledgeExternalCommit()` is the single atomic install boundary: after all
fields and reconciled bytes validate, it infallibly installs exactly the
candidate table, consumes the authorization/candidate, advances the live image
generation, and clears reconciliation-required in one mutation.
Owner/generation/checksum/operation binding rejects foreign, stale, changed,
and double-used values.

Duplicate successful acknowledgement is idempotent only when owner,
candidate/base generation, operation, slot, view pointer metadata
(length/slot/generation), checksum, and reconciled bytes exactly identify the
already installed image; it returns success without mutation. Every other
duplicate, stale view, changed metadata, negative durable status, or mismatch
returns the typed storage/reconciliation fault without mutation.

`reset()` clears observation, authorization, lockout, and pending candidate
state, but preserves the last committed bindings, image generation, and
source-domain replay identity. It rejects without mutation while reconciliation
is required. `shutdown()` always publishes the inactive lifecycle but
preserves a retryable reconciliation latch and scratch identity; initialization
must reconcile the supplied two slots before new work. It otherwise clears transient values and
marks the object uninitialized; it is idempotent. Reinitialize is required
after shutdown or structural storage fault. At most one public registry object
is `128 B`; no public value embeds a complete image. The two 160-byte canonical
slots, live binding table, and 160-byte candidate image are explicit
caller-owned storage and are included in whole-sketch SRAM rather than hidden
in the object.

### Lesson 049 deterministic matrix

- every identity length, canonical padding, byte position, checksum, revision,
  bin `0..binCount-1`, capacity, null storage, and duplicate table case;
- known, unknown, repeat, rapidly repeated, duplicate enrollment, occupied
  bin, full table, cancel, commit, double commit, foreign owner, stale
  generation, and mutated candidate;
- failure count below/at saturation, lockout start, one tick before/exactly at
  expiry, rollover, future time, half-range ambiguity, and regression;
- source failure, stale evidence, malformed identity, corrupt initial image,
  changed same-sequence payload, and no partial mutation;
- reset and restart from identical fixed images;
- fieldwise byte-identical replay without `memcmp` over padded structs; and
- non-copyable/non-movable traits and measured AVR object size.

### Lesson 049 example and publication

`Lesson049LocalIdentityRecords` uses a const synthetic trace and caller-owned
eight-slot RAM array, two 160-byte image slots, and one 160-byte candidate
buffer. `setup()` acquires no hardware, configures the registry, and starts
replay state. `loop()` observes one copied identity, decides whether to
preview/export/reconcile/commit a scripted enrollment, then stores a result cell. Predictions
include known bin, unknown, duplicate, lockout, cancel, corrupt-image restart,
and recovery. The sketch must never contain an RFID library, SPI transaction,
keypad scan, EEPROM/SD call, or pin number.

HTML is the terse API and boundary reference. The PDF teaches local identifiers,
fixed records, duplicate detection, lockout timing, transactional enrollment,
and corrupt-image diagnosis. Required pencil visuals are a token-to-bin
mapping, fixed-slot record anatomy, and lockout timeline. There is no formal
schematic at E0.

## Lesson 050 — bounded homing and positioning

### Responsibility and state machine

`BoundedHomingPolicy` converts copied home/stop evidence and explicit time into
semantic signed-step and stop intent. It has no Lesson 047 or coil dependency. A
successful synthetic home establishes logical zero for the current session
only. Reset, shutdown, restart, source fault, or interrupted travel returns
position to unknown.

```cpp
enum struct HomingPhase : uint8_t
{
    Uninitialized,
    PositionUnknown,
    SeekingHomeRelease,
    SeekingHome,
    Homed,
    Moving,
    Stopped,
    Fault
};

enum struct HomingFault : uint8_t
{
    None,
    InvalidConfiguration,
    HomeStuckActive,
    HomeNotFound,
    TravelExceeded,
    EvidenceFault,
    TimingFault,
    Interrupted
};

struct BoundedHomingConfig
{
    int32_t  minimumLogicalPosition;
    int32_t  maximumLogicalPosition;
    int32_t  homeLogicalPosition;
    int8_t   homeSearchDirection;
    uint16_t maximumReleaseSteps;
    uint16_t maximumSearchSteps;
    Duration maximumReleaseDuration;
    Duration maximumSearchDuration;
    Duration stepInterval;
    Duration maximumEvidenceAge;
    Duration maximumInputSkew;
};

struct HomingInput
{
    TimePoint            frameAt;
    uint32_t             frameSequence;
    CopiedBinaryEvidence home;
    CopiedBinaryEvidence stop;
};

struct HomingCommand
{
    bool    requestHome;
    bool    requestMove;
    int32_t targetLogicalPosition;
};

struct HomingSnapshot
{
    HomingPhase phase;
    HomingFault fault;
    int32_t     logicalPosition;
    bool        positionKnown;
    int8_t      stepDirection;
    bool        stepRequested;
    int8_t      requestedStepDirection;
    bool        stopIntent;
    uint16_t    homingSteps;
    uint32_t    homeEpoch;
    uint32_t    acceptedFrameSequence;
    Status      status;
};

struct HomingPreview
{
    uint32_t owner;
    uint32_t generation;
    HomingSnapshot snapshot;
    Status status;
};

struct HomingExcursionBounds
{
    int32_t minimum;
    int32_t maximum;
};
```

Files are `src/bounded_homing_policy.h` and
`src/bounded_homing_policy.cpp`. The surface is:

```cpp
struct BoundedHomingPolicy
{
    explicit BoundedHomingPolicy (const BoundedHomingConfig& config) noexcept;

    Status initialize () noexcept;
    void   reset      () noexcept;
    void   shutdown   () noexcept;

    Status preview (TimePoint now, const HomingInput& input,
                    const HomingCommand& command,
                    HomingPreview& candidate) const noexcept;
    bool   canCommit (const HomingPreview& candidate) const noexcept;
    Status commit (const HomingPreview& candidate) noexcept;

    HomingSnapshot snapshot () const noexcept;
    HomingExcursionBounds excursionBounds () const noexcept;

    BoundedHomingPolicy (const BoundedHomingPolicy&) = delete;
    BoundedHomingPolicy& operator= (const BoundedHomingPolicy&) = delete;
    BoundedHomingPolicy (BoundedHomingPolicy&&) = delete;
    BoundedHomingPolicy& operator= (BoundedHomingPolicy&&) = delete;
};
```

Initialization requires `homeLogicalPosition == 0` and validates
`minimumLogicalPosition < 0 < maximumLogicalPosition`, direction exactly `-1`
or `+1`, nonzero release/search step and duration limits and interval, and
freshness, skew, and durations below the modular half range. Home and ordinary
frame timestamps must each be within `maximumInputSkew` of `frameAt` by
rollover-safe modular distance, and home/stop timestamps must be within that
same bound of each other. The independently admissible stop exception
validates stop against `now` and its own source history without requiring
malformed home/frame fields. Initialization rejects a release or search bound
whose signed displacement cannot be represented from the home and either
logical bound. Validation and stepping use checked subtraction before
addition, never `position + direction` at an extreme. It begins
`PositionUnknown` with no step request and asserted stop intent.
It also computes immutable `excursionBounds()` from zero,
`-homeSearchDirection * maximumReleaseSteps`, and
`homeSearchDirection * maximumSearchSteps` with signed 64-bit intermediates;
range checking precedes narrowing to `int32_t`.

Home-active at request begins `SeekingHomeRelease` opposite the search
direction. Release must occur within `maximumReleaseSteps`; otherwise
`HomeStuckActive`. Home-inactive begins `SeekingHome` in the configured
direction. The first accepted inactive-to-active edge within
`maximumSearchSteps` establishes exactly logical zero. Starting
inactive and never observing an edge yields `HomeNotFound`; a level alone
cannot establish home after a missed provenance epoch.

Only qualified observations participate. The initial state, release, and
acquisition must retain one source kind/ID, configuration revision, and
nonzero qualification epoch; any change faults atomically. A repeated active
level is not an edge, an unqualified inactive value is not a release, and
unqualified or failed evidence is never treated as inactive.

Only a homed session accepts a target. Targets are inside inclusive logical
bounds. Lesson 050 has no Lesson 047 dependency or child transaction. One due
semantic signed-step request may be emitted per accepted commit; delayed calls skip
no requests and never burst. The pure policy advances its logical fixture
coordinate only with its own accepted request and separately emits
`stopIntent` on stop/fault/shutdown. The request is not a coil vector, applied
step, movement, or physical-position claim. Lesson 051 is solely responsible
for translating an accepted request into its separately owned Lesson 047
preview/commit transaction.

In the qualified acquisition frame Lesson 050 atomically clears step request,
increments the home epoch, and establishes exactly logical zero.
Later target arithmetic uses checked displacement from that coordinate. No
Lesson 047 coordinate is read, rewritten, or reinterpreted, and no published
Lesson 047 contract changes.

Frame structure is validated first, but an independently valid, qualified
stop source is extracted and committed even when unrelated home or command
fields are malformed. Thus stop has precedence over every request, evidence
fault, due step, home edge, final step, and target, and clears step request
while asserting stop intent in the same commit. If stop itself is malformed,
complete frame rejection wins.
After stop, source/evidence or child faults dominate bound expiry; bound
expiry dominates an ordinary edge or due step; a qualified home edge dominates
ordinary approach completion. Stop does not preserve position knowledge
during homing or movement: an interrupted operation enters `Stopped` with
unknown position. A stop while idle/homed retains known position only because
no motion was in progress. Release plus explicit new home request recovers
from `Stopped`. Fault requires shutdown and initialize. No timeout silently
retries.

Release and search are bounded independently by both issued transitions and
elapsed duration, with the exact step and time boundary accepted and the next
frame failing. A successful edge increments a nonzero `homeEpoch`; reset,
shutdown, stop during motion, policy/source fault, source revision change, or
interruption invalidates it. Each public object must measure at most `128 B`;
otherwise promotion stops for local state-layout remediation; Lesson 047 is
not added to or hidden behind Lesson 050.

### Lesson 050 deterministic matrix

- every invalid bound, home point, direction, step limit, interval,
  freshness, target, and arithmetic edge;
- exact signed release/search excursion bounds for both directions and
  maximum-width step limits without intermediate overflow;
- home initially inactive, initially active then released, exact release/search
  step limits, one step beyond, stuck active, never found, chatter, and edge
  provenance;
- one tick before/exactly at cadence, delayed updates, rollover, future time,
  half-range ambiguity, frame replay, same-sequence mutation, and input skew;
- moves to both bounds, zero-distance move, reversal after completion, target
  rejection, and exact signed-step/stop intent vectors;
- stop colliding with home, home edge, due step, final step, target, evidence
  failure, and reset;
- interruption/restart always makes position unknown; no stale logical
  position is reused;
- source-fault attribution and atomic rejection without policy mutation; and
- fieldwise golden replay, traits, and measured AVR size.

### Lesson 050 example and publication

`Lesson050BoundedHoming` replays copied home/stop frames and records phase,
known-position, signed-step request, and stop intent in fixed result cells.
Predictions cover release-first home, ordinary home, missing/stuck home,
bounded travel, interrupt, restart, and recovery. It has no pin, timer,
interrupt, delay, driver, or motor object.

HTML documents the state machine and explicit loss of position. The PDF uses
pencil drawings for the logical carousel coordinate map, home-edge sequence,
and interrupted-recovery timeline. An intent bit table is a formal truth table,
not an electrical schematic. No coil wiring is shown.

## Lesson 051 — inert parts carousel

### Responsibility and composition

`InertPartsCarousel` coordinates one admitted input frame across the identity
registry, confirmation policy, homing policy, project-owned Lesson 047
sequencing, semantic gate intent, local
presentation, and fixed audit record. It owns no endpoint, driver, bus, clock,
or durable medium.

```cpp
enum struct CarouselPhase : uint8_t
{
    Uninitialized,
    Idle,
    AwaitingConfirmation,
    Homing,
    Positioning,
    ReadyAtBin,
    GateIntent,
    Complete,
    Cancelled,
    Stopped,
    Fault
};

enum struct CarouselFault : uint8_t
{
    None = 0,
    InvalidFrame = 1,
    EvidenceFault = 2,
    TimingFault = 3,
    IdentityUnknown = 4,
    IdentityLocked = 5,
    IdentityStorageFault = 6,
    ConfirmationConflict = 7,
    AuthorizationExpired = 8,
    HomingFault = 9,
    PositionFault = 10,
    GateFault = 11,
    AuditFull = 12,
    AuditCorrupt = 13,
    AuditUnsupported = 14,
    AuditIndeterminate = 15,
    AuditStorageFault = 16,
    PresentationFault = 17
};

enum struct CarouselAuditStatus : uint8_t
{
    Success = 0,
    Cancelled = 1,
    Stopped = 2,
    AuthorizationExpired = 3,
    EvidenceFault = 4,
    TimingFault = 5,
    IdentityFault = 6,
    ConfirmationConflict = 7,
    HomingFault = 8,
    PositionFault = 9,
    GateFault = 10,
    AuditCorrupt = 11,
    AuditUnsupported = 12,
    AuditIndeterminate = 13,
    AuditStorageFault = 14,
    PresentationFault = 15,
    RecoveredInterrupted = 16
};

struct CarouselConfig
{
    uint8_t  binCount;
    int32_t  binPositions[maximumCarouselBins];
    uint32_t projectConfigurationId;
    uint8_t  confirmationDigits;
    uint16_t binConfirmationCodes[maximumCarouselBins];
    uint8_t  confirmKey;
    uint8_t  cancelKey;
    uint8_t  digitKeys[10];
    Duration confirmationWindow;
    Duration gateIntentDuration;
    Duration logicalStepInterval;
    Duration maximumStepCommandAge;
    Duration maximumFrameAge;
    Duration maximumInputSkew;
};

struct CopiedKeyBatch
{
    CarouselSource source;
    TimePoint      observedAt;
    uint32_t       sequence;
    uint8_t        digitCount;
    uint8_t        digits[4];
    bool           confirm;
    bool           cancel;
    Status         status;
};

struct CopiedPresentationStatus
{
    TimePoint observedAt;
    uint32_t  sequence;
    Status    status;
};

struct CarouselInputFrame
{
    TimePoint            observedAt;
    uint32_t             sequence;
    bool                 hasIdentity;
    IdentityEvidence     identity;
    bool                 hasKey;
    CopiedKeyBatch       key;
    CopiedBinaryEvidence home;
    CopiedBinaryEvidence stop;
    CopiedPresentationStatus presentation;
};

enum struct CarouselGateIntent : uint8_t
{
    Closed = 0,
    Open = 1
};

struct CarouselIntent
{
    uint8_t  coilBits;
    CarouselGateIntent gate;
    TimePoint gateExpiresAt;
    uint8_t  selectedBin;
    CarouselAuditStatus statusCode;
};

struct CarouselAuditRecord
{
    uint16_t magic;
    uint8_t schemaVersion;
    uint8_t encodedLength;
    uint32_t projectConfigurationId;
    uint32_t operationId;
    uint16_t authorizationEpoch;
    uint16_t recordSequence;
    TimePoint occurredAt;
    uint8_t recordKind;
    CarouselPhase phase;
    uint8_t binId;
    uint16_t identityDigest;
    uint16_t bindingRevision;
    uint32_t identityImageGeneration;
    uint32_t homeEpoch;
    CarouselAuditStatus auditStatus;
    uint16_t checksum;
};

static constexpr uint16_t carouselAuditMagic       = 0x4341;
static constexpr uint8_t  carouselAuditVersion     = 1;
static constexpr uint8_t  carouselAuditMaximum     = 8;
static constexpr uint8_t  carouselAuditRecordBytes = 40;

struct DurableAuditCandidate
{
    uint32_t owner;
    uint32_t generation;
    uint32_t operationId;
    uint8_t  slot;
    uint8_t  recordKind;
    uint16_t checksum;
};

struct AuditRecordView
{
    const uint8_t* bytes;
    uint8_t        length;
    uint8_t        slot;
    uint32_t       operationId;
};

struct AuditDurableCommitEvidence
{
    uint32_t        owner;
    uint32_t        generation;
    uint32_t        operationId;
    uint8_t         slot;
    AuditRecordView reconciledRecord;
    bool            synchronized;
    bool            rereadValidated;
    Status          durableStatus;
};

struct CarouselSnapshot
{
    CarouselPhase phase;
    CarouselFault fault;
    uint8_t       requestedBin;
    bool          authorizationCurrent;
    bool          positionKnown;
    int32_t       logicalPosition;
    CarouselIntent intent;
    bool           hasAuditRecord;
    CarouselAuditRecord auditRecord;
    bool           durableAdmissionPending;
    bool           terminalReconciliationPending;
    uint32_t       operationId;
    Status         status;
};
```

Absence is canonical, not ignored garbage. When `hasIdentity == false`, every
identity field is zero, source kind is `SyntheticIdentity`, and status is
`Ok`. When `hasKey == false`, every key-batch field is zero, source kind is
`SyntheticKey`, and status is `Ok`. Noncanonical absent payload rejects the
frame. A present key batch contains at most four already-qualified digits
`0..9`; bytes beyond `digitCount` are zero. It has
independent confirm/cancel flags, so a same-observation confirm+cancel
collision is representable; cancel dominates confirm and digits, records
`ConfirmationConflict`, and grants no authority. Presentation status is
copied, timestamped evidence: failure is a pure preflight candidate and never
changes authorization, motion, gate, or audit semantics.

The audit codec is also normative and independent of C++ padding. Multibyte
integers are unsigned little-endian. An erased slot is forty `0xff` bytes and
is not a record.

| Offset | Bytes | Encoding |
|---:|---:|---|
| 0 | 2 | magic `0x4341` |
| 2 | 1 | schema version `1` |
| 3 | 1 | encoded length `40` |
| 4 | 4 | nonzero project configuration ID |
| 8 | 4 | nonzero operation ID |
| 12 | 2 | nonzero authorization epoch |
| 14 | 2 | record sequence |
| 16 | 4 | occurrence time |
| 20 | 1 | kind: start `1`, terminal `2`, recovered terminal `3` |
| 21 | 1 | `CarouselPhase` numeric encoding in declaration order `0..10` |
| 22 | 1 | bin `0..7` |
| 23 | 1 | `CarouselAuditStatus` explicit value `0..16` |
| 24 | 2 | non-security identity digest |
| 26 | 2 | nonzero identity binding revision |
| 28 | 4 | identity image generation |
| 32 | 4 | home epoch; zero is allowed only in start/failure records |
| 36 | 2 | reserved zero |
| 38 | 2 | checksum |

The checksum is CRC-16/CCITT-FALSE with the identity-image parameters and
covers bytes `0..37`. Unknown kind/phase/status, noncanonical reserved bytes,
wrong length/configuration, or invalid field combination is corrupt.

Domain attribution never collapses into the generic `Status`. Audit/image
corruption maps to `InternalInvariant`, unsupported schema to `Unsupported`,
capacity to `CapacityExceeded`, not-ready lifecycle to `NotInitialized`, and
indeterminate or external storage failure to `HardwareFailure`.
Invalid/future/skewed evidence maps to `InvalidArgument`; invalid static
configuration maps to `InvalidConfiguration`. The snapshot additionally
retains the exact typed `IdentityDisposition`, `HomingFault`,
`CarouselFault`, and `CarouselAuditStatus`, so child, evidence, timing, gate,
corrupt, unsupported, indeterminate, storage, and presentation causes remain
distinguishable.

The likely surface is:

```cpp
struct InertPartsCarousel
{
    InertPartsCarousel (const CarouselConfig& config,
                        LocalIdentityRegistry& identityRegistry,
                        BoundedHomingPolicy& homingPolicy,
                        uint8_t* auditSlotBytes,
                        uint16_t auditSlotByteExtent,
                        uint8_t auditSlotStride,
                        uint8_t auditCapacity,
                        uint8_t* auditCandidateBytes,
                        uint8_t auditCandidateCapacity) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    Status update     (TimePoint now, const CarouselInputFrame& frame) noexcept;
    Result<DurableAuditCandidate> previewAuditWrite () const noexcept;
    Result<AuditRecordView> previewAuditExport (
        const DurableAuditCandidate& candidate) const noexcept;
    Status acknowledgeAuditWrite (
        const DurableAuditCandidate& candidate,
        const AuditDurableCommitEvidence& evidence) noexcept;
    CarouselSnapshot snapshot () const noexcept;

    InertPartsCarousel (const InertPartsCarousel&) = delete;
    InertPartsCarousel& operator= (const InertPartsCarousel&) = delete;
    InertPartsCarousel (InertPartsCarousel&&) = delete;
    InertPartsCarousel& operator= (InertPartsCarousel&&) = delete;
};
```

The `CarouselConfig`, borrowed Lesson 049/050 collaborators, and caller-owned arrays
must outlive the project and must not move or mutate while it exists. The
coordinator borrows the immutable config rather than copying its bin/key
tables, which keeps its own state within `128 B`. While initialized, the
project is the borrowed children's exclusive coordinator and no other writer may call
them. It does not embed their state. Initialization
validates the entire bin map, nonzero project configuration ID, time bounds,
distinct key encoding,
and an even audit capacity `2, 4, 6, or 8` before child initialization. Audit
extent must equal `capacity * 40`, stride must equal `40`, and candidate
capacity must equal `40`; overlaps and ambiguous extra bytes are rejected.
The project config deliberately does not duplicate immutable identity or
homing configs: those are copied and validated by their explicitly borrowed
children, whose lifetime is fixed by the constructor references. Confirmation is exactly
`confirmationDigits` configured decimal digits (`1..4`) followed by `confirmKey`;
digits map through `digitKeys[10]`, leading zeros are significant, cancel is
distinct, repeated source sequences cannot add a digit, and overflow or any
other key is a conflict. Every bin has an explicit
`binConfirmationCodes[bin]` whose decimal representation must fit exactly the
configured digit width (`code < 10^confirmationDigits`), including leading
zeros in entry comparison. The
completed code is compared with that configured per-bin code, never with raw
identity bytes. Failure rolls children back in reverse order. Shutdown first
publishes closed gate and zero coil intent and invalidates authorization and
home epochs, then shuts down child policies;
it is idempotent and leaves the project uninitialized.

The canonical bin map has distinct positions, its derived minimum and maximum
satisfy `minimum <= 0 <= maximum` and `minimum < maximum`, and it includes the
negative or positive side needed to make that strict range (both sides when
zero is not itself a bin). Lesson 050's home coordinate is exactly zero.
Consequently the owned Lesson 047 child initializes at its native logical zero
and uses the same coordinate system; no offset, rebasing, or arbitrary home
translation exists.

Lesson 051 owns its one Lesson 047 `BoundedStepperSequence` and its private
preview. Its child bounds cover the union of every bin position, zero, and the
borrowed Lesson 050 policy's immutable validated `excursionBounds()`. Union
derivation uses signed 64-bit comparison, rejects an endpoint outside
`int32_t`, and requires the final minimum `< 0 <` final maximum. Both pre-home
directions are therefore legal around native zero without wrap or a hidden offset. It
constructs the child with those validated bounds, minimum and maximum interval both equal to
`logicalStepInterval`, `maximumStepCommandAge`, and project-fixed
`holdAtRest = false`. No caller can substitute a differently configured
sequencer. Aggregate coordinator-plus-child size and stack are measured; a
failure of the existing `128 B` public-object ceiling stops implementation for
design review rather than changing Lesson 047.

There is no E0 servo child. `CarouselGateIntent::Open` and `Closed` plus the
explicit expiry are semantic project results only. A future E2 adapter may
translate them through a separately configured `BoundedServo`, but no servo
position, pulse, timer, endpoint, or physical gate state enters E0.

For each ordinary non-stop motion frame, Lesson 051 first derives a Lesson 050
`HomingPreview`. A requested signed step becomes an exact one-step Lesson 047
command. It then derives the Lesson 047 preview, verifies both
owner/generation identities and both `canCommit()` results, and commits each
exactly once with no fallible work remaining. Failure of either preview mutates
neither. The published coil bits come only from the committed Lesson 047
snapshot; Lesson 050 never sees them. This project-local joint preflight
preserves the existing Lesson 047 API and keeps its logical coil/position
meaning separate from home evidence.

Stop is the deliberate safety exception to joint preview. After independently
validating stop, and before unrelated frame validation, the coordinator
inspects its exclusively owned Lesson 047 snapshot. “Live” means phase
`Moving` or `Holding`, or nonzero coil intent. If live, it calls the child's
direct `stop(now)` first; admitted coordinator time guarantees `now` is not
before the child's last accepted update. It then commits the Lesson 050 stop
candidate. If idle, it skips child mutation—project-fixed
`holdAtRest = false` guarantees zero coil intent—and commits only Lesson 050
stop. It never manufactures an idle cancel command or advances the child's
generation unnecessarily. An unexpected child-stop failure invokes the
child's all-off `reset()` fallback, latches `CarouselPhase::Fault` with
`PositionFault`, retains the original stop attribution, and then commits the
Lesson 050 stop; no ordinary work proceeds.

Qualified home acquisition is a second explicit synchronization boundary, not
an ordinary step commit. Stop, malformed/faulted evidence, and child fault
retain their earlier precedence and prevent acquisition. Otherwise Lesson 051
derives but does not yet commit the Lesson 050 acquisition preview. If the
owned Lesson 047 child is live, it calls direct `stop(now)` first; it then
calls `reset()` unconditionally to establish native logical zero, inactive
phase, and zero coil intent. It verifies those three snapshot fields before
committing the Lesson 050 preview that publishes the home epoch and logical
zero. No Lesson 047 preview exists across the reset generation change.

If direct stop fails, the coordinator still resets all-off but latches
`PositionFault`, leaves position unknown, and does not commit home success. If
the post-reset snapshot invariant fails, it likewise latches fault/all-off and
does not commit home. Thus no accepted home epoch can coexist with a
nonzero/pre-home Lesson 047 coordinate. An independent stop colliding with the
qualified edge follows the stop path only and never the home-synchronization
path.

The coordinator privately owns candidate generations but borrows the explicit
children above. Returned
audit candidates are copies bound to owner, generation, operation ID, target
slot, record kind, and checksum; the exact canonical bytes live in the
caller-owned 40-byte candidate buffer and are exposed only by a bounded
retryable view. The project and retained-storage
children are non-copyable and non-movable. Every public object must measure at
most `128 B`; identity images, audit byte slots, replay cells, and scratch
candidates remain explicit caller-owned buffers. This is a natural ownership
split and does not relax the whole-sketch SRAM gate.

### Transaction and precedence

Each operation follows these bounded stages, with at most one stage and one
logical motion transition committed per call:

1. validate complete frame structure, source domains, time, sequence,
   freshness, skew, canonical absences, and same-frame identity while
   independently validating the stop field;
2. commit a valid stop even if unrelated fields are malformed, publishing
   closed/off intent before audit or presentation work;
3. preflight identity, confirmation, homing, gate, presentation, and audit
   candidates without mutation, including atomically reserving two adjacent
   audit slots for the authorization-start and its eventual terminal record;
4. allocate a project-local operation ID and authorization epoch only after a
   known identity and exact confirmation match;
5. export the authorization-start audit candidate and remain motion-inhibited
   until an external coordinator writes, synchronizes, rereads, validates, and
   acknowledges that exact operation ID;
6. commit admission, then permit homing and positioning by at most one logical
   transition per accepted frame;
7. create gate intent only after the invariant below is true; and
8. publish terminal safe intent first, then export a terminal record with the
   same operation ID and retain pending reconciliation until acknowledged.

The safety invariant is:

```text
gate-open intent
  => known identity
  && matching explicit confirmation within its window
  && durable authorization-start record for this operation
  && successful home in this session
  && known logical position equals the confirmed bin position
  && stop is inactive
  && every admitted source is healthy and current
```

Identity presentation never grants movement by itself. Confirmation must
match the requested bin; conflicting digits cancel authorization and record a
conflict. A new identity while awaiting confirmation replaces nothing: it is
rejected until cancel or expiry. Authorization expires at the exact configured
boundary before a colliding confirmation.

Collision precedence is exact: safe intent from an independently valid stop;
already-latched fault; malformed stop/frame structure; source or child fault; authorization
expiry/cancel/conflict; audit admission/full/indeterminate state;
homing/travel bound; ordinary identity/confirmation/home/motion/gate progress;
terminal audit reconciliation; presentation. A valid stop therefore commits
closed/off intent even if identity, key, home, or command fields are malformed.
A valid stop always clears coil intent, publishes closed gate intent, and cancels new authority, but it
never erases, replaces, or downgrades an already latched `Fault` phase or
fault attribution; the snapshot remains faulted. A malformed stop prevents the
independent-stop exception, rejects the whole frame, and cannot advance
ordinary identity, confirmation, homing, motion, gate, audit, or presentation
work. No malformed unrelated field can delay a valid stop. Audit capacity is preflighted before
authorization-start admission, so `AuditFull` cannot leave a half-applied
home, move, or gate intent. A new operation requires at least two free slots;
one remaining slot is `AuditFull`, not partial capacity. Presentation failure
cannot authorize or prolong an actuator intent and retains the underlying project phase and fault
separately.

Gate intent is a bounded semantic request, never a servo-write claim. It lasts
at most `gateIntentDuration`; then closed intent is restored before `Complete`.
No automatic retry follows any fault. Fault recovery is shutdown plus
initialize with a fresh frame. Completed and cancelled sessions require a
fresh identity sequence.

The audit is an externally persisted append-only array of at most eight fixed
40-byte canonical encoded records; the C++ semantic struct is encoded
fieldwise and its padded `sizeof` is never persisted. Each record encodes
fixed magic/schema/length, project
configuration ID, operation ID, authorization epoch, strictly advancing
record sequence, occurrence time, start-or-terminal kind, phase, bin,
non-security identity digest, home epoch, status, canonical reserved bytes,
and checksum. External storage owns writes and synchronization.
`previewAuditWrite()` and `previewAuditExport()` are retryable.
`acknowledgeAuditWrite()` accepts only matching owner/generation/operation/
slot plus byte-identical reconciled record, successful synchronization,
reread validation, and successful durable status. On success it atomically
installs the candidate record in the admitted prefix, consumes the scratch
candidate, and advances project state; there is no later fallible commit.
Failed or indeterminate evidence retains the candidate, latches
reconciliation-required, and prohibits reset, new input work, and candidate
replacement. Shutdown still clears intents but preserves reconciliation;
initialization must reconcile supplied bytes before work.

A repeated acknowledgement is idempotent only if owner, candidate generation,
operation, slot, view pointer metadata (length/slot/operation), checksum, and
all reconciled bytes equal the already installed record. It succeeds without
mutation. Any stale/foreign/changed duplicate or failed status returns the
typed audit fault without mutation.

Recovery scans at most eight slots once, accepts only a canonical valid prefix,
and rejects unsupported schema, wrong project identity, duplicate or
half-range-ambiguous sequence, terminal without start, mismatched operation
fields, a second terminal, noncanonical tail, or corruption before a later
apparently valid record. An erased canonical tail is unused capacity. Start
admission atomically reserves both the current start slot and immediately
following terminal slot before the start candidate is exposed, so a durable
start can never occupy the final slot. A start without terminal is interrupted:
recovery requires its reserved following slot to be erased, starts inhibited,
gate closed, motion off, position unknown, and prepares a terminal-recovery
record for that exact slot and operation ID. A durable start in the final slot,
or a non-erased mismatching reserved terminal slot, is corrupt rather than
recoverable. An indeterminate write is resolved from reread bytes before retry,
so an event never receives a new operation ID. E0 proves these supplied
byte-image semantics only; actual media atomicity, retention, wear, and
power-loss survival remain E1.

A legal pair has adjacent slots and identical project configuration ID,
operation ID, authorization epoch, bin, identity digest, binding revision, and
identity image generation. Only record sequence, occurrence time, kind, phase,
audit status, home epoch, reserved bytes, and checksum may differ; reserved
bytes still must be zero. Start has kind `1`, an admission phase, zero home
epoch, and a success status. Ordinary terminal has kind `2`; recovered
terminal has kind `3`, phase `Fault`, status `RecoveredInterrupted`, and zero
home epoch. Record sequences advance by modular 16-bit deltas `1..0x7fff`;
zero is duplicate, `0x8000` ambiguous, and larger deltas regress. Recovery
never invents a new operation or authorization identity.

The Lesson 051 E0 fixture constructs the Lesson 049 registry with its live
bindings, two 160-byte identity slots, and 160-byte identity candidate buffer,
plus the independent Lesson 050 policy. The carousel owns its fixed-config
Lesson 047 child and receives references to Lessons 049/050,
`auditCapacity * 40` caller-owned audit bytes, and one 40-byte audit candidate
buffer. No storage workspace is implicit in the coordinator.

### Lesson 051 deterministic matrix

- known/unknown/duplicate/rapid identity traces and every local bin;
- correct digit sequence, wrong digit, duplicate key, cancel, confirm,
  simultaneous confirm/cancel, and authorization expiry boundaries;
- home release/search success, missing/stuck home, position bounds, movement
  interruption, restart, and explicit re-home;
- release/search excursions in both configured directions at exact bounds and
  one beyond, union with extreme bin positions, signed derivation overflow,
  and proof that every admitted pre-home step stays inside the owned Lesson
  047 bounds;
- exact coil and semantic open/closed gate intent vectors with actuator power absent;
- proof that no gate-open intent occurs before every invariant term is true;
- stop colliding with identity, confirmation, home edge, due step, arrival,
  gate-open request, gate timeout, presentation fault, and audit-full;
- stop with the Lesson 047 child live versus idle, proving direct stop occurs
  only when live, idle stop does not change child generation, direct-stop
  failure resets all-off and latches fault, and unrelated malformed fields
  cannot delay either path;
- qualified home with the Lesson 047 child live and idle at nonzero pre-home
  coordinates, proving live stop then unconditional reset, zero/off/inactive
  verification, no stale preview across reset, home commit only afterward,
  stop/home collision dominance, and stop/reset failure yielding fault with no
  home epoch;
- stale/faulted source, excessive skew, future time, rollover, half-range,
  frame regression, repeat, and changed same-sequence payload;
- child fault plus audit/presentation fault attribution without partial
  mutation;
- valid audit prefix, corrupt checksum, torn record, full capacity, restart,
  shutdown from every phase, and fresh recovery;
- audit capacities `1..9`, proving only even `2,4,6,8` initialize; odd
  capacity, one free slot, start in the final slot, and a pending reserved
  terminal all reject new admission without mutation;
- exhaustive bin-position permutations proving bin meaning is configuration,
  not array index or pin identity; and
- bin maps whose extrema are negative/zero/positive, zero-only, one-sided,
  duplicate, reversed, and integer extremes, proving only distinct maps with
  `minimum <= 0 <= maximum` and `minimum < maximum` initialize;
- two independent instances replaying byte-identical fieldwise snapshots,
  intent mirrors, and audit images.

## Maximum composition and architecture stress gate

The pre- and post-implementation stress passes must exercise the complete
eight-binding/eight-bin configuration, full audit buffer edge, longest homing
path, farthest move, shortest accepted confirmation window, maximum allowed
frame age/skew, and simultaneous stop/fault/audit/presentation collision.

The design is natural only if:

- Lesson 049 remains a local record policy rather than a security abstraction;
- Lesson 050 remains independent and emits semantic signed-step/stop intent;
- Lesson 051 alone coordinates that intent with its owned Lesson 047 child
  without changing the child's physical meaning;
- Lesson 051 does not reach through child snapshots to mutate their state;
- caller-owned fixed storage has one named writer and explicit lifetime;
- source, authorization, position, audit, and presentation failures remain
  independently attributable;
- no diagnostic consumes a hidden pin, timer, bus, interrupt, or current
  budget at E0; and
- later E1/E2 adapters can consume intents without changing E0 semantics.

If implementation requires changing `BoundedStepperSequence`,
`FixedStorage`, status semantics, public lifecycle, or several existing
consumers, promotion stops for a bounded design decision and migration review.
Project-local duplication is preferable to a premature generic transaction or
motion framework.

### Pre-implementation stress-pass closure

This plan closes the bounded local blockers in the three companion stress
passes without changing a published dependency:

- Lesson 049 now fixes canonical versioned images, exact two-slot recovery,
  retryable preview/export/external-acknowledge/commit ordering, private
  owner/generation identity, and caller-owned memory.
- Lesson 050 now fixes qualified edge provenance, independent release/search
  step and time bounds, checked arithmetic, exact collision precedence, and a
  standalone preview/commit semantic step-intent seam.
- Lesson 051 now fixes explicit child configurations and key encoding,
  independently admissible stop, durable authorization-start admission before
  motion eligibility, project-local operation IDs, terminal reconciliation,
  fixed audit capacity/encoding, bounded recovery, and the only Lesson 047
  preview/commit composition.

Implementation remains subject to the stress passes' requested independent
review and measured proofs. If the published Lesson 047 seam cannot make the
stated joint commit infallible after preflight, or if the external record
protocol requires changing `Storage`, this closure is invalid and promotion
stops for architectural remediation.

## Resource, size, and packaging budgets

### E0 resources

All three lessons own exactly zero pins, ADC channels, timers, interrupts,
buses, resource-registry entries, endpoints, supplies, storage transports, or
moving hardware.

| Lesson | AVR object target | AVR hard ceiling | Sketch flash | Sketch static SRAM |
|---:|---:|---:|---:|---:|
| 049 | 112 B | 128 B | 16,384 B | 1,024 B |
| 050 | 112 B | 128 B | 16,384 B | 1,024 B |
| 051 coordinator and each child | 112 B each | 128 B each | 28,672 B | 2,048 B |

Caller-owned fixture, identity, and audit arrays are counted in complete-sketch
static SRAM. The Lesson 051 acceptance record must also reserve at least
1,024 bytes after measured globals plus a conservative documented stack/ISR
estimate. If that margin cannot be met, reduce capacities or snapshot
retention; do not hide storage on the heap or relax the gate.

Stack/ISR evidence is mandatory, not absorbed into the static-SRAM column.
Lessons 049 and 050 each require a documented conservative stack plus ISR
allowance of at least `256 B` and at least `768 B` remaining after globals and
that allowance. Lesson 051 requires at least `512 B` stack plus ISR allowance
and the stated `1,024 B` remaining afterward. Measurements include the largest
simultaneously live preview/image candidate; tests fail if compiler layout
pushes any public object above `128 B` or any aggregate margin below its gate.

Every header compiles alone. Sources are registered in the strict host,
sanitizer, archive, Arduino, package-consumer, and umbrella-header inventories.
Each component remains non-copyable and non-movable where it retains
caller-owned storage or transactional identity.

### Future E1/E2 reservation, not authorization

A future powered design must create an exact pin/timer/current budget before
selecting pins. Candidate consumers include SPI RFID, matrix keypad,
Hall/reed home input, LCD, LEDs, independent stop, four stepper driver inputs,
and one timer-backed servo. Mega pin abundance does not resolve SPI SS/reset,
Timer ownership, keypad fan-out, current, ground, or motor-noise conflicts.

E1 must identify the RFID reader silicon/module, logic and supply levels,
antenna boundary, SPI mode/rate, reset behavior, keypad topology/pulls, exact
Hall/reed circuit, LCD transport, nonvolatile medium, and every indicator
resistor. It must separately prove resource acquisition and electrical
safe-state evidence.

E2 must identify the motor winding/voltage, driver and clamp topology, servo
model and stall current, separate current-limited actuator supply, common
reference, mechanical restraint, lightweight gate, travel envelope, thermal
limit, independent stop and power removal, and de-energized stop/fault/shutdown.
RFID identity never authorizes hazardous, valuable, access-controlled, or
unattended dispensing.

## Narrative examples and presentation matrix

All sketches use `setup()` as acquire fixture storage, configure policies,
start replay; `loop()` as observe one copied frame, decide, record intent.
They contain no `delay()`, hardware adapter, pin constant, or Serial-only
acceptance path. Serial may print supplementary trace text.

| Phase | E0 retained evidence | Future circuit-native evidence |
|---|---|---|
| Ready | initialized result cell | distinct ready indicator after acquisition |
| Identity | disposition, digest, requested bin | LCD/bin indicators after qualified RFID input |
| Confirmation | key result and expiry | keypad/local display |
| Homing | phase, edge, step count, coil mirror | home LED/test point and inert intent LEDs |
| Positioning | known flag, logical position, coil mirror | position indicators with actuator power removed |
| Gate intent | semantic open/closed request and expiry | inert gate-intent mirror before any servo adapter |
| Stop/fault | independent cause and cleared intents | independent stop path and all-actuators-off evidence |
| Shutdown | zero intents and uninitialized lifecycle | separately measured high-impedance/de-energized state |

Future LEDs or LCDs cannot alone prove physical home or actuator safe state.
Named test points, expected levels, observation time, and interpretation belong
in the E1/E2 acceptance card.

## HTML/PDF division and visual classification

Each HTML page is a concise supported-interface reference: responsibility,
types, lifecycle, deterministic behavior, example/source/test links, resource
budget, E0 limitations, and next lesson.

Each PDF is complementary teaching material with prediction, staged replay,
diagnosis, exercises, and a blank acceptance record:

- 049: identifier anatomy, local mapping, enrollment transaction, duplicates,
  lockout, and corrupt fixed images;
- 050: unknown position, release-first homing, bounded search, logical travel,
  interruption, and re-home;
- 051: identity-confirm-home-position-gate invariant, transactional frame
  admission, audit reconstruction, collision diagnosis, and staged future
  E1/E2 experiments.

Every visual is classified in the TeX source and inventory. Token/bin maps,
state diagrams, timelines, carousel layouts, record anatomy, and learner
orientation art use pencil-drawing presentation. Tables generated as text
remain text. A future electrically authoritative conventional circuit may be
marked as a formal schematic only after exact E1/E2 specimens are qualified;
there is no formal schematic in the E0 PDFs. Filenames, grayscale, or a
“schematic-like” layout do not establish compliance.

## Promotion and persistence decisions

Promotion requires:

1. reviewed pre- and post-implementation stress passes for all three lessons;
2. strict/custom/sanitizer tests and complete deterministic matrices;
3. standalone headers, archive/package/umbrella registration, and no legacy
   dependency;
4. Mega 2560 compile and measured flash/static SRAM/object/stack margins;
5. complementary HTML and PDF, visual classification/pencil gates, links,
   navigation, downloads, and newest-lesson verifier advancement;
6. reconciled curriculum, cadence, projects, components, roadmap, work queue,
   size baseline, release claims, and live site; and
7. explicit retention of every E1/E2 specimen, powered, durability, and bench
   gate.

### Exact canonical-document reconciliation

Integration must edit these canonical claims in the same checkpoint; the plan
does not silently supersede them:

- `docs/projects/component_project_cadence.md`: Lessons 049--051 say E0
  replays copied identifiers, logical homing and gate intent, and supplied
  audit images; move RFID/keypad/home endpoints and nonvolatile media to E1,
  and motor/servo movement to E2.
- `docs/CURRICULUM.md`: identify 049 as local identifier records rather than
  authentication, 050 as volatile logical homing, and 051 as an inert
  coordinator whose motion admission waits for an externally acknowledged
  authorization-start record.
- `docs/PROJECTS.md` and `docs/ROADMAP.md`: replace unqualified “dispense,”
  “homes,” “opens,” and “survives interrupted write” claims with the E0/E1/E2
  split and make current physical position unknown after restart.
- `docs/COMPONENTS.md`: add only the three specific component surfaces and
  their zero-resource E0 ownership; do not introduce generic credential,
  transaction, or motion frameworks.
- `docs/WORK_QUEUE.md`: record E0 implementation gates plus separate E1 input,
  durable-media, and E2 powered-motion/bench work; preserve exact specimens
  and pencil-visual audits as open gates.
- `docs/SAFETY_MODEL.md`: state that local identifier match is not
  authentication, issued steps do not prove position, durable start admission
  is required before motion eligibility, and restart inhibits with closed/off
  intent and unknown position.
- `docs/PDF_POLICY.md`: no policy change is required; each 049--051 inventory
  must classify every non-schematic visual as pencil presentation and must not
  claim a formal schematic at E0.

The durable decisions carried forward are:

- local UID-shaped values are identifiers only;
- E0 uses supplied fixed byte images and explicit simulated external durable
  acknowledgement; actual nonvolatile media behavior remains E1;
- physical position is unknown until bounded homing succeeds in the current
  powered session;
- stop is independent and has same-frame precedence;
- no gate-open intent precedes identity, confirmation, home, exact position,
  freshness, and health;
- E0 owns no resource and makes no powered-observation claim;
- non-schematic PDF visuals are pencil drawings; and
- later work may qualify adapters without silently widening E0 support.

Lessons 052--054 remain independent infrared protocol work. This block must
not create a reusable credential system or unknown-protocol replay path for
them.
