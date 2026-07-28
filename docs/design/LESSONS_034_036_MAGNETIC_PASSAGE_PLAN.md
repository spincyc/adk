# Lessons 034–036 magnetic passage plan

Status: implementation-depth planning complete, 2026-07-27. Planning only; no
first-class code or physical acceptance is claimed.

## Scope correction

The authorized Elegoo union lists `Linear Hall` and `Magnetic Spring`. It does
not separately list analog Hall, digital Hall, a magnet, or removable storage.
Exact output topology, polarity, pinout, transfer curve, and active level
remain specimen gates. Direction comes from two geometric boundaries A/B, not
one sensor. Encoder evidence is optional corroboration. Physical RTC and
removable media remain deferred; Lesson 036 uses deterministic clock and
ledger fixtures.

Use these terms throughout: raw observation, qualified boundary, passage
candidate, accepted passage, committed record, and presented count.

## 034 — Magnetic observations

Files: `src/magnetic_observation.h/.cpp`.
Energy class: E0 in host tests; eventual E1 for the inert, current-limited
fixture after specimen qualification.

Public values:

```cpp
enum struct MagneticPolarity : uint8_t
{
    Negative, Neutral, Positive, Unspecified
};
enum struct MagneticQuality : uint8_t
{
    Unqualified, Valid, BelowQualifiedRange, AboveQualifiedRange
};
enum struct MagneticSource : uint8_t { LinearAnalog, ContactDigital };
struct MagneticObservation
{
    MagneticSource source;
    uint16_t raw;
    Level rawLevel;
    TimePoint observedAt;
    MagneticPolarity polarity;
    bool activationEvent, deactivationEvent, active;
    Duration stableFor;
    MagneticQuality quality;
    Status status;
};
struct LinearHallConfig
{
    PinId pin;
    uint16_t qualifiedMinimum, qualifiedMaximum;
    uint16_t negativeActivate, negativeRelease;
    uint16_t positiveRelease, positiveActivate;
    Duration dwell;
    bool reversePolarity;
};
struct MagneticContactConfig
{
    PinId pin;
    Pull pull;
    Level closedLevel;
    Duration dwell;
};
```

`LinearHall(ResourceRegistry&, const LinearHallConfig&)` owns one
`AnalogInput`. Configuration holds pin, qualified ADC
range, negative activate/release, positive release/activate, dwell, and
reversal. Widened validation requires:

```text
qualifiedMin <= negativeActivate < negativeRelease
negativeRelease < positiveRelease < positiveActivate
positiveActivate <= qualifiedMax <= 1023
```

Activate/release bands supply hysteresis; a candidate persists for `dwell`.
Reversal swaps reported polarity after interpretation. Out-of-range is quality
evidence, not an open/short diagnosis.

`MagneticContact(ResourceRegistry&, const MagneticContactConfig&)` owns one
`DigitalInput`; configuration holds pin, pull, closed level, and dwell.
Polarity is `Unspecified`. Constant open/closed cannot prove open circuit,
short, stuck contact, or magnet presence.

Both types provide `Status initialize()`, `void update(TimePoint)`,
`MagneticObservation snapshot() const`, `bool initialized() const`, and
`void shutdown()`. They are inert, noncopyable/nonmovable, validate before
claiming, sample their owned endpoint once per explicit-time update, publish
stable snapshots, roll back, shut down idempotently, retain last raw evidence,
and release resources in destruction. Before the first successful update,
snapshots are unqualified with `Status(StatusCode::NotInitialized)`.
Out-of-qualified-range evidence clears a pending dwell candidate without
manufacturing an event. Backward time sets
`Status(StatusCode::InvalidArgument)` without state transition. Direct
Negative-to-Positive or Positive-to-Negative travel
must qualify Neutral and the new polarity in separate dwell intervals:
deactivation is emitted first and activation only after the new dwell.
For `LinearAnalog`, `rawLevel` is canonically `Level::Low`; for
`ContactDigital`, `raw` is canonically `0` for Low and `1` for High. Thus all
snapshot bytes are defined. Time advances when unsigned modular elapsed is
strictly less than `0x80000000`; natural `uint32_t` rollover is valid, exact
half-range or a larger apparent jump is `InvalidArgument` without mutation.

