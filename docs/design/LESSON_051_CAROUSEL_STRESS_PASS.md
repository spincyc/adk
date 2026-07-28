# Tabletop-parts-carousel design stress pass

This record applies the
[component design stress pass](../templates/component-design-stress-pass.md)
to the planned Lesson 051 tabletop parts carousel. It is a pre-implementation
architecture review updated after the detailed
[Lessons 049--051 plan](LESSONS_049_051_PARTS_CAROUSEL_PLAN.md) incorporated
the bounded remediation. It is not an implementation or hardware
qualification. It authorizes no RFID, keypad, Hall/reed, EEPROM/SD, LCD,
stepper, servo, wiring, powered motion, or physical-position claim.

## Boundary

- Name and lesson/project: tabletop parts carousel, Lesson 051
- Reviewer and date: pre-implementation architecture review, 2026-07-28
- Public types and operations: planned copied identity request,
  confirmation, homing/position evidence, interruption and stop observations;
  one bounded transactional update; copied gate, motion, display and audit
  intents; explicit lifecycle and inspectable snapshot
- Direct dependencies: planned Lesson 049 fixed-size local identity records,
  planned Lesson 050 bounded homing policy, one project-owned Lesson 047
  bounded logical step sequencer, semantic gate/presentation intent,
  project-local audit records, `Status`, explicit `TimePoint`/`Duration`, and
  fixed-size storage
- Existing decisions and interfaces reconsidered: identifiers are not
  authentication; E0 bounded homing establishes session-local logical
  position only, while physical position remains unproved until separate E2
  powered acceptance; endpoint-owned electrical lifetime; explicit time and
  bounded work; deterministic replay; fail-closed stop/fault behavior;
  storage atomicity; diagnostic isolation; E0 intent versus E1/E2 physical
  evidence

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural only as a project-local coordinator.** Lesson 049 remains the authority for copied identifier/enrollment/storage outcomes and Lesson 050 for unknown/homing/known-position transitions. Lesson 051 alone owns a fixed Lesson 047 sequencer for logical coil vectors and emits only semantic open/closed gate intent with expiry; E0 has no gate-actuator policy. It may authorize and order those policies, but must not parse raw RFID traffic, scan a keypad, debounce a home switch, write EEPROM/SD, drive pins, infer authentication, or turn issued steps into position evidence. Electrical adapters and persistent transports remain below the project. |
| Ownership and lifecycle | **Natural after the plan-fixed bounded remediation.** Construction is inert; caller-owned buffers explicitly outlive non-copyable/non-movable owners; child/candidate identities remain private; initialization completely prevalidates and rolls back in reverse order. The plan does not promise atomic rollback across an external durable write: retryable prepare/durable-admit/apply/terminal-reconcile stages publish one final E0 snapshot and a fixed fail-closed path. Shutdown clears authorization/home epochs and publishes closed/off intent before child teardown; endpoint owners separately prove de-energization. |
| Time and ordering | **Natural if one supplied timestamp governs one admitted frame.** Identity freshness, confirmation expiry, duplicate/repeat suppression, home-sensor evidence, motion deadlines, gate dwell, audit ordering, retries, rollover and same-time changed input need explicit rules. Stop is inspected before new authorization. One call does bounded work and publishes at most one logical motion transition; no blocking home loop, storage wait, servo dwell or catch-up loop is permitted. Delayed inputs retain their original identities and cannot be restamped into one simultaneous transaction. |
| Errors and status | **Natural with project-specific phase/fault enums over existing `Status`.** Outcomes must distinguish unknown/duplicate/locked identity, expired or conflicting confirmation, position unknown, missing/stuck home, travel exhaustion, interruption, storage unavailable/full/corrupt, audit commit indeterminate, diagnostic failure, stop and dependency failure. Precedence is stop or already-latched fault; invalid/unknown position; authorization/confirmation failure; audit-admission failure; ordinary progress; presentation. A display or audit diagnostic must not disguise a motion or gate failure. No diagnostic string, exception or silent fallback belongs in the core. |
| Resource budget | **Limits fixed; measurement remains a promotion gate.** E0 owns zero hardware resources. The plan caps each public object at `128 B`, the Lesson 051 sketch at 28,672 B flash and 2,048 B static SRAM, counts caller-owned buffers and largest live candidates, reserves at least 512 B for conservative stack/ISR, and requires 1,024 B remaining afterward. Future E1/E2 work separately totals pins, claims, timers, buses, interrupts, actuator channels and current. A measured aggregate Mega image, not isolated sizes, establishes fit. |
| Deterministic proof | **Matrix fixed; execution remains a promotion gate.** The repaired plan covers identity/confirmation permutations, expiry, qualified home edges, missing/stuck home, every-phase interruption, every bin, stop collisions, supplied interrupted record images, corruption, capacity bounds, dependency/audit/presentation collisions and two-instance fieldwise replay. Tests must prove the complete durable-start/current-home/exact-position gate-open invariant. Host results never prove physical movement, home detection or de-energization. |
| Packaging and public surface | **Natural if project-local.** The eventual boundary needs an ordinary standalone header, out-of-line implementation, umbrella/archive/native inventories, tests, canonical Mega example, size baseline, HTML and pencil-drawing PDF. Public names describe identity records, homing evidence, authorization epochs, bin selection, gate/motion intents and audit disposition—not RFID registers, EEPROM addresses, GPIO polarity, coil pins or servo pulses. |
| Example and documentation fit | **Natural as an E0-first progression.** The canonical sketch can replay copied evidence into fixed intent/result cells using acquire, configure, start and observe, decide, actuate flow. It must call identifiers local labels rather than credentials. Position, home, gate and stop evidence are separately visible. Every non-schematic PDF visual is a pencil drawing; only an exact qualified, electrically authoritative circuit can use the schematic exception. Powered input/indicator work is E1 and restrained stepper/servo work is E2. |
| Downstream effects | **Contained only if no shared contract is bent for the composition.** Lessons 016--018, 022--024, 034--036, 046--048 and planned 049--050 retain their ownership, timing, persistence, motion and safety meanings. Existing audit buffers are volatile and existing persistent ledgers have domain-specific schemas; neither may be relabeled as a generic transactional project journal. If Lesson 051 needs cross-policy rollback, a shared transaction manager, a generic durable log, persisted physical position, or a new global failure convention, architectural remediation and migration review are required before implementation. |

