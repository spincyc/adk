# Lessons 064--066 thermal mapper plan

Status: implementation-depth E0 plan; exact powered specimens, single-wire
electrical behavior, thermal accuracy, persistence, and bench acceptance remain
open.

This arc teaches bounded single-wire transactions, qualified sets of identified
DS18B20 observations, and a thermal-gradient presentation policy. E0 owns no
pin, resource claim, timer, interrupt, bus, pull-up, supply, probe, LCD, RTC,
storage, or clock. It consumes copied synthetic receipts and supplied time,
then emits inert transaction, presentation, and record intent.

The authorized inventory lists `18B20 Temp` separately from the unidentified
`Digital Temperature` used in Lesson 062. Listing authorization is not
specimen identity. A configured ROM with family byte `0x28` and valid Dallas
CRC identifies the protocol role used by E0; it does not authenticate a
physical package, prove a genuine DS18B20, or authorize wiring.

## Evidence levels

| Level | Authorized work |
|---|---|
| E0 | Pure policy over copied line receipts, configured ROM identities, synthetic scratchpads, supplied time, and inert display/record intent |
| E1a | Exact DS18B20 specimen identity, package, pinout, supply mode, ordinary pull-up, switched rail if used, rise time, timing margins, current, and one-probe acceptance |
| E1b | Exact strong-pull-up circuit and parasite-power campaign, including transistor topology, current, voltage, duration, rollback, and independent power removal |
| E1c | Fixed set of up to four independently qualified probes on one exact bus, including discovery, capacitance, disappearance, simultaneous conversion, and thermal comparison |
| E1d | Exact LCD/LED presentation and optional RTC/media adapters with their own resource, rollback, safe-state, and acceptance evidence |

No immersion, hot surface, flame, heater, medical measurement, food-safety
claim, unattended heat source, waterproof-probe claim, or unknown three-pin
module is authorized. The Lesson 066 tabletop gradient uses only
room-temperature and hand-warmed objects after E1 qualification.

## Boundary and dependency order

| Lesson | Boundary | Depends on | Owns at E0 |
|---:|---|---|---|
| 064 | `OneWireTransactionPolicy` | `Status`, microsecond time, copied receipts | One bounded typed reset/presence/search/addressed transaction, external-power policy, timeout, rollback, and accepted prefix |
| 065 | `Qualified18B20ProbeSetPolicy` | Lesson 064 values, copied scratchpads | Four stable identities, CRC/resolution/conversion/freshness/disappearance state, and qualified temperatures |
| 066 | `ThermalGradientMapper` | Lesson 065 observations and copied controls | Configured spatial order, adjacent interval gradients, fault-dominant pages, and caller-owned record intent |

Implementation order is strict:

1. review this plan and the three stress dispositions;
2. implement and exhaustively test Lesson 064;
3. measure and reassess Lesson 064 before freezing Lesson 065;
4. implement and exhaustively test Lesson 065;
5. measure and reassess Lesson 065 before freezing Lesson 066;
6. implement Lesson 066 only from promoted copied observations;
7. add synthetic Mega replays, exact resource evidence, HTML, and
   pencil-drawing PDFs;
8. run terminal stress passes and all non-hardware publication gates;
9. leave every powered, thermal, display, RTC, media, and persistence card
   explicitly open.

## Shared ordering and status rules

Every copied producer value has nonzero source/configuration identity,
sequence, observation time, and `Status`. Supplied time is the only policy
clock. Every duration is nonzero and below the modular half range. Equal time
is idempotent only for byte-identical evidence or an explicitly receipt-only
transition. Changed duplicates, future/backward evidence, exact half-range
ambiguity, identity change, sequence regression, and exhaustion reject
atomically.

`Status` reports malformed input, lifecycle misuse, producer failure, or
capacity failure. Domain enums retain valid but unhealthy outcomes such as no
presence, CRC failure, conversion pending, stale, disappearance,
disagreement, or rollback pending. No fault can become a cold or healthy
temperature through voting.

## Lesson 064 -- bounded single-wire transactions

### Responsibility

`OneWireTransactionPolicy` owns one copied transaction lifecycle. It is not a
hardware `OneWireBus`: it emits line intent and validates copied receipts but
cannot drive or release a physical line. A future E1 bus may execute these
intents while owning the endpoint and timing resources.