These are generic host contracts. They do not prove that the listed
`Linear Hall` specimen has analog output, bipolar response, a neutral center,
or polarity semantics, and `MagneticContact` does not assert that `Magnetic
Spring` is a reed switch. Those claims remain blocked until the exact retained
specimens are qualified. Any polarity or hysteresis bench experiment also
requires a separately user-authorized, identified, retained magnetic
stimulus; do not improvise with a loose magnet.

Tests cover thresholds ±1/exact from every state, hysteresis, reversal,
zero/nonzero dwell, invalid-range recovery, contact polarity/pulls,
bounce/chatter, rollover/backward time, invalid config/enums, unsupported/busy
pins, initialization failure/rollback/reuse, exact samples, byte-identical
replay, and ownership traits. Staleness and runtime source failure are not
invented: the existing concrete endpoints expose neither.

Provisional E1 resources, still PX:

| Resource | Role |
|---|---|
| A0/D54 `TP-H` | qualified Linear Hall output |
| D22 `TP-R` | qualified Magnetic Spring/contact output |
| D30/D31/D32 | raw-range, qualified-contact, ready/fault LEDs via 1 kΩ |

No PWM, interrupt, bus, or exclusive timer. Direct LEDs are at most 5 mA each.
All module/base/aggregate currents remain blank pending exact qualification.
Test points are electrical evidence; LEDs are interpreted evidence.

## 035 — Passage qualification

`PassageQualifier` is pure and hardware-neutral. It consumes two
`MagneticObservation` values and optional copied Lesson 032 position evidence.
It owns no pin, clock, storage, endpoint, or callback.
Energy class: E0.

Values include `PassageBoundary {None,A,B}`,
`PassageDirection {Unknown,AToB,BToA}`,
`PassageDisposition {Accepted,TimedOut,DuplicateSuppressed,Ambiguous,
EvidenceFault}`, and `PassagePhase {Idle,FirstBoundary,AwaitingSecond,
Suppressing,Fault}`. The fixed values are:

```cpp
struct PassagePositionEvidence
{
    bool present, reliable, saturated;
    int32_t onsetPosition, endPosition, delta;
};
struct PassageQualifierConfig
{
    Duration boundaryDwell, passageTimeout, duplicateWindow;
};
struct PassageInput
{
    TimePoint observedAt;
    MagneticObservation boundaryA, boundaryB;
    bool hasPosition;
    int32_t position;
    Status positionStatus;
};
struct PassageRecord
{
    uint32_t sequence;
    PassageDirection direction;
    PassageDisposition disposition;
    TimePoint onset, end;
    Duration elapsed;
    MagneticPolarity onsetPolarity, endPolarity;
    PassagePositionEvidence position;
    uint32_t acceptedCount, suppressedCount;
    Status status;
};
struct PassageSnapshot
{
    PassagePhase phase;
    PassageBoundary firstBoundary;
    Duration elapsed;
    uint32_t nextSequence, acceptedCount, suppressedCount;
    bool hasRecord;
    PassageRecord record;
    Status status;
};
```

`PassageQualifier(PassageQualifierConfig)`, `Status initialize()`,
`void reset()`, `void update(const PassageInput&)`, `PassageSnapshot
snapshot() const`, and `bool initialized() const` are the complete callable
surface. `reset()` returns to the just-initialized state and clears counts,
sequence, pending record, and time history; there is no distinct restart
operation. `hasRecord` is true for exactly the snapshot following terminal
emission and is cleared by the next later update. The record is the sole
downstream authority for encoder evidence; Lesson 036 does not accept a second
encoder copy.

`PassageInput::observedAt` must equal both observation timestamps; mismatch is
an `EvidenceFault` before any state mutation. `Unqualified`,
`BelowQualifiedRange`, `AboveQualifiedRange`, or non-Ok observation status
immediately emits one `EvidenceFault` for an active candidate, enters Fault,
and otherwise enters Fault without a record. Recovery follows rule 12.
Missing position produces `present=false`, zero canonical positions/delta,
`reliable=false`, and `saturated=false`. Non-Ok `positionStatus` retains the
copied onset/end values but sets `reliable=false`. Otherwise delta is computed
in widened signed arithmetic and clamped to `int32_t`, with `saturated=true`
on clamping. Reliability requires nonzero unclamped delta whose sign agrees
with the configured convention A-to-B positive and B-to-A negative; zero,
opposite sign, source error, or saturation is unreliable but does not reject
an otherwise accepted passage. `PassageRecord::status` reports passage
qualification only; position reliability is carried solely by position
fields.

Frozen policy:

1. only healthy `Valid` observations participate;
2. a sole active boundary must dwell continuously;
3. timeout begins when the first boundary finishes dwell; the opposite
   boundary must finish dwell at or before timeout;
4. simultaneous qualification is ambiguous;
5. retreat of the first boundary before the opposite boundary finishes never
   accepts and returns to idle only after both are inactive;
6. acceptance increments once and enters suppression;
7. rearm requires both inactive plus duplicate-window expiry;
8. activation during suppression increments only suppressed evidence;
9. precedence is invalid time, source fault, ambiguity, timeout, completion;
10. identical same-time frames are idempotent with no record or count change;
    changed same-time frames fault without partial mutation;
11. a terminal disposition emits exactly one record with the next saturating
    sequence; only `Accepted` increments the accepted count;
12. fault recovery requires a later healthy both-inactive frame and emits no
    record.

All durations are nonzero and strictly below unsigned half-range; elapsed
comparisons use modular subtraction and an exact half-range jump faults.
Timeout is at least dwell. Records/counts and position delta saturate, never
wrap; saturation is explicit evidence.

Golden traces cover both directions, dwell/timeout exactness, chatter,
simultaneous activation, retreat/reversal, late second boundary, stuck startup,
suppression edges, encoder missing/agreement/disagreement/invalid/saturated,
precedence, capacity, chunking, reset, and rollover.

## 036 — Magnetic passage logger

`MagneticPassageLogger` consumes accepted Lesson 035 records, a local
`None/A/B/C` label, abstract `Rtc`, and one `PassageLedger`.
Labels are metadata, not identity/authentication. The logger does not resample
sensors, decode quadrature, implement a filesystem, or actuate a load.
Energy class: E0 for core and persistence fixtures; eventual E1 for the inert
indicator fixture.

`LoggedPassage` retains accepted sequence, resulting committed count,
acceptance time, direction, label, position evidence, and complete
`ClockReading`; degraded clock state is never called a valid timestamp.

```cpp
enum struct PassageLabel : uint8_t { None, A, B, C };
struct PassageCheckpoint
{
    uint32_t generation, committedCount, committedSequence;
};
struct LoggedPassage
{
    uint32_t sequence, committedCount;
    TimePoint acceptedAt;
    PassageDirection direction;
    PassageLabel label;
    PassagePositionEvidence position;
    ClockReading clock;
    bool sequenceGap;
};
enum struct PassageLedgerRecoveryDisposition : uint8_t
{
    Empty, Recovered, RecoveredWithErasedPeer, RecoveredWithTornPeer,
    RecoveredWithCorruptPeer, RecoveredWithUnsupportedPeer, BothInvalid,
    DuplicateIdentical, DuplicateGeneration, AmbiguousGeneration,
    UnsupportedVersion
};
struct PassageLedgerRecovery
{
    PassageLedgerRecoveryDisposition disposition;
    bool hasCheckpoint;
    PassageCheckpoint checkpoint;
    bool hasEntry;
    LoggedPassage entry;
    Status status;
};
struct PassageLedgerStorage
{
    virtual ~PassageLedgerStorage () noexcept;
    virtual uint16_t capacity () const noexcept = 0;
    virtual Result<uint8_t> read (uint16_t address) noexcept = 0;
    virtual Status write (uint16_t address, uint8_t value) noexcept = 0;
    virtual Status synchronize () noexcept = 0;
};
enum struct LedgerCommitDisposition : uint8_t
{
    NotCommitted, Committed, CommittedAfterReconciliation
};
struct LedgerCommitResult
{
    LedgerCommitDisposition disposition;
    PassageCheckpoint checkpoint;
    Status status;
};
struct PassageLedger
{
    virtual ~PassageLedger () noexcept;
    virtual Status initialize () noexcept = 0;
    virtual void shutdown () noexcept = 0;
    virtual PassageLedgerRecovery recover () noexcept = 0;
    virtual LedgerCommitResult commit (const LoggedPassage&,
                                       const PassageCheckpoint&) noexcept = 0;
};
struct LoggerConfig
{
    PassageLabel initialLabel;
};
struct LoggerSnapshot
{
    PassageLabel selectedLabel;
    uint32_t committedCount, committedSequence, displayedCount;
    bool displayValid, pending, overrun;
    PassageRecord pendingInput, overrunInput;
    bool hasFrozenEntry;
    LoggedPassage frozenEntry;
    bool acceptedPulse, committedPulse, persistentFault;
    PassageLedgerRecoveryDisposition recovery;
    Status presentationStatus, status;
};
```

