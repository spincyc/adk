# Lesson 055 constraint-model architecture stress pass

This pre-implementation architecture stress pass evaluates the exact
`ClueConstraintModel` contract in the Lessons 055--057 inert escape-console
plan. The proposed boundary is a natural component-layer policy. A bounded
pre-implementation representation probe found that the required copied state
does not fit the original object ceiling, but does fit a revised provisional
budget without changing ownership, capacity, or public semantics. This record
therefore permits E0 implementation under that bounded remediation; measured
AVR evidence remains a promotion gate.

## Boundary

- Name and lesson/project: `ClueConstraintModel`, Lesson 055
- Reviewer and date: pre-implementation architecture review, 2026-07-29
- Public types and operations: `ClueCategory`, `ClueQuality`,
  `ClueTermRelation`, `ClueRuleDisposition`, `ClueModelDisposition`,
  `ClueSourceIdentity`, `ClueObservation`, `ClueTerm`,
  `ClueRuleDefinition`, `ClueConstraintConfig`, `ClueEvidenceSnapshot`,
  `ClueRuleSnapshot`, `ClueConstraintSnapshot`, `ClueConstraintUpdate`, and
  non-copyable/non-movable `ClueConstraintModel` with
  `initialize()`/`shutdown()`/`reset()`/`initialized()`, atomic `update()`,
  `snapshot()`, and canonical-failure `Result<ClueEvidenceSnapshot>` /
  `Result<ClueRuleSnapshot>` observation
- Direct dependencies: `Status`, `MicrosecondTimePoint`,
  `MicrosecondDuration`, immutable construction-time copied configuration,
  explicit copied categorical observations, and fixed-width integer values
- Existing decisions and interfaces reconsidered: four-layer dependency
  direction; inert construction and lifecycle; ordinary `Status` semantics;
  explicit modular time; fixed-capacity/no-heap/no-recursion policy;
  stable copied snapshots; project-specific rather than generic policy;
  circuit-native observation; E0/E1/E2 evidence separation; Mega 2560
  resource gates; and the Lesson 057 owning-composition contract

The exact responsibility is narrow: validate one fixed 12-clue/12-rule
project configuration, copy atomic clue observations, and evaluate a bounded
DAG in deterministic topological order. It owns no hardware, endpoints,
resource claims, rendering, persistence, callbacks, actuator authority, or
security decision. “Solved” means only that this fictional puzzle's configured
rules are satisfied; it must never mean authenticated, authorized, unlocked,
safe, or fit for egress.

## Exact contract under review

The plan fixes these invariants:

- clue and rule IDs are dense and zero-based within counts `1..12`;
- there are exactly twelve rule slots, at most four terms and four
  prerequisite rule IDs per rule, and twelve copied evidence cells;
- configuration is copied at construction and remains immutable after
  successful initialization;
- each configured clue has one exact nonzero expected
  `ClueSourceIdentity`; an admitted observation must match that clue's source
  ID, configuration revision, and session epoch, and changing the configured
  identity requires reset plus reinitialization with a newly constructed
  configuration;
- every unused rule, term, prerequisite, observation, and mask bit has one
  canonical zero representation;
- `ClueConstraintUpdate` has one identity rule: array slot `i`, clue ID `i`,
  and `observationMask` bit `i` refer to the same clue for `i` in `0..11`;
  every set bit requires its same-index slot to carry that clue ID, every
  clear bit requires its same-index slot to be the canonical zero
  observation, and array order has no independent semantic meaning;
- initialization rejects invalid IDs, counts, categories, duplicate terms,
  duplicate prerequisites, self-dependencies, out-of-range dependencies, and
  cycles before becoming initialized;
- graph validation and evaluation are iterative and fixed-capacity, never
  recursive;
- one `update()` validates the whole envelope, copies every selected
  observation as one generation, and evaluates all rules exactly once in the
  copied topological order;
- any rejected update leaves evidence, rules, masks, generation, and retained
  status unchanged;
- same-sequence identical evidence is idempotent, while changed payload at the
  same sequence is a contradiction/source fault;
- sequence and time order use unsigned modular half-range rules; future,
  regressing, or exact-half-range values reject atomically;
