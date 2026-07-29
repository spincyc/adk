# Inert escape-console design stress pass

This record applies the
[component design stress pass](../templates/component-design-stress-pass.md)
to the pre-implementation Lesson 057 `InertEscapeConsole` contract in the
[Lessons 055--057 plan](LESSONS_055_057_ESCAPE_CONSOLE_PLAN.md). It reviews
the exact maximum E0 composition before public code freezes. It is not a
hardware qualification and authorizes no door, lock, latch, relay, servo,
egress, alarm, access-control, confinement, or life-safety use.

## Boundary

- Name and lesson/project: inert escape console, Lesson 057
- Reviewer and date: pre-implementation architecture stress review,
  2026-07-29
- Public types and operations: fixed six-family configuration; one copied
  atomic update envelope containing `auditImagePresent` and a complete
  `PanelAuditImage`, plus `solvePreviewPresent` and a complete
  `EscapeConsolePreview`; prepare/can-commit solve transaction; update-driven
  supplied-image recovery and commit; copied family, puzzle, panel, audit,
  presentation, latch, and lamp results; explicit lifecycle and child
  snapshots
- Direct dependencies: one owned Lesson 055 `ClueConstraintModel`, one owned
  Lesson 056 `FaultAwareOperatorPanel`, copied `PanelAuditImage` values,
  `Status`, explicit `MicrosecondTimePoint`, and fixed-size copied values