## Composition pressure scenario

The maximum authorized E0 composition is one copied Lesson 049 identity
record/outcome, one copied keypad confirmation, one copied independent stop
observation, one Lesson 050 homing/position policy, one Lesson 047 logical
step sequencer, one bounded gate-intent policy, one fixed-capacity persistent
audit fixture, and copied LCD, position, home, gate and stop result cells.
There are no pins, powered endpoints or physical position claims in E0.

The maximum collision starts after identity and confirmation are admitted,
while homing or bin travel is pending and an audit record is being committed.
At one supplied timestamp it injects stop, changed identity, expired
confirmation, stuck-home evidence, logical travel exhaustion, interrupted
audit write, display failure and reset/power-loss recovery. The final snapshot
must retain independently attributable failures, publish closed-gate and
all-off motion intent, invalidate position, and never manufacture a committed
authorization or completed move.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open.** Bound identity and confirmation admission, home-policy update, at most one logical step, semantic gate-intent policy, one bounded persistence operation and presentation at the fastest supported cadence. Prove stop inspection cannot be delayed by storage retry, LCD work, home search, chatter, large elapsed jumps or rollover. Retry and recovery advance one bounded stage per call. |
| Total memory and hardware resources | **Applicable; open.** Measure the full canonical Mega ELF and worst-live stack with all policies, candidates, dual persistent slots/scratch, audit capacity, display buffers, result cells and core ISR allowance. Future E1/E2 evidence additionally accounts for every pin, timer, interrupt, bus/address, claim entry, current and power domain. Test capacities immediately below, at and above every supported limit. |
| Shared bus or transport | **Applicable to the planned physical composition, absent from E0.** RFID, LCD and optional SD may contend for SPI/I2C or another transport depending on exact specimens. Before any powered plan, name each bus owner, borrower lifetime, chip-select/address, bounded transaction, arbitration order, partial-acquisition rollback and participant restart. An RFID or display transaction may not hold a bus while motion/stop processing waits. E0 consumes copied records only and proves no transport behavior. |
| Persistence and recovery | **Applicable and promotion-blocking.** “Audit records survive a simulated interrupted write” requires a named fixed-size schema, configuration/project identity, monotonic generation/sequence rules, checksum, capacity/full policy, commit marker and synchronization semantics, exact cut-point fault injection, corruption/erased/ambiguous-slot recovery, retry identity and wear budget. Authorization must not become effective merely because an intent to append exists. An indeterminate commit is recovered from durable bytes before retry; the same logical event cannot be duplicated under a new identity. Persisted step counts never establish physical position after reset: recovery starts gate-closed, motion-off and position-unknown, then requires fresh bounded homing. |
| Motion, external power, or stored energy | **Applicable; E2 remains independently blocked.** E0 publishes only copied intent. E1 may qualify exact inputs, storage and inert indicators with actuators absent. E2 requires exact stepper/driver, servo, supplies, protection, current limits, restraint, guarded lightweight paper parts, bounded homing/travel, gate geometry, command expiry, de-energized startup/reset/fault/shutdown and independent physical removal of actuator power. Stop intent and a stop LED are not an emergency stop or proof of removed power. A gate cannot open while position is unknown, homing, moving, interrupted or faulted. |
| Observation identity and provenance | **Applicable; open.** Each frame preserves source kind/ID, timestamp and sequence, validity/status, configuration revision, identity-record generation, confirmation epoch, home-evidence epoch, homing generation, target bin and audit event identity. Evidence from different epochs cannot be combined. A repeated RFID presentation or held key cannot authorize a new cycle after expiry, reset or recovery. The audit record links the authorization and terminal outcome without claiming the identifier authenticates a person. |
| Diagnostic interference | **Applicable; open.** LCD, position LEDs, home LED, coil and semantic gate-intent mirrors, stop LED, optional Serial and audit storage share time, memory, transports and possibly pins. Filling, disabling, disconnecting or failing any diagnostic cannot change authorization, homing, travel, gate-close intent or stop precedence. If durable audit admission is an explicit precondition to starting a cycle, its failure prevents new motion; that is policy, not diagnostic side effect. Once motion exists, audit failure still forces the named safe transition without delaying close/off intent. |
| Failure collision and recovery | **Applicable; open.** Structural malformed input without independent stop/fault rejects without mutation. A separately valid stop commits closed-gate/all-off intent and invalidates pending authorization and position even when another field is malformed. Before any new motion, identity plus confirmation plus target must be admitted, the authorization-start audit event must be durably resolved, and homing state must be valid for the commanded phase. If later audit persistence fails, primary safe-state intent publishes first while terminal evidence remains pending/failed and attributable. Recovery reads durable state, reports ambiguity, starts inhibited and unknown, then requires faults absent, a fresh identity/confirmation epoch and successful homing; it never resumes an interrupted move or gate dwell. |

