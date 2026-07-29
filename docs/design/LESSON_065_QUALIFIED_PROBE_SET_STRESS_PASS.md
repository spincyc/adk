# Lesson 065 qualified-probe-set architecture stress pass

Status: initial pre-implementation review; Lesson 064 transaction evidence,
exact powered specimens, electrical adapters, and bench acceptance remain
open.

This pass reviews the Lesson 065 subject fixed by the
[extended component/project cadence](../projects/component_project_cadence.md):
a fixed-capacity set policy for qualified 18B20 observations.

`Qualified18B20ProbeSetPolicy` is a natural component-layer policy only when it
consumes complete copied Lesson 064 transaction evidence and preserves four
configured ROM identities in stable configured order. It owns no single-wire
bus, GPIO pin, pull-up, strong-pull-up switch, power rail, clock, transport
buffer, sensor, or discovery operation.

## Boundary

- Name and lesson/project: `Qualified18B20ProbeSetPolicy`, Lesson 065
- Reviewer and date: initial architecture review, 2026-07-29
- Public responsibility: qualify one bounded copied search/conversion/read
  cycle against four configured ROM identities, then publish four stable
  attributed observation slots
- Direct dependencies: `Status`, `TimePoint`, fixed-width values, and complete
  copied Lesson 064 transaction/search evidence
- Existing decisions reconsidered: endpoint/component layering, explicit
  supplied time, deterministic ordering, fixed storage, source provenance,
  circuit-native observation, the distinct unidentified Lesson 062 Digital
  Temperature family, and the Lesson 064 transport boundary

The capacity is exactly four configured probes. Construction fixes all four
ROM codes, expected resolutions, maximum age, and maximum plausible
per-conversion step. Runtime search order never changes slot identity or
capacity. A future need for fewer probes uses a separate explicitly designed
boundary; zero-filled or wildcard identities are not hidden optional slots.

## Frozen E0 values and operations

The public value model must retain these facts without caller pointers:

- `Ds18b20Rom`: the complete eight ROM bytes;
- `Ds18b20Resolution`: exactly 9, 10, 11, or 12 bits;
- one configuration record for each of the four ROMs, including expected
  resolution, maximum age, and maximum step in sixteenths Celsius;
- one copied set envelope with nonzero source/configuration/cycle identity,
  supplied observation time, up to four chained copied `SearchRomPass`
  request/result witnesses, explicit completion/over-capacity state, and one
  role-specific conversion/read record per returned device;
- each copied transaction's Lesson 064 owner, lifecycle/configuration,
  transaction sequence/generation, addressed ROM, operation, reset/presence,
  exact byte count and bytes, start/completion times, disposition, and complete
  `Status`; and
- one four-slot snapshot retaining configured ROM, expected resolution, latest
  copied transaction attribution, decoded signed temperature in sixteenths
  Celsius, age, current-cycle presence, quality, and status.

The required lifecycle is inert construction, `initialize()`, `reset()`,
caller-owned `update(now, envelope, result)`, `snapshot()`, and
`initialized()`. The policy is non-copyable and non-movable, allocates no heap
memory, invokes no callback, reads no clock, and performs no transaction.
Reset retains the four configured identities but clears ordering, conversion,
last-value, presence, and step history to four `Unqualified` slots.

The quality vocabulary must distinguish at least:

- `Unqualified`;
- `Current`;
- `ConversionPending`;
- `RomCrcFault`;
- `ScratchpadCrcFault`;
- `ResolutionMismatch`;
- `ResetDefaultWithoutConversion`;
- `Stale`;
- `Missing`;
- `TransportFault`;
- `DuplicateIdentity`; and
- `ImplausibleStep`.

