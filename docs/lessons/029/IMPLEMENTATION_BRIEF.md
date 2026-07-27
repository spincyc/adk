# Lesson 029 implementation brief: inert cue schedule and audit

Status: implementation-ready design; host implementation and physical
acceptance remain open.

## Scope

Lesson 029 teaches a deterministic schedule, explicit operator confirmation,
bounded audit storage, stable record encoding, and replay. The core is a pure
value-driven state machine. It owns no pins, clock, display, stream, callback,
or dynamically allocated storage.

An inert cue is only a numbered visual interval. The supported Mega 2560
example presents cues through resistor-limited LEDs and presents scheduler
state through an RGB LED. The public core has no generic output adapter.

The lesson is E0 for host work and E1 for the canonical Mega circuit.
Lesson 028 is not a dependency; Lesson 030 composes both components.

## Public values

All declarations use fixed-width values, `struct`, `noexcept`, clean headers,
and out-of-line implementation.

```cpp
using InertCueId = uint8_t;

struct InertCue
{
    InertCueId id;
    Duration   offset;
    Duration   visibleFor;
};

struct InertCuePlan
{
    static const uint8_t capacity = 32;

    InertCue cues[capacity];
    uint8_t  count;
};

struct CueOperatorInput
{
    bool reviewHeld;
    bool runPressed;
    bool confirmPressed;
    bool skipPressed;
    bool cancelPressed;
};
```

IDs are opaque labels in the inclusive range 0--31. They do not identify a
pin, address, waveform, connector, or physical channel.

`runPressed`, `confirmPressed`, `skipPressed`, and `cancelPressed` are
debounced edge events supplied by the caller. `reviewHeld` is a sampled level.
The scheduler neither debounces nor infers edges.

## Configuration and ownership

```cpp
struct InertCueSchedulerConfig
{
    InertCuePlan plan;
    Duration     confirmationWindow;
};

struct CueAuditBuffer
{
    CueAuditBuffer (CueAuditEntry* storage, uint8_t capacity) noexcept;

    Status               initialize  () noexcept;
    void                 shutdown    () noexcept;
    bool                 initialized () const noexcept;
    uint8_t              count       () const noexcept;
    uint8_t              capacity    () const noexcept;
    Result<CueAuditEntry> entry       (uint8_t index) const noexcept;

private:
    friend struct InertCueScheduler;

    Status append (const CueAuditEntry& entry) noexcept;
};

struct InertCueScheduler
{
    InertCueScheduler (const InertCueSchedulerConfig& config,
                       CueAuditBuffer&                 audit) noexcept;

    Status               initialize  () noexcept;
    void                 shutdown    () noexcept;
    bool                 initialized () const noexcept;
    Status               update      (TimePoint now,
                                      const CueOperatorInput& input) noexcept;
    CueSchedulerSnapshot snapshot    () const noexcept;
};
```

The configuration is copied into the scheduler at construction. A caller may
discard or change its source value without changing the scheduler. The copied
plan is immutable until destruction. This prevents a schedule from changing
mid-run and keeps the maximum storage cost explicit.

The audit buffer is caller-owned and must outlive the scheduler. It is a
bounded append-only ledger, not an overwriting ring. It preserves every
accepted record through `shutdown()` and until its next `initialize()`. It
never allocates. Its public observation returns values, not mutable references.

Construction is inert. `initialize()` validates the complete copied
configuration and audit relationship before changing either object. It
initializes an uninitialized audit buffer, writes `Initialized`, and leaves the
scheduler in `Idle`. Any failure rolls both objects back to their pre-call
state. Repeated initialization succeeds without changing state or appending a
record.

The audit needs at least three entries. One is used by `Initialized`; one is
reserved for a capacity-induced `Held` record; and one is reserved for
`Shutdown`. An ordinary transition is accepted only when the buffer has space
for every record produced by that transition plus both reserves. A capacity
failure consumes the hold reserve, enters `Held`, records `Held` with
`CapacityExceeded`, and exposes no cue. No later cue can become active.
`shutdown()` always has its separate reserved record and appends exactly one
`Shutdown` record.

