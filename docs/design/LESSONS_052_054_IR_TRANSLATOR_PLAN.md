# Lessons 052--054 infrared translator plan

Status: implementation-depth E0 plan; exact powered fixtures remain open.

This arc extends Lesson 025 without changing `PulseCapture`, `PulseFrame`,
`InfraredDecoder`, `InfraredFrame`, or `InfraredRecordEncoder`. E0 consists of
copied receive evidence, a pure locally authored emission policy, and an inert
translator. It owns no pin, interrupt, timer, carrier output, emitter,
receiver, keypad, display, LED, bus, or optical power path.

Unknown, malformed, and repeat captures remain inspectable evidence. They can
never become transmit authority. The only transmit authority is an immutable,
firmware-authored catalog of symbolic local commands. Lesson 054 is a genuine
translator: a fixed allowlist maps one valid, attributable receive symbol to a
different local transmit symbol. It is not a learning remote or a raw protocol
bridge.

## Evidence levels and cadence correction

| Level | Authorized work |
|---|---|
| E0 | Pure fixed-storage policies, caller-owned copied pulse words, synthetic or checked-in evidence, immutable local symbols, compile-only Mega replay, and inert semantic intent |
| E1 | Separately qualified exact receiver and emitter fixtures, carrier endpoint, current-limited optical output, keypad, indicators, LCD, and adjacent-breadboard acceptance |

The old cadence phrase “eye-safe current-limited operation” is unsupported.
Current, duty, and duration limits reduce exposure but do not establish eye
safety. ADK has no qualified wavelength, radiant-intensity, beam-geometry, or
exposure evidence for such a claim. E1 therefore requires bounded adjacent
operation, no deliberate viewing or reflective alignment, and default-off
electrical behavior, while making **no eye-safety claim**.

The listed Elegoo IR-emission family is not an exact electrical identity. No
emitter is selected here; TSAL6200 remains only a possible external reference.
Timer1/D11 and Timer2/D9 suggestions are both premature. E1 selects the exact
emitter, driver, resistor, timer, compare channel, pin, supply, and observation
path only after specimen and aggregate-resource qualification.

## Dependency order

| Lesson | Boundary | Depends on | Explicit exclusions |
|---:|---|---|---|
| 052 | Stable copied receive evidence with full provenance, categorical strength, and exact disposition | Existing Lesson 025 pulse values and decoder | New capture owner, caller-authored decode result, numeric confidence, transmission |
| 053 | Transactional O(1) semantic envelope for immutable local symbols | `Status`, explicit microsecond time, fixed catalog | Pulse/frame/raw-address input, arbitrary waveform, pin, timer, emitter |
| 054 | Fixed valid-receive-to-different-local-symbol translation and attributable round trip | Owned Lessons 052--053 policies | Learned mapping, identity mapping, repeat inference, unknown replay, generic bridge |

Plan, stress pass, implementation, tests, example, measured resources, HTML,
PDF, indexes, and promotion review proceed in that order. An architecture
stress pass runs before each public shape is fixed and again before promotion.
E1 is a separately admitted later boundary.

## Shared identity and time rules

Every external observation retains source kind, nonzero source ID, nonzero
configuration revision, nonzero session epoch, source sequence, status, and
the time software observed it. Modular sequence and microsecond-time deltas
use the repository half-range rule. A future, ambiguous, or regressing value
rejects without mutation. An exact same-sequence duplicate is idempotent; a
changed payload at the same sequence is a source fault.

Lesson 025 does not publish capture-start or capture-complete timestamps.
Lesson 052 therefore records `observedAt`, the supplied time at which software
copied the published frame. It must never label that value a capture interval,
occurrence time, or optical arrival time. E1 round-trip timing requires actual
endpoint start/completion and qualified receive timing evidence rather than
inventing those times from E0 observation.

All arc time uses `MicrosecondTimePoint` and `MicrosecondDuration`. No API mixes
the millisecond `TimePoint` domain with carrier-envelope or echo timing.

```cpp
enum struct IrSourceKind : uint8_t
{
    SyntheticFixture = 0,
    QualifiedReceiver = 1,
    LocalCatalog = 2,
    QualifiedEmitter = 3
};

struct IrSourceIdentity
{
    IrSourceKind kind;
    uint8_t      sourceId;
    uint16_t     configurationRevision;
    uint32_t     sessionEpoch;
};
```

## Lesson 052 — copied capture evidence

### Responsibility

`CapturedIrEvidence` synchronously invokes the promoted
`InfraredDecoder` on one borrowed, published `PulseFrame`, checks that the
decoder sequence matches the capture sequence, and copies every admitted pulse
into caller-owned `uint32_t` storage before the caller calls
`PulseCapture::acknowledge(frame.sequence)`. Lesson 052 never acknowledges,
consumes, or rearms `PulseCapture`; the caller retains that ownership and
chooses whether to retry, discard, or acknowledge after the copy result. The
component does not accept a caller-supplied `InfraredFrame`, address, command,
disposition, or strength.

One compact word represents one pulse. Bit 31 is `Mark` when set and `Space`
when clear; bits 0--30 contain a nonzero duration in microseconds. The
implementation uses masks and shifts, not bitfields, type punning, padding, or
a persistence encoding. The maximum remains `PulseCapture::capacity` (100
words, 400 bytes).