Structural configuration, identity, lifecycle, ordering, and malformed-value
failures use `Status`. The snapshot retains every side-specific domain outcome
even when a deterministic configured-slot precedence selects one returned
status.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | Natural only as copied-evidence policy above Lesson 064. ROM validation, CRC, conversion correlation, stable set membership, freshness, and step policy are component meaning. Reset/presence slots, ROM commands, bit timing, GPIO ownership, pull-up switching, and rollback stay in Lesson 064 or a future exact adapter. |
| Ownership and lifecycle | Natural with four value-owned configurations and four copied result slots. Construction is inert; initialize validates all four identities and thresholds; reset clears volatile evidence without changing configured slot meaning. No borrowed transport or sensor lifetime exists. |
| Time and ordering | Natural with supplied policy time plus copied transaction start/completion times. Complete-cycle sequence, each transaction generation, conversion/read correlation, freshness, disappearance, duplicate replay, rollover, half-range ambiguity, and exhaustion are explicit and bounded. |
| Errors and status | Natural if structural rejection is atomic while valid unhealthy evidence remains typed quality. A CRC fault, pending conversion, missing probe, stale value, and transport failure are not interchangeable and never become an extreme temperature. |
| Resource budget | Four ROMs, four configuration records, four observations, one bounded input/result pair, and no dynamic discovery storage. Exact AVR ABI, ordinary sketch, isolated no-LTO, stack, object, caller-buffer, and Lesson 066 aggregate measurements are promotion gates. E0 claims zero pins, timers, interrupts, buses, registry entries, or power resources. |
| Deterministic proof | Natural: every decision is reproducible from fixed configuration, copied bytes/statuses, sequences, and supplied time. CRC, conversion generation, stable ordering, disappearance, mixed resolution, duplicate identity, step, rollover, reset, and replay all have exhaustive finite fixtures. |
| Packaging and public surface | One standalone header/implementation, umbrella export, host target, compile-only Mega replay, exact resource probe, HTML reference, and complementary pencil-drawing PDF. No single-wire implementation, adapter, wiring, or formal schematic belongs in Lesson 065 E0. |
| Example and documentation fit | The Mega sketch replays copied cycles into named volatile result cells in configured ROM order. Predict/observe/interpret covers current, CRC fault, pending, missing, reappearance, and step outcomes. Compilation and memory cells do not prove a powered probe. |
| Downstream effects | Lesson 066 consumes the four stable slots and may page/display/record by ROM identity rather than discovery order. Lesson 062 remains distinct and unidentified. Lesson 064 transport remains below this policy. RTC/SD persistence and exact LCD/probe fixtures retain their own gates. |

## Identity, CRC, and stable ordering

Each configured ROM is exactly eight bytes:

1. byte 0 must be the listed 18B20 family code `0x28`;
2. bytes 1--6 retain the complete serial identity; and
3. byte 7 must equal the specified CRC-8 of bytes 0--6.

All four complete ROMs must be distinct. Invalid family or ROM CRC is invalid
configuration for a configured role. The policy never repairs a ROM, trusts a
printed serial number instead of bytes, truncates identity to a hash, or uses
discovery order as identity.

Every successful complete search cycle contains up to four retained Lesson 064
pass witnesses. Each witness copies both its request `OneWireSearchState` and
its completed `searchResult`. The first request is empty; every later request
must equal the preceding completed result fieldwise, and no pass may follow a
result with `lastDevice == true`. The terminal retained result must set
`lastDevice` when the cycle is complete. Returned ROMs are derived only from
these completed results, then CRC-validated before matching; there is no
parallel caller-supplied ROM list.

A configured ROM always maps to its configured slot, regardless of derived
search-result permutation. Repeating one derived ROM in the same complete
search is `DuplicateIdentity`, never silent de-duplication. A valid unexpected
ROM remains bounded, attributable set-fault evidence and is never inserted or
substituted for a missing configured probe. An explicit over-capacity marker
is valid only when four retained results end with `lastDevice == false`, which
proves another result exists beyond the envelope. It is mutually exclusive
with a complete enumeration and cannot silently discard the extra device.

A scratchpad read is exactly nine bytes. Its final byte must equal the
specified CRC-8 of the preceding eight before temperature, resolution, or
configuration is interpreted. One corrupt byte cannot partially refresh
temperature, freshness, conversion, presence, or step baseline.

## Conversion and resolution

Conversion request, completion evidence, and scratchpad read must correlate on
the exact ROM, Lesson 064 owner/lifecycle/configuration, and one nonzero
conversion generation. Evidence from different probes, lifecycles, or
generations rejects before mutation.