The scheduler does not shut down the caller-owned audit buffer. Its destructor
calls its own `shutdown()`, after which records remain observable. Before
reusing that storage for a new scheduler session, the caller shuts down the
audit buffer; its next `initialize()` clears the old entries. A scheduler
rejects an already initialized audit buffer so records cannot be erased
implicitly. Cleanup invokes no user code.

## Plan validation

A valid configuration satisfies every rule:

1. cue count is 1--32;
2. every cue ID is 0--31;
3. cue IDs are unique;
4. `confirmationWindow` is nonzero and less than `0x80000000` ticks;
5. every `visibleFor` is nonzero and less than `0x80000000` ticks;
6. every offset is less than `0x80000000` ticks;
7. offsets are strictly increasing;
8. `offset + visibleFor` does not overflow and is less than `0x80000000`;
9. visible intervals do not overlap:
   `previous.offset + previous.visibleFor <= current.offset`.

Validation failure returns `InvalidConfiguration`, appends nothing, and leaves
the scheduler uninitialized. Zero is a valid first offset.

## State and snapshot

```cpp
enum struct CueDecision : uint8_t
{
    Waiting,
    ConfirmationRequired,
    Active,
    Complete,
    Skipped,
    Held,
    Cancelled
};

enum struct CueSchedulerPhase : uint8_t
{
    Idle,
    Review,
    Waiting,
    Confirmation,
    Active,
    Held,
    Complete,
    Cancelled,
    Fault
};

struct CueSchedulerSnapshot
{
    CueSchedulerPhase phase;
    CueDecision       decision;
    InertCueId        cue;
    uint8_t           cueIndex;
    Duration          planElapsed;
    Duration          cueElapsed;
    Status            status;
    bool              hasCue;
};
```

`cue` and `cueIndex` are meaningful only when `hasCue` is true. `planElapsed`
is zero before an accepted run. `cueElapsed` is zero outside `Active`.
Snapshots remain stable until the next lifecycle or update call.

`Idle`, `Held`, `Complete`, `Cancelled`, `Fault`, and shutdown have no active
cue. `Fault` is reserved for invalid runtime input or an internal invariant
failure. Configuration errors do not initialize the scheduler and therefore
do not enter `Fault`.

## Clock and confirmation contract

The first update with `reviewHeld` true enters `Review`. An accepted
`runPressed` in `Review` establishes the plan epoch at that update's supplied
time and records `RunRequested`. Offsets are measured from that epoch.

When the next cue becomes due, the scheduler enters `Confirmation` and records
`ConfirmationRequested`. Its logical confirmation window starts at
`planEpoch + cue.offset`, even when a delayed update first observes that
boundary later. The window is inclusive: confirmation is timely when
`planElapsed <= cue.offset + confirmationWindow`.

A timely `confirmPressed` records `Confirmed` and `CueShown`, then enters
`Active`. Visibility lasts for exactly `visibleFor` ticks measured from the
accepted confirmation time. At the exact visibility boundary the cue becomes
inactive and `CueHidden` is recorded.

Offsets determine when a cue may first request confirmation; confirmation
latency does not move later offsets. If an earlier cue finishes after a later
offset, the delayed cue is considered immediately, but never becomes active
without its own confirmation.

When the confirmation window expires, that cue records `CueSkipped` with
`Timeout` status. An accepted `skipPressed` in `Confirmation` records
`CueSkipped` with `Ok` status. Both advance to the next cue. No update activates
more than one cue, and no delayed update activates a past cue as catch-up.

When a delayed update observes multiple unhandled offsets, it skips every cue
whose confirmation window would already have expired, in plan order, then
requests confirmation for the latest cue whose window can still be open. It
preflights audit capacity for the complete coalesced decision before changing
state. If all cues are past, it records their skips and enters `Complete`.

All duration comparisons use unsigned subtraction and remain within the
documented half-range. A timestamp that is earlier than the prior accepted
timestamp in half-range ordering returns `InvalidArgument`, enters `Fault`,
records `Faulted`, and exposes no cue. Repeated timestamps are valid.

## Input priority and hold behavior

For each update, policy is applied in this order:

1. accept `cancelPressed`, regardless of every other input;
2. reject two or more remaining pressed edge fields as an invalid chord;
3. apply release of `reviewHeld`;
4. accept the one phase-valid edge;
5. process elapsed-time boundaries.