```cpp
static constexpr uint8_t  capturedIrPulseCapacity = PulseCapture::capacity;
static constexpr uint32_t capturedIrMarkMask       = UINT32_C (0x80000000);
static constexpr uint32_t capturedIrDurationMask   = UINT32_C (0x7fffffff);

struct IrPulseStorage
{
    uint32_t* data;
    uint8_t   capacity;
};

enum struct IrCaptureDisposition : uint8_t
{
    None = 0,
    KnownValid = 1,
    KnownRepeat = 2,
    UnknownProtocol = 3,
    TimingInvalid = 4,
    IntegrityInvalid = 5,
    Truncated = 6,
    DecoderOverflow = 7,
    CaptureOverflow = 8,
    CaptureTimingFault = 9,
    SourceFault = 10
};

enum struct EvidenceStrength : uint8_t
{
    None = 0,
    ShapeRecognized = 1,
    IntegrityVerified = 2
};

struct CapturedIrProvenance
{
    IrSourceIdentity      source;
    MicrosecondTimePoint  observedAt;
    uint32_t              captureSequence;
    CaptureState          captureState;
    InfraredProtocol      protocol;
    FrameValidity         decoderValidity;
    Status                sourceStatus;
};

struct CapturedIrSnapshot
{
    IrCaptureDisposition disposition;
    EvidenceStrength     strength;
    CapturedIrProvenance provenance;
    uint32_t             address;
    uint32_t             command;
    uint32_t             evidenceGeneration;
    uint8_t              pulseCount;
    Status               status;
};

struct CapturedIrView
{
    const uint32_t* words;
    uint8_t         size;
    const void*     owner;
    uint32_t        evidenceGeneration;
};

struct CapturedIrEvidence
{
    CapturedIrEvidence
        (InfraredDecoder& decoder,
         IrPulseStorage pulseStorage,
         uint8_t maximumPulseCount) noexcept;

    ~CapturedIrEvidence () noexcept;

    CapturedIrEvidence&
        operator= (const CapturedIrEvidence&) = delete;
    CapturedIrEvidence (const CapturedIrEvidence&) = delete;
    CapturedIrEvidence&
        operator= (CapturedIrEvidence&&) = delete;
    CapturedIrEvidence (CapturedIrEvidence&&) = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    void   reset      () noexcept;

    Status admit (const PulseFrame&      frame,
                  const IrSourceIdentity& source,
                  Status                 sourceStatus,
                  MicrosecondTimePoint   observedAt) noexcept;

    CapturedIrSnapshot snapshot () const noexcept;
    Result<CapturedIrView> view  () const noexcept;
    Result<uint8_t> requiredWords
        (const CapturedIrView& view) const noexcept;
    Result<uint8_t> exportWords (const CapturedIrView& view,
                                 IrPulseStorage destination) const noexcept;
};
```

`IrPulseStorage` is lesson-specific mutable storage, not a general span
abstraction. `data` must be non-null when `capacity` is nonzero; constructor
storage must have capacity `1..100`, meet `maximumPulseCount`, and outlive the
initialized component. An export destination may have zero capacity only to
exercise required-size failure, and no operation dereferences a null pointer.
The component writes its constructor storage exclusively. A view is valid
until the next successful non-idempotent admission, reset, shutdown, or
destruction. Foreign and stale owner/generation views reject. Export preflights
the destination and leaves it unchanged when short. `requiredWords()` exposes
the exact required extent before export; a successful export returns the exact
written count.

`KnownValid` maps only to `IntegrityVerified`. Recognized NEC repeat, timing,
integrity, truncation, or decoder-overflow evidence maps to
`ShapeRecognized`; repeat never inherits a previous address or command.
Unknown protocol, capture overflow, and capture timing fault map to `None`.
Every non-`KnownValid` record has canonical zero address and command. Capture
and decoder dispositions that cannot coexist reject atomically.

The exact overflow pairs are frozen: `CaptureState::Complete` plus
`FrameValidity::Overflow` publishes `DecoderOverflow`;
`CaptureState::Overflow` plus canonical decoder overflow publishes
`CaptureOverflow`; and no other capture/decoder overflow combination is
admissible. A non-OK `sourceStatus` publishes `SourceFault` with canonical
decoded fields while retaining the status and full source attribution.

Same-sequence identity compares source, configuration, session, capture state,
decoder result, strength, decoded fields, pulse count, and every pulse word;
only a later `observedAt` is ignored. An exact duplicate retains the original
observation time and generation. Decode or copy failure leaves the previous
record and caller storage unchanged, allowing the caller to retry or discard
the still-published Lesson 025 frame explicitly.
After the first admitted record, changing source kind/ID, configuration
revision, or session epoch requires `reset()`; it cannot silently begin a new
sequence domain.

### Lesson 052 proof and publication

Tests cover storage capacities 0, 1, 99, 100, and 101; configured maxima below,
at, and above storage; every pulse level and duration boundary; 0, 1, 99, and
100 pulses; every capture/decoder disposition and prohibited cross-product;
decoder/capture sequence mismatch; full provenance; duplicate/change/wrap/
regression/half-range cases; failure at first/middle/last copy; generation and
view lifetime; short/exact/large export; reset, shutdown, destruction, and
byte-stable replay. Tests also prove that source/configuration/session changes
reject atomically until reset. Tests compare fields and words, never struct
padding.

The canonical “IR detective” example makes the narrative phases explicit.
`setup()` acquires inert dependencies, configures caller storage and source
identity, and starts the policy. `loop()` observes one synthetic published
frame, decides its disposition and evidence strength, and actuates only named
result cells for known, repeat, unknown, malformed, and fault evidence. It
initializes no `PulseCapture` or physical receiver.

HTML documents the stable-copy API and has direct links to the header,
implementation, tests, canonical sketch download, PDF, Lesson 025, and the
Lesson 053 “Use with” boundary. The PDF teaches ownership, provenance, strength
versus disposition, and why observation time is not capture time; it links
back to the HTML and canonical sketch download. All E0 visuals carry the PDF
policy classification marker, are pencil drawings, and receive rendered-page
visual review; there is no E0 formal schematic.

## Lesson 053 — known local emission policy

### Immutable catalog and structural no-capture rule

`KnownIrEmissionPolicy` accepts only `LocalIrCodeId`. No public constructor,
configuration, request, candidate, helper, or conversion accepts
`PulseFrame`, `InfraredFrame`, `CapturedIrView`, protocol/address/command
fields, duration arrays, or raw bytes.