The transactional composition cannot promise one atomic commit across RAM
policies, durable media and eventual physical writes. The bounded local
protocol must instead define irreversible boundaries:

1. validate one complete copied frame and independently recognize stop;
2. prepare identity, confirmation, homing/motion, gate and audit candidates
   without mutation;
3. durably resolve the authorization-start record before motion becomes
   eligible;
4. commit one project state and publish only its final E0 intents;
5. let separately qualified adapters apply closed/off before any diagnostic
   or storage follow-up when a stop/fault occurs; and
6. durably resolve a terminal record with the same operation identity,
   retaining pending/failed attribution until recovery.

The repaired detailed plan adopts the required choice: externally
acknowledged authorization-start durability is mandatory before motion
eligibility. Unaudited operation is not a fallback. The acknowledgement
remains an E0 simulated boundary over supplied images; actual nonvolatile
durability remains E1.

## Post-remediation closure

The repaired detailed plan closes every bounded pre-implementation issue
identified above without changing a published dependency:

- `InertPartsCarousel` is a project-local coordinator. Lesson 049 owns local
  identity/image semantics, Lesson 050 owns session-local homing, and Lesson
  051 owns one fixed Lesson 047 logical coil sequencer plus semantic
  open/closed gate intent with expiry. The project owns no endpoint, bus,
  clock, driver or durable medium.
