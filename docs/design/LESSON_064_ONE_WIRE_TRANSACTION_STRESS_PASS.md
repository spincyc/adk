# Lesson 064 1-Wire transaction architecture stress pass

Status: implementation reconciliation; the E0 copied intent/receipt policy
matches the boundaries below and awaits final review against a frozen core,
while every powered endpoint and physical timing claim remains open.

Lesson 064 introduces a bounded transaction policy for the listed DS18B20
family. It does not introduce a GPIO driver, delay-based bit banging, a
generic captured-protocol player, a temperature decoder, or a claim that an
inventory label identifies a physical specimen.

## Boundary under stress

| Pressure | Disposition |
|---|---|
| API and layering | Natural only as a copied intent/receipt state machine. The policy expands a closed typed operation into reset, presence, write, read, release, and sample phases. It owns no pin or endpoint. Lesson 065 owns ROM qualification, scratchpad interpretation, conversion freshness, and temperature values. |
| Ownership and lifecycle | One fixed configuration, one copied operation, one outstanding phase intent, and one copied receipt. Construction is inert. `initialize` and `reset` invalidate prior generations and enter cleanup while publishing a canonical release-line intent. `shutdown` enters a closing cleanup state that remains initialized only to accept its correlated release receipt; `confirmCleanup()` then makes the policy inert. None of these transitions claims that a physical line moved. No caller pointer, callback, heap, virtual dispatch, transport reference, or hidden acquisition is retained. |
| Time and ordering | All protocol timing uses a dedicated unsigned 32-bit microsecond time domain. Existing millisecond `TimePoint` is not reused. Every configured duration is nonzero where required and strictly below the modular half range. Equal timestamps are idempotent only for byte-identical receipts; backward time, exact half-range ambiguity, changed duplicates, sequence regression, and generation exhaustion reject atomically. |
| Commands | The public request is a closed enum of reviewed DS18B20-family bus operations. No API accepts captured pulse trains, arbitrary replay frames, or unknown protocol bytes. Raw slot mechanics remain private implementation details. Adding an opcode is an architecture and safety review boundary. |
| Electrical behavior | Not applicable at E0. `DriveLow`, `Release`, and `Sample` are semantic intents. No intent means output-high drive, internal pull-up sufficiency, open-drain conformance, rise time, rail voltage, or physical safe state. |
| Parasite power | `Unsupported` at this boundary. No E0 operation requests a strong pull-up, and no E1 adapter may reinterpret ordinary release or output-high intent as strong-pull power. Parasite operation requires a separate reviewed power capability and stress pass. |
| Errors and status | Structural, lifecycle, timing, ordering, receipt-correlation, and producer failures use `Status`. Presence absence, bit value, timeout, incomplete cleanup, and collision remain attributed transaction outcomes. A failed phase does not silently advance the operation. |
| Rollback | Failure, cancellation, timeout, reset, and shutdown request line release and invalidate the active transaction. Only `confirmCleanup()` with the matching copied release receipt may report cleanup observed or close a shutdown lifecycle. Without that receipt, the result remains `ReleaseUnconfirmed`; it never claims that the physical bus is idle or de-energized. |
| Resources | Fixed storage and bounded service work are mandatory. `update()` validates at most one receipt and emits at most one next intent. Receipt-free `advance()` emits at most one deadline cleanup intent, and `confirmCleanup()` validates at most one release receipt. None contains polling, retry, catch-up, search, recursion, or a delay loop. |
| Downstream effects | Lesson 065 consumes only attributed successful transaction results and separately proves ROM identity, CRC, discovery, conversion, and scratchpad semantics. Lesson 066 consumes qualified Lesson 065 observations. Lesson 067 inherits no 1-Wire, ROM, power, or thermal meaning. |

## Frozen E0 operation surface

The exact spelling may change during header review, but the information
boundary may not be weakened silently.

```cpp
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
```

An operation request carries a nonzero request sequence, typed operation,
optional complete 64-bit ROM identity where the operation is addressed,
supplied start time, external-power mode, and producer `Status`. Owner token
and configuration revision are frozen policy configuration; lifecycle and
transaction generations are policy-assigned. Search state is a fixed copied
value, not a callback or dynamically growing device list.

Each emitted phase intent binds the complete request identity, transaction
generation, phase sequence, line intent, earliest and latest legal action
times, and whether a sample is required. Each copied receipt repeats that
correlation, supplies its observation time, observed line level when
applicable, completion disposition, and producer `Status`.