- Existing decisions and interfaces reconsidered: project-local composition;
  construction-time inertness; explicit time; copied provenance; fixed
  capacities; parent/child atomicity; two-slot audit-image recovery; status
  versus domain disposition; diagnostic isolation; E0 semantic intent versus
  separately qualified E1/E2 behavior; no access, security, egress, or
  life-safety claim

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural as one project-local coordinator.** Lesson 055 retains clue provenance, dependency evaluation, and solved-policy meaning. Lesson 056 retains stop, operator-chord, presentation, and audit-image policy. Lesson 057 alone owns the fixed mapping of exactly two clue IDs to each of six fictional families, cross-child precedence, solve admission, and inert output intent. Both children expose private friend-only pure `preflightUpdate()` plus infallible `applyPreparedUpdate()` seams so the parent can prove the whole transition before mutation without exposing a public transaction framework. The public surface accepts no endpoint, address, arbitrary family, callback, script, raw actuator command, storage operation, clock, or rendering buffer. Electrical lifetime remains in future endpoints and capability/claim policy remains below them. A generic puzzle engine or automation coordinator would exceed this boundary. |
| Ownership and lifecycle | **Natural under the repaired value-owned boundary.** The config-only coordinator owns both non-copyable/non-movable policy children and retains no caller reference. Each policy owns a nonzero `lifecycleGeneration` copied into every private prepared update and public preview. It advances before an actual uninitialized-to-initialized transition, before every explicit reset, and before an actual initialized-to-shutdown transition; it never resets within one object lifetime. Repeated initialization and repeated shutdown are idempotent and neither advance the generation nor invalidate a candidate. Every candidate comparison requires an exact match, preventing same-object preview resurrection across real lifecycle boundaries. If initialization would wrap the generation to zero, it returns `StatusCode::CapacityExceeded`, invalidates all candidates, remains uninitialized, and permits no new prepare. Because reset and shutdown return `void`, a wrap there still completes the inert reset/shutdown transition, invalidates all candidates, retains `CapacityExceeded` in the snapshot/status, and blocks new preparation. Each update supplies `auditImagePresent` plus one complete copied `PanelAuditImage` and `solvePreviewPresent` plus one complete copied parent preview; every false presence flag requires its canonical zero value. Construction and successful initialization publish blank, released, image-unreconciled state from configuration alone. The first image is admitted only through `update()`. Partial child initialization rolls back in reverse order. Reset and shutdown clear volatile solve authority and publish inactive/off intent without erasing or inventing audit records. Destruction delegates to shutdown and ends the lifetime. |
| Time and ordering | **Natural.** One supplied `now` governs each admitted envelope and both child preflights. The frozen precedence is configuration/internal fault, stop, evidence faults in source/configuration/timing/stale/contradiction order, audit indeterminate, invalid chord, then normal progress. Structural invalidity rejects before semantic precedence. Freshness, sequence ordering, rollover, regression, exact-half-range ambiguity, preview lifetime, and copied-image recovery are explicit. One call performs fixed bounded work with no hidden clock, blocking wait, retry loop, recursion, or catch-up loop. |
| Errors and status | **Natural.** `Status` reports whether an invocation or configuration was admitted; `EscapeConsoleDisposition`, child dispositions, audit disposition, and copied snapshots retain domain meaning. A stale clue may be a successfully admitted update with `StaleEvidence`; a malformed envelope rejects atomically. Independently present lower-tier causes remain inspectable even when a dominant cause selects presentation. Presentation failure cannot replace puzzle, stop, audit, or fault status, and acknowledgement cannot clear stop, invalid configuration, internal failure, bad evidence, or unresolved audit state. |
| Resource budget | **Credible but not yet measured.** E0 owns zero pins, timers, interrupts, buses, claims, endpoints, or power domains. Work is bounded by 12 clues, 12 rules, four terms and prerequisites per exercised join, six family summaries, two child policies, one copied two-slot image per update, and one retained candidate. The six canonical parent solve bindings add 20 logical bytes to each audit record before ABI padding; the AVR probe must include that cost in both slots, previews, panel object, and maximum composition. The plan fixes Lesson 057 targets/hard ceilings at 32/40 KiB flash, 4,096/4,608 B static SRAM, 1,024/1,280 B synchronous stack, and 1,024/1,280 B object size. The stack table excludes the separately added 128 B interrupt reserve. At simultaneous hard ceilings the independent margin passes: `8192 - 4608 - (1280 + 128) = 2176 B`, exceeding the required 2,048 B. Each fixed buffer or image is at most 512 B. Compile/run size, trait, stack-path, symbol, and aggregate-composition probes are mandatory before promotion; a hard or residual-margin miss is a blocker, not an implied exception. |
| Deterministic proof | **Naturally testable and mandatory before promotion.** Configuration, timestamps, observations, audit images, child acknowledgements, parent solve previews, presentation evidence, and failure points are copied or supplied. The planned host matrix covers all `2^12` clue masks, family assignments, all ordinary-envelope presence masks and noncanonical absent values, both solve-preview presence states, all 16 operator masks, precedence collisions, exact parent/child preview copies and single-field/cross-pair mutations, owner-token swaps between otherwise field-identical live instances, lifecycle-generation mutation and stale-preview replay after actual initialize/reset/shutdown/reinitialize transitions, repeated initialize/shutdown proving no advance or candidate invalidation, and `UINT32_MAX - 1`, `UINT32_MAX`, and wrap-to-zero boundaries separately for initialize, reset, and shutdown. Initialize wrap must return `CapacityExceeded` and remain uninitialized; reset/shutdown wrap must complete inert state, retain `CapacityExceeded`, invalidate candidates, and block preparation. The matrix also covers every pure-preflight and infallible-apply boundary, audit bootstrap/predecessor/slot/restart states, freshness boundaries, destruction/reconstruction caveats, and two independent fieldwise replays. Failed `Result<T>` values and absent aggregates must be canonical zero. Raw struct bytes or padding are never replay evidence. Host proof does not establish hardware behavior. |
| Packaging and public surface | **Natural, with implementation evidence open.** The planned standalone header and out-of-line implementation can remain native and Arduino-neutral. The umbrella header, native/archive inventories, canonical compile-only Mega sketch, size baseline, HTML reference, rich PDF, downloads, routes, and publication checks must expose the same fixed project vocabulary without duplicated source or hardware exceptions. No first-class file may depend on `legacy/`. |
| Example and documentation fit | **Natural for E0.** The canonical “six stations, one quiet console” replay constructs the configuration and config-only coordinator, then supplies complete copied audit images through atomic updates; it uses acquire/configure/start and observe/decide/actuate flow and renders deterministic result cells rather than hardware effects. It visibly demonstrates family completion, deliberate solve preparation and acknowledgement, and inactive latch intent under stop, stale/contradictory clues, invalid chord, and torn audit image. “Release” remains demonstration intent. Every E0 PDF visual is pencil-classified; any later formal schematic requires an exact E1/E2 fixture and authoritative electrical review. |
| Downstream effects | **Contained if the boundary stays project-local.** Lessons 055 and 056 keep independent ownership and semantics. Existing endpoint, timing, status, audit, packaging, safety, and publication conventions need no exception. Future E1 adapters may consume copied presentation intent but cannot feed rendering success back into primary policy. Future E2 actuation must remain an independently qualified demonstration adapter and cannot turn E0 intent into a physical-state claim. A generic solver, transaction manager, storage layer, actuator API, access decision, or changed shared status/lifecycle rule would challenge prior decisions and requires architectural review before implementation. |