- The coordinator borrows explicit `LocalIdentityRegistry`,
  `BoundedHomingPolicy`, and canonical audit-slot and candidate-byte
  workspaces; it owns its fixed Lesson 047 child. Every borrow must outlive it
  and is exclusively coordinated while initialized; no borrowed child or
  workspace state is implicit. Candidate generations remain private,
  retained-storage objects are non-copyable/non-movable, initialization
  completely prevalidates and rolls children back in reverse order, and
  shutdown publishes closed/off intent before invalidating authorization and
  home epochs.
- One supplied timestamp and one completely validated copied frame govern one
  bounded stage and at most one logical transition. Source half-range,
  freshness/skew, changed same-sequence, confirmation-expiry, gate-expiry and
  no-catch-up rules are explicit.
- Independently valid stop has same-frame precedence even when unrelated fields
  are malformed. The plan fixes the remaining exact fault order and prevents
  presentation or audit follow-up from authorizing or prolonging actuator
  intent.
- The cross-medium protocol is project-local and explicit: prepare candidates,
  export a retryable authorization-start record, wait for external
  write/synchronize/reread acknowledgement whose owner, generation, operation,
  slot, canonical bytes and checksum all match and whose synchronization,
  reread-validation and durable status all succeed; admit motion only then,
  publish terminal closed/off intent, and reconcile the terminal record under
  that operation ID.
- The audit format is fixed at no more than eight canonical 40-byte records
  with magic, schema, length, project ID, operation and authorization epochs,
  strictly advancing sequence, occurrence, kind, phase, bin, non-security
  digest, home epoch, status, reserved bytes and checksum. Recovery accepts
  only a canonical prefix, rejects ambiguous/corrupt structure, and never
  creates a new operation identity for an indeterminate retry. Admission
  atomically reserves adjacent start and terminal slots, rejects one remaining
  slot as `AuditFull`, never exposes a start in the final slot, and recovers an
  interrupted start only into its erased reserved terminal slot.
- Reset, interruption and recovery begin inhibited, gate closed, motion off
  and position unknown. Persisted records or issued steps never establish
  physical position; a fresh qualified homing session is required.
- The complete gate-open invariant now requires known identity, exact current
  confirmation, acknowledged durable start, successful current-session home,
  exact target position, inactive stop and healthy/current evidence.
- Confirmation is complete rather than implicit: configuration provides an
  exact `1..4` digit width and one explicit width-valid decimal code per bin;
  leading zeros are significant, digit/confirm/cancel keys are distinct, and
  the completed entry is compared with the selected bin's configured code,
  never raw identifier bytes.
- E0 owns zero pins, timers, interrupts, buses, claims, endpoints, supplies and
  storage transports. The plan caps the coordinator and each child at `128 B`,
  the sketch at 28,672 B flash and 2,048 B static SRAM, requires a conservative
  stack/ISR allowance of at least 512 B, and requires 1,024 B remaining after
  globals plus that allowance.
- The deterministic matrix now names all identity/confirmation/bin cases,
  qualified home edges and bounds, every-phase stop collisions, interrupted
  audit images, child/audit/presentation fault attribution, shutdown/restart,
  and two-instance fieldwise replay. Packaging, compile-only examples, HTML,
  pencil-PDF publication and canonical-document reconciliation are explicit.