- evidence is fresh through the inclusive age boundary and stale one tick
  later; `maximumEvidenceAge` is valid in `[0, half-range)`, so zero requires
  evidence observed at exactly `now`, while the exact half range and larger
  values reject;
- prerequisite failure precedes term classification, while term evidence
  precedence is invalid/source/timing, stale, contradictory, missing, then
  ordinary mismatch. The first blocker is diagnostic only; complete masks
  remain deterministic;
- `firstBlockingTerm` and `firstBlockingPrerequisite` use `UINT8_MAX` for “no
  blocker,” because zero is a valid term or prerequisite index;
- a zero-term rule is satisfied when all prerequisites are satisfied,
  including vacuous satisfaction when it has no prerequisites;
- invalid `evidence()` and `rule()` IDs return
  `StatusCode::InvalidArgument` with a canonical-zero result record, without
  mutation; every failed `Result<T>` in the arc follows that canonical-zero
  value rule; and
- public `update()` is the ordinary wrapper around a private pure
  `preflightUpdate()` and infallible `applyPreparedUpdate()` seam. The private
  prepared value binds the simultaneously live object/address identity with
  `ownerToken` and the object's nonzero `lifecycleGeneration`. That generation
  advances before an actual uninitialized-to-initialized transition, before
  every explicit reset, and before an actual initialized-to-shutdown
  transition. Repeated idempotent `initialize()` or `shutdown()` calls do not
  advance it or invalidate preparations. It is copied into every prepared
  update and never resets within the object lifetime. If the next advance
  would wrap to zero, `initialize()` returns
  `StatusCode::CapacityExceeded`; void `reset()` or `shutdown()` still
  completes its inert transition and publishes `CapacityExceeded` in retained
  snapshot/status. Every exhausted path invalidates all preparations and the
  object admits no new work. These are ordinary lifecycle guards, not
  persisted identity, authentication, or unforgeable capabilities, and
  same-address reconstruction creates a new lifecycle rather than proving
  historical identity. Only friend `InertEscapeConsole` may use the seam to
  preflight the parent and both children before any mutation.