## Composition pressure scenario

The maximum authorized E0 composition is one 12-clue/12-rule Lesson 055
model, including exercised four-term/four-prerequisite joins; all six fixed
families with exactly two clue IDs each; one Lesson 056 panel; one complete
copied canonical two-slot audit image in the update envelope; one retained solve candidate;
one complete atomic update envelope; copied presentation, diagnostic, latch,
lamp, snapshot, and trace/export values; and no hardware.

The maximum collision begins with a solved retained clue generation and a
prepared solve/audit candidate. At one supplied timestamp the fixture presents
stop, malformed or stale/contradictory clue evidence, an exact child audit
acknowledgement, an exact or mutated parent solve preview, an invalid operator
chord, a diagnostic acknowledgement, and failed presentation, with the audit
image also indeterminate. Structural invalidity,
including either noncanonical absent preview, must reject the complete call without
mutation. With structurally valid values, stop must dominate; independent
child causes remain attributable; every candidate becomes unusable where the
contract requires; the canonical audit image cannot be guessed or partially
advanced; and latch intent remains inactive. Recovery through restart and a
fresh copied-image update may recover an attributable audit record but cannot restore
release intent without a fresh qualified clue generation, deasserted stop,
and new operator-confirmed solve transaction.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; bounded by contract, measurement open.** One update validates one fixed envelope, at most 12 clue cells, at most 12 rules with four terms and four prerequisites, one panel transition, one candidate transition, and one six-family summary. There is no queue, blocking I/O, recursion, retry loop, or input-sized work. Tests must bound worst-case simultaneous work and prove conceptual arrival permutations form the same canonical result; exact freshness, rollover, regression, half-range, and missed-update behavior must be replayed. Reviewed compiler stack reports plus a conservative complete call-path and interrupt reserve are required before promotion. |
| Total memory and hardware resources | **Applicable for memory; hardware resources are absent at E0.** Measure every public value, both child objects, the coordinator, copied audit image/update envelope, complete fixture, diagnostics, trace/export storage, flash, static SRAM, and largest synchronous stack path. Prove the 1,024/1,280 B object target/hard gate and all buffer ceilings. The 1,024/1,280 B stack table covers synchronous stack only; add a separate 128 B ISR reserve in the residual check. The hard-limit arithmetic is `8192 - 4608 - (1280 + 128) = 2176 B >= 2048 B`; actual measurements must also pass `8192 - measuredStaticSram - (measuredSynchronousStack + 128) >= 2048`. Report every image and optional buffer separately while including all simultaneously live copies in aggregate SRAM. E0 must report zero pins, timers, interrupts, buses, claim entries, endpoints, and power domains. Test capacity immediately below, at, and above every supported fixed bound. |
| Shared bus or transport | **Not applicable to E0 by contract.** The coordinator and children consume copied values and own no bus, transport, display, keypad, source, or storage medium. The supplied audit image is a copied memory value, not proof of persistence or transport, and creates no retained borrower lifetime. E1 becomes applicable once exact passive input and presentation specimens are selected and must identify ownership, borrower lifetime, addressing, arbitration, bounded transactions, partial acquisition rollback, congestion, participant failure, and restart. A physical storage adapter is outside this arc and requires a separate safety and scope decision. |
| Persistence and recovery | **Applicable only to the E0 supplied-image model.** The two-slot image carries exact provenance, schema/configuration identity, owner epoch, operation identity, the six persisted parent solve bindings, canonical FNV-1a digests, and prepare/acknowledge/recovery state. Slot selection is fixed: preparation replaces only the older committed/empty slot, never the newest committed predecessor. A fully empty image has exactly one bootstrap path: canonical sequence `1`, no predecessor, and the live candidate's exact operation, payload, checksum, configuration, epoch, selected slot, and image digest. Every later prepared record requires the adjacent committed predecessor with the exact preceding modular sequence. Tests must cover both slot mappings; empty bootstrap and every mutation; missing, wrong, gapped, duplicate, backward, and half-range-ambiguous predecessors; every frozen image disposition; clean, prepared, acknowledged, torn, indeterminate, corrupt, and rollover images; failure before every mutation; restart; and recovery through a fresh copied-image update. E0 does not claim a durable write. It may recover an attributable record but cannot guess, silently discard ambiguity, or restore volatile solve/release authority. A physical storage adapter, medium, atomic-write unit, wear policy, synchronization, reread, and failure behavior are outside E1 and require a separate future safety and scope decision. |
| Motion, external power, or stored energy | **Excluded at E0; E2 independently applicable.** The E0 API has no actuation path: latch, lamp, and presentation values are copied semantic intents. E1 permits only separately qualified passive inputs and current-limited presentation with every actuator absent. E2 may later qualify only a restrained demonstration servo or inert current-limited low-voltage relay/lamp load with separate load supply and independent physical power removal. It may never connect to a door, lock, gate, occupied enclosure, egress route, alarm, emergency lighting, or life-safety system. |
| Observation identity and provenance | **Applicable and central.** Each clue preserves configured source, sequence, observation time, quality, validity/status, configuration revision, and clue generation. Parent and child previews each carry an opaque `ownerToken` that distinguishes exact simultaneously live objects plus a nonzero `lifecycleGeneration` that distinguishes successive lifecycles of the same object. Equal revisions, epochs, ordinary generations, operations, and payloads cannot revive a preview when either exact binding differs. A token alone grants no authority; a complete exact fieldwise preview copy is accepted only while its retained candidate and lifecycle remain live. Neither token nor lifecycle generation is persisted or hashed. Every explicit reset, an actual initialized-to-shutdown transition, consumption, or another actual lifecycle advance invalidates the retained binding; repeated initialization or repeated shutdown does not. Destruction ends the lifetime. Reconstructing a later object at the same storage address is outside the cross-reconstruction guarantee: neither field is authentication or a security capability. The parent preview also binds the exact solved clue generation and rule mask, policy digest, complete child preview, and image digest. At commit, resulting intents are recomputed rather than trusted from the caller. Delayed, foreign, stale, same-sequence-colliding, differently configured, or post-reset values cannot be combined into one solve. |
| Diagnostic interference | **Applicable.** Presentation intent/evidence, lamp intent, Serial, trace/export storage, and later display/indicator resources must be included in time and memory budgets. Presentation is computed last. Its absence, delay, mismatch, fullness, or failure cannot change primary disposition, stop handling, acknowledgement eligibility, audit bytes, candidate identity, or latch intent. E1/E2 must additionally prove diagnostic pin/timer/bus/current failures cannot delay inactive physical output and that non-Serial safe-state evidence remains independently visible. |
| Failure collision and recovery | **Applicable.** The deterministic matrix must inject configuration/internal failure plus every cause; stop plus evidence, audit, chord, and solve; each ordered evidence fault plus audit/chord/solve; audit indeterminate plus invalid chord/solve; and valid solve plus the exact child acknowledgement and parent preview. Invalid absent image, acknowledgement, solve-preview payload, or externally supplied `InputRecovered`/`PresentationRecovered` diagnostic rejects before collision policy. Failure injection before every parent preflight, after every child preflight, and at each allowed mutation boundary must leave both children, parent generation, audit image, and intents unchanged unless the one infallible acknowledged transition occurs. Restart reconstructs stop only from the latest attributable committed `StopAsserted`; a blank/unattributable image cannot infer release, and only newer qualified deasserted evidence whose source sequence and observation time are both newer, followed by its exact committed `StopReleased` record, clears stopped state. Equality, backward order, or exact-half-range ambiguity in either comparison cannot release. Restart with any other fault remains inert and requires fresh authority. |