```cpp
enum struct OneWireSupplyMode : uint8_t
{
    ExternallyPowered,
    ParasitePower
};

enum struct OneWireOperation : uint8_t
{
    ResetPresence,
    SearchRomPass,
    ReadRomSingleDrop,
    MatchRomReadPowerSupply,
    MatchRomStartConversion,
    MatchRomReadConversionStatus,
    MatchRomReadScratchpad
};

enum struct OneWirePhase : uint8_t
{
    Inert,
    ResetLow,
    PresenceWindow,
    WriteSlot,
    ReadSlot,
    Complete,
    RollingBack,
    Fault
};

enum struct OneWireLineIntent : uint8_t
{
    Release,
    DriveLow,
    Sample
};

enum struct OneWireTransactionQuality : uint8_t
{
    Unqualified,
    Pending,
    Complete,
    NoPresence,
    Collision,
    TimedOut,
    ProducerFault,
    ReleaseUnconfirmed
};

struct OneWireRomCode
{
    uint8_t bytes[8];
};

struct OneWireTransactionConfig
{
    uint32_t            ownerToken;
    uint16_t            configurationRevision;
    uint8_t             expectedReceiptSourceId;
    uint16_t            expectedReceiptConfigurationRevision;
    bool                singleDrop;
    MicrosecondDuration resetLowMinimum;
    MicrosecondDuration resetLowMaximum;
    MicrosecondDuration resetReleaseMinimum;
    MicrosecondDuration resetReleaseMaximum;
    MicrosecondDuration presenceStartMinimum;
    MicrosecondDuration presenceStartMaximum;
    MicrosecondDuration presenceLowMinimum;
    MicrosecondDuration presenceLowMaximum;
    MicrosecondDuration writeZeroLowMinimum;
    MicrosecondDuration writeZeroLowMaximum;
    MicrosecondDuration writeOneLowMinimum;
    MicrosecondDuration writeOneLowMaximum;
    MicrosecondDuration readInitiateMinimum;
    MicrosecondDuration readInitiateMaximum;
    MicrosecondDuration readSampleMinimum;
    MicrosecondDuration readSampleMaximum;
    MicrosecondDuration completeSlotMinimum;
    MicrosecondDuration completeSlotMaximum;
    MicrosecondDuration slotRecoveryMinimum;
    MicrosecondDuration slotRecoveryMaximum;
    MicrosecondDuration transactionDeadline;
    uint16_t            maximumSlots;
};

struct OneWireSearchState
{
    OneWireRomCode rom;
    uint8_t        lastDiscrepancy;
    bool           lastDevice;
};

struct OneWireOperationRequest
{
    uint32_t               requestSequence;
    OneWireOperation       operation;
    OneWireRomCode         addressedRom;
    OneWireSearchState     search;
    MicrosecondTimePoint   startedAt;
    OneWireSupplyMode      supplyMode;
    Status                 status;
};

struct OneWireStepIntent
{
    uint32_t            ownerToken;
    uint32_t            lifecycleGeneration;
    uint16_t            configurationRevision;
    uint32_t            requestSequence;
    uint32_t            transactionGeneration;
    OneWireOperation    operation;
    OneWirePhase        phase;
    uint32_t            phaseSequence;
    uint16_t            slotIndex;
    bool                writeBit;
    OneWireLineIntent   lineIntent;
    bool                sampleRequired;
    MicrosecondTimePoint earliestAt;
    MicrosecondTimePoint latestAt;
    OneWireRomCode      addressedRom;
};

struct OneWireStepReceipt
{
    uint8_t             sourceId;
    uint16_t            configurationRevision;
    uint32_t            sequence;
    MicrosecondTimePoint observedAt;
    uint32_t            ownerToken;
    uint32_t            lifecycleGeneration;
    uint32_t            requestSequence;
    uint32_t            transactionGeneration;
    OneWireOperation    operation;
    OneWirePhase        phase;
    uint32_t            phaseSequence;
    uint16_t            slotIndex;
    OneWireLineIntent   appliedIntent;
    bool                sampledHigh;
    bool                accepted;
    Status              status;
};

struct OneWireTransactionSnapshot
{
    OneWireOperation           operation;
    OneWirePhase               phase;
    OneWireTransactionQuality  quality;
    OneWireOperationRequest    request;
    OneWireSearchState         searchResult;
    OneWireRomCode             returnedRom;
    uint8_t                    readBytes[9];
    uint8_t                    readByteCount;
    uint16_t                   acceptedSlotCount;
    bool                       presenceSeen;
    bool                       releaseRequested;
    bool                       releaseConfirmed;
    MicrosecondTimePoint       completedAt;
    Status                     status;
};

struct OneWireTransactionPolicy
{
    explicit OneWireTransactionPolicy (
        const OneWireTransactionConfig& config) noexcept;

    Status initialize (MicrosecondTimePoint now,
                       OneWireStepIntent&   releaseIntent) noexcept;
    Status begin      (MicrosecondTimePoint             now,
                       const OneWireOperationRequest&   request,
                       OneWireStepIntent&               intent) noexcept;
    Status update     (MicrosecondTimePoint       now,
                       const OneWireStepReceipt& receipt,
                       OneWireStepIntent&         intent) noexcept;
    Status advance    (MicrosecondTimePoint now,
                       OneWireStepIntent&   intent) noexcept;
    Status cancel     (MicrosecondTimePoint now,
                       OneWireStepIntent&   intent) noexcept;
    Status reset      (MicrosecondTimePoint now,
                       OneWireStepIntent&   releaseIntent) noexcept;
    Status shutdown   (MicrosecondTimePoint now,
                       OneWireStepIntent&   releaseIntent) noexcept;
    Status confirmCleanup (MicrosecondTimePoint       now,
                           const OneWireStepReceipt& receipt) noexcept;

    Status snapshot    (OneWireTransactionSnapshot& snapshot) const noexcept;
    bool   initialized () const noexcept;
};
```