These are architecture-plan closures, not test results. The following remain
promotion gates:

- implement the three plan-fixed surfaces and execute the exhaustive matrices
  plus the eight-binding/eight-bin/eight-audit-slot maximum collision;
- prove the published Lesson 047 preview/commit seam remains infallible after
  the stated preflight without changing its public contract;
- measure object sizes, complete Mega ELF flash/static SRAM, largest-live
  stack/ISR margin, bounded update cadence and capacity below/at/above limits;
- complete standalone-header, strict/custom/sanitizer, archive, Arduino,
  package, umbrella, example, HTML, pencil-PDF, site and canonical-document
  gates; and
- retain exact E1 input/storage electrical and durability evidence and exact
  E2 powered-motion, restraint, de-energization and named-person acceptance as
  separate open work.

The E0 implementation is explicitly permitted against this repaired plan.
That permission authorizes writing and testing only the fixed E0 surfaces,
workspaces, compile-only examples and inert publication artifacts. It does not
grant promotion, powered work, durable-media claims or physical-position
claims; promotion still awaits the evidence above and the post-implementation
stress pass.

### Latest normative closure addendum

The later normative repair preserves the closure above and makes its storage
and attribution requirements implementation-exact:

- The borrowed `CarouselConfig` and its bin/key tables must outlive the
  coordinator and remain stationary and immutable. The separately borrowed
  identity, homing and gate children own and validate their own configurations;
  the carousel neither embeds nor duplicates them.
- Audit storage is an exact byte extent, not an array of padded semantic
  structs. Capacity must be even and exactly `2`, `4`, `6`, or `8`; extent
  equals `capacity * 40`, stride equals `40`, and the candidate workspace is
  exactly 40 bytes. Null, overlap, extra/short extent, wrong stride, odd
  capacity and ambiguous workspace layouts reject initialization.
- The 40-byte little-endian record layout is normative down to offsets:
  header and project/operation/authorization/sequence/occurrence fields occupy
  bytes `0..19`; kind/phase/bin/typed audit status bytes `20..23`; identity
  digest, nonzero binding revision, identity-image generation and home epoch
  bytes `24..35`; zero reserved bytes `36..37`; and checksum bytes `38..39`.
  CRC-16/CCITT-FALSE covers bytes `0..37` with polynomial `0x1021`, initial
  value `0xffff`, no reflection and final XOR `0x0000`.
- Start/terminal pairing is exact. Adjacent records share project
  configuration, operation, authorization epoch, bin, identity digest,
  binding revision and identity-image generation. Start is kind `1`, an
  admission phase, success, and zero home epoch. Ordinary terminal is kind
  `2`; recovered terminal is kind `3`, phase `Fault`, typed
  `RecoveredInterrupted`, and zero home epoch. Only the enumerated terminal
  fields may differ. Sequence advances use modular 16-bit deltas
  `1..0x7fff`; zero duplicates, `0x8000` is ambiguous, and larger deltas
  regress.
- A successful rich acknowledgement validates owner, candidate generation,
  operation, slot, view metadata, checksum, byte identity, synchronization,
  reread and durable status, installs the record, consumes the candidate and
  advances state without a later fallible commit. Failed or indeterminate
  evidence retains the candidate, latches reconciliation-required and blocks
  reset, replacement and new input work. An exact repeated acknowledgement is
  idempotent; a changed repeat is a typed audit fault without mutation.
- Recovery never synthesizes identifiers. An interrupted start may produce
  only the recovered terminal defined above in its already reserved erased
  adjacent slot. A start in the final slot or a non-erased mismatching reserved
  slot is corruption, not a recoverable partial transaction.
- Generic `Status` remains the transport/lifecycle category while
  `IdentityDisposition`, `HomingFault`, `CarouselFault`, and
  `CarouselAuditStatus` retain exact domain attribution. Corrupt, unsupported,
  capacity, lifecycle, evidence/timing, indeterminate/storage, gate and
  presentation outcomes therefore cannot collapse into one generic fault.