## Parent/child atomicity

The sole ordinary ingress is `EscapeConsoleUpdate`. It validates every presence
flag, canonical absent value, identity, time, acknowledgement, complete copied
audit image, and copied parent solve preview before mutation. In particular,
`solvePreviewPresent == false` requires a canonical zero `solvePreview`; when
true, the parent `ownerToken`, every other parent field, embedded child
`ownerToken`, both parent and child `lifecycleGeneration` values, every other
preview field, current canonical image field and digest, and recomputed result
intent must match the retained capabilities. The coordinator
derives the proposed Lesson 055 result and resulting panel diagnostic, calls
both children's private pure `preflightUpdate()` seams without mutation, and
only after all parent and child checks pass invokes their infallible
`applyPreparedUpdate()` seams and publishes at most one atomic parent
transition. Public child `update()` wraps the same preflight/apply pair. There
is no independent clue, stop, acknowledge, commit, show, or actuator operation
whose call order could alter precedence.

`prepareSolve()` may reserve exactly one parent candidate and the exact child
audit candidate only from a solved retained clue generation, qualified
deasserted stop, clear dominant diagnostic, ready audit image, and no other
live candidate. The copied preview binds all owner, generation, operation,
rule-mask, policy-digest, child-preview, and image-digest fields. Any exact
fieldwise copy is acceptable while the sole private candidate remains live;
the opaque parent and child `ownerToken` values are simultaneously-live
address-identity discriminators, while exact lifecycle generations prevent
same-object resurrection after a lifecycle advance. Neither is caller-selected
identity, persisted provenance, authentication, or a security capability.
Mutation, foreign ownership, new evidence,
changed child state, stop, diagnostic, reset, shutdown, a newly supplied image,
consumption, or reuse invalidates it.

