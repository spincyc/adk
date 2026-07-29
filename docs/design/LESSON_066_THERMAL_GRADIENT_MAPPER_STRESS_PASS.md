# Lesson 066 thermal-gradient-mapper architecture stress pass

Status: initial pre-implementation E0 review; Lessons 064--065 public
interfaces, exact resource evidence, powered specimens, presentation, and
recording media remain open.

This pass reviews the queued Lesson 066 project boundary from the
[extended component/project cadence](../projects/component_project_cadence.md).

`ThermalGradientMapper` is a natural E0 composition only when it consumes one
complete copied Lesson 065 qualified-probe-set observation, retains configured
spatial order, and emits inert presentation plus caller-owned record intent.
It is not a single-wire transport, probe qualifier, thermometer, storage
driver, preservation instrument, or safety monitor.

## Boundary

- Name and lesson/project: `ThermalGradientMapper`, Lesson 066
- Review state: initial pre-implementation review
- Proposed public responsibility: validate one copied fixed-capacity qualified
  probe-set observation, project it into configured spatial slots, derive
  conservative adjacent raw-sixteenth intervals, and fill bounded inert
  presentation and record-intent values
- Direct dependencies: promoted Lesson 065 qualified-probe-set observation,
  `Status`, `TimePoint`, fixed-width value types, and no hardware endpoint
- Existing decisions reconsidered: single-wire ownership remains in Lesson
  064; identity, conversion, CRC, disappearance, stale, resolution, and
  implausible-step qualification remain in Lesson 065; RTC and removable-media
  durability remain deferred under Lessons 022 and 024

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | Natural if the mapper consumes a complete copied Lesson 065 observation without reaching through it to a bus, scratchpad, or discovery mechanism. Configuration owns one ordered list of expected identities representing physical tabletop position. Presentation and record values are semantic intent only. |
| Ownership and lifecycle | Inert construction, noncopyable/nonmovable coordinator, fixed configuration, explicit `initialize`/`reset`/`shutdown`, and no heap, callback, retained caller pointer, child reference, or hidden clock. `update` fills caller-owned result and record-intent buffers synchronously and retains no reference to either. |
| Time and ordering | `TimePoint now` is supplied. Probe adjacency is always configuration order, never discovery order, ROM numeric order, arrival order, current temperature, or display-page order. Equal accepted frame identity is idempotent only for a byte-identical copied observation. Changed duplicate, regression, future time, exact half-range ambiguity, and sequence exhaustion reject atomically. Wrap-valid stale evidence commits a fault result. |
| Errors and status | Lifecycle and structural invalidity use `Status`. Valid but unhealthy child evidence remains domain state. Missing, disappeared, CRC-failed, conversion-in-progress, stale, implausible-step, mixed-resolution, or otherwise unqualified required slots dominate all numeric presentation: they can never be rendered as a cold value or a valid gradient. |
| Resources | E0 claims zero pins, timers, interrupts, buses, endpoints, supplies, displays, LEDs, clocks, or media. Measure the linked Lessons 064--066 maximum composition with the exact-four Lesson 065 configuration and every permitted mapped-subset size. Planning targets are 16/24 KiB flash, 1,536/2,048 B static SRAM, 768/1,024 B conservative synchronous stack, and 512/768 B mapper object, with each fixed caller-owned ABI value at or below 256/384 B and at least 4 KiB formula residual SRAM. Targets/hard limits require exact implementation-plan review before code. |
| Deterministic proof | Host fixtures can scramble discovery order while preserving configured position; inject every child quality, raw-sixteenth extreme, interval boundary, mapping error, stale/future/rollover condition, page transition, fresh record edge, replay, reset, and shutdown. Tests compare complete fixed result and caller-owned record images byte for byte and protect them with canaries. |
| Packaging/public surface | One project header/source, strict host test, compile-only Mega replay, exact linked resource probe, HTML reference, and complementary pencil-drawing PDF. No adapter, single-wire command, LCD/LED driver, RTC/SD include, physical wiring, or formal schematic enters the E0 package. |
| Example and documentation fit | The canonical replay presents acquire/configure/start and observe/decide/actuate phases over copied fixtures. Named result cells expose configured slot identity, raw-sixteenth interval, fault token, page intent, gradients, and record intent without making Serial the only evidence. |
| Downstream effects | Lessons 064--065 retain transport and qualification ownership. Existing Lesson 062 converted/categorical thermal evidence is not reinterpreted as 18B20 evidence. Lessons 022/024 retain RTC/media scope. Later sensor characterization and recording work cannot inherit a durability or calibrated-physical claim from this project. |