The E0 policy accepts only externally powered operation. A request naming
parasite power returns `StatusCode::Unsupported` before transaction state
changes. `Copy Scratchpad` is intentionally absent: Lesson 064 has no need to
write EEPROM, consume endurance, or introduce the strong-pull requirement for
nonvolatile copying.

`ReadRomSingleDrop` is permitted only under an explicit single-drop
configuration. Multidrop identity uses bounded `SearchRomPass`; enumeration
order is evidence order, never physical position. `MatchRom` operations
require a complete nonzero ROM value supplied by the caller. Lesson 064
transports that identity but does not declare its family code or CRC valid.

## Microsecond transaction model

The policy uses modular microsecond values because DS18B20-family bus slots
cannot be represented by the repository's millisecond clock. The initial
configuration freezes reviewed timing windows for:

- reset-low duration, release, presence-start, and presence-low observation;
- write-zero and write-one low duration;
- read-slot initiation and sample window;
- complete slot duration and inter-slot recovery; and
- an overall operation deadline.

The deterministic model covers every equality immediately below, at, and
above each lower and upper boundary. It also covers timestamp rollover,
backward time, the exact modular half range, late receipts, early receipts,
duplicate receipts, phase-sequence exhaustion, cancellation, and shutdown.

These values are protocol acceptance windows in copied evidence. They are not
proof that a Mega pin met them. E1 must measure interrupt latency, drive-low
voltage, release rise time, sampling position, presence timing, and the
complete slot waveform at named test points.

No service call waits for a deadline. Normal phase transitions require one
copied receipt through `update()`. The receipt-free `advance(now, intent)`
path observes only supplied time: before a deadline it leaves an owed phase
unchanged, and at a deadline crossing it emits at most one correlated release
intent. It never accepts or invents phase evidence, polls, retries, or
synthesizes a cleanup receipt.

`MatchRomReadConversionStatus` is a distinct allowlisted continuation, not a
new reset/Match ROM/Convert T transaction and not a hidden poll loop. It is
admitted only immediately after a successful
`MatchRomStartConversion` transaction and its final release confirmation,
with the same nonzero ROM copied in the new request. That request-bound ROM
proves which completed conversion is being continued; it is not repeated in
the phase receipt. One continuation performs exactly one direct read slot and
reports the copied status bit. Another status request requires another
completed conversion request; the policy never retries or polls implicitly.

## Atomicity and cleanup

Structural validation precedes phase semantics. A malformed or foreign
receipt leaves the accepted request, phase, deadline, result, and cleanup
state byte-identical. A valid producer failure records its attributed failure
and moves to cleanup without pretending the phase succeeded.

The policy never drives high. Its canonical inactive and cleanup intent is
`Release`. The distinction is mandatory:

| Evidence | Permitted claim |
|---|---|
| policy emitted `Release` | software requested release |
| matching copied release receipt | producer reported release application |
| E1 voltage/timing observation | physical line reached the qualified idle window |

Reset and shutdown invalidate all earlier receipts. Reinitialization advances
the lifecycle generation. Before admitting a transaction, the policy reserves
the complete phase-sequence budget and rejects lifecycle, transaction,
phase-sequence, or prior-receipt-sequence exhaustion atomically with
`CapacityExceeded`; no counter wraps and no new operation is partially
started. Cleanup transitions likewise reject if their generation or phase
sequence cannot advance. An already emitted cleanup intent remains owed until
its matching receipt; exhaustion never fabricates confirmation.

Initialization, reset, rollback, timeout, cancellation, and shutdown can
leave a correlated release receipt outstanding. `confirmCleanup(now,
receipt)` is the sole closing seam for that receipt: it rejects an early,
late, foreign, changed, or duplicate receipt atomically, records producer
failure without claiming release, and confirms cleanup only from the matching
copied successful receipt. Shutdown remains in its initialized closing state
until that confirmation, then becomes inert. Reset becomes transaction-ready
only after its cleanup confirmation. Receipt-free `advance()` cannot bypass
either lifecycle seam.

## Deterministic proof matrix

Host tests must include:

- every timing boundary and exact-half-range ambiguity;
- reset with no presence, valid presence, early/late presence, and stuck-low
  evidence;
- write-zero, write-one, and read-slot phase ordering;
- both read values without treating either as a transport failure;
- exact `MatchRomReadConversionStatus` continuation-only direct read-slot
  trace, same-ROM request binding, rejection without the immediately prior
  confirmed conversion, both copied status-bit values, and no implicit retry;