The exact child audit acknowledgement and exact parent preview arrive together
through that envelope. `canCommit()` is deliberately narrower: it checks only
retained parent-candidate liveness and fields carried directly by the preview,
namely `ownerToken`, `lifecycleGeneration`, owner revision/epoch, console
generation, operation, clue generation, satisfied-rule mask, policy digest,
and the complete copied child preview including its own `ownerToken`,
`lifecycleGeneration`, and image digest. It does not inspect a caller image, reconcile
image state, call a child commit, evaluate collisions, or certify that a later
update will commit. `update()` alone validates the current image, exact child
acknowledgement, parent/child pairing, recomputed intent, precedence
collisions, and final commit. A mutated, stale, foreign, consumed,
child-mismatched, image-mismatched, or noncanonical absent preview rejects the
whole envelope without mutation. Complete parent and child preflight at one
supplied timestamp must make the child acknowledgement infallible; the parent
then publishes the same operation, `Solved`
disposition, `RequestDemonstrationRelease`, solved lamp, and canonical audit
result in one mutation. Tests inject failure around every preflight and
mutation boundary. If implementation needs sequential best-effort mutation,
rollback after an observable child change, or a generic transaction manager,
promotion stops for architectural remediation.

The parent configuration identity is independently bound. Parent
`configurationRevision` and `instanceEpoch`, and each child's revision and
epoch, must all be nonzero; parent and child values need not equal one another.
Lesson 057 `policyDigest` is derived, not caller-selected. Initialization
recomputes 32-bit FNV-1a with offset `0x811c9dc5`, prime `0x01000193`, and
domain `"ADK.ESCAPE.POLICY.V1\0"`, then consumes in order:

1. parent revision and epoch;
2. the complete canonical Lesson 055 configuration, including its revision,
   epoch, maximum age, counts, all 12 expected source identities, and every
   used and canonical-unused rule, term, and prerequisite slot;
3. all 12 family enumerators in clue-slot order; and
4. the complete Lesson 056 configuration, including revision, epoch, maximum
   input age, selectable count, and every field of both source identities.