These rules are coherent as behavior. The blocker is their physical
representation under the separately fixed object-size limit, not ambiguity in
the policy itself.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural.** A fixed puzzle-rule evaluator is component-layer circuit meaning over copied values. Electrical acquisition, sampling, debounce, and safe pin lifetime remain below it and outside E0. The public API exposes domain values rather than pins, callbacks, graph nodes, allocator hooks, or an implementation-owned clock. It deliberately avoids a generic solver until a second consumer exists. |
| Ownership and lifecycle | **Natural.** The model owns its copied configuration, exact expected identity per clue, and current copied evidence, is inert after construction, and is non-copyable/non-movable. It borrows nothing and can make failed initialization, rejected update, reset, repeated shutdown, destruction, and restart deterministic. A private prepared update carries `ownerToken` to distinguish simultaneously live object/address identities and the exact nonzero `lifecycleGeneration` to distinguish boundaries within that lifetime. The generation advances before an actual uninitialized-to-initialized transition, every explicit reset, and an actual initialized-to-shutdown transition; repeated idempotent initialize/shutdown calls neither advance it nor invalidate preparations. It never resets in that lifetime, and each advancing boundary plus destruction invalidates older preparation context. At attempted wrap, initialize returns `StatusCode::CapacityExceeded`; void reset/shutdown completes the inert transition and retains/publishes that status. Every exhausted case invalidates candidates and permanently prevents new work for that object. Neither field is persisted, authenticates the object, forms an unforgeable capability, or proves continuity after same-address reconstruction. The adopted remediation preserves this contract rather than moving storage to the caller. |
| Time and ordering | **Natural.** `now` and every observation time are explicit. The inclusive freshness interval, per-source sequence order, simultaneous observations, rollover, future values, regression, and half-range ambiguity are specified without a clock, wait, retry, catch-up loop, or scheduler dependency. Update-array position expresses clue identity only: slot `i` equals clue ID `i` equals mask bit `i`, so iteration or caller insertion order has no semantic role. One update performs fixed scans over at most twelve observations/rules and four terms/prerequisites per rule. |
| Errors and status | **Natural with one implementation check.** Ordinary malformed configuration/envelope and lifecycle failures can use existing `Status`; evidence quality and model/rule dispositions remain semantic data rather than new status codes. Rejection preserves retained state and returns the operation status. Invalid evidence/rule IDs return existing `StatusCode::InvalidArgument` plus a fieldwise canonical-zero record, and all failed `Result<T>` values follow that rule. Implementation tests must verify that `InvalidConfiguration` and `InternalFault` snapshot values do not silently replace failed-operation status or mutate a previously valid generation. |
| Resource budget | **Bounded local remediation approved for implementation.** AVR layout estimates are 527--557 B for transparent lossless representations, with a credible compact representation near 481 B. The revised provisional Lesson 055 target/hard ceiling is 512/640 B, preserving all twelve rules, twelve full evidence records, owned copied state, and public semantics. Lesson 057's provisional object target/hard ceiling is correspondingly 1,024/1,280 B. Actual compiled object, static SRAM, stack, flash, aggregate composition, and remaining-margin measurements still block promotion. |
| Deterministic proof | **Specified but not executed.** The plan names exhaustive four-rule DAG enumeration, maximum graph shapes, capacity boundaries, every evidence-quality position, sequence/time boundaries, canonical padding, lifecycle, rejection atomicity, and independent replay. That is sufficient test intent, but no implementation or measured fixture exists yet. Raw-struct byte comparison remains prohibited; replay must compare fields and canonical arrays. |
| Packaging and public surface | **Natural but unproven.** A declarative standalone header, out-of-line implementation, umbrella export, native/archive inventories, canonical Mega example, size evidence, HTML, and PDF fit existing packaging without a new framework. Nothing is implemented or packaged yet; promotion awaits the ordinary packaging and measurement gates. |
| Example and documentation fit | **Natural.** The planned evidence-wall replay can read as acquire/configure/start and observe/evaluate/present. Its twelve labeled clue cells, rule dispositions, and solved result provide non-Serial semantic presentation without implying hardware sampling or actuation. The E0 PDF needs pencil drawings only and no formal schematic; exact powered inputs and authoritative wiring remain E1 work. |
| Downstream effects | **Contained behaviorally, material structurally.** Lesson 057 is the first and only authorized owner/consumer and depends on exact expected source identities, copied provenance, masks, generation, dispositions, and bounded evaluation. Its friend-only access to the private pure-preflight/infallible-apply seam is a narrow composition mechanism, not a public transaction framework. Changing ownership, capacity, provenance, preflight atomicity, or the object ceiling affects the frozen project plan, aggregate SRAM/stack proof, canonical example, tests, HTML/PDF vocabulary, and acceptance gates. Lessons 001--054 need no API change. |

## Representation lower bound and bounded remediation

The original 128-byte hard ceiling was not supported by the exact state
contract.
Without relying on ABI padding, a single retained evidence cell needs the
information content of:

- clue presence/ID, category, and quality;
- source ID and source configuration revision;
- source session epoch and sequence;
- observation time; and
- full `Status`.

Twelve such cells already require substantially more than 128 bytes in an
ordinary fixed-width representation. The model must additionally retain the
immutable rule graph or an equivalent lossless compiled form, topological
order, rule results, first-blocker diagnostics, generation, masks,
configuration/instance identity, maximum age, and lifecycle state. Private
bit-packing can reduce the rule graph and enum storage, but it cannot erase
the required 32-bit provenance/time fields or full status of twelve
independent observations.

The bounded representation study estimates 527--557 B for transparent
lossless AVR layouts and about 481 B for a credible compact private layout.
The adopted remediation raises the Lesson 055 provisional object target/hard
ceiling to 512/640 B and the
Lesson 057 provisional object target/hard ceiling to 1,024/1,280 B. It
preserves owned copied state, the exact 12/12 capacities, all provenance, and
the public API. No caller-owned Lesson 055 storage, reduced evidence, or
capacity change is authorized.

These estimates permit implementation; they are not promotion evidence. The
implementation must report actual AVR `sizeof`/alignment and linked aggregate
measurements. A target miss requires size-focused review. A hard-ceiling or
project remaining-margin miss stops promotion and requires a new durable
decision rather than silent compression, dropped provenance, or borrowed
storage.