```cpp
enum struct LocalIrCodeId : uint8_t
{
    StationPing = 0,
    StationReady = 1,
    StationCancel = 2,
    StationAcknowledge = 3
};

enum struct IrEnvelopeIntent : uint8_t
{
    Inactive = 0,
    CarrierOn = 1,
    CarrierOff = 2
};

enum struct IrEmissionDisposition : uint8_t
{
    Idle = 0,
    Prepared = 1,
    Active = 2,
    Complete = 3,
    Cancelled = 4,
    Fault = 5,
    Shutdown = 6
};

struct KnownIrCatalogIdentity
{
    uint16_t revision;
    uint32_t digest;
};

struct KnownIrEmissionConfig
{
    uint16_t                configurationRevision;
    uint32_t                instanceEpoch;
    MicrosecondDuration     maximumEnvelopeDuration;
};

struct KnownIrEmissionPreview
{
    const void*             owner;
    uint16_t                configurationRevision;
    uint32_t                instanceEpoch;
    uint32_t                policyGeneration;
    uint32_t                candidateGeneration;
    uint32_t                transactionId;
    LocalIrCodeId           codeId;
    KnownIrCatalogIdentity  catalog;
    uint32_t                candidateDigest;
    MicrosecondTimePoint    startAt;
    MicrosecondTimePoint    completeAt;
    IrEnvelopeIntent        firstIntent;
};

enum struct IrEmissionTerminalCause : uint8_t
{
    None = 0,
    Completed = 1,
    CancelledBeforeCommit = 2,
    CancelledActive = 3,
    ShutdownBeforeCommit = 4,
    ShutdownActive = 5,
    Faulted = 6
};

struct KnownIrEmissionSnapshot
{
    uint16_t                configurationRevision;
    uint32_t                instanceEpoch;
    uint32_t                policyGeneration;
    uint32_t                candidateGeneration;
    LocalIrCodeId           codeId;
    KnownIrCatalogIdentity  catalog;
    uint32_t                transactionId;
    MicrosecondTimePoint    startAt;
    MicrosecondTimePoint    completeAt;
    uint8_t                 repeatIndex;
    IrEnvelopeIntent        intent;
    IrEmissionDisposition   disposition;
    IrEmissionTerminalCause terminalCause;
    uint32_t                terminalTransactionId;
    MicrosecondTimePoint    terminalAt;
    Status                  status;
};

struct KnownIrEmissionPolicy
{
    explicit KnownIrEmissionPolicy
        (const KnownIrEmissionConfig& config) noexcept;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    void   reset      () noexcept;

    Result<KnownIrEmissionPreview>
        prepare (LocalIrCodeId codeId,
                 uint32_t transactionId,
                 MicrosecondTimePoint now) noexcept;
    bool canCommit (const KnownIrEmissionPreview& preview,
                    MicrosecondTimePoint now) const noexcept;
    Status commit (const KnownIrEmissionPreview& preview,
                   MicrosecondTimePoint now) noexcept;
    Status cancel (const KnownIrEmissionPreview& preview,
                   MicrosecondTimePoint now) noexcept;
    Status cancel (uint32_t transactionId,
                   MicrosecondTimePoint now) noexcept;
    Status update (MicrosecondTimePoint now) noexcept;

    KnownIrEmissionSnapshot snapshot () const noexcept;
};
```

The four-entry catalog is immutable firmware data. Each entry has a symbolic
ID, harmless local meaning, encoding revision, bounded bit fields and repeat
count. The catalog revision and digest cover every semantic entry field and
are copied into every preview and snapshot. There is no public catalog-mutating
operation. The digest is 32-bit FNV-1a over entries in catalog order. Each
entry contributes the one-byte code ID, the four payload bytes least
significant first, the one-byte encoding revision, and the one-byte repeat
count, in that order. Direct indexing by validated enum yields the entry in
O(1).

The policy never returns or retains a large waveform. From catalog metadata,
transaction start, and supplied `now`, it directly computes the single current
carrier-envelope intent in O(1). It never iterates carrier cycles, scans every
prior cell, queues missed transitions, or emits catch-up bursts.

`prepare()` validates the request, mutates only the private candidate slot, and
reserves exactly one candidate. The returned preview is a small capability
bound to the exact internally retained owner, configuration revision, instance
epoch, policy generation, candidate generation, transaction, catalog digest,
candidate digest, and start time. A second prepare while reserved returns
`Busy`. Preview values have copied-capability semantics, not authenticity:
exact fieldwise copies of the sole retained issued preview are valid while that
candidate remains current. Mutating any field, presenting a foreign/stale
generation, or presenting any copy after the first successful commit or
precommit cancellation rejects.

`canCommit()` and `commit()` require `now == preview.startAt`; delayed commits
reject instead of backdating or shifting. After full validation, commit is one
infallible mutation from the retained candidate to the active transaction.
Foreign, mutated, stale, consumed, cancelled, changed-digest, or post-reset
previews reject. The contract does not claim it can distinguish an issued
preview from an independently reconstructed field-identical value while the
sole matching retained candidate is live. Before commit,
`cancel(preview, now)` must identify that exact retained candidate and records
`CancelledBeforeCommit`; it cannot cancel by code ID or an approximate match.
After commit, `cancel(transactionId, now)` validates the exact active identity,
publishes `Inactive`, and records `CancelledActive`. Repeated cancellation of
the same terminal identity is idempotent. Shutdown distinguishes prepared from
active work, records the matching terminal identity/time, and reinitialization
advances the policy generation before any prior preview can become valid.

A late `update()` derives the phase that is true at `now`. If `now` is at or
after `completeAt`, it publishes `Inactive` and `Complete`; it never reenacts
missed active intervals. An apparent regressing or exact-half-range time
rejects atomically and leaves policy state and intent unchanged; the E1
endpoint's independent expiry/default-off path, not a malformed policy update,
owns electrical fail-off. This pure policy intent is not electrical carrier
generation.