Every lower/upper timing pair, including reset release and complete slot, is
explicit, ordered, nonzero where the protocol requires it, and below the
modular half range. No window is inferred by subtracting another configured
duration. Reset and shutdown take supplied
microsecond time, invalidate earlier generations, and return a correlated
`Release` intent. They cannot report physical cleanup without its copied
receipt.

Initialization emits a baseline correlated `Release` intent and does not
become transaction-ready until a matching copied release receipt confirms
cleanup. `advance(now, intent)` is the receipt-free timeout/deadline path: it
emits at most one release intent and never polls or synthesizes a receipt.
Shutdown enters a closing cleanup state that remains initialized only for its
matching release receipt; `confirmCleanup()` then makes the policy inert.
Without confirmation the snapshot remains `ReleaseUnconfirmed`.

The exact spelling may change during implementation review, but the
information boundary may not weaken silently. Public large results are
caller-owned. The policy is noncopyable and nonmovable.

Lesson 064 addressed operations reject only the all-zero ROM. Every other
complete 64-bit value, including a wrong-family or bad-CRC nonzero ROM, is
transported byte-for-byte without reinterpretation. Lesson 064 does not
declare family or CRC valid; Lesson 065 exclusively owns family-`0x28` and ROM
CRC qualification.
`ReadRomSingleDrop` requires `singleDrop`. Multidrop enumeration uses exactly
one bounded `SearchRomPass` with copied fixed search state. Enumeration order
has no spatial meaning. Bits are serialized least-significant first. The
request is frozen at `begin()`, and the closed operation enum is the only
command surface: arbitrary bytes, captured pulse trains, and replay scripts
are excluded.

The phase order is reset-low, released presence sample, bounded write slots,
bounded read slots, then completion. Each matching
receipt advances at most one phase or slot. A duplicate never advances.
Partial read bytes remain attributable prefix evidence, not a completed device
response.

Only `ExternallyPowered` requests are admitted at E0. `ParasitePower` returns
`StatusCode::Unsupported` before mutation. There is no strong-pull-up phase or
line intent, and no E1 adapter may reinterpret `Release` as output-high or
strong-pull power. Parasite execution requires a separate reviewed power
capability and stress pass.

Every timeout, cancellation, producer failure, contradictory receipt, or
sequence exhaustion emits one canonical `Release` rollback intent. Only its
matching receipt confirms rollback. Until then the snapshot remains
rollback-pending. E0 never claims the physical line is released.

### Deterministic proof

Tests freeze an ordinary ROM bit stream, rejection of the all-zero addressed
ROM, and byte-identical transport of all-one, wrong-family, and bad-CRC
nonzero ROMs without qualification. They cover every reset-low, reset-release,
presence-start, presence-low, write-zero-low, write-one-low, read-initiate,
read-sample, complete-slot, and recovery lower/upper boundary immediately
below, at, and above; absent presence, stuck levels, both bit values,
LSB-first byte assembly, every first/
middle/final slot failure, changed duplicates, crossed transactions,
closed-operation validation, parasite-power `Unsupported`, cancellation and
rollback from every phase, bounded Search ROM collision/branch/completion,
rollover, lifecycle/sequence exhaustion, and byte-stable replay.

## Lesson 065 -- qualified DS18B20 sets

### Responsibility

`Qualified18B20ProbeSetPolicy` consumes copied transaction results and
scratchpads for exactly four configured ROM identities. It does not discover a physical bus,
start a conversion, or convert an unidentified device into a DS18B20.