## Frozen E0 information rules

### Configured spatial order

Lesson 065 configuration contains exactly four distinct source ROM identities.
Lesson 066 configuration selects an ordered subset of two through four of
those exact identities in explicit left-to-right or near-to-far order. Every
mapped ROM must occur once in the Lesson 065 configuration, and a mapped ROM
cannot repeat. The remaining configured Lesson 065 ROMs are valid unmapped
sources; their presence is permitted and does not become a foreign-identity
fault.

The mapped subset order defines slots, adjacent pairs, presentation pages,
extrema tie breaks, and record layout for the entire initialized lifecycle.
The mapper projects by identity into those slots. It never treats enumeration
or discovery order as spatial evidence, and it does not publish pages or
gradients for permitted unmapped sources. Fresh values may change without
reordering mapped pages. Equal extrema use the lowest mapped slot as
deterministic primary attribution; a fixed-capacity tie mask retains all equal
mapped slots when the budget permits.

### Raw-sixteenth intervals

Lesson 066 retains the Lesson 065 exact signed raw-sixteenth-degree-Celsius
value domain. It does not introduce floating point, silently round to whole
degrees, or claim greater calibrated accuracy. Each qualified slot exposes a
closed raw-sixteenth interval:

```text
[slotLowerRawSixteenth, slotUpperRawSixteenth]
```

The interval must come from the promoted Lesson 065 contract. If Lesson 065
does not publish a justified interval, Lesson 066 must stop at an exact
raw-sixteenth value and categorical validity rather than invent uncertainty.

For configured adjacent slots `left` and `right`, widened signed arithmetic
derives the conservative gradient interval:

```text
lower = right.lower - left.upper
upper = right.upper - left.lower
```

Classification uses the complete interval. `Rising` requires `lower` at or
above the positive meaningful-gradient threshold. `Falling` requires `upper`
at or below the negative threshold. `Flat` requires the complete interval
strictly inside the two thresholds. An interval crossing either boundary is
`Indeterminate` or `Disagreement`, never rounded into a direction. Equality
is tested explicitly. The overall spread retains the two configured
identities that support it.

### Fault-dominant presentation pages

Every configured probe page contains identity, age, qualification/validity,
and either a raw-sixteenth interval or an explicit fault token. Every adjacent
gradient page contains both configured identities and either a conservative
interval/classification or an explicit fault token.

A required-slot fault makes the overall mapper health `Fault`. It also faults
every incident gradient page. Healthy nonincident pages may retain their own
valid evidence, but the overall page and record retain the complete fault
mask. A missing, stale, CRC-failed, disappeared, conversion-in-progress,
mixed-resolution, or implausible-step probe can never appear as zero, a cold
temperature, a flat gradient, or cached healthy evidence. Page selection,
diagnostic failure, and record handling cannot change classification.

Presentation is inert intent: selected configured slot/pair, stable identity
token, raw-sixteenth value-or-fault content, age visibility, active-slot LED
index, and grayscale-safe health/fault blink code. E0 owns no LCD, LED, pin,
timer, or refresh schedule.

### Caller-owned record intent

A fresh, ordered copied record edge may produce exactly one bounded
caller-owned record intent in mapped slot order. Record creation belongs to
that edge, not to temperature change or arrival of a new Lesson 065
observation. A fresh record edge may therefore capture the unchanged current
qualified mapper snapshot. A repeated edge is idempotent only when its
complete copied evidence is byte-identical; a changed duplicate or regressing
edge rejects atomically.