### Lesson 053 proof and publication

Tests derive the catalog digest independently from every semantic field of
every entry and flip each field in turn to prove the digest changes. Golden
vectors inspect code-dependent data boundaries for every catalog entry:
first/last data bit, every zero-to-one and one-to-zero transition, first/last
mark and space, repeat boundary, and exact terminal boundary. The remaining
matrix covers invalid enum representations, all duration/repeat/overflow
bounds, exact envelope vectors, direct lookup, prepare reservation, candidate
identity and generation, exact copied preview acceptance, every single-field
mutation, foreign/stale/post-consumption copies, exact commit timestamp,
delayed commit, exact preview cancellation before commit, cancellation during
every active phase, same-time cancel dominance,
one tick before/at/after every boundary, missed service, repeated time,
rollover, atomic regressing/half-range rejection, reset, distinct prepared/
active shutdown attribution, restart, and byte-stable replay. Compile-time and
compile-fail checks prove no capture/raw overload exists.

The canonical “secret handshake” example makes the narrative phases explicit.
`setup()` acquires the inert policy, validates the immutable catalog identity,
and starts idle. `loop()` observes a fixed symbolic request and supplied time,
decides through prepare/can-commit/cancel rules, and actuates only
`CarrierOn`/`CarrierOff`/`Inactive` result cells. It commits at the same
supplied microsecond and samples representative phase boundaries; there is no
optical endpoint.

HTML documents the transaction and catalog identity and links directly to the
header, implementation, tests, canonical sketch download, PDF, Lesson 052, and
the Lesson 054 “Use with” composition. The PDF teaches closed authority,
direct phase derivation, cancellation, missed service, and intent versus
physical output, with reciprocal HTML/sketch links. Every E0 visual carries
its classification marker, uses pencil presentation, and passes rendered-page
review. E0 has no wiring or formal schematic.

## Lesson 054 — inert command translator

### Genuine fixed translation

The translator owns its `InfraredDecoder`, `CapturedIrEvidence`, and
`KnownIrEmissionPolicy`, but borrows one caller-owned `IrPulseStorage` of
exactly 100 words for the receive child. The mutable storage must outlive the
translator, remains exclusively writable by the initialized translator, and
must not overlap another live component's storage. Construction is inert and
the coordinator is non-copyable/non-movable. Initialization validates the
storage and complete immutable mapping, then initializes children in dependency
order; failure rolls back in reverse order. Shutdown first publishes
cancellation and inactive intent, shuts down children, and invalidates every
view/generation before releasing its exclusive use of the borrowed storage.

The fixed mapping is deliberately not identity:

| Valid received local symbol | Different transmitted local symbol |
|---|---|
| `StationPing` | `StationReady` |
| `StationReady` | `StationAcknowledge` |
| `StationCancel` | no transmission; cancel dominates |
| `StationAcknowledge` | `StationPing` |

The receive-side known local symbol is derived only when Lesson 052 reports
`KnownValid`, `IntegrityVerified`, the configured qualified source, and a
catalog revision/address/command combination fixed by the translator's
immutable receive allowlist. Repeat, unknown, malformed, truncated, overflow,
source fault, self-echo, stale, and unlisted valid frames never select output.
The mapping digest covers receive tuple, receive source constraint, and output
symbol.

The E0 test/example seam is frozen as public read-only receive-fixture data:

```cpp
struct SyntheticIrReceiveFixture
{
    InfraredProtocol protocol;
    uint32_t         address;
    uint32_t         command;
    LocalIrCodeId    receivedCode;
    LocalIrCodeId    translatedCode;
    bool             transmissionAllowed;
};

static constexpr uint16_t syntheticIrMappingRevision = 1;
static constexpr uint8_t  syntheticIrFixtureCount    = 4;
static constexpr uint32_t syntheticIrMappingDigest   =
    UINT32_C (0xa8f94d6b);

extern const IrSourceIdentity syntheticIrReceiveSource;
extern const SyntheticIrReceiveFixture
    syntheticIrReceiveFixtures[syntheticIrFixtureCount];
```

`syntheticIrReceiveSource` is exactly
`{IrSourceKind::SyntheticFixture, 52, 1, 1}`. The entries, in enum/index
order, are:

| Protocol | Address | Command | Received symbol | Translated symbol | Allowed |
|---|---:|---:|---|---|---|
| NEC | `0x00000052` | `0x00000010` | `StationPing` | `StationReady` | yes |
| NEC | `0x00000052` | `0x00000020` | `StationReady` | `StationAcknowledge` | yes |
| NEC | `0x00000052` | `0x00000030` | `StationCancel` | canonical `StationPing` placeholder | no; cancellation only |
| NEC | `0x00000052` | `0x00000040` | `StationAcknowledge` | `StationPing` | yes |

The digest is 32-bit FNV-1a (offset `0x811c9dc5`, prime `0x01000193`) over
canonical bytes: little-endian mapping revision; source kind, ID,
little-endian configuration revision and session epoch; then, for each table
row, protocol byte, little-endian 32-bit address and command, received-symbol
byte, and translated-symbol byte (`0xff` when transmission is not allowed).
Thus the third row contributes `0xff`, not its canonical in-memory placeholder.
Tests recompute and compare `0xa8f94d6b`; examples use the exported const table
without mutable casting.

These tuples are synthetic, harmless, and receive-only. They are sufficient to
construct Lesson 025 golden `PulseFrame` fixtures and validate Lesson 054's
allowlist. No public operation accepts one of their protocol/address/command
fields for transmission, and the Lesson 053 catalog is not populated from this
table. Changing any tuple, source constraint, mapping, encoding order, revision,
or digest is an explicit plan/configuration revision, never runtime learning.