Integers use little-endian declared widths, enums their underlying `uint8_t`,
booleans canonical `0` or `1`, and no padding or object representation.
Initialization rejects a zero revision/epoch, noncanonical unused field,
wrong family mapping, or supplied digest mismatch before either child becomes
live. Tests mutate every consumed field, canonical-unused slot, domain byte,
and child/parent revision or epoch independently; they also cross otherwise
identical parent and child configurations and prove the digest and owner-token
checks reject cross-instance capabilities without mutation.

Audit encoding is fieldwise and frozen. Every nonempty record has
`formatMagic = UINT32_C(0x41444b41)` and
`formatVersion = UINT16_C(1)`; any other value rejects structurally. All three
digests use 32-bit FNV-1a
with offset basis `0x811c9dc5` and prime `0x01000193`; each begins with its
domain tag including the terminating zero byte. Integers are consumed
least-significant byte first at declared width, enums as their underlying
`uint8_t`, booleans only as `0` or `1`, and `Status` as its canonical code.
`payloadDigest` uses `"ADK.PANEL.PAYLOAD.V1\0"` and the normative payload-field
order, including `parentConfigurationRevision`, `parentInstanceEpoch`,
`parentGeneration`, `clueGeneration`, `satisfiedRuleMask`, and `policyDigest`
in declaration order. `checksum` uses `"ADK.PANEL.RECORD.V1\0"` and every
record field in declaration order except `checksum`, again including all six
solve bindings. `imageDigest` uses
`"ADK.PANEL.IMAGE.V1\0"`, then each slot-index byte and every field of slots 0
and 1, including record checksums. Padding and C++ object representation are
never hashed. Preparation, acknowledgement, recovery, and tests must share
these exact algorithms. Tests accept only the frozen magic/version pair and
mutate each constant independently in both slots, previews, prepared and
committed records, including cases with otherwise correctly recomputed
digests, and require structural rejection without mutation.

The image classifier is also frozen:

| Canonical two-slot form | `PanelAuditDisposition` |
|---|---|
| Both slots empty | `PrepareRequired` |
| One committed plus one empty, or two adjacent committed with an unambiguous newest | `Ready` |
| Empty plus the exact live sequence-one prepared record, or committed predecessor plus its exact adjacent live prepared record | `AcknowledgeRequired` |
| Bad format/configuration/epoch, noncanonical record field, or digest/checksum failure | `Corrupt` |
| Canonical records with duplicate, gap, backward or half-range-ambiguous ordering, two prepared records, or an unattributable prepared record | `Indeterminate` |

`Empty` is reserved for the pre-initialization or no-image snapshot; a present
all-zero image is `PrepareRequired`. Two adjacent committed records are
`Ready`, and their older record is the next replaceable slot. This
latest-two-record model has no capacity-full disposition.

Audit admission is closed by kind:

| `PanelAuditKind` | Sole admission path |
|---|---|
| `None` | Never admitted |
| `AcknowledgedDiagnostic` | Panel-only while committing the exact live acknowledgement for the current internally generated `InputRecovered` or `PresentationRecovered`; all six parent solve-binding fields and all stop fields are zero |
| `PuzzleSolved` | Only private friend-only `FaultAwareOperatorPanel::preparePuzzleSolved()` called by `InertEscapeConsole::prepareSolve`; every public `prepareAudit(PuzzleSolved)` attempt rejects. It binds the exact parent configuration revision, parent instance epoch, parent generation, operation, solved clue generation, satisfied-rule mask, and policy digest; diagnostic and stop fields are zero |
| `StopAsserted` | Panel-only for the exact qualified released-to-asserted transition, binding configured source identity, sequence, observation time, and asserted level; all six parent solve-binding fields are zero |
| `StopReleased` | Panel-only for the exact qualified asserted-to-deasserted transition newer than the retained assertion under both sequence and observation-time comparisons; stop remains effective until this record commits and all six parent solve-binding fields are zero |

For `StopAsserted` and `StopReleased`, `stopPresent` is true, `stopAsserted`
matches the kind, and source identity, source sequence, and `stopObservedAt`
exactly match admitted evidence. Every other kind has all stop fields zero. A
repeated stop level is not a transition. No public call may manufacture an
arbitrary kind, diagnostic, or payload.