The scratchpad configuration resolution must be one of 9, 10, 11, or 12 bits
and must equal that configured slot's expected resolution. The decoder treats
resolution-dependent low temperature bits only according to the proved
scratchpad resolution. Mixed resolutions across the four slots are supported;
one slot's deadline or bit mask is never applied to another.

`ConversionPending` retains the previous trusted temperature and its original
observation time and age. It does not read an incomplete scratchpad, refresh
freshness, or publish a cold/default value. The documented `+85 °C` power-on
register value is `ResetDefaultWithoutConversion` only when no exact matching
completed conversion precedes the read. The same numeric value after a fully
correlated completed conversion is a valid possible reading and cannot be
rejected by heuristic.

Freshness begins at the correlated completed scratchpad observation time, not
policy-update, search, request, or polling time. CRC failures, transport
failures, incomplete conversion, and duplicate replay do not refresh it. Age
at the configured maximum remains current; one millisecond older is `Stale`.

## Missing, reappearance, and step semantics

Only a structurally complete successful chained search cycle with a terminal
`lastDevice` result can prove that a configured ROM is `Missing`. A failed,
truncated, discontinuous, unterminated, capacity-exceeded, or otherwise
incomplete search is `TransportFault`; it cannot prove absence. Exactly four
configured slots does not require four discoveries: a terminal enumeration of
one through four devices can prove absent configured identities. Encountering
a fifth device exceeds the bounded evidence capacity and cannot prove the
four-slot set complete. A zero-pass envelope cannot claim a complete
enumeration because it has no completed Lesson 064 search result; no-presence
evidence remains `TransportFault` and cannot prove all four configured roles
missing at this boundary.

A missing slot retains its configured identity, last trusted temperature,
original observation time, and increasing age. It never collapses the array,
changes index, or allows another ROM to take its place. Reappearance requires
forward, exact-ROM, CRC-valid, conversion-correlated evidence and returns in
the same slot.

Step comparison applies only between consecutive CRC-valid, correlated,
completed conversions for one ROM. Absolute signed subtraction uses widened
arithmetic. A delta at the configured maximum is accepted; one sixteenth
larger is `ImplausibleStep`. This is a domain warning, not proof that the
physical change was impossible. The snapshot retains the new decoded value
and both transactions' attribution, and the new valid sample becomes the next
comparison baseline so one real step cannot permanently lock the slot.

No Lesson 065 value claims medical, food, preservation, process-control, hot
surface, immersion, absolute accuracy, response-time, or waterproof behavior.

## Ordering and atomicity

The complete set envelope has nonzero source/configuration/cycle identity.
Equal cycle sequence and observation time accept only a byte-identical full
duplicate. A changed duplicate rejects atomically. An identical duplicate at
a later valid policy time is idempotent: it does not extend conversion state,
refresh a value, age a disappearance counter, or change step history.

Forward sequence and time use modular half-range ordering. Regression,
backward time, exact-half-range ambiguity, source/configuration/lifecycle
change, sequence exhaustion, a search-pass count above four, crossed conversion
identity, contradictory search completion/over-capacity flags, broken
request/result search continuity, or any partial transaction rejects the
complete update without advancing one slot. A structurally valid explicit
over-capacity marker on four nonterminal results instead commits bounded
`TransportFault` evidence; it never masquerades as a complete search.

A structurally valid complete cycle commits all four side outcomes together.
One CRC fault or missing slot does not erase three current slots. Deterministic
returned-status precedence follows configured slot order after set-level
structural and transport failures; the complete snapshot retains every
collision.

## Deterministic proof matrix

Host tests must include:

- four configured ROMs with literal known-good CRCs, family mismatch, one
  corruption in every ROM byte, and duplicate configuration;
- search permutations across all four identities proving invariant slot order;
- exact initial state, request/result continuity, early and missing
  `lastDevice`, a pass after terminal state, and explicit fourth-result
  nonterminal over-capacity;
- derived result counts below and at four, explicit beyond-four
  over-capacity, and unknown, missing, duplicated, and unknown-plus-missing
  combinations;