```cpp
enum struct IrTranslationDisposition : uint8_t
{
    Idle = 0,
    Translated = 1,
    Cancelled = 2,
    RepeatRejected = 3,
    UnknownObserved = 4,
    MalformedObserved = 5,
    SelfEchoSuppressed = 6,
    UnlistedValidObserved = 7,
    AttributionMismatch = 8,
    ReceiveTimeout = 9,
    Fault = 10
};

struct IrEmitterEvidence
{
    IrSourceIdentity       source;
    uint32_t               transactionId;
    MicrosecondTimePoint   startedAt;
    MicrosecondTimePoint   completedAt;
    Status                 status;
};

struct IrReceiveEnvelope
{
    IrSourceIdentity      source;
    Status                sourceStatus;
    MicrosecondTimePoint  observedAt;
    PulseFrame            frame;
};

struct IrTranslatorConfig
{
    uint16_t               configurationRevision;
    uint32_t               instanceEpoch;
    uint32_t               mappingDigest;
    MicrosecondDuration    maximumEnvelopeDuration;
    IrSourceIdentity       qualifiedReceiveSource;
    IrSourceIdentity       localEmitterSource;
    MicrosecondDuration    echoGuard;
    MicrosecondDuration    responseWindow;
};

struct IrTranslatorPreview
{
    const void*            owner;
    uint32_t               instanceEpoch;
    uint16_t               configurationRevision;
    uint32_t               mappingDigest;
    KnownIrCatalogIdentity emissionCatalog;
    uint32_t               parentGeneration;
    uint32_t               operationId;
    uint32_t               inputDigest;
    uint32_t               evidenceGeneration;
    CapturedIrProvenance   receiveProvenance;
    LocalIrCodeId          receivedCode;
    LocalIrCodeId          transmitCode;
    KnownIrEmissionPreview emission;
};

struct IrRoundTripResult
{
    bool                       complete;
    uint32_t                   operationId;
    LocalIrCodeId              transmittedCode;
    KnownIrCatalogIdentity     emissionCatalog;
    IrSourceIdentity           transmitSource;
    MicrosecondTimePoint       actualStartedAt;
    MicrosecondTimePoint       actualCompletedAt;
    MicrosecondDuration        elapsed;
    IrCaptureDisposition       receiveDisposition;
    EvidenceStrength           receiveStrength;
    CapturedIrProvenance       receiveProvenance;
    uint32_t                   receiveEvidenceGeneration;
    IrTranslationDisposition   correlationDisposition;
    Status                     status;
};

struct IrTranslatorSnapshot
{
    uint32_t                  operationId;
    LocalIrCodeId             receivedCode;
    LocalIrCodeId             transmitCode;
    CapturedIrProvenance      receiveProvenance;
    IrSourceIdentity          transmitSource;
    uint32_t                  transmitTransactionId;
    IrTranslationDisposition  disposition;
    uint32_t                  suppressedEchoCount;
    IrEnvelopeIntent          transmitIntent;
    IrRoundTripResult         roundTrip;
    Status                    status;
};

struct IrTranslatorUpdateInput
{
    MicrosecondTimePoint  now;
    bool                  cancelPresent;
    uint32_t              cancelOperationId;
    bool                  commitPresent;
    IrTranslatorPreview   commitPreview;
    bool                  receivePresent;
    IrReceiveEnvelope     receive;
    bool                  actualEmissionPresent;
    IrEmitterEvidence     actualEmission;
};

struct InertIrTranslator
{
    InertIrTranslator
        (const IrTranslatorConfig& config,
         IrPulseStorage receiveStorage) noexcept;
    ~InertIrTranslator () noexcept;

    InertIrTranslator& operator= (const InertIrTranslator&) = delete;
    InertIrTranslator (const InertIrTranslator&) = delete;
    InertIrTranslator& operator= (InertIrTranslator&&) = delete;
    InertIrTranslator (InertIrTranslator&&) = delete;

    Status initialize () noexcept;
    void   shutdown   () noexcept;
    void   reset      () noexcept;

    Result<IrTranslatorPreview>
        prepareTranslation (uint32_t operationId,
                            MicrosecondTimePoint now) noexcept;
    bool canCommit (const IrTranslatorPreview& preview,
                    MicrosecondTimePoint now) const noexcept;
    Status update (const IrTranslatorUpdateInput& input) noexcept;

    IrTranslatorSnapshot snapshot () const noexcept;
    CapturedIrSnapshot receiveSnapshot () const noexcept;
    KnownIrEmissionSnapshot emissionSnapshot () const noexcept;
};
```

`IrTranslatorConfig` field order is normative: configuration revision,
instance epoch, mapping digest, `maximumEnvelopeDuration`, qualified receive
source, local emitter source, echo guard, then response window. Aggregate
initializers, golden fixtures, examples, and tests use that order.

`maximumEnvelopeDuration` is mandatory, nonzero, within the unambiguous
microsecond half range, and copied directly into the owned Lesson 053 child's
`KnownIrEmissionConfig`. The translator cannot silently choose a wider child
duration, derive it from a capture, or leave the owned child partially
configured. Initialization rejects when the immutable catalog's longest
envelope exceeds this bound.

`update()` is the sole coordinator ingress. One copied update envelope
represents one microsecond scheduling boundary and can carry cancellation,
optional prepared commit, optional receive evidence, and optional
actual-emission evidence simultaneously. The coordinator first validates every presence flag,
canonical absent field, identity, timestamp, and operation relationship
without mutation. It then applies this frozen precedence independent of call
or source order:

1. reject a structurally invalid whole envelope atomically;
2. preserve any already-latched dependency or safety attribution;
3. apply readable matching cancellation and invalidate emission candidates;
4. admit and classify optional receive evidence independently;
5. correlate optional actual-emission evidence;
6. commit an optional still-current parent/child candidate, then perform
   ordinary child and parent progress; and
7. publish presentation intent last.