```cpp
enum struct Ds18b20Resolution : uint8_t
{
    Bits9,
    Bits10,
    Bits11,
    Bits12
};

enum struct Ds18b20ProbeQuality : uint8_t
{
    Unqualified,
    ConversionPending,
    Current,
    RomCrcFault,
    ScratchpadCrcFault,
    ResolutionMismatch,
    ResetDefaultWithoutConversion,
    ImplausibleStep,
    Stale,
    Missing,
    DuplicateIdentity,
    TransportFault
};

enum struct Ds18b20SetQuality : uint8_t
{
    Unqualified,
    Complete,
    TransportFault,
    DuplicateIdentity,
    UnknownIdentity,
    Missing
};

struct OneWireTransactionEvidence
{
    uint32_t               ownerToken;
    uint32_t               lifecycleGeneration;
    uint16_t               configurationRevision;
    uint32_t               transactionGeneration;
    uint32_t               requestSequence;
    OneWireOperation       operation;
    OneWireRomCode         addressedRom;
    bool                   presenceSeen;
    uint8_t                bytes[9];
    uint8_t                byteCount;
    MicrosecondTimePoint   startedAt;
    MicrosecondTimePoint   completedAt;
    OneWireTransactionQuality quality;
    Status                 status;
};

struct Ds18b20ConversionReadEvidence
{
    OneWireRomCode            rom;
    uint32_t                  conversionGeneration;
    OneWireTransactionEvidence conversionRequest;
    OneWireTransactionEvidence conversionCompletion;
    OneWireTransactionEvidence scratchpadRead;
};

struct Ds18b20SetEnvelope
{
    uint8_t                       sourceId;
    uint16_t                      configurationRevision;
    uint32_t                      cycleSequence;
    TimePoint                     observedAt;
    OneWireTransactionEvidence    search;
    OneWireRomCode                returnedRoms[4];
    uint8_t                       returnedRomCount;
    bool                          searchComplete;
    Ds18b20ConversionReadEvidence reads[4];
    uint8_t                       readCount;
    Status                        status;
};

struct Ds18b20ProbeConfig
{
    OneWireRomCode       rom;
    Ds18b20Resolution    resolution;
    Duration             conversionDeadline;
    Duration             maximumAge;
    int16_t              minimumRawSixteenths;
    int16_t              maximumRawSixteenths;
    uint16_t             maximumStepRawSixteenths;
};

struct QualifiedDs18b20Probe
{
    OneWireRomCode       rom;
    uint32_t             cycleSequence;
    uint32_t             conversionGeneration;
    uint32_t             readTransactionGeneration;
    TimePoint            observedAt;
    int16_t              rawSixteenths;
    int16_t              lowerRawSixteenths;
    int16_t              upperRawSixteenths;
    Ds18b20Resolution    resolution;
    Ds18b20ProbeQuality  quality;
    Duration             age;
    Status               status;
};

struct QualifiedDs18b20Snapshot
{
    uint8_t                 sourceId;
    uint16_t                configurationRevision;
    uint32_t                cycleSequence;
    TimePoint               observedAt;
    QualifiedDs18b20Probe probes[4];
    uint8_t                validCount;
    uint8_t                presentMask;
    uint8_t                faultMask;
    Ds18b20SetQuality      quality;
    Status                 status;
};

struct QualifiedDs18b20SetConfig
{
    uint8_t            expectedSourceId;
    uint16_t           expectedConfigurationRevision;
    uint32_t           expectedOneWireOwnerToken;
    uint16_t           expectedOneWireConfigurationRevision;
    Ds18b20ProbeConfig probes[4];
};

struct Qualified18B20ProbeSetPolicy
{
    explicit Qualified18B20ProbeSetPolicy (
        const QualifiedDs18b20SetConfig& config) noexcept;

    Status initialize () noexcept;
    void   reset      () noexcept;
    Status update     (TimePoint                         now,
                       const Ds18b20SetEnvelope&         envelope,
                       QualifiedDs18b20Snapshot&         snapshot) noexcept;

    Status snapshot    (QualifiedDs18b20Snapshot& snapshot) const noexcept;
    bool   initialized () const noexcept;
};
```

Capacity is exactly four configured slots; there is no runtime count,
zero-filled optional slot, wildcard, or insertion. Configuration rejects
duplicate ROMs, wrong family/ROM CRC, invalid signed raw-sixteenth ranges, and
resolution/deadline mismatch. Dallas CRC-8 uses reflected polynomial `0x8c`,
initial value zero, least-significant bit first, and no final XOR.

The set envelope preserves one complete bounded ROM-search result and the full
Lesson 064 owner/lifecycle/configuration/request/transaction attribution for
search, conversion request, conversion completion, and scratchpad read.
Conversion/read correlation requires the exact ROM and one nonzero conversion
generation. Crossed ROM, lifecycle, configuration, request, transaction, or
generation rejects the complete set atomically.