## Composition pressure scenario

The maximum currently authorized E0 composition is the exact Lesson 057
console: one owned Lesson 055 model at twelve clues/twelve rules with four-term
and four-prerequisite pressure, one owned Lesson 056 panel with its
caller-owned two-slot audit image, one parent update envelope, semantic
presentation/lamp/latch intents, diagnostics and trace/export storage, and no
physical endpoint. The Lesson 055 standalone slice uses all twelve
observations at one timestamp, a maximum dependency join, sequence wrap, the
freshness boundary, and simultaneous quality collisions.

The collision trace initializes the maximum DAG, admits a full qualified
generation with each clue in its identity-matched slot, then at one timestamp
supplies: a changed same-sequence clue, another source at sequence wrap, stale
evidence exactly one tick beyond its age, contradictory evidence, an
observation with a future time, and a diagnostic presentation failure.
Structural validation must reject the whole invalid envelope without
mutation. Separate malformed envelopes prove that a set mask bit with the
wrong clue ID and a clear mask bit with a nonzero slot both reject atomically.
A following structurally valid generation must deterministically classify
independent invalid/stale/contradictory/missing pressures, apply prerequisite
precedence in topological order, and publish stable masks and first blockers.
Shutdown and restart with the same faults present must return to inert/empty
state and reproduce the same fieldwise result only after fresh admission.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable and bounded in design, unmeasured.** One update scans at most 12 mask positions, validates at most 12 observations, and evaluates 12 rules with at most four prerequisites and four terms each. No recursion or catch-up is allowed. Tests must bound worst-case validation plus evaluation at one timestamp, zero-age same-time evidence, exact freshness edges, simultaneous inputs, rollover, half-range rejection, and missed calls. The private preflight is pure and apply is infallible; neither may repeat input-sized work through a retry loop. The implementation must report the largest call path and prove no parent/diagnostic work can induce starvation. |
| Total memory and hardware resources | **Applicable; bounded remediation permits implementation.** E0 owns zero pins, timers, interrupts, buses, ADC channels, registry claims, power domains, or physical test points. Memory is nevertheless a hard resource. Standalone gates are 16/20 KiB flash, 1,536/2,048 B static SRAM, 384/512 B stack, and 512/640 B reusable object; transparent lossless AVR estimates are 527--557 B and the credible compact estimate is about 481 B. The complete project provisionally permits a 1,024/1,280 B object and must still satisfy 4,096/4,608 B static SRAM, 1,024/1,280 B stack, every caller buffer at most 512 B, and at least 2,048 B remaining after static SRAM plus stack/interrupt reserve. Actual measurements govern promotion. |
| Shared bus or transport | **Not applicable at E0 by contract.** The model accepts copied values and owns no endpoint, display, storage medium, bus borrower, queue, address, or transport. Any future input or presentation bus belongs to separately qualified E1 endpoints and cannot alter constraint evaluation semantics. |
| Persistence and recovery | **Not applicable by explicit volatility.** Configuration and evidence are runtime copies; reset/shutdown clear live evaluation state, and no medium, schema, commit, wear, corruption recovery, or power-loss durability is claimed. Lesson 056's audit image is a separate caller-owned volatile protocol and is not a Lesson 055 dependency. |
| Motion, external power, or stored energy | **Not applicable by absence of an actuation path.** `ClueConstraintModel` publishes dispositions and masks only. It has no latch/lamp/servo/relay command, switched supply, retained energy, or physical output. Lesson 057 may map solved policy to inert semantic intent, while any restrained E2 demonstration remains separately gated and can never control a door, lock, occupied space, or egress. |
| Observation identity and provenance | **Applicable and central.** Every configured clue has one exact expected source identity, and every retained clue value must match and keep its source ID, configuration revision, session epoch, sequence, observation time, quality, category, and full status. Slot `i`, clue ID `i`, and mask bit `i` are one identity; there is no caller-defined or array-order precedence. Whole-envelope validation prevents mixing an accepted subset with rejected fields. Tests must prove every expected-source field match/mismatch, slot/ID/bit match/mismatch, canonical absent slots, same-sequence collision, attempted identity change, delayed/future observations, simultaneous sources, reset/reconstruction, and replay. The revised budget preserves these fields; a later size repair may not drop or conflate them silently. |
| Diagnostic interference | **Applicable.** Evidence-wall cells, rule indicators, solved/fault presentation, trace/export storage, and Serial must be included in aggregate time/SRAM/resource accounting. Disablement, failure, saturation, or full trace storage must not change admission, rule precedence, masks, generation, or solved state. Serial cannot be the sole observation. Presentation failure is downstream evidence and cannot mutate the model. |
| Failure collision and recovery | **Applicable.** Structural invalidity rejects the whole envelope before semantic precedence. For a valid envelope, source/timing/invalid quality precedes stale, stale precedes contradictory, contradictory precedes missing, and mismatch remains ordinary unsatisfied evidence; failed prerequisites precede terms. Parent composition must prove all child preflights complete before any apply, a foreign/stale/mutated `ownerToken` or mismatched `lifecycleGeneration` preparation cannot apply, and every apply after successful complete preflight is infallible. Actual initialize/shutdown transitions and every explicit reset must invalidate candidates before their effects become observable; repeated idempotent initialize/shutdown calls must preserve both generation and candidates. Wrap-to-zero must never publish zero or reuse an earlier generation: initialize returns `StatusCode::CapacityExceeded`, while void reset/shutdown completes inert transition and retains/publishes that status; all three invalidate candidates and fail closed against later work. Tests must retain every independent attribution, prove no partial copied generation, and cover reset/shutdown/reinitialize while faults persist. No diagnostic or later project intent may reclassify a source fault as a solved result. |