Cancellation cannot be masked by malformed receive evidence or an emission
transition at the same timestamp. Receive and emission dispositions remain
independently visible when cancellation dominates. A caller cannot obtain a
different result by splitting or reordering those co-inputs because no
separate receive, cancel, commit, or emission ingress exists.

When receive evidence is present, its source identity must equal the configured
qualified receive source before its frame, identity, status, and observation
time are forwarded to the owned Lesson 052 child. That child admission is the
exact immutable receive-generation seam: it may change only copied receive
evidence and cannot start transmission.

`prepareTranslation()` validates the already-admitted generation and fixed
mapping, reserves one parent candidate, and asks Lesson 053 to reserve its
exact child candidate. The returned preview binds full copied provenance,
the coordinator `instanceEpoch`, translator `configurationRevision`,
`mappingDigest`, immutable emission catalog revision/digest, parent
generation, operation ID, input digest, and the Lesson 053 child preview.
The parent preview is a copied capability, not an authenticity token. Exact
fieldwise copies are valid while the sole internally retained parent candidate
and its child candidate remain live. The coordinator does not claim it can
distinguish an issued preview from an independently reconstructed
field-identical value during that interval. A mutated, foreign, stale,
post-reset, post-shutdown, or post-consumption copy rejects.
Parent
`canCommit()` verifies all parent and child fields before mutation.
The caller returns the preview only through `commitPreview` in the atomic
update envelope. A simultaneous valid cancellation invalidates it before
commit. Otherwise `KnownIrEmissionPolicy::commit()` is infallible after that
preflight at the same microsecond, so the child and parent publish one
attributable operation.
If implementation cannot preserve that property, promotion stops; sequential
best effort may not be described as atomic.

The update-envelope cancellation field is inspected before receive
classification or ordinary progress. It invalidates outstanding candidates
and forces semantic `Inactive`; a simultaneous malformed receive remains
independently attributable. Already-latched dependency/safety faults remain
visible. Diagnostics never participate in eligibility, cancellation, or
correlation.

### Actual emission evidence and self-echo

E0 uses explicitly marked synthetic `IrEmitterEvidence`. E1 supplies it only
from the exact qualified endpoint. A policy commit time is not actual optical
start, and logical completion is not actual endpoint completion.

Self-echo suppression uses the actual transmit interval:

```text
[actual startedAt, actual completedAt + echoGuard)
```

A receive observation at or after `startedAt` and strictly before the guard
end is copied and classified `SelfEchoSuppressed`. Exactly at the guard end it
is eligible for normal evaluation. Because Lesson 052 has only `observedAt`,
E0 classification is explicitly observation-time correlation, not a claim
about capture onset. E1 may claim optical interval correlation only after its
receiver adapter publishes qualified timing evidence in a separately reviewed
value.

The response interval begins exactly where echo suppression ends:

```text
responseStart = actualCompletedAt + echoGuard
response interval = [responseStart, responseStart + responseWindow)
elapsed = receiveObservedAt - actualCompletedAt
```

Thus no timestamp is both self-echo and response-eligible. An observation one
tick before `responseStart` is self-echo suppressed; one exactly at
`responseStart` is response-eligible. An observation one tick before the
exclusive response end is eligible; one exactly at the end or later is a
timeout. `elapsed` deliberately includes the post-completion echo guard and
retains physical latency from actual completion rather than restamping the
response-window start.

Both additions and every subtraction use the unsigned modular half-range
rules. If `actualCompletedAt + echoGuard` or
`responseStart + responseWindow` wraps into an ambiguous ordering, exceeds the
unambiguous half range, or otherwise cannot form the two ordered half-open
intervals, the evidence/configuration rejects or faults without publishing a
round-trip result. The implementation never saturates, silently shortens, or
turns an overflowed interval into an immediately due response.

Suppression is never silent: it retains the latest evidence identity and
increments a saturating count. It cannot complete a round trip. A valid
response must be from the qualified receive source, outside the suppression
window, inside the response window, valid and integrity-verified, mapped to the
expected response symbol, and attributable to the exact operation, mapping,
catalog, transmitter source, and actual emission evidence. Payload equality
alone is insufficient.

`IrRoundTripResult` is the one bounded atomic publication. Until completion it
is canonical and `complete == false`. On completion one parent mutation
publishes actual start and completion, modular elapsed duration, transmitted
symbol, catalog revision/digest, transmitter identity, exact receive
disposition and strength, full receive provenance, evidence generation, and
status together. No getter assembles these fields from mutable child
snapshots, and no partial result can combine identities from different
operations.

### Bounded scheduling and missed service

There is one active operation and no queue. A second operation returns `Busy`.
Each atomic update performs at most one cancellation, one receive admission,
one actual-emission correlation, one parent transition, one direct child phase
calculation, and fixed presentation work. There is no retry or catch-up loop.

An apparent regressing or exact-half-range operation time rejects atomically:
parent snapshot, candidate, child policies, and semantic intent remain
unchanged. Such an invalid update cannot be repurposed as cancellation or
electrical fault handling. The qualified E1 endpoint independently expires to
off if valid service is missed.

If service occurs after logical or actual completion, the intent is
immediately `Inactive`; missed carrier intervals are not replayed. If the
qualified endpoint misses its maximum service deadline, loses its timer, or
cannot prove current time, its independent expiry/default-off mechanism must
disable carrier without waiting for the coordinator. Firmware policy,
diagnostic LEDs, LCD, Serial, and a camera are not protective interlocks.

### Lesson 054 proof and publication