Only a structurally complete successful search can prove `Missing`. Truncated,
failed, or over-capacity search is `TransportFault`. Duplicate returned ROMs
are `DuplicateIdentity`; a CRC-valid unknown ROM remains bounded set-fault
evidence and never substitutes for a configured identity. Returned
permutation never changes configured slot order.

A scratchpad is exactly nine bytes and its CRC is checked before decoding.
The configuration-byte oracle requires bit 7 to be zero and bits 4--0 to be
one. Bits 6--5 are R1:R0; they select 9-, 10-, 11-, or 12-bit resolution and
must match the configured slot. Undefined temperature-register low bits at coarser resolution are masked
into the represented code-space interval rather than rejected. For signed
decoded raw `masked` and resolution-dependent `undefinedMask`, the interval is
`[masked, masked + undefinedMask]` using widened signed arithmetic. This is a
digital quantization/code-space interval, not physical accuracy or uncertainty.
A source configuration/resolution mismatch remains `ResolutionMismatch`. The
published value remains native
signed sixteenths Celsius. Resolution masks create a closed quantization
interval: 12-bit is the exact sixteenth, while 9/10/11-bit values retain the
complete interval represented by their unavailable low bits. No
`milliCelsius` conversion or extra precision is published.

`+85 C` is `ResetDefaultWithoutConversion` only without an exactly correlated
completed conversion. After a matching completed conversion it is a valid
possible raw value. `ConversionPending` retains the last trusted value,
interval, observation time, and age without refreshing them. Step comparison
uses widened absolute subtraction between consecutive correlated current raw
values for one ROM.

Completion evidence is a typed Lesson 064
`MatchRomReadConversionStatus` transaction for the same exact ROM and
conversion generation, with a correlated completed-high read receipt. Elapsed
time alone never proves completion. Configured externally powered conversion
ceilings are exactly 94, 188, 375, and 750 milliseconds for 9-, 10-, 11-, and
12-bit roles respectively; a configured deadline may be no greater than its
resolution ceiling and equality is explicit.

The set `cycleSequence` is nonzero and strictly monotonically forward without
wrap. `UINT32_MAX` exhausts before zero; only supplied time may cross ordinary
modular rollover. Nested Lesson 064 request, phase, and transaction
generations obey their own no-wrap exhaustion.

Structural equality and duplicate checks are fieldwise over the complete
envelope and nested transactions; padding is never compared. Set-level
structural/transport failure precedes slot qualities. For a valid complete
cycle, all four side qualities commit together and returned `Status`
precedence follows configured slot order.

Set-level precedence and status mapping are exact:

1. malformed structure, zero/changed identity, invalid enum, crossed
   correlation, or count above four rejects atomically with
   `InvalidArgument` or `CapacityExceeded`;
2. producer failure or incomplete/failed search commits
   `Ds18b20SetQuality::TransportFault` and returns the complete producer
   `Status`;
3. duplicate returned ROM commits `DuplicateIdentity` with `StatusCode::Ok`;
4. a CRC-valid foreign ROM commits `UnknownIdentity` with `StatusCode::Ok`;
5. absence proved by complete search commits `Missing` with `StatusCode::Ok`;
6. otherwise the set is `Complete`, with per-slot ROM CRC, scratchpad CRC,
   resolution, reset-default, pending, stale, and step outcomes retained in
   configured slot order. Domain qualities do not replace `Status`; a nested
   producer failure returns the first complete non-OK status in configured
   slot order.

Tests cover literal ROM and scratchpad CRC vectors with corruption in every
byte; all search permutations/counts and complete versus incomplete search;
unknown/duplicate/missing collisions; every owner/lifecycle/configuration/
request/transaction/conversion/read correlation field; all signed raw
endpoints and resolution masks/reserved bits; quantization intervals; `+85 C`
with and without completed conversion; deadline equality; pending/current/
stale/missing/recovery; exact step boundaries; fieldwise duplicate,
regression, rollover, half-range ambiguity, exhaustion, reset, atomic
rejection, and byte-stable replay.

## Lesson 066 -- thermal gradient mapper

### Responsibility

`ThermalGradientMapper` consumes one copied four-probe snapshot plus copied
page controls. It emits stable presentation and record intent. It
does not poll probes, drive an LCD/LED, read an RTC, write media, or claim a
physical temperature gradient.