## Required deterministic proof

Before any promotion reconsideration, executable host tests and resource probes
must cover:

1. clue/rule counts below, at, and above `1..12`; dense IDs; every configured
   expected source with source ID, configuration revision, and session epoch
   at zero, valid, and mismatched values; every invalid enum; all used and
   unused mask bits; and canonical-zero mutation of every unused expected
   source, rule, term, prerequisite, and observation field;
2. term and prerequisite counts zero through five; duplicate terms and
   prerequisites; self edges; out-of-range edges; two-node and long cycles;
   maximum chains, diamonds, four-way joins, and disconnected graphs;
3. all `2^12` non-self directed-edge masks over four labeled rules, accepting
   exactly DAGs and producing a stable iterative order for every accepted DAG;
4. every project-used four-term relation/category combination and every
   evidence quality at every term position; zero-term rules with no
   prerequisites and with satisfied/failed prerequisites; exact prerequisite
   and term precedence; `UINT8_MAX` no-blocker sentinels versus valid index
   zero; and deterministic first-blocker diagnostics and complete masks;
5. all observation masks; for every index `0..11`, a set bit with matching
   slot/clue ID, a set bit with every mismatched clue ID, a clear bit with a
   canonical-zero slot, and a clear bit with each nonzero field mutation;
   proof that no permutation or insertion order exists beyond this fixed
   identity mapping; exact duplicates; changed same-sequence values in every
   retained field; forward sequence, wrap, regression, and exact-half-range
   ambiguity;
6. `maximumEvidenceAge` zero, one, one below half range, exact half range, and
   above; observed time one tick before, exactly at, and one tick after `now`
   and the inclusive freshness limit; zero-age same-time freshness and
   one-tick staleness; future time, rollover, overflow, regression, and
   exact-half-range ambiguity;
7. atomic rejection at the first, middle, and last selected observation and at
   each graph-validation boundary, proving previous evidence, rules, masks,
   generation, status, and private compiled order remain unchanged;
8. invalid `evidence()` and `rule()` IDs below/at/above the configured range,
   proving `StatusCode::InvalidArgument`, fieldwise canonical-zero result
   records, and no mutation; every other failed `Result<T>` path likewise
   carrying canonical-zero `T`;