Tests recompute the frozen fixture digest from every canonical byte, compare
all four exported tuples and source fields, reject every single-field
mutation/revision mismatch, and prove the cancel row never reaches Lesson 053.
They also cover `maximumEnvelopeDuration` at zero, one tick below, exactly at,
and one tick above the longest catalog envelope; translator receive storage
with null data, capacities 0, 99, 100, and 101; two translators with disjoint
buffers; compile-time/documentation checks that identify overlapping live
storage as a caller contract violation rather than promising an undetectable
global runtime registry; storage unchanged on failed initialization; and
generation/view invalidation on reset, shutdown, and destruction. The
remaining matrix covers every fixed mapping and digest field; proof that
outputs differ from inputs; valid, repeat, unknown, malformed, self-echo,
stale, source-fault, and unlisted-valid receive evidence; attempts to use
captured values as catalog authority; exact copied parent-preview acceptance;
every single-field mutation; foreign/stale/reset/shutdown/post-consumption
copies; atomic failure at each preflight boundary; every presence-mask
combination in
`IrTranslatorUpdateInput`; noncanonical absent fields; and all permutations of
same-timestamp cancel, prepared commit, valid/malformed/unknown receive,
actual-emission transition, child fault, and presentation failure, proving one
order-independent result and cancellation precedence; exact
start, actual-completion, echo-guard, response-window, and timeout boundaries;
one tick before, exactly at, and one tick after `responseStart` and the
exclusive response end; echo/window addition overflow and half-range
ambiguity; exact elapsed-from-actual-completion values; missed service;
rollover; atomic regressing/half-range rejection; reset and
shutdown from every phase; suppressed-count
saturation through direct compile-time proof that the saturating increment
helper maps `UINT32_MAX` to `UINT32_MAX`, plus an ordinary reachable increment
test through the public coordinator path (not a `2^32`-event runtime replay);
source/configuration/session/operation/mapping/catalog mismatch;
atomic round-trip publication with every actual-time/catalog/disposition/
strength/provenance field; canonical incomplete result; and fieldwise
byte-stable replay.

The coordinator exposes no independent local-selection API: transmission can
begin only from one fixed-allowlisted, valid, integrity-verified receive
generation. The canonical staged story combines “IR detective” evidence with
the “secret handshake”: first result cells identify the received source and
disposition, then a different-symbol translation candidate appears, then
commit produces inert envelope and attribution result cells. Repeat, unknown,
malformed, unlisted, and self-echo stages visibly end without a transmit
candidate.

The E0 example declares one `uint32_t receiveWords[100]`, constructs
`IrPulseStorage {receiveWords, 100}`, and passes it with the canonically ordered
configuration to the translator. The translator owns both child policies while
the sketch owns the buffer. It replays copied receive, actual-emission, and
cancellation evidence through explicit acquire/configure/start and
observe/decide/actuate flow. Actuation is inert semantic intent. HTML
documents translation, attribution, atomicity, and E1 limits and links directly
to both child references, header, implementation, tests, canonical sketch
download, PDF, and a “Use with” section that distinguishes E0 copied fixtures
from future E1 endpoints. The PDF teaches the fixed different-symbol mapping,
provenance, actual-versus-logical timing, self-echo, diagnosis, exercises, and
blank E1 card, with reciprocal HTML/sketch links. Every E0 visual carries the
PDF-policy classification marker, uses pencil presentation, and passes
rendered-page review.

## Resource gates

The following are promotion gates, not measurements:

| Boundary | Flash target / hard | Static SRAM target / hard | Stack target / hard |
|---|---:|---:|---:|
| Lesson 052 maximum standalone composition | 16 / 20 KiB | 3,072 / 3,584 B | 640 / 768 B |
| Lesson 053 maximum standalone composition | 16 / 20 KiB | 1,024 / 1,536 B | 512 / 640 B |
| Lesson 054 complete maximum composition | 28 / 32 KiB | 3,584 / 4,096 B | 800 / 1,024 B |

The implemented E0 boundary produced the following measured evidence:

| Boundary | Flash | Static SRAM | Object/storage | Conservative stack |
|---|---:|---:|---:|---:|
| Lesson 052 standalone | 5,530 B | 1,096 B | 48 B object | 197 B |
| Lesson 053 standalone | 4,854 B | 276 B | 74 B object | 155 B |
| Lesson 054 standalone | 16,162 B | 1,343 B | 407 B object + 400 B caller storage | measured in maximum path |
| Lessons 052--054 maximum composition | 21,864 B | 3,531 B | all live objects and caller storage included | 888 B |

All flash, static-SRAM, child-object, coordinator-object, and fixed-buffer hard
gates pass. The maximum 888 B conservative stack path misses the 800 B target
by 88 B but passes the 1,024 B hard ceiling by 136 B. This target miss was
evaluated against the complete linked composition rather than waived: 3,531 B
static SRAM plus 888 B stack leaves 3,773 B of the Mega's 8,192 B SRAM. The
hard safety margin is therefore retained, and 888 B becomes the measured
promotion evidence; growth beyond the 1,024 B hard ceiling still blocks
promotion.

Lesson 025 currently measures 12,400 B flash and 2,537 B static SRAM;
`PulseCapture` is 2,069 B. Lesson 052 must wrap rather than duplicate it.
The reusable Lesson 052 and Lesson 053 child objects each target at most 96 B
and have a 128 B hard ceiling, excluding caller storage. The named Lesson 052
`uint32_t[100]` storage is exactly 400 B and is approved under the repository's
512 B fixed-buffer limit. An embedded version measured 805 B and failed the
Lesson 054 640 B object hard gate; it is rejected. The bounded remediation
keeps the 400 B buffer caller-owned while the translator owns both child
policies. Lesson 054 remains explicitly approved as an oversized project
coordinator: its object, excluding caller storage, targets 512 B and has a
640 B hard ceiling. The aggregate measurement still includes the coordinator
and its one required 400 B buffer. These are
implementation measurement gates, not estimates that waive promotion review;
crossing one requires a size-focused repair or durable budget decision.

Measurements report, separately:

1. each reusable object, preview, snapshot, view, and caller storage;
2. each lesson's standalone linked example and conservative stack path;
3. Lesson 054 with both owned children and the full 100-word receive storage;
4. the maximum E0 composition including Lesson 025 capture/ISR state,
   translator, presentation intents, fixture export, and diagnostics; and