`PassageLedgerRecovery` distinguishes valid empty storage from failure.

`MagneticPassageLogger(LoggerConfig, Rtc&, PassageLedger&,
PassageCountDisplay&)` is destructible and noncopyable/nonmovable. The
`SevenSegmentPassageCountDisplay` adapter binds that narrow presentation
boundary to `SevenSegmentDisplay`. The logger provides
`Status initialize()`, `Status update(const PassageRecord&)`,
`Status cycleLabel()`, `LoggerSnapshot snapshot() const`, `bool initialized()
const`, and `void shutdown()`. Initialization validates configuration,
initializes ledger then RTC then the already-constructed display, recovers a
checkpoint, and rolls back in reverse order on failure. Shutdown is
idempotent. `LoggerSnapshot` exposes selected label, committed count/sequence,
displayed count, pending state, overrun evidence, recovery disposition,
presentation status, and logger status. LED states are external presentation
intents in the snapshot; the narrative example owns the three
`DigitalOutput`s and mirrors them after each logger call. Accepted and
committed pulses are true for the snapshot immediately following the
respective event and clear on the next call; persistent fault remains true
until successful shutdown and reinitialization. `status` precedence is ledger,
RTC, sequence/input, then presentation.
When `pending`, `overrun`, or `hasFrozenEntry` is false, its associated value
is the all-zero canonical value. Before RTC success, `pendingInput` and its
frozen label are valid while `hasFrozenEntry` is false; after RTC success,
`frozenEntry` is valid and immutable through ledger retries.

Recovery returns the complete last committed entry whenever it returns a
checkpoint. For an incoming sequence equal to committed, the logger compares
the record's sequence, direction, acceptance time, and position fields with
the recovered entry; equality is an idempotent no-op, and disagreement is
`InternalInvariant`. Label and clock are excluded because they were frozen by
the prior logger instance and are absent from the input record.

This is one logical durability boundary. Its deterministic implementation uses
two alternating fixed-size slots with version, generation, full entry,
resulting count/sequence, length, and checksum. Recovery exposes erased, torn,
corrupt, duplicate-generation, unsupported-version, and two-invalid states.
The fixture backend is a new byte-addressed `PassageLedgerStorage`, not
`FixedStorage`; reads and writes are one byte, return the first injected
failure, and synchronization makes all preceding writes durable or fails
without promising them durable. There is no erase operation: `0xff` is the
erased byte and invalidating the inactive slot writes its marker to `0xff`.
Each slot is `[magic:u32, version:u8, length:u16, generation:u32, entry,
count:u32, sequence:u32, checksum:u32, valid-marker:u8]`, with fixed
little-endian widths, magic `0x3147504d`, version `1`, valid marker `0xa5`,
and IEEE CRC-32 over every field through sequence. `LoggedPassage` receives an
explicit encoding: sequence/count/accepted milliseconds are `u32`; direction
and label are `u8`; position flags are one `u8` bit field followed by
onset/end/delta `i32`; clock seconds are `u32`, clock state is `u8`, and
sequence-gap is `u8`. All are little-endian in that order; no struct memory
image is serialized.
Commit writes the inactive slot with an erased marker, writes body and checksum
in address order, synchronizes, writes the valid marker last, synchronizes
again, then exposes the generation. Generation comparison uses modular
ordering; an exact half-range separation is ambiguous and recovery fails.
`commit(entry, checkpoint)` treats `checkpoint` as the expected current
checkpoint. It requires `entry.committedCount` to equal saturated
`checkpoint.committedCount + 1`, and entry sequence to be strictly newer than
the checkpoint sequence; it writes generation
`checkpoint.generation + 1` and the entry's validated count/sequence. Mismatch
with recovered/current state returns `NotCommitted/InvalidArgument` without
writing. Empty recovery supplies synthetic checkpoint `{0,0,0}` and the first
commit deterministically uses slot zero.

Recovery maps two erased slots to `Empty/Ok`; one valid plus an erased, torn,
checksum-invalid, or unsupported-version peer to the corresponding
`RecoveredWith.../Ok`; two
invalid non-erased slots to `BothInvalid/HardwareFailure`; equal valid
generations with identical bytes to `DuplicateIdentical/Ok` using slot zero,
and equal generations with unequal bytes to
`DuplicateGeneration/InternalInvariant`;
an exact half-range generation difference to
`AmbiguousGeneration/InternalInvariant`; and a structurally valid unsupported
version with no supported-valid peer to `UnsupportedVersion/Unsupported`. A torn slot has a
non-`0xa5` marker or a short body; corrupt means valid marker with bad
magic/length/checksum. One valid supported slot always wins over an invalid
peer.