- fieldwise changed duplicates and byte-identical idempotence;
- request-bound ROM, phase, lifecycle, owner, configuration, and receipt
  correlation failures, without pretending the receipt carries a ROM field;
- single-drop `ReadRom` rejection when multidrop is configured;
- bounded Search ROM branch, collision, completion, cancellation, and replay
  fixtures without assigning location meaning to enumeration order;
- timeout, producer failure, cancellation, reset, and shutdown through
  `ReleaseUnconfirmed` and confirmed-release outcomes;
- `advance()` immediately below, at, and above the operation deadline,
  including an owed ordinary receipt, repeated calls, a single correlated
  release intent, and proof that no receipt or successful phase is
  synthesized;
- `confirmCleanup()` with early, late, foreign, fieldwise changed, failed,
  matching, and duplicate release receipts, including reset readiness and
  shutdown remaining initialized while closing and becoming inert only after
  confirmation;
- `completedEvidence()` remaining busy after successful operation semantics
  until the exact final `Release` receipt is confirmed, then publishing the
  successful copied operation evidence;
- rejection of parasite-power and every invalid enum value before mutation;
- exhaustion before sequence or generation wrap; and
- two independent policies producing byte-identical canonical witnesses from
  the same copied trace.

The compile-only Mega replay uses supplied microsecond values, copied line
receipts, and named volatile result cells. It owns no pin and must not be
uploaded as a bus driver.

## Initial resource gates

The detailed Lessons 064--066 plan may tighten these values but may not raise
a hard limit without another stress review.

| Metric | Target | Hard limit |
|---|---:|---:|
| ordinary sketch flash | 10 KiB | 14 KiB |
| ordinary static SRAM | 768 B | 1,024 B |
| isolated synchronous stack | 320 B | 448 B |
| policy object | 192 B | 256 B |
| each caller-owned intent/receipt/search buffer | 128 B | 256 B |

Exact probes record ordinary and no-LTO flash/static SRAM, policy and public
value sizes, caller-owned buffers instantiated once, compiler-callgraph
synchronous stack including retained return-address edges, residual Mega
SRAM, and the complete compiler/core/flag fingerprint. Heap use, recursion,
indirect calls, unknown callgraph edges, dynamic stack, or stale reviewed
target markers fail the gate. Hard-limit and residual-hard-floor failures are
not reviewable.

## E1 reopen triggers

Any of the following requires a separate powered-adapter review and recorded
bench acceptance:

- choosing an exact DS18B20 specimen, package, pin order, supply, or pull-up;
- claiming that a kit label or lookalike device is a genuine DS18B20;
- owning a Mega GPIO, timer, interrupt policy, or open-drain emulation;
- selecting a weak pull-up value or claiming bus length, capacitance, device
  count, rise time, or noise margin;
- adding parasite power, a MOSFET strong pull-up, EEPROM copying, or a
  high-current bus phase;
- permitting mixed external/parasite power on one bus;
- polling conversion completion or sharing the bus during conversion;
- making a temperature, accuracy, resolution, conversion-freshness, ROM CRC,
  scratchpad CRC, multidrop identity, or physical-position claim; or
- adding arbitrary opcodes, raw captured frames, or unknown-protocol replay.

Initial E1 should use an externally powered, exact reference specimen and a
qualified external weak pull-up. The adapter must prove drive-low and release
behavior, loaded high/low rails, rise time, reset/presence/slot timing,
interrupt interference, timeout cleanup, startup, reset, shutdown, stuck-low
handling, and an independent physical line-release observation. Parasite
power remains closed until its separate power endpoint proves the required
handoff timing, conversion hold, exclusivity, current/rail margin, stuck-on
handling, and independent removal.

## Gate result

- Disposition: `natural fit` for an E0 copied intent/receipt policy with a
  dedicated microsecond clock and typed allowlisted operations
- Promotion permitted: yes for E0 design and implementation after the
  Lessons 064--066 plan freezes the public types; no for a powered adapter,
  parasite power, temperature decoding, or physical identity claims
- Open risks: exact E1 specimen identity, counterfeit/compatible parts,
  microsecond scheduling latency, pull-up and topology, ROM search capacity,
  line cleanup evidence, and aggregate Lesson 066 resources
- Remediation owner and next action: Lesson 064 implementation lane freezes
  the header and exhaustive copied trace before any endpoint work
- Verification status: document review only; implementation, exact resource,
  publication, and physical checks remain pending