The witness retains owner and lifecycle, configuration revision, mapper record
sequence, record-edge source identity/revision/sequence/time, current accepted
set identity and sequence, supplied observation time, every mapped probe
identity and raw-sixteenth interval or fault, every adjacent mapped gradient
interval/quality, overall health/fault mask, and a deterministic digest. Full
fields remain authoritative; a digest never substitutes for the witness.

The mapper fills fixed caller-owned result and record-intent values during
`update` and keeps no pointer to either. Their types and sizes are part of the
public ABI rather than nullable or runtime-sized buffers. Record-sequence and
record-edge exhaustion fault before zero. E0 does not open a file, retry
storage, accept a media receipt, or assert that a caller preserved the intent.
If later requirements add outstanding records, acknowledgements, retry,
coalescing, torn-write recovery, or durable generations, that is an
architectural extension requiring a separate decision rather than an
opportunistic Lesson 066 implementation detail.

## Composition pressure

The maximum E0 fixture uses the frozen maximum probe count, reverse discovery
order, one mixed-resolution disagreement, one disappearing middle probe,
raw-sixteenth extrema on its two neighbors, page rollover, frame and time
rollover, a fresh record edge over an unchanged current snapshot, reset,
shutdown, and deterministic replay. The fixture proves that stable order,
faults, and sequential record capture survive without a hardware or durability
claim.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time | Applicable. One bounded projection, fixed adjacent-pair pass, page selection, and fixed caller-owned ABI fill per update; no polling, search retry, conversion wait, storage retry, or catch-up loop. Bound worst-case work for all four Lesson 065 sources with the maximum four-source mapped subset and prove page/time rollover. |
| Total memory and hardware resources | Applicable. Measure ordinary and isolated no-LTO Lesson 066 plus linked Lessons 064--066. Include all three live objects, copied-once exact-four child observation, mapper result, caller-owned record image, fixed identity/order tables, hidden-return probes, conservative stack, ISR reserve, and formula residual. Mapped-subset counts below two, at two, and at four are mandatory boundaries. |
| Shared bus or transport | Not applicable at E0 because the mapper receives copied qualified values and makes no transport call. A future LCD, RTC, SD, or probe adapter requires its own owner, borrower lifetime, arbitration, bounded transaction, rollback, congestion, and restart evidence. |
| Persistence and recovery | Not applicable as durable state. Record intent is volatile caller-owned output. Reset/shutdown invalidate mapper sequence and page history without claiming a stored record. RTC accuracy, media schema, atomic commit, corruption, torn write, capacity, wear, and power-loss recovery remain open. |
| Motion, external power, or stored energy | Not applicable at E0 because no actuation or energized path exists. The physical lesson may use only qualified safe tabletop objects within specimen and material limits; no immersion, hot surface, heater, flame, ignition, medical target, or unattended heating is authorized. |
| Observation identity and provenance | Applicable. Preserve exact probe-set source/configuration/calibration identity, accepted set/frame sequence, per-probe ROM identity, resolution, conversion/CRC/age/quality evidence, configured spatial slot, and transformation into every gradient and record field. Delayed or invalid values are never presented as simultaneous qualified measurements. |
| Diagnostic interference | Applicable. Page selection, LCD/LED intent, result cells, Serial, and record-edge presence cannot reorder probes, change intervals, suppress a fault, or be required for classification. Future physical diagnostics enter the combined pin/timer/current/memory budget. |
| Failure collision and recovery | Applicable. Reverse discovery order plus a disappearing middle mapped probe faults both incident gradients; a simultaneous healthy mapped endpoint, permitted unmapped source, page transition, fresh record edge, rollover, reset, and shutdown are exercised as a sequential atomic trace. Each accepted step retains attribution; each rejected step preserves the prior complete result and record image. Recovery requires a new complete accepted Lesson 065 frame and cannot reuse cached healthy display values. |

## Required deterministic proof

Tests must cover:

- exact-four Lesson 065 source configuration with mapped-subset counts one
  (rejected), two, three, and four;
- every permutation of discovery/input order for at least three configured
  identities, proving byte-identical configured output order;
- zero, duplicate, missing, and changed mapped identities, plus one and two
  permitted configured-but-unmapped source ROMs;
- each Lesson 065 valid, conversion-in-progress, CRC, stale, disappeared,
  mixed-resolution, implausible-step, and producer-fault outcome;
- raw-sixteenth minimum/maximum and widened subtraction without overflow;
- positive and negative meaningful-gradient thresholds immediately below, at,
  and above equality, plus intervals crossing zero and either threshold;
- equal extrema, tie attribution, and one bad interior probe faulting both
  incident pairs without rewriting healthy nonincident evidence;
- page zero/last/wrap, age boundary, ordinary unsigned rollover, future time,
  exact half-range ambiguity, changed duplicate, regression, and exhaustion;
- fixed caller-owned result/record ABI size, canary, hidden-return,
  atomic-rejection, and byte-stable golden images;
- no record without an edge; a fresh edge over changed and unchanged current
  snapshots; byte-identical edge replay; changed duplicate; regression;
  record-edge and record-sequence exhaustion; and sequential atomic traces
  without a false durability claim;
- reset, shutdown, repeated shutdown, reinitialize, and recovery while the
  copied source remains faulted.

The canonical Mega replay is compile-only and writes named memory result
cells. The future physical acceptance record separately requires prediction,
non-Serial observation, interpretation, resource-acquisition evidence, and
safe-state evidence.

## Prior-decision impact

- The Lessons 064--066 dependency order is **preserved**: transport and
  DS18B20 identity/conversion belong below the qualified set and mapper.
- Stable identity recording in the cadence is **extended** into explicit
  configured spatial order; discovery order receives no semantic authority.
- The Lesson 065 validity contract is **preserved**: Lesson 066 consumes and
  exposes faults instead of requalifying, averaging, or silently substituting
  values.
- Raw DS18B20 representation is **preserved**: interval arithmetic remains
  signed raw sixteenths with widened intermediates and no floating-point or
  extra-precision claim.
- Lessons 022/024 persistence scope is **preserved**: caller-owned record
  intent is not RTC/SD support, durability, or power-loss recovery.
- Circuit-native observability is **extended** only as inert LCD/LED/fault
  intent at E0; exact powered endpoints and independent safe-state evidence
  remain E1 gates.
- Safety limits are **preserved**: no immersion, unknown waterproof assembly,
  hot surface, heater, flame, ignition, medical/safety use, or unattended
  thermal stimulus is introduced.

## Gate result

- Disposition: `natural fit` for an E0 composition that consumes only a
  complete Lesson 065 copied observation, freezes configured spatial order,
  keeps raw-sixteenth interval arithmetic conservative, makes pages
  fault-dominant, and emits only caller-owned volatile record intent
- Open risks: Lessons 064--065 public types and maximum capacity; exact
  interval provenance; aggregate flash/SRAM/stack/object and caller-buffer
  sizes; exact DS18B20 specimens and supply mode; powered LCD/LED observation;
  any RTC/SD recorder
- Required discussion or decision IDs: none for the bounded E0 shape; adding
  storage receipts/retries/durability, automatic source ordering, or a generic
  thermal abstraction requires a separate architectural decision
- Remediation owner and next action: the Lessons 064--066 implementation-plan
  owner freezes the exact Lesson 065 copied observation and capacity, then
  replaces planning budgets with exact gates before code
- Verification commands and results: document review only; strict host,
  sanitizer, style, standalone-header, Mega, exact resource, PDF, site, and
  publication gates remain pending
- Maximum-composition scenario and proof: specified above; deterministic
  fixture and exact linked resource evidence remain pending
- Promotion permitted: yes for implementation planning after Lessons 064--065
  interfaces and exact budgets are frozen; no for publication, powered
  sensing/presentation, physical gradients, RTC/SD storage, preservation,
  medical, or safety claims