- complete successful search versus reset failure, truncated search, partial
  ROM, transport failure, and capacity failure;
- scratchpad CRC vectors plus one corruption in every scratchpad byte;
- positive, negative, zero, minimum, maximum, and sign-extension vectors at
  9-, 10-, 11-, and 12-bit resolution;
- all four resolutions in one set, resolution mismatch, and
  resolution-specific low-bit handling;
- conversion request/pending/completion/read at immediately before, at, and
  after each resolution-dependent deadline;
- crossed ROM, generation, owner, lifecycle, configuration, transaction
  sequence, and completion evidence;
- `+85 °C` without a completed conversion and the same value after a
  correlated completed conversion;
- present-to-missing-to-reappeared traces with complete and incomplete search;
- freshness immediately below, at, and one past maximum age;
- positive and negative steps immediately below, at, and one sixteenth above
  the configured magnitude;
- one bad slot with three current slots, all simultaneous quality collisions,
  and deterministic configured-slot status precedence;
- byte-identical duplicate, changed duplicate, regression, rollover,
  exact-half-range ambiguity, sequence exhaustion, reset, restart, and
  byte-identical full replay.

The compile-only Mega replay copies fixed transactions and observations into
named volatile result cells. It requests no bus or hardware resource and never
uploads to a powered fixture as part of E0 acceptance.

## Composition pressure

The maximum authorized fixture is the Lesson 066 thermal-gradient mapper with
all four configured probes represented, mixed 9/10/11/12-bit evidence, one
conversion pending, one scratchpad CRC fault, one disappeared and reappearing
ROM, one implausible step, discovery permutation, a simultaneous display/record
decision, timestamp rollover, reset, and restart with faults present.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | Applicable. One bounded four-slot update performs no polling, retry, bus search, catch-up, or blocking conversion. Prove fixed iteration bounds, resolution-specific copied deadlines, simultaneous cycle handling, rollover, and missed-update freshness. |
| Total memory and hardware resources | Applicable. Measure four configurations, four observations, complete copied envelope/result, policy object, canonical sketch static SRAM/flash, conservative stack, and linked Lesson 066 maximum composition. E0 hardware claims remain exactly zero. |
| Shared bus or transport | Not applicable inside Lesson 065: the API accepts only completed copied Lesson 064 values and cannot call, borrow, arbitrate, or restart a bus. Lesson 064 and E1 own any physical shared-bus pressure. |
| Persistence and recovery | Not applicable: configured identities are firmware configuration and all observation/conversion history is volatile. Lesson 065 makes no RTC, SD, EEPROM, media, power-loss, or durable-record claim. |
| Motion, external power, or stored energy | Not applicable at E0 because no actuation or supply path exists. Parasite power, strong pull-up, switched rail, immersion, and thermal stimuli remain E1 exclusions until exact qualification. |
| Observation identity and provenance | Applicable. Every slot retains complete ROM, source/configuration, Lesson 064 owner/lifecycle/transaction/conversion generation, copied completion time, scratchpad CRC, expected resolution, age, quality, and status. |
| Diagnostic interference | Applicable in Lesson 066. Named result cells, future LCD/LED intent, Serial, and record intent cannot change CRC, identity, freshness, step, or disappearance decisions. Their exact resources remain aggregate gates. |
| Failure collision and recovery | Applicable. Replay simultaneous incomplete search, duplicate/unknown ROM, pending conversion, scratchpad corruption, missing configured identity, stale value, and step warning. Structural failure rejects atomically; valid side faults remain independently attributable; reset returns four stable unqualified slots. |

## Initial resource budgets

These are promotion gates, not measurements:

| Boundary | Target | Hard limit |
|---|---:|---:|
| ordinary Mega replay flash | 12 KiB | 16 KiB |
| ordinary Mega replay static SRAM | 1,024 B | 1,536 B |
| conservative synchronous stack | 448 B | 640 B |
| `Qualified18B20ProbeSetPolicy` object | 512 B | 768 B |
| each caller-owned envelope/result buffer | 256 B | 512 B |
| residual Mega SRAM after static, stack, and 128 B reserve | 3,072 B | 2,048 B floor |