- Stop and fault attribution are orthogonal. A valid stop always clears coil
  intent, publishes semantic closed-gate intent, and cancels new authority,
  including beside malformed unrelated evidence, but it never erases,
  replaces or downgrades an already latched `Fault` phase or its cause. A
  malformed stop cannot exercise the independent-stop exception.

These additions remove representational ambiguity; they do not add evidence.
They therefore preserve **E0 implementation permitted: yes** and
**Promotion permitted: no** until the implementation, exhaustive malformed
layout/pair/acknowledgement/recovery matrix, measured gates and
post-implementation stress pass succeed.

### Final child-and-input closure addendum

The latest plan supersedes the earlier tentative child composition while
preserving the transactional, storage and safety closures:

- Lesson 050 is now a standalone pure home-evidence coordinator. Lesson 051
  owns exactly one fixed `BoundedStepperSequence`, constructs it from validated
  bin extrema with `logicalStepInterval` as both interval bounds,
  `maximumStepCommandAge`, and project-fixed `holdAtRest = false`, and owns its
  private preview. A caller cannot substitute a differently configured
  sequencer.
- The coordinate contract is native and offset-free. Lesson 050 requires
  `homeLogicalPosition == 0` and bounds satisfying `minimum < 0 < maximum`.
  Lesson 051 derives its owned Lesson 047 bounds from the union of every bin
  position, zero, and Lesson 050's immutable validated `excursionBounds()`.
  Union comparison uses signed 64-bit intermediates, rejects either endpoint
  outside `int32_t`, and requires final minimum `< 0 <` final maximum. Release,
  search and bin travel in either configured direction therefore share one
  native coordinate and remain representable. Rebasing, arbitrary home
  translation and a separate carousel offset are forbidden.
- For each ordinary non-stop motion-capable frame the project derives a Lesson
  050 `HomingPreview`, translates its signed step into an exact one-step Lesson
  047 command, derives the Lesson 047 preview, validates both owner/generation
  identities and both `canCommit()` results, and commits each exactly once
  only after no fallible work remains. Either preview failing mutates neither
  policy. Published coil bits come only from the committed Lesson 047
  snapshot; Lesson 050 never observes them.
- Stop is the deliberate safety exception to joint preview and ordinary work.
  After independently validating stop, the coordinator inspects its
  exclusively owned Lesson 047 child before validating unrelated fields. If
  the child is live (`Moving`, `Holding`, or nonzero coil intent), it calls
  direct `stop(now)` first, then commits the Lesson 050 stop. If idle, it does
  not mutate or advance the child: project-fixed `holdAtRest = false` already
  guarantees zero coil intent, so only Lesson 050 stop commits. An unexpected
  direct-stop failure invokes the child's all-off `reset()` fallback, latches
  `CarouselPhase::Fault` with `PositionFault`, preserves the original stop
  attribution, commits Lesson 050 stop, and permits no identity,
  confirmation, home, motion, gate, audit or presentation work in that call.
- Qualified home acquisition is a second synchronization boundary, not an
  ordinary joint step. After its earlier-precedence stop, malformed/faulted
  evidence and child-fault gates pass, Lesson 051 derives but does not commit
  the Lesson 050 acquisition preview. It directly stops a live Lesson 047
  child, unconditionally resets that child, and verifies logical position
  zero, inactive phase and zero coil intent before committing the Lesson 050
  preview that publishes home epoch and logical zero. No Lesson 047 preview
  survives the reset generation change.
- A qualified-home direct-stop failure still resets all-off but latches
  `PositionFault`, leaves position unknown and does not commit home success.
  Failure of the post-reset zero/inactive/off invariant has the same result.
  An independent stop colliding with the qualified home edge takes the stop
  path only; it never enters home synchronization or publishes a home epoch.
- E0 has no gate-actuator child. `CarouselGateIntent::Open` and `Closed` plus
  `gateExpiresAt` are semantic project results. Servo position, pulse, timer,
  endpoint and physical gate state remain absent; a future E2 adapter may
  translate the semantic intent only after its separate qualification.