If either synchronization fails, the ledger immediately rereads both slots.
It returns `CommittedAfterReconciliation/Ok` with the recovered new checkpoint
when the new slot is valid and newest; `NotCommitted` when the old checkpoint
still wins; and `NotCommitted/HardwareFailure` with the ledger unusable when
the outcome is ambiguous or reread fails. The logger advances logical state
for either committed disposition and retries only `NotCommitted`.

For a new sequence, freeze label/position, read RTC once, build the resulting
count, then commit entry plus checkpoint. Ledger success changes logical
committed count, sequence, and generation. Ledger failure changes none of
committed state, display, or generation.
Same-sequence ledger retry after a successful RTC read is byte-identical
without rereading RTC. A newer passage
while pending produces bounded overrun evidence.

An RTC read attempt occurs once per logger update until one succeeds. A failed
read freezes the input record and label as pending, emits the accepted pulse,
returns the RTC error, performs no ledger or display operation, and does not
read the failed `Result` value. A retry of the same sequence reads RTC again;
the first successful `ClockReading` is then frozen and never reread during
ledger retries. Newer inputs follow the same bounded-overrun rule.

Only `PassageDisposition::Accepted` is consumable; other dispositions return
`InvalidArgument`. A sequence equal to committed is an idempotent no-op if its
bytes agree and otherwise faults; an older sequence is rejected; a forward
gap is accepted and recorded as gap evidence. A saturated maximum sequence
may be committed once but no distinct successor can be accepted. While a
ledger commit is pending, the same sequence retries the frozen bytes and one
newer record is copied into `overrunInput` as bounded overrun evidence but not
queued or committed; later arrivals replace it and leave `overrun=true`.

The one-digit display shows `committedCount % 10`; decimal point means larger
total. Presentation runs only after durable commit. Its failure cannot mutate
committed state or cause a ledger retry; it sets presentation status and the
persistent-fault LED. The label button cycles `None,A,B,C`; the selected label
is frozen on first processing of a sequence, so a simultaneous button edge
uses the prior label.

Provisional fixture-only Mega resources:

| Resource | Role |
|---|---|
| A0/D54 `TP-H`, D25 `TP-R` | qualified boundaries |
| D26/D27 | optional polled encoder |
| D28 | label button |
| D22/D23/D24 | 74HC595 data/clock/latch |
| D30/D31/D32 | accepted, committed, persistent-fault LEDs |

Segments retain Lesson 010's 8 mA/path and 40 mA total. Direct LEDs use 1 kΩ
and at most 5 mA, for at most 55 mA known indicator/segment load. That figure
explicitly excludes 74HC595 quiescent/package current, sensor modules, pulls,
and the Mega baseline. Qualification must separately budget per-pin,
per-port, 74HC595 package, 5 V rail, and aggregate current. No physical bus,
PWM, interrupt, timer, conveyor, or motor.

Replay covers both directions, rejections, all position/label/clock states,
failure at every ledger byte/sync, byte-identical retry, pending overrun, torn
restart, newest-slot fallback, generation/sequence boundaries, lifecycle
rollback/shutdown, and byte-identical snapshots, slots, presentation, and
trace digest.

## Documentation and safety gates

HTML contains normative APIs, states, precedence, resources, commands, and
links. PDFs contain monochrome architecture/timing drawings, prediction
worksheets, staged experiments, diagnosis, exercises, and blank bench cards.

Before power, record exact revision, PCB faces, connectors/markings, primary
sources, pin order, supply/output topology, pull-up rail, polarity, impedance,
module LEDs, and current. Cracked glass, mercury/ambiguity, back-power, or
unresolved pinout remains quarantined. Place probes unpowered. Remove USB for
heat, odor, reset/rail instability, limit trip, out-of-rail output, unexpected
state, movement, specimen/schematic disagreement, loose or unretained magnetic
stimulus, cracked/exposed contact capsule, or current above any pin, port,
package, rail, or aggregate budget. Disconnect or reconfigure only with USB
removed. Inject faults in host fixtures; never create a live short.

Physical RTC/media, actual timestamps, exact Hall transfer behavior, and
persistence remain open. Compilation, replay, and PDFs are not bench evidence.