An invalid chord enters `Fault`, records `Faulted`, and exposes no cue.
`cancelPressed` is valid in every initialized phase except `Cancelled`; it
records `Cancelled`, enters that terminal phase, and exposes no cue. Repeated
cancel in `Cancelled` is idempotent and appends nothing.

Releasing `reviewHeld` from `Review`, `Waiting`, `Confirmation`, or `Active`
immediately enters `Held`, records `Held`, clears pending confirmation, and
exposes no cue. Release at the same timestamp as a visibility boundary wins;
there is no `CueHidden` record because the `Held` transition explains why the
cue disappeared.

Restoring `reviewHeld` does not resume automatically. A single
`runPressed` in `Held` records `Resumed`, establishes
`planEpoch = now - nextCue.offset`, and returns to `Waiting` or
`Confirmation`.
Completed and skipped cues remain completed and skipped. The interrupted
active cue is skipped and is never resumed mid-interval.

Edges that are not meaningful in the current phase enter `Fault`. A held edge
cannot repeat because edge qualification belongs to `Button`; tests still
inject repeated true values to prove the scheduler's explicit rejection.

`Complete`, `Cancelled`, and `Fault` are terminal. A fresh session requires
scheduler shutdown, audit-buffer shutdown, and initialization in that order.
It starts at cue zero with sequence zero in the freshly initialized empty audit
buffer.

## Audit contract

```cpp
enum struct CueAuditEvent : uint8_t
{
    Initialized,
    ReviewStarted,
    RunRequested,
    ConfirmationRequested,
    Confirmed,
    CueShown,
    CueHidden,
    CueSkipped,
    Held,
    Resumed,
    Cancelled,
    Faulted,
    Completed,
    Shutdown
};

struct CueAuditEntry
{
    uint32_t      sequence;
    TimePoint     recordedAt;
    CueAuditEvent event;
    InertCueId    cue;
    uint8_t       cueIndex;
    Status        status;
    bool          hasCue;
};
```

Sequence numbers begin at zero and increase by one. Every state-changing
decision has one record, except confirmation, which intentionally has
`Confirmed` followed by `CueShown` at the same timestamp. Completion records
`Completed`. Entries at one timestamp retain the enum order shown by the
transition contract, never container iteration order.

`recordedAt` is the supplied update time. `Initialized` uses zero because
initialization receives no time. `Shutdown` uses the most recently accepted
time, or zero if no update was accepted. Cue fields are meaningful only when
`hasCue` is true.

An update either appends its complete ordered record set and changes state, or
does neither. Audit capacity cannot leave a half-applied transition.

## Stable record grammar

`CueAuditEncoder` writes one complete ASCII record into caller-provided
storage:

```text
adk-cue,1,<sequence>,<ticks>,<event>,<cue-or-dash>,<index-or-dash>,<status>\n
```

The encoder is locale-independent, allocation-free, and deterministic.
`requiredSize(entry)` includes the newline but excludes a terminating null.
`encode(entry, output, capacity)` returns `Result<uint8_t>`. It rejects null
storage with nonzero capacity and returns `CapacityExceeded` without writing
when the complete record does not fit. It never emits a partial record and
does not append a null byte.

Event and status spellings are fixed lowercase ASCII tokens. The HTML
reference will publish the complete token table. Version 1 must not be changed;
a later grammar uses a new version.

## Mega 2560 narrative example

Object order is:

```text
Mega platform
  -> resource registry
  -> four Button components
  -> eight MonoLed cue indicators
  -> one RgbLed state indicator
  -> caller-owned audit storage
  -> CueAuditBuffer
  -> InertCueScheduler
```

`setup()` reads as acquire, configure, start. `loop()` reads as observe,
decide, present:

```cpp
void loop ()
{
    const TimePoint        now   = observeTime ();
    const CueOperatorInput input = observeOperator ();

    decideSchedule (now, input);
    presentCue      ();
    presentState    ();
}
```

The sketch uses a fixed three-cue plan. It contains no blocking delay and no
dynamic allocation. Serial may print encoded audit entries, but unplugging the
Serial monitor changes neither decisions nor visible evidence.