The `PuzzleSolved` record carries persistent parent attribution rather than an
opaque digest alone. Tests mutate each of the six stored solve-binding fields
individually in the record, payload digest, checksum, child preview, parent
preview, and supplied image and require rejection without mutation. A valid
restart can recover that committed audit fact with exact bindings but cannot
recreate volatile solved or release authority.

The recovered-diagnostic policy remains closed under composition:

| Panel diagnostic | Project acknowledgement result |
|---|---|
| `InputRecovered` at the exact current diagnostic generation | Acknowledgeable after a newer qualified control or stop observation clears the immediately prior attributable input fault |
| `PresentationRecovered` at the exact current diagnostic generation | Acknowledgeable after successful evidence for the exact current presentation-intent generation follows its attributable failure |
| `None` or any other `PanelDiagnostic` | Not acknowledgeable; it cannot clear stop, audit ambiguity, invalid configuration, internal failure, or clue/source/timing/stale/contradictory evidence |

Neither recovered diagnostic may be asserted directly by the caller. An
envelope with `diagnosticPresent == true` and either recovered value is
structurally invalid and rejects without mutation. Successful acknowledgement
of an internally generated recovered diagnostic records
`AcknowledgedDiagnostic` but cannot create solve authority or release intent.

## Inert intent and staged electrical boundary

`EscapeLatchIntent::RequestDemonstrationRelease` is a volatile E0 teaching
value, not authorization, physical release, latch position, or proof that
anything moved. Stop and every fault tier force inactive latch intent.
Construction, failed initialization, reset, restart, shutdown, destruction,
audit ambiguity/corruption, and a new configuration also start inactive.
Audit-image recovery cannot restore it automatically. A restart that recovers
`StopAsserted` remains stopped and inactive until a newer qualified deasserted
observation and its exact committed `StopReleased` audit record complete; even
that release only removes stop and does not restore a prior solved intent.

E1 may qualify only exact passive clue/operator inputs and low-voltage
current-limited indicators or presentation hardware, with servo, relay,
latch, lock, and every other powered actuator physically absent. Its plan must
record voltage, current, polarity, pull/floating behavior, pin/bus ownership,
schematic, unpowered inspection, named observation points, failure injection,
and signed physical evidence.

E2 is a new, separately reviewed hardware boundary. It requires an exact
restrained demonstration actuator or inert relay/lamp fixture, driver and
protection, separate current-limited load supply, guarded geometry, full
simultaneous-load and thermal budgets, independent physical load-power
removal, and named-person bench acceptance. Startup, stop, source loss, fault,
reset, shutdown, destruction, communication loss, stall/jam where applicable,
logic-power loss, and load-power removal must reach the measured inactive
state. Software stop is not an emergency stop. E0 host or compile evidence
cannot satisfy E1 or E2.

## Prior-decision impact