```cpp
enum struct ThermalGradientHealth : uint8_t
{
    Qualifying,
    Stable,
    Gradient,
    Disagreement,
    Fault
};

enum struct ThermalGradientQuality : uint8_t
{
    Unqualified,
    Flat,
    Rising,
    Falling,
    Indeterminate,
    Fault
};

enum struct ThermalMapperPageKind : uint8_t
{
    Overall,
    Probe,
    AdjacentGradient
};

struct ThermalMapperControl
{
    uint8_t   sourceId;
    uint16_t  configurationRevision;
    uint32_t  sequence;
    TimePoint observedAt;
    bool      nextPressed;
    bool      recordPressed;
    Status    status;
};

struct ThermalGradientPair
{
    uint8_t                leftSlot;
    uint8_t                rightSlot;
    int32_t                lowerRawSixteenths;
    int32_t                upperRawSixteenths;
    ThermalGradientQuality quality;
    uint8_t                faultMask;
};

struct ThermalGradientIntent
{
    ThermalGradientHealth health;
    ThermalMapperPageKind  pageKind;
    uint8_t                pageIndex;
    uint8_t                selectedSlot;
    OneWireRomCode         selectedRom;
    int16_t                selectedLowerRawSixteenths;
    int16_t                selectedUpperRawSixteenths;
    Duration               selectedAge;
    ThermalGradientPair    adjacent;
    uint8_t                configuredCount;
    uint8_t                overallFaultMask;
    OneWireRomCode         minimumRom;
    OneWireRomCode         maximumRom;
    int16_t                minimumLowerRawSixteenths;
    int16_t                maximumUpperRawSixteenths;
    uint8_t                minimumTieMask;
    uint8_t                maximumTieMask;
    uint8_t                ledSelectionMask;
    bool                   lcdShowsIdentity;
    bool                   lcdShowsAgeOrFault;
    bool                   outputsInactive;
};

struct ThermalMapperConfig
{
    uint32_t       ownerToken;
    uint16_t       configurationRevision;
    uint8_t        expectedSetSourceId;
    uint16_t       expectedSetConfigurationRevision;
    uint8_t        expectedControlSourceId;
    uint16_t       expectedControlConfigurationRevision;
    OneWireRomCode spatialOrder[4];
    uint8_t        spatialCount;
    Duration       maximumControlAge;
    uint16_t       meaningfulGradientRawSixteenths;
};

struct ThermalMapperEnvelope
{
    TimePoint                      now;
    QualifiedDs18b20Snapshot       probes;
    ThermalMapperControl           control;
};

struct ThermalMapperRecordProbe
{
    OneWireRomCode        rom;
    int16_t               lowerRawSixteenths;
    int16_t               upperRawSixteenths;
    Ds18b20Resolution     resolution;
    Ds18b20ProbeQuality   quality;
    Duration              age;
    uint32_t              conversionGeneration;
    uint32_t              readTransactionGeneration;
    Status                status;
};

struct ThermalMapperRecordImage
{
    uint8_t                formatVersion;
    uint32_t               ownerToken;
    uint32_t               lifecycleGeneration;
    uint16_t               configurationRevision;
    uint32_t               recordSequence;
    uint8_t                recordEdgeSourceId;
    uint16_t               recordEdgeConfigurationRevision;
    uint32_t               recordEdgeSequence;
    TimePoint              recordEdgeObservedAt;
    uint8_t                setSourceId;
    uint16_t               setConfigurationRevision;
    uint32_t               setCycleSequence;
    TimePoint              observedAt;
    ThermalMapperRecordProbe probes[4];
    ThermalGradientPair    gradients[3];
    uint8_t                probeCount;
    uint8_t                gradientCount;
    ThermalGradientHealth  health;
    uint8_t                faultMask;
    uint32_t               witnessDigest;
};

struct ThermalMapperResult
{
    ThermalGradientIntent intent;
    bool                  hasRecord;
    ThermalMapperRecordImage record;
    Status                status;
};

struct ThermalGradientMapper
{
    explicit ThermalGradientMapper (const ThermalMapperConfig& config) noexcept;

    Status initialize (TimePoint now) noexcept;
    Status reset      (TimePoint now) noexcept;
    Status update     (const ThermalMapperEnvelope& envelope,
                       ThermalMapperResult&         result) noexcept;
    Status shutdown   () noexcept;

    Status                snapshot    (ThermalGradientIntent& intent) const noexcept;
    bool                  initialized () const noexcept;
};
```

Configuration freezes two to four distinct ROM identities in explicit spatial
order and the exact Lesson 065 source/configuration revision. The list is an
ordered subset of the source's exact four configured slots. The source may
retain nonmapped configured slots; they are ignored for gradient mapping but
remain in source provenance. A ROM outside the exact Lesson 065 configuration
is still a fault. One accepted set must contain every mapped identity exactly
once. Spatial order, never
search, arrival, ROM numeric, temperature, or page order, defines slots and
adjacent pairs.