The initial LED sweep proves output acquisition. It does not prove scheduler
correctness. After the sweep, blue means review, amber means confirmation,
green means a cue interval, violet pulse means held, red means cancelled or
fault, and off means idle or shutdown. Only one cue LED may be lit.

TP29 is the selected cue LED anode relative to Mega GND. The learner predicts
its level before confirming, observes it during the configured interval, then
interprets whether timing and cue selection match the snapshot. The separate
safe-state check confirms every LED endpoint is high impedance after
shutdown.

## Deterministic host test matrix

Tests must cover:

- every plan rule independently, plus a valid capacity-32 plan;
- copied-plan ownership after caller mutation and caller lifetime end;
- inert construction, initialization, repeated initialization, rollback,
  repeated shutdown, destruction while active, and reinitialization;
- null, zero, one-entry, exact, and exhausted audit storage;
- reserved shutdown capacity and atomic multi-record append;
- every phase, decision, event, and legal transition;
- the exact four-step review, run, inspect, confirm narrative;
- first offset zero and nonzero;
- one tick before, at, and after every due, confirmation, and visibility
  boundary;
- repeated timestamps, rollover on every timed boundary, and rejected reverse
  time;
- delayed updates with one and many elapsed cues;
- proof that delayed updates never create an activation burst;
- every single edge in every phase;
- every two-edge and larger chord;
- cancel priority, review-release priority, and release at a visibility
  boundary;
- hold from `Review`, `Waiting`, `Confirmation`, and `Active`;
- explicit resume with completed, skipped, pending, and interrupted cues;
- timeout skip, operator skip, and completion after the final cue;
- terminal behavior and restart from `Complete`, `Cancelled`, and `Fault`;
- snapshots with valid and intentionally absent cue fields;
- audit sequence, timestamps, cue presence, status, and same-time ordering;
- encoder minimum and maximum numeric fields, absent cue fields, every token,
  exact capacity, short capacity, null storage, and unchanged output on
  failure;
- two complete replays with byte-identical snapshots, audit entries, and
  encoded records;
- fixed-capacity and generated deterministic traces with the seed and minimal
  replay prefix reported on failure.

Tests compile with C++11, warnings as errors, exceptions disabled, and RTTI
disabled. Public headers compile alone.

## Lesson and circuit evidence

HTML is the searchable contract:

- public values and lifecycle;
- state and priority tables;
- complete record-token table;
- source, example, test, PDF, and trace links;
- copyable CLI commands;
- explicit host-verified and bench-open status.

The black-and-white PDF is the bench companion:

- pencil-style orientation drawing and separate exact schematic;
- prediction prompts for review, confirmation, visibility, hold, and shutdown;
- state worksheet, timing diagram, audit worksheet, and replay exercise;
- one fault-injection exercise and one delayed-loop exercise;
- TP29 measurement table;
- separate acquisition and safe-state evidence;
- troubleshooting tree;
- open Mega 2560 acceptance record.

The PDF and HTML both state the pin table, E1 limits, API contract, and stop
conditions. Neither claims measured hardware behavior before a completed bench
record.

## Electrical and safety boundary

The canonical circuit uses one USB-powered Mega 2560, four momentary-button
inputs, eight cue-LED outputs, three RGB-LED outputs, and one resistor of at
least 330 ohms in series with every LED channel. Exact pins must pass the Mega
capability and resource review before the sketch lands. Total simultaneous LED
current is calculated from the selected pin plan; the design permits one cue
LED and one RGB channel at a time.

Wire only with USB power removed. Stop on a hot part, unstable USB connection,
unexpected LED, incorrect polarity, missing resistor, or disagreement between
schematic and breadboard. Remove USB power to stop the circuit; software
shutdown is only an observable lifecycle state.

The lesson uses only its documented buttons, LEDs, optional character display,
and named test point. It does not connect to external equipment or export an
electrical action seam.

## CLI acceptance

Implementation is ready for integration only after these commands pass:

```sh
make style
make headers-check
make host
make sanitize
make arduino
make size-check
make lessons
make lessons-check
make site-check
make check
```

Record the Mega 2560 flash and static-RAM result against a lesson-local budget.
Run the package smoke gate at the integration boundary so the installed
archive proves that the header, implementation, and example are self-contained.
Physical acceptance remains explicitly open until the published bench card
contains measured observations.