9. public update equivalence to private preflight plus apply; pure preflight
   with no mutation; simultaneous-live/address `ownerToken` and exact nonzero
   `lifecycleGeneration` matching; generation advance before actual
   uninitialized-to-initialized and initialized-to-shutdown transitions and
   before every explicit reset; repeated idempotent initialize/shutdown
   preserving generation and live preparations; the value never resetting
   within one object lifetime; foreign, stale, prior-generation,
   reset-invalidated, shutdown-invalidated, and field-mutated preparations;
   boundary tests at generation 1, `UINT32_MAX - 1`, and `UINT32_MAX`;
   attempted initialize advance from `UINT32_MAX` returning
   `StatusCode::CapacityExceeded`; attempted reset/shutdown advance from
   `UINT32_MAX` completing the inert transition and retaining/publishing
   `CapacityExceeded`; all exhaustion paths invalidating every candidate,
   never publishing zero, and rejecting all new preparation/work; destruction
   plus same-address reconstruction explicitly not treated as persisted or
   authenticated continuity; parent failure before and after each child
   preflight; and infallible apply only after complete successful preflight;
10. failed and repeated initialization, reset before/after admission, shutdown
   before/after initialization, repeated shutdown, destruction while active,
   rejected in-place identity changes, and reconstruction with a new exact
   expected-source configuration;
11. two independent maximum models replaying identical fields and timestamps
   to identical fieldwise public results, without comparing raw struct
   representation or padding; and
12. capacity immediately below, at, and above every limit; maximum
    simultaneous work; compiler-derived conservative stack path; exact AVR
    flash/static SRAM; every public/private object and fixture size; and the
    complete Lessons 055--057 aggregate with diagnostics and buffers.

Host proof remains E0 software evidence. It cannot establish input voltage,
debounce, wiring, display behavior, storage durability, physical latch state,
or safety.

## E0, E1, and E2 separation

E0 permits only deterministic replay over copied categorical observations.
The evidence wall may render semantic cells through already-supported
presentation components, but Lesson 055 itself acquires nothing. Its
non-Serial proof is the visible rule/evidence presentation produced from the
copied snapshot; resource-acquisition evidence is explicitly “no resources
claimed,” separate from the absence of actuator intent.

E1 remains blocked on exact passive clue and operator-input specimens,
markings and primary sources, electrical qualification, voltage/current/
polarity/pull behavior, endpoint ownership, complete pin/interrupt/timer/bus/
power budgets, authoritative schematic, rollback and safe-state evidence, and
signed bench acceptance. Servo, relay, latch, and other powered actuation must
be physically absent.

E2 remains separately blocked on the exact restrained demonstration
actuator/driver/load, a separate current-limited load supply, protection,
guarded geometry, independent physical power removal, simultaneous-load and
thermal budgets, and signed acceptance for every startup/fault/reset/shutdown/
power-loss case. It may never be a door, lock, occupied enclosure, egress
route, alarm, access decision, or life-safety system.

## Prior-decision impact

| Decision or contract | Disposition and evidence |
|---|---|
| Four-layer architecture and endpoint-owned electrical lifetime | **Preserved.** Lesson 055 is pure component policy and owns no endpoint or resource. |
| Inert construction and explicit lifecycle | **Preserved.** Configuration may be copied at construction, while no observable active behavior exists before successful `initialize()`. Failed initialization and shutdown remain atomic/inert. |
| Explicit time, modular half-range order, and bounded update work | **Preserved.** Every time enters in the update/observation values, and fixed scans replace recursion or catch-up. |
| Ordinary `Status`/semantic-value separation | **Preserved.** Operation rejection uses `Status`; clue qualities and model/rule dispositions remain copied domain evidence. |
| Fixed storage, no heap, no recursion, no callbacks | **Preserved through bounded remediation.** The revised 512/640 B Lesson 055 ceiling accommodates the 527--557 B transparent lossless estimates below the hard gate and gives the credible compact estimate of about 481 B a path below target, without changing owned fixed storage. |
| Stable snapshots and copied provenance | **Preserved and central.** There are no borrowed observation views or pointers, and rejection retains the prior generation. |
| Project-specific policy before shared abstraction | **Preserved.** No generic constraint solver, scripting language, dynamic graph, or shared puzzle framework is introduced. |
| Lesson 057 sole composition ownership | **Preserved behaviorally.** Lesson 055 does not refer to Lesson 056 and owns no cross-child precedence or actuator intent. Any storage-contract revision must be reconciled with Lesson 057 construction and aggregate budgets. |
| Circuit-native observation and diagnostic isolation | **Extended.** The evidence wall provides visible semantic results while diagnostics remain outside correctness and physical endpoints remain gated. |
| E0/E1/E2 evidence and safety separation | **Preserved.** “Solved” remains inert puzzle policy with explicit prohibition on access control, confinement, egress, and safety use. |
| Mega resource gates in the exact Lessons 055--057 plan | **Extended by bounded remediation.** Lesson 055 now uses a provisional 512/640 B object target/hard ceiling and Lesson 057 uses 1,024/1,280 B. All other static SRAM, stack, flash, caller-buffer, and 2,048 B remaining-margin gates remain controlling. |
| Pencil visuals and formal-schematic exception | **Preserved.** E0 uses pencil visuals; only an exact, electrically authoritative later schematic may use the exception. |