The exact gate must fingerprint compiler, core, flags, sources, commands, and
input evidence; assert all public enum and structure sizes/alignments/traits;
prove policy non-copy/non-move; include retained large-return call edges; and
instantiate caller-owned buffers exactly once. The Lesson 066 plan must freeze
its aggregate target before Lesson 065 promotion. A target miss requires an
independent reviewed disposition; a hard or residual-floor failure is not
reviewable.

## E1 specimen and powered-adapter gates

E0 does not authorize wiring, a formal schematic, a powered Mega, or a probe.
E1 may begin only after each exact specimen and the complete shared fixture are
qualified. Required evidence includes:

- exact manufacturer marking, package, ROM/family behavior, datasheet, cable
  and encapsulation construction, and incoming inspection;
- local versus parasite supply mode, pin order, supply limits, pull-up value,
  bus capacitance/length, low/high rails, sink current, and reset/presence/slot
  timing;
- exact strong-pull-up topology, enable timing, current, contention
  prevention, rollback, shutdown, and physical power removal when parasite
  mode is proposed;
- resolution-dependent conversion timing, scratchpad bytes/CRC, repeated
  conversion behavior, simultaneous four-device operation, disappearance,
  reconnect, and fault injection;
- independent non-Serial conversion/activity and selected-probe observation
  paths with resource and safe-state evidence kept separate; and
- authoritative schematic, pin/resource map, current budget, bench acceptance,
  and recorded uncertainty for the intended safe tabletop range.

No immersion, hot surface, parasite-power operation, unknown three-pin module,
medical/food/preservation use, unattended thermal source, or waterproof claim
is permitted without separately bounded evidence. Listing authorization for
`18B20 Temp` is not specimen identity. The Lesson 062 `Digital Temperature`
family remains distinct and cannot satisfy this gate.

## Prior-decision impact

- Component layering is **preserved**: Lesson 065 interprets copied transaction
  evidence and owns no endpoint or transport.
- Lesson 064 ownership is **preserved**: reset, presence, ROM commands, bit
  slots, pull-up policy, timeout, and rollback stay below this boundary.
- Lesson 062 identity gating is **preserved**: unidentified Digital
  Temperature evidence is never treated as an 18B20.
- Fixed-storage and deterministic replay decisions are **extended** to exactly
  four stable configured ROM slots.
- Circuit-native observation is **preserved** through result cells at E0 and a
  separately qualified display/LED/test-point path at E1.
- RTC/SD and persistence decisions are **preserved**: stable ROM identity may
  key future record intent, but Lesson 065 stores nothing durably.
- Safety taxonomy is **preserved**: E0 has no energy path; specimen, pull-up,
  power mode, immersion, and physical thermal claims remain gated.

## Gate result

- Disposition: `natural fit` as an E0 fixed-capacity copied-evidence policy
- Open risks: exact Lesson 064 evidence types; final CRC/status vocabulary;
  AVR ABI/object/stack pressure; Lesson 066 aggregate budget; exact specimen,
  supply mode, bus topology, cable construction, conversion behavior, and
  bench acceptance
- Required discussion or decision IDs: none if implementation retains exactly
  four configured identities and no transport ownership; changing capacity,
  admitting runtime identity replacement, or adding a powered adapter requires
  a new review
- Remediation owner and next action: Lessons 064--066 planning freezes the
  copied transaction seam and aggregate budgets before implementation; Lesson
  065 implementation supplies the exhaustive matrix and exact resource probe
- Verification commands and results: document structure and diff checks only;
  implementation, host, Mega, resource, PDF, and publication gates are pending
- Maximum-composition scenario and proof: four mixed-resolution configured
  probes inside the Lesson 066 mapper with simultaneous search, conversion,
  CRC, missing, step, diagnostic, rollover, reset, and restart pressure;
  deterministic fixture and measured aggregate evidence pending
- Promotion permitted: yes for E0 implementation after the Lesson 064 copied
  evidence contract and Lesson 066 aggregate budget are frozen; no for powered
  adapters, wiring, schematics, or physical claims