- Key input is one copied, timestamped batch, not an implicit sequence of
  per-call key edges. A present batch contains at most four qualified decimal
  digits with canonical zero tail and independent confirm/cancel flags.
  Same-batch cancel plus confirm is representable; cancel dominates confirm
  and digits, records `ConfirmationConflict`, and grants no authority.
  Canonically absent key/identity payloads are fully zeroed with their named
  synthetic source and `Ok`; garbage in an absent payload rejects the frame.
- Presentation health is copied timestamped evidence and is preflight-only.
  Its failure remains independently typed and can never alter authorization,
  home/motion commit, gate intent or audit meaning.
- A `Known` Lesson 049 result carries the exact nonzero matched binding
  revision from the committed identity image. Every non-known result carries
  zero. Lesson 051 binds that revision and the identity-image generation into
  both audit records, so later record-pair validation proves which committed
  mapping authorized the bin rather than merely repeating a digest.
- The maximum fixture now borrows only Lessons 049 and 050, owns its fixed
  Lesson 047 child, and receives explicit audit bytes/candidate workspace.
  Whole-composition size and stack measurements include the owned sequencer.
  Exceeding the `128 B` public-object ceiling stops for design review rather
  than weakening Lesson 047 or hiding state.

This addendum is the controlling final composition where it differs from
earlier review text. It keeps **E0 implementation permitted: yes** because the
joint preflight, semantic gate, copied batch/presentation evidence and audit
binding are fully specified. **Promotion permitted remains no** until their
exhaustive implementation, aggregate measurement and post-implementation
review pass.

The implementation matrix must specifically prove:

- Lesson 050 rejects nonzero home and either bound that does not strictly
  surround zero;
- every valid Lesson 051 bin-map shape shares native logical zero with the
  owned Lesson 047 child and covers the full Lesson 050 release/search
  excursion union, while duplicate, degenerate, non-spanning and
  out-of-`int32_t` union bounds reject without child initialization;
- live-child stop calls direct `stop(now)`, clears coil intent before unrelated
  validation, preserves stop attribution, and advances no ordinary work;
- idle-child stop leaves the Lesson 047 generation and snapshot unchanged
  while Lesson 050 reaches its stopped state;
- direct-stop failure executes all-off reset, latches `Fault`/`PositionFault`,
  retains the stop cause and performs no ordinary work; and
- qualified home with live and idle Lesson 047 states at nonzero pre-home
  coordinates directly stops when live, resets unconditionally, verifies
  zero/inactive/off, invalidates every pre-reset preview, and only then commits
  Lesson 050 home success;
- qualified-home direct-stop failure and zero/inactive/off verification
  failure both retain all-off `Fault`/`PositionFault`, leave position unknown
  and publish no home epoch;
- independent stop colliding with a qualified home edge executes only the stop
  path, including its live/idle/fallback rules, and never resets for or commits
  home acquisition; and
- the live, idle and fallback cases collide independently with malformed
  identity/key/home/presentation fields, pending audit reconciliation and an
  already latched fault without weakening the established precedence.

## Prior-decision impact

- Local identifiers are not authentication, access credentials or proof of a
  person: **preserved**.
- Position remains unknown across construction, initialization, reset, power
  interruption and failed/ambiguous homing: **preserved**.
- Issued logical steps are intent, not movement or position evidence:
  **preserved**.
- Explicit supplied time, bounded work, deterministic precedence and no
  blocking catch-up: **preserved**.
- Fixed storage, corruption/torn-write recovery and deterministic replay:
  **extended** to a project-specific audit schema; a generic storage contract
  is not yet justified.
- Endpoint-owned pins, claims, buses, electrical lifetime and shutdown:
  **preserved**.
- Stop dominance, closed gate and all-off motion on fault/shutdown:
  **extended** across the composition without calling software an interlock or
  emergency stop.
- Presentation failure cannot change primary authorization, motion, position
  or gate behavior: **preserved**.