Each slot retains its raw-sixteenth interval. For adjacent left/right slots,
widened arithmetic computes
`lower = right.lower - left.upper` and
`upper = right.upper - left.lower`. `Rising` requires `lower >= threshold`;
`Falling` requires `upper <= -threshold`; `Flat` requires the complete
interval strictly inside both thresholds. Any interval crossing a boundary is
`Indeterminate`. Equality is explicit.

Any configured probe that is not `Current` makes overall health `Fault` and
faults both incident gradient pages. Nonincident healthy pages retain their
own evidence, but every page carries the overall fault mask. No unhealthy
probe can render as zero, cold, flat, cached current, or a numeric-only page.
Page order is fixed: overall, configured probe pages, then adjacent-gradient
pages. A fresh control edge advances once; invalid or repeated controls cannot
change classification.

Health aggregation is exact: any mapped probe/pair fault is `Fault`; otherwise
mixed rising and falling pairs, or any `Indeterminate` pair, is
`Disagreement`; otherwise any rising or falling pair is `Gradient`; all-flat
pairs are `Stable`. `Qualifying` exists only before the first accepted frame.
The meaningful threshold is nonzero and at most 2,880 raw sixteenths.
Extrema retain the supporting ROM identities, widened endpoint values, and
complete tie masks; equal extrema choose the lowest spatial slot only for the
primary page token.

Every update synchronously fills one caller-owned result; a fresh record edge
may mark at most one versioned `ThermalMapperRecordImage` present. It contains the complete
ordered ROM/interval/quality/age slots, all adjacent gradient intervals,
health/fault mask, set provenance, mapper lifecycle/sequence, and a
deterministic digest. Full fields are authoritative. A byte-identical replay
emits no new record. There is no receipt, acknowledgement, outstanding slot,
retry, coalescing, storage call, or durability claim. Record-sequence
exhaustion faults before zero.

The record is a fixed field inside caller-owned `ThermalMapperResult`; there is
no nullable pointer, runtime size, or buffer-too-small path. Compile-time ABI
checks, canaries, and exact caller-buffer measurement guard its bounds.
The record-edge source, configuration, sequence, and observation time are
copied from the fresh control edge that requested this image. FNV-1a32 uses
offset `0x811c9dc5`, prime `0x01000193`, and domain
`ADK.THERMAL.MAPPER.RECORD.V1` without NUL. Hash order is format version,
owner, lifecycle, configuration, record sequence, record-edge
source/configuration/sequence/time, set source/configuration/cycle/time,
spatial count, then every mapped probe's ROM,
signed lower/upper raw sixteenths, resolution, quality, age, conversion/read
generations and status, followed by every adjacent pair's slot indices,
signed lower/upper, quality and fault mask, then overall health/fault mask.
Integers are fixed-width little-endian, enums/status are one byte, and padding
is never hashed.

`nextPressed` and `recordPressed` are independently edge-triggered by a fresh
forward control sequence. Held or duplicate controls do nothing. A simultaneous
fresh next+record edge first advances the page, then records the current
accepted snapshot with that selected-page intent. A record edge may record the
current accepted snapshot even when no new set cycle arrived; repeated/held
record never duplicates it without a new control edge. A newly admitted frame
without a record edge updates presentation but emits no record.

Initialize/reset advance a nonzero lifecycle generation without wrap and
restart record sequence at one for that lifecycle. Generation or record
sequence exhaustion faults before zero. Shutdown emits no record and makes
presentation inert.

Tests cover configured counts below two, two, three, four, and above four;
every input permutation; duplicate/foreign/missing identity; all child
qualities; raw-sixteenth widened extrema; every adjacent threshold
below/equal/above and crossing interval; one bad interior slot faulting both
incident pairs; fixed page zero/last/wrap; edge controls; fieldwise duplicate,
regression, rollover and exhaustion; record canaries/golden image/replay
suppression; reset, shutdown, restart, and byte-stable replay.

The canonical collision trace is sequential and replayable: accept a
reverse-ordered healthy four-slot source frame; apply a simultaneous
next+record control edge; accept a later frame where the second mapped probe
disappears and fault both incident pairs; apply a fresh record edge for that
fault snapshot; accept a complete recovered frame across time rollover; then
reset and shutdown. Each step produces at most one record, preserves source
provenance, and never implies storage success.

## State and precedence

Structural identity, bounds, enum, ordering, time, and transaction correlation
reject before semantic mutation. For accepted evidence:

1. lifecycle/shutdown keeps outputs inert;
2. transport or rollback uncertainty remains a fault, never presence;
3. ROM and scratchpad CRC failure prevents temperature use;
4. conversion pending/reset-default/stale/missing remain distinct;
5. only qualified current values participate in gradient arithmetic;
6. any mapped noncurrent slot or incident pair fault produces `Fault`;
7. otherwise indeterminate or mixed directions produce `Disagreement`,
   directional evidence produces `Gradient`, and all-flat evidence produces
   `Stable`;
8. control edges apply atomically after classification and cannot alter the
   thermal decision;
9. presentation and record fields preserve configured spatial ROM identity
   rather than discovery order.

## Resource budgets

Exact AVR measurement uses the canonical linked Lesson 066 replay, no LTO for
stack attribution, caller-owned buffers instantiated exactly once, and a
fingerprint of compiler, core, flags, sources, and review markers.
These planning budgets permit implementation; exact measurements gate
promotion and may not be claimed before the implementation exists.

| Lesson | Flash target/hard | Static SRAM target/hard | Stack target/hard | Object target/hard |
|---:|---:|---:|---:|---:|
| 064 | 10/14 KiB | 768/1,024 B | 320/448 B | 192/256 B |
| 065 | 12/16 KiB | 1,024/1,536 B | 448/640 B | 512/768 B |
| 066 aggregate | 16/24 KiB | 1,536/2,048 B | 768/1,024 B | mapper 512/768 B |

Lesson 064 caller buffers target/hard-limit 128/256 bytes; Lesson 065 uses
256/512 bytes; Lesson 066 uses 256/384 bytes. Residual SRAM is
`8192 - static - synchronous_stack - 128`; the Lesson 066 aggregate target is
4,096 bytes and the non-reviewable hard floor is 2,048 bytes. Hard/residual
failures cannot be waived. Target misses require current tuple-bound
independent review.

No heap, virtual dispatch, callbacks, recursion, polling loop, retry loop,
catch-up loop, arbitrary-length script interpreter, hidden clock, or hidden
large return is permitted.

## Pre-implementation stress dispositions

### Lesson 064

- Disposition: `natural fit` only as an E0 copied-evidence transaction policy;
  publishing it as a hardware `OneWireBus` would falsely claim resource and
  electrical ownership.
- Main pressures: microsecond rollover, exact receipt correlation, explicit
  semantic open-drain release, parasite-power `Unsupported`, bounded buffers, rollback
  attribution, and future E1 execution without changing policy semantics.
- Promotion permitted: E0 implementation after exhaustive phase/rollback and
  exact resource proof; no physical bus or safe-line claim.

### Lesson 065

- Disposition: `natural fit` above Lesson 064 with fixed configured identities;
  transport, scratchpad qualification, and thermal-set policy remain separate.
- Main pressures: four-probe object size, signed fixed-point conversion,
  resolution-dependent deadlines, identity stability, CRC/reset-default
  precedence, and disappearance without discovery-order aliasing.
- Promotion permitted: E0 implementation after exhaustive CRC/conversion/set
  tests and exact resource proof; no accuracy or specimen claim.

### Lesson 066

- Disposition: `natural fit` as an inert project over copied qualified sets.
- Main pressures: aggregate memory/stack, adjacent interval subtraction,
  simultaneous disappearance and record-edge sequence exhaustion, stable
  identity presentation, caller-result canaries, and
  preventing optional RTC/media work from entering the core.
- Promotion permitted: E0 implementation after maximum-composition replay,
  publication, and resource gates; no powered display, logger, gradient, or
  thermal-performance claim.

Any implementation requiring a generic protocol base, borrowed caller
pointers, unbounded search/script storage, hidden hardware access, persistent
media, or changes to the promoted Lesson 022/024 bus/storage contracts is
architectural strain and requires a separate decision before promotion.

## Primary sources and claim boundary

- [Analog Devices DS18B20 programmable-resolution 1-Wire digital thermometer
  datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf)
  controls family code, ROM/scratchpad layout, CRC, resolution, conversion,
  power-mode, and strong-pull-up claims.
- [Microchip ATmega640/1280/1281/2560/2561
  datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf)
  controls only MCU electrical/resource facts used by a future exact adapter.
- The repository's
  [authorized Elegoo set](../inventory/AUTHORIZED_ELEGOO_SET.md) controls
  listing scope and preserves `18B20 Temp` as distinct from `Digital
  Temperature`.

Primary documentation defines a candidate protocol and conservative limits;
it does not bind an unseen kit specimen. E1 must record exact marking,
package, provenance, pinout, pull-up and supply topology, unpowered
conformance, powered timing/current/voltage evidence, and bench acceptance
before any wiring, schematic, accuracy, strong-pull-up, or thermal claim is
promoted.