5. stack plus interrupt reserve and remaining Mega SRAM after both static and
   stack maxima.

At the Lesson 054 hard limits, 4,096 B static plus 1,024 B stack leaves
3,072 B of the Mega's 8,192 B SRAM; at targets, 3,584 + 800 leaves 3,808 B.
Reports must state this post-stack margin and may not describe 4 KiB static
headroom as total free memory. Stack evidence includes the largest synchronous
decode/copy/prepare/update path plus ISR reserve, not merely one leaf function.

Host tests are partitioned by coherent contract (052 storage/provenance, 053
catalog/transaction, 054 translation/atomicity, and 054 timing/collision) for
review clarity, failure isolation, and practical sanitizer runs—not because
the repository defines a host executable-size gate. Every required case runs
across the partitions. Partitioning does not replace the linked Mega
maximum-composition measurement.

Replay comparisons are fieldwise for public values and wordwise for compact
pulse storage. Tests never compare, hash, serialize, or persist raw C++ struct
representations or their padding.

E0 hardware ownership is exactly zero. Every production operation is O(1)
except Lesson 052's bounded O(n), `n <= 100`, decode/copy admission. There is
no heap, recursion, blocking wait, callback, input-sized queue, or catch-up
loop.

## E1 exact-fixture admission

No powered adapter, wiring table, schematic, or physical claim may enter until:

1. exact receiver, emitter, driver, resistor, keypad, indicators, LCD, board,
   and supplies have photographs, markings, pinouts, primary sources, and
   acceptance identities;
2. receiver supply/output/polarity/carrier behavior and qualified timing
   evidence are established without changing Lesson 025;
3. emitter wavelength and electrical ratings, calculated and measured current,
   driver behavior, resistor tolerance, carrier frequency/duty, burst bound,
   supply droop, and thermal behavior are recorded, without an eye-safety
   claim;
4. the final Mega map covers receiver interrupt, emitter timer/compare pin,
   keypad, LCD bus/address, separate indicators, claim entries, current,
   scheduler/ISR latency, flash, SRAM, stack, and post-stack margin;
5. every partial initialization failure rolls back; construction, startup,
   timer conflict, missed service, cancellation, fault, reset, shutdown,
   destruction, and logic-power loss leave the emitter electrically off;
6. the endpoint has an independent bounded expiry/default-off path and
   independent physical power removal; software cleanup is not the sole stop.
   The exact authoritative schematic must include a hardware inactive bias
   (pulldown or an electrically equivalent fail-off network), and bench
   acceptance must leave the emitter supply present while removing MCU/logic
   power and verify at the exact optical/electrical test points that the
   emitter remains inactive;
7. only the documented adjacent harmless fixture and immutable local catalog
   are used—never a consumer remote, access control, unknown appliance,
   cloning, brute force, long range, high power, launcher, or pyrotechnics;
8. separate TX-intent, RX-activity, and fault indicators, exact emitter and
   receiver electrical test points, and LCD categorical presentation are
   qualified; resource acquisition and emitter-off evidence are separate; and
9. predict/observe/interpret records cover nominal translation, source
   attribution, actual start/completion, cancellation, self-echo boundaries,
   saturation, missing carrier, timeout, fault, reset, shutdown, and power
   removal. E1 must cancel at representative `CarrierOn`/mark and
   `CarrierOff`/space boundaries and measure the worst-case elapsed time from
   accepted cancellation to carrier-off at the exact electrical and optical
   test points; that measured maximum must pass the documented endpoint
   latency bound.

A phone camera is optional qualitative troubleshooting only. It is not
qualification evidence, a carrier measurement, proof of command content,
proof that an emitter is off, or proof of eye safety. The exact accepted
receiver/photodiode and electrical test point provide the optical evidence.

## Packaging and promotion

Every header compiles independently under strict C++11. Additive sources appear
once in native and Arduino inventories, the umbrella header, archive
consumers, host tests, size baselines, examples, downloads, HTML, TeX/PDF, and
indexes. No supported file depends on `legacy/`.

Before Lesson 054 promotion, add and review its project hazard-gate row in
`docs/SAFETY_MODEL.md`. The row must name the E0 inert boundary, the E1
intentional optical-emission hazard, immutable known-local-command authority,
exact-fixture/current/duty/burst limits, hardware default-off behavior,
cancellation latency, self-echo treatment, independent power removal, and the
prohibition on eye-safety, unknown-replay, access-control, launcher, and
pyrotechnic claims.

Before promotion run:

```sh
make headers-check
make check
make sanitize-check
make arduino
make lessons-check
make site-check
make release-check
```

Promotion requires all deterministic partitions, golden fixtures, compile-fail
surface checks, measured standalone and maximum-composition resources, both
stress-pass reviews, canonical compile-only Mega examples, HTML,
pencil-drawing PDFs, archive identity, and explicit open E1 cards. Host replay,
compilation, an intent LED, camera image, or generated PDF is never physical
acceptance.

Stop and record architectural remediation if implementation needs to change a
Lesson 025 public type, accept raw/captured transmit input, mutate the catalog,
infer repeats, hide observation time as capture time, use best-effort
parent/child mutation, generate catch-up bursts, select an unqualified timer or
emitter, or depend on diagnostics for safe state.

## Definition of done

E0 is complete only when the additive Lesson 052 decoder-and-copy component,
transactional direct-index Lesson 053 policy, and owned-child Lesson 054
translator implement the contracts above; every boundary and collision test
passes; standalone and maximum resources fit with reported post-stack margin;
publication artifacts agree; and no public path can turn repeat, unknown,
malformed, self-echo, or arbitrary captured evidence into transmission.

E1 remains open until exact fixtures, actual timing, electrical default-off,
independent expiry and power removal, optical observations, and signed bench
acceptance are complete.