- E0 copied intent before exact E1 inputs/storage/indicators and exact E2
  actuators: **preserved**.
- Exact specimen, ratings, supply, protection, restraint and named-person
  physical acceptance before powered claims: **preserved**.
- Pencil presentation for every non-schematic PDF visual: **preserved**.

No published interface is challenged. The repaired plan resolves the local
cross-medium strain with project-local owner/candidate generations, operation
and authorization identities, retryable external durable acknowledgement,
bounded prepare/admit/apply stages, safe intent before terminal
reconciliation, and canonical-prefix recovery. This remains a planned
protocol until its exhaustive implementation fixtures pass.

If implementation requires a generic transaction manager, changes an
existing persistent-record schema, persists “known position” across reset,
allows motion before durable audit resolution, needs unbounded recovery or
storage queues, borrows endpoint/bus ownership into the project, changes
published status/time semantics, or permits a diagnostic write to delay safe
intent, the disposition becomes **architectural remediation required**.
Promotion stops while affected consumers, compatibility/migration costs,
safety consequences and bounded alternatives are discussed and recorded in a
durable decision.

## Stress disposition

**Natural fit after bounded local remediation.** The repaired detailed plan
closes the pre-implementation architecture strain without changing a
published dependency. It fixes exact child boundaries, caller-owned storage,
private transaction identities, independent stop admission, durable
authorization-start acknowledgement before motion, gate-open invariants,
terminal reconciliation, bounded recovery, quantitative E0 budgets and
explicit E1/E2 exclusions.

E0 can prove copied-input admission, exact intent vectors, interruption
recovery and semantic replay. It cannot prove an RFID read, persistent-media
electrical behavior, home sensing, physical position, movement, gate closure,
de-energization or safety. Those remain separate E1/E2 gates.

## Gate result

- Disposition: natural fit after bounded local remediation
- E0 implementation permitted: yes, strictly against the repaired detailed
  plan and without widening any E1/E2 or durability claim
- Closed pre-implementation risks: project-local API/layering; caller-owned
  lifetimes; shared native logical-zero coordinate and full homing-excursion
  union; independent live/idle direct-stop and all-off fallback; qualified-home
  stop/reset/zero-off synchronization; prepare, durable-admit, apply and
  terminal-reconcile ordering; exact audit encoding/capacity/recovery;
  source/authorization/home/operation provenance; gate-open invariant;
  failure precedence; bounded work; quantitative E0 budgets; E1/E2 exclusions
- Residual gates: exhaustive implementation and collision replay; proof that
  the published Lesson 047 preview/commit and direct-stop/reset seams satisfy
  the plan; native-zero/bin-plus-excursion union, live/idle/fallback stop, and
  qualified-home stop/reset/verify/collision matrices;
  object/flash/static-SRAM/stack/ISR/cadence measurement; packaging, example,
  HTML, pencil-PDF, site and canonical-document integration; exact E1
  storage/input evidence and exact E2 powered-motion acceptance
- Required discussion or decision IDs: none for the repaired E0 plan; required
  if implementation permits unaudited motion, persists known position,
  changes `BoundedStepperSequence`, `FixedStorage`, status or lifecycle
  contracts, introduces an E0 gate-actuator child, cannot make the joint
  Lesson 050/047 preflight infallible after validation, or selects any
  remediation trigger above
- Remediation owner and next action: Lessons 049--051 implementation owner
  builds the plan-fixed surfaces and full maximum-composition fixture; the
  post-implementation reviewer repeats this pass with measured evidence
- Verification commands and results: canonical queue, cadence, curriculum,
  project catalog, development, safety, PDF policy, stress template and
  relevant existing motion/persistence surfaces and repaired detailed plan
  inspected; implementation, compile, size, package, lesson, site and hardware
  checks not run because this remains a pre-implementation pass
- Maximum-composition scenario and proof: scenario and required collision
  replay specified above; deterministic fixture, aggregate measurement and
  all physical evidence remain open
- Promotion permitted: no