| Decision or authority | Impact |
|---|---|
| `docs/DEVELOPMENT.md` component/project hierarchy | **Preserved.** Lessons 055 and 056 remain reusable policy boundaries; Lesson 057 composes them and owns only project meaning. |
| `docs/STYLE.md` public C++ and implementation layout | **Preserved.** The proposed fixed, explicit, non-owning/owning surface supports standalone headers and out-of-line implementation without Arduino leakage. |
| `docs/CURRICULUM.md` Lesson 057 project role | **Preserved.** A lesson number divisible by three is a multi-component project, and the fixed six-family console composes the two preceding lessons. |
| `docs/TESTING.md` deterministic and hardware evidence separation | **Preserved.** Complete host replay and resource evidence are required; physical claims remain separate and cannot be inferred from CI. |
| `docs/SAFETY_MODEL.md` Lessons 055--057 boundary | **Preserved.** E0 owns no hardware and makes no access, security, confinement, egress, interlock, alarm, or life-safety claim. E1 and E2 retain the exact stricter staged gates. |
| `docs/PACKAGING.md` canonical-source and archive requirements | **Preserved.** One native implementation, one canonical sketch, umbrella/archive registration, and no `legacy/` dependency are planned. |
| `docs/PDF_POLICY.md` visual classification and printable evidence | **Preserved.** E0 visuals are pencil drawings; a formal schematic exists only for a later exact electrically authoritative fixture. |
| Explicit time, `Status`, fixed storage, and no allocation conventions | **Preserved.** Time is supplied; domain dispositions remain separate; all work and storage are bounded; no exception, allocation, recursion, callback, or hidden clock is needed. |
| Lesson 056 two-slot audit decision | **Extended.** Lesson 057 coordinates the existing exact preview/acknowledgement protocol over complete copied images supplied in its atomic update and adds parent binding; it retains no caller reference and does not invent persistence or a second audit convention. |
| Single-ingress collision and diagnostic-isolation policy | **Extended.** The project freezes one cross-child precedence and computes presentation last without weakening either child contract. |
| Mega 2560 resource ceilings and post-stack margin | **Extended.** The larger authorized composition receives a 1,024/1,280 B object target/hard gate, a separate 1,024/1,280 B synchronous-stack gate, an additional 128 B ISR reserve, and a 2,048 B aggregate margin. Even simultaneous hard ceilings leave 2,176 B; measurements remain a promotion gate. |

No reviewed decision is challenged by the proposed E0 contract. A request for
generic constraint solving, more clue families, shared transaction machinery,
endpoint/storage/actuator ownership, multiple ordinary ingresses, relaxed
precedence, unaudited solve, a resource exception, or any real access/egress
role would challenge these decisions and requires user discussion plus a
durable consequential decision before implementation.

## Stress disposition

**Bounded local remediation, reassessed after the ownership repair.** The
architecture is a natural project-local composition and does not require a
shared-contract change. The exact parent-preview commit ingress closes the
prior public-path ambiguity. The remaining
strain is confined to the unpromoted Lesson 057 representation and proof:
aggregate object/static/synchronous-stack/flash sizes and the infallible
parent/child commit seam have not yet been implementation-proven. The repaired
config-only constructor and copied-image ingress remove the prior alias and
borrowed-lifetime pressure. The implementation owner may
use compact masks/indices, remove duplicated derived snapshots, retain only
one candidate, compute presentation from canonical state, and move large
private temporaries into exclusively owned fixed scratch while preserving the
exact public semantics and atomic preflight. Heap allocation, globals,
retained borrowed views, partial mutation, or a budget exception are not local
remediation.

## Gate result

- Disposition: bounded local remediation
- Open risks: implementation could exceed the 1,280 B object, 4,608 B static
  SRAM, 1,280 B synchronous stack, 40 KiB flash, 512 B per-buffer, or 2,048 B
  residual SRAM hard gates after separately adding the 128 B ISR reserve;
  parent/child acknowledgement could prove fallible after
  observable mutation; packaging/publication evidence and exact E1/E2
  fixtures do not yet exist
- Required discussion or decision IDs: none for the exact E0 contract;
  required before any shared abstraction, public-contract change, resource
  exception, physical access/egress use, or materially different E1/E2 outcome
- Remediation owner and next action: Lesson 057 implementation owner; build
  the exact fixed-capacity coordinator and first prove candidate atomicity,
  exhaustive collision/rejection behavior, traits, and the complete
  maximum-composition resource probe before freezing or promoting the public
  shape
- Verification commands and results: document consistency inspected against
  the stress template, detailed Lessons 055--057 plan, Safety Model, and prior
  project stress-pass pattern; implementation, compile, sanitizer, Arduino,
  size, stack, archive, HTML, PDF, link, and physical checks are not yet run
  and remain required
- Maximum-composition scenario and proof: exact 12-clue/12-rule, six-family,
  two-child, one-image, one-candidate E0 scenario is specified above; proof
  remains the required exhaustive replay, injected atomic-boundary failures,
  two-instance fieldwise replay, and measured aggregate resource evidence
- Promotion permitted: **yes only into bounded E0 implementation and
  verification of this exact contract; no for final E0 publication until all
  named evidence passes, and no for E1 or E2 without separate exact physical
  qualification**