## Design-buckling review

The policy boundary does not buckle under graph, provenance, time, lifecycle,
or failure pressure. Existing ADK contracts naturally express a fixed,
iterative, copied-value DAG evaluator. The original 128-byte ceiling buckled,
and the measured bounded remediation resolves that pre-implementation strain
without changing the component contract.

Implementation or promotion must stop if fitting the revised ceilings would
require:

- borrowed durable configuration or observation pointers not present in the
  exact API;
- reduced clue/rule/term/prerequisite capacities;
- dropped or conflated source, revision, session, sequence, time, quality, or
  status provenance;
- hidden globals, heap allocation, recursion, callbacks, or a generic solver;
- partial update mutation or recomputation from caller data after return;
- changing earlier lifecycle, status, time, snapshot, or resource policy;
- moving electrical sampling, display rendering, persistence, or actuation
  into the model; or
- interpreting “Solved” as identity, access, lock, egress, or safety authority.

After a resource-policy decision, compact private indices, masks, enum
encodings, iterative topological state, and elimination of duplicated derived
state are legitimate bounded repairs if they preserve every public field and
behavior. They still require measured object, stack, aggregate SRAM, and
fieldwise replay proof.

## Stress disposition

**Bounded local remediation; E0 implementation permitted.** API layering,
lifecycle, timing, status, deterministic proof strategy, packaging,
documentation, and safety scope are natural fits. The representation study
identified and bounded the only current strain: 527--557 B transparent
lossless AVR estimates and a credible compact estimate near 481 B replace the
impossible original ceiling with a 512/640 B Lesson 055 target/hard ceiling.
The complete Lesson 057 object's
provisional target/hard ceiling is 1,024/1,280 B.

This remediation is confined to the unpromoted Lessons 055--057 boundary. It
preserves owned copied state, exact capacity, provenance, behavior, public
API, and prior consumers. Implementation may proceed, but promotion remains
blocked until compiled AVR measurements and the complete maximum-composition
proof pass every revised object, static SRAM, stack, flash, buffer, and
remaining-margin gate.

## Gate result

- Disposition: bounded local remediation; natural-fit behavior with revised
  provisional resource ceilings
- Open risks: actual AVR object/layout, stack, flash, static SRAM, aggregate
  Lesson 057 fit, and 2,048 B remaining margin are unmeasured; all
  deterministic tests and publication artifacts remain unimplemented
- Required discussion or decision IDs: no further discussion if the adopted
  512/640 B Lesson 055 and 1,024/1,280 B Lesson 057 ceilings and exact owned
  contract are retained; a new decision is required for any hard-gate miss,
  caller-storage/API change, capacity reduction, or provenance loss
- Remediation owner and next action: Lessons 055--057 implementation owner;
  implement the exact owned contract, report actual AVR layout and complete
  composition measurements, run the deterministic matrix, and rerun this
  stress pass before promotion
- Verification commands and results: documentation structure and diff hygiene
  only at this pre-implementation stage; no implementation, host, sanitizer,
  Arduino, size, package, lesson, or hardware result exists
- Maximum-composition scenario and proof: scenario specified above;
  pre-implementation estimates permit work, while executable and measured
  proof remains a promotion gate
- E0 implementation permitted: yes, under the bounded remediation
- Promotion permitted: no
- E1 permitted: no; exact physical qualification and bench acceptance remain
  open
- E2 permitted: no; exact restrained-actuation qualification and bench
  acceptance remain open
