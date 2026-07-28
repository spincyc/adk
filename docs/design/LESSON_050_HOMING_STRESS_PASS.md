# Lesson 050 bounded homing architecture stress pass

This is the pre-implementation and post-plan-remediation architecture stress
pass for Lesson 050
positioning and homing in the Lessons 049--051 identity-controlled parts
carousel arc. It evaluates a volatile E0 policy over copied home-sensor
evidence and logical motion intent. It does not authorize a Hall or reed
adapter, a stepper renderer, energized motion, an absolute-position claim, or
physical acceptance.

## Boundary

- Name and lesson/project: bounded homing and position policy, Lesson 050
- Reviewer and date: pre-implementation design pass, 2026-07-28
- Proposed public responsibility: qualify a bounded home acquisition from
  copied sensor evidence, publish logical motion requests, and expose position
  as known only after the complete homing protocol succeeds
- Direct dependencies: `Status`, explicit-time values, and fixed-width copied
  observations; no Lesson 047 dependency
- Existing decisions and interfaces reconsidered: semantic intent versus
  physical position, unknown-until-homed recovery, explicit time,
  one-request-per-commit work, preview/commit atomicity, stop dominance, fixed
  storage, and E0/E1/E2 evidence separation

The final implementation-depth plan fixes a standalone pure-policy surface.
It deliberately removes the earlier proposed Lesson 047 composition from
Lesson 050; the historical reviews and their supersession are recorded below.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural as a standalone pure homing policy.** Lesson 050 admits copied home/stop evidence and publishes only semantic signed one-step request and stop intent. It owns no sequencer, coil vector, endpoint, adapter, or physical-position evidence. Lesson 051 alone translates an accepted `HomingPreview` into its separately owned Lesson 047 transaction, so neither published API is reinterpreted or changed. |
| Ownership and lifecycle | **Natural with unknown position as the inactive invariant.** Construction, failed initialization, reset, shutdown, source change, interrupted homing, stop during motion, and accepted policy failure leave step request cleared, stop intent asserted where applicable, and position unknown. The policy owns only copied configuration and fixed logical state; it borrows no child, endpoint, buffer, or storage. It is non-copyable/non-movable. A stop while already idle or homed may retain known position because no displacement was in progress. |
| Time and ordering | **Applicable and fixed.** Every preview supplies time; no hidden clock, delay, ISR, blocking seek, or catch-up loop is allowed. Directional occurrence admission rejects future/ambiguous evidence before age and skew. Release/search step and duration bounds, exact-bound acceptance, late calls, same-time identity, stop/fault precedence, and one semantic signed-step request per accepted commit are explicit. |
| Errors and status | **Natural with semantic homing outcomes plus existing `Status`.** Invalid configuration/frames are non-mutating status failures. Searching, clearing, approaching, homed, stopped, missing-home, stuck-active, motion-fault, and source-fault are semantic outcomes with retained attribution. Stop dominates motion. Structural invalidity is checked before semantic precedence. A failed motion candidate cannot be relabeled as missing home, and invalid sensor evidence cannot be treated as inactive. |
| Resource budget | **Applicable; quantitatively fixed for implementation.** E0 owns no pins, timers, interrupts, buses, ADC channels, supplies, heap, actuator, child object, or caller-owned storage. The plan sets a 112/128-byte Lesson 050 object target/ceiling, 16,384-byte flash and 1,024-byte static-SRAM sketch ceilings, at least 256 bytes of stack/ISR allowance, and at least 768 bytes remaining after globals and that allowance. Lesson 051 accounts separately for its own Lesson 047 child and both previews. Measurement remains a post-implementation promotion gate. |
| Deterministic proof | **Naturally expressible once the protocol is fixed.** Host replay must cover inactive-start and active-start acquisition, bounce, exact qualification/release/deadline/step bounds, never-active and never-released sensors, polarity/source changes, reversal, same-time changed evidence, rollover, malformed enums/status, command rejection, stale/foreign previews, stop and fault collisions, reset at every phase, and byte-stable replay. No host trace is physical homing evidence. |
| Packaging and public surface | **Natural.** One declarative header and out-of-line source expose `preview`/`canCommit`/`commit` plus lifecycle and snapshot. `HomingPreview` is owner/generation bound. There are no callbacks, pins, Arduino conditionals, child references, coil fields, or generic motion abstractions. |
| Example and documentation fit | **Natural at E0.** The canonical sketch replays copied sensor frames, writes logical result cells, and visibly distinguishes unknown, clearing, searching, homed, and fault states without claiming GPIO or motion. HTML documents the API; the PDF teaches the release-then-approach rule, bounds, recovery, and intent-versus-position distinction. Every non-schematic visual is a pencil drawing; no authoritative electrical schematic exists before exact E1/E2 qualification. |
| Downstream effects | **Contained if position authority remains local and revocable.** Lesson 051 may require a current successful home epoch before accepting a bin move or gate-open intent. Reset, stop during homing/motion, motion fault, sensor-source change, or interrupted movement invalidates that epoch. Lessons 047--048 remain unchanged. Persisted identity or audit records from Lesson 049/051 cannot persist position authority or bypass a fresh home after restart. |

## Required homing protocol

An inactive initial home observation may enter the bounded approach phase
directly. An active initial observation is ambiguous: the carriage may be
correctly at home, stopped anywhere over a broad target, wired with reversed
polarity, or reporting a stuck sensor. Therefore active-at-start is never
accepted as home.

The protocol is:

1. admit one structurally valid, qualified copied home observation;
2. when initially active, command a bounded move away until a qualified
   inactive release is observed;
3. fail `stuck-active` without further intent if release does not occur inside
   both the configured step and time bounds;
4. approach home in the configured direction, again under independent step and
   time bounds;
5. accept home only on a qualified inactive-to-active acquisition associated
   with the same source/configuration domain and an admitted approach frame;
6. stop logical motion before publishing a new home epoch and logical position
   zero; and
7. invalidate the home epoch on reset, shutdown, stop during homing or
   movement, motion/source failure, source/configuration change, or any
   interruption whose completed physical displacement cannot be established.
   An independently valid stop while already idle or homed may retain known
   position because no displacement was in progress.

Qualification may consume a previously supported copied contact observation,
but Lesson 050 must not duplicate debounce or silently reinterpret raw
electrical levels. If the chosen observation type cannot prove a qualified
release and acquisition with source, revision, time, sequence, polarity, and
status, planning is blocked until that copied boundary is specified.

The release and approach phases use distinct policy request identities. Bounds
apply to issued semantic one-step requests; E0 calls them search bounds, not
measured distance. Exhausting a bound without a qualifying edge yields
`stuck-active` during clearing or `missing-home` during approach. It never
establishes an applied step or physical travel.

## Atomic preview contract

Lesson 050 performs no child operation. For each candidate it must:

1. validate complete frame structure, identity, time, and source domain;
2. derive one side-effect-free owner/generation-bound `HomingPreview`;
3. verify that candidate is current and committable;
4. commit it exactly once, with no fallible validation remaining; and
5. publish one snapshot whose sensor evidence, signed-step/stop intent, state, and home
   epoch all describe that same accepted frame.

A foreign, stale, changed, replayed, or consumed preview rejects without
policy mutation. Lesson 051 may jointly preflight this candidate with its own
Lesson 047 preview, but that composition cannot add child or coil meaning to
Lesson 050.

Home acquisition and motion completion at one timestamp require an explicit
fieldwise identity rule. The recommended conservative rule is that a
qualified acquisition can stop the approach in that frame, while the new
home epoch becomes visible only in the jointly committed result. Stop, source
fault, or motion fault at the same timestamp prevents home publication and
retains the dominating attribution.

## Composition pressure scenario

The maximum currently authorized E0 composition is:

```text
copied identified home observation + explicit stop/fault frame
  -> Lesson 050 standalone HomingPreview
  -> semantic signed one-step / stop intent
  -> Lesson 051 identity-confirmed carousel policy
  -> project-owned Lesson 047 preview
  -> inert position/home/gate result cells
```

The stress collision is an initially active sensor that qualifies release at
the clearing bound, then qualifies acquisition at the approach bound while a
due semantic step, stop assertion, source failure, and stale homing preview
compete. Lesson 051 separately proves joint homing/sequencer admission. No
powered endpoint or gate is authorized in this scenario.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open implementation gate.** Work per preview/commit must be O(1), independent of search bounds and lateness. Replay exact phase deadlines, one tick before/after, fastest cadence, delayed calls, rollover, repeated timestamps, and the maximum collision. At most one semantic step request is published per commit; no catch-up loop may delay stop or identity processing. |
| Total memory and hardware resources | **Applicable; open implementation gate.** Measure Lesson 050 alone and the full Lesson 051 live composition, including both policy previews, copied identity and home frames, the project-owned sequencer, display/result cells, worst live stack, flash, and safety margin. E0 hardware totals are zero. |
| Shared bus or transport | **Not applicable to E0.** All inputs and outputs are copied values and the policy owns no transport. Future RFID, LCD, persistence, or shift-register buses belong to qualified owners in Lesson 051 and cannot be borrowed implicitly through this policy. |
| Persistence and recovery | **Applicable by prohibition.** Position and home epoch are intentionally volatile. Restart always begins unknown and requires a fresh bounded home, even if an audit record says a prior home succeeded. Persisting configuration or an interrupted-write audit cannot restore position authority. Any proposal to persist or reconstruct position needs a separate schema, provenance, torn-write, wear, and physical-continuity decision. |
| Motion, external power, or stored energy | **Applicable at E2 and a blocker for physical claims.** Exact sensor, driver, motor, supply, travel fixture, approach direction, speed, release distance, overtravel margin, independent stop, power removal, stall, reset, and colliding-fault behavior require authoritative sources and bench evidence. Firmware bounds and all-off intent do not prove that the carriage stopped or moved. |
| Observation identity and provenance | **Applicable and central.** Each frame retains sensor kind/source/revision, configuration or polarity revision, observation time, sequence, qualification state, and status. The home epoch identifies the successful acquisition frame and homing attempt. Values from different sources, revisions, attempts, or times cannot be combined, restamped, or treated as one edge. |
| Diagnostic interference | **Applicable.** Home/unknown/position LEDs, LCD, logical coil mirrors, optional Serial, and audit storage have independent evidence meanings and resource budgets. Filling, failing, or disabling them cannot alter bounds, qualify an edge, delay stop, publish home, or open a gate. A home LED shows accepted copied policy state, not sensor voltage or physical alignment. |
| Failure collision and recovery | **Applicable; exhaustive precedence table required.** Structural invalidity rejects without mutation. Independent stop then accepted source/motion faults dominate due motion and home acquisition. Clearing timeout yields stuck-active; approach timeout yields missing-home. Every terminal path clears motion intent, invalidates home, retains the exact source, phase, attempt, and bound evidence, and requires faults cleared plus a new explicit homing attempt. |

## Prior-decision impact

- Lesson 047 logical count and coil intent are not physical position:
  **preserved by separation**; Lesson 050 has no Lesson 047 dependency and
  adds only a separate, revocable synthetic home epoch.
- Unknown position after construction, reset, interruption, or power loss:
  **preserved**.
- Explicit supplied time, rollover-safe ordering, and bounded update work:
  **preserved**, with exact replay gates open.
- Preview/commit atomic composition and no compensating rollback:
  **preserved**; Lesson 050 owns one side-effect-free `HomingPreview`, and
  Lesson 051 separately owns joint homing/sequencer admission.
- Stop dominance and all-off logical intent on fault:
  **preserved**.
- Fixed storage, no heap, and ordinary `Status` semantics:
  **preserved in design**, with measured aggregate gates open.
- E0 intent versus E1 sensing versus E2 powered motion:
  **preserved**.
- Lesson 051 identity + confirmation + position before gate-open intent:
  **extended** by requiring a current home epoch, not merely a numeric count.
- Persisted records do not establish current physical state:
  **preserved**.
- Pencil presentation except authoritative formal schematics:
  **preserved**.

## Post-remediation review history

The repaired `LESSONS_049_051_PARTS_CAROUSEL_PLAN.md` closes the earlier
underspecification as follows:

- `CopiedBinaryEvidence` now fixes synthetic source kind/ID, configuration
  revision, occurrence time, sequence, qualified state, nonzero qualification
  epoch, and producer status. Release and acquisition remain in one admitted
  source/configuration/qualification domain; unqualified or failed evidence
  cannot masquerade as inactive.
- `BoundedHomingConfig` now gives release and search independent nonzero step
  and duration bounds, a step interval, evidence age, signed search direction,
  inclusive logical bounds, and a home coordinate. The exact bound is
  accepted and the next frame fails. Arithmetic requires checked subtraction
  before addition. Time admission is now directional: `now` is compared to
  each occurrence first, future and half-range-ambiguous evidence is rejected,
  and only present/past evidence proceeds to age and
  `maximumInputSkew` checks. The skew bound then covers home-to-frame,
  stop-to-frame, and home-to-stop occurrence-time relationships, while the
  independent stop exception validates its own time and history without
  depending on malformed unrelated fields.
- Initially active input now enters `SeekingHomeRelease` opposite the search
  direction; a qualified release is mandatory before approach. Failure to
  release yields `HomeStuckActive`; failure to acquire a new edge yields
  `HomeNotFound`.
- Frame structure, same-sequence identity, freshness, rollover, qualification,
  source-domain continuity, stop exception, and collision precedence are now
  stated. A valid stop dominates unrelated malformed evidence and every
  ordinary transition; a malformed stop rejects the frame.
- The coordinator privately controls the Lesson 047 candidate. Complete
  validation and `preview`/`canCommit` precede the single child commit; stale
  or noncommittable candidates reject without either state changing. The
  deterministic matrix explicitly covers atomic rejection and child
  non-mutation.
- Home epochs are session-local, nonzero on successful acquisition, and
  invalidated by reset, shutdown, interruption, motion/source failure, and
  source revision change. Restart never reconstructs position from persisted
  records.
- E0 resource ownership remains exactly zero and quantitative object, sketch,
  stack, and aggregate Lesson 051 gates are now fixed.

The final repair closes the two residuals without changing Lesson 047:

- the constructor explicitly borrows `BoundedStepperSequence&`; the child must
  outlive the policy, is exclusively coordinated while initialized, is never
  copied or moved, and remains separately visible in whole-composition memory;
- the jointly admitted home-acquisition frame commits stopped/off child
  intent, captures the committed child coordinate as private
  `sequenceHomeOrigin`, advances the home epoch, and publishes the configured
  `homeLogicalPosition`; later targets use checked displacement and
  `sequenceHomeOrigin + displacement`, rejecting signed overflow and child
  bounds before mutation.

This translation preserves the child's own logical coordinate and gives the
coordinator a separate session-local coordinate domain. Implementation tests
must still exercise both signed extremes, child-bound rejection, home
acquisition collisions, and foreign/shared-child misuse, but those are proofs
of a now-complete design rather than missing contracts.

## Final normative closure

The earlier Lesson 047 composition, borrowed-child, and
`sequenceHomeOrigin` findings remain above as review history. The latest
normative plan materially simplifies and supersedes all of them:

- Lesson 050 has no Lesson 047 dependency, child, coil vector, child preview,
  borrowed lifetime, or coordinate translation;
- `BoundedHomingPolicy` copies its small configuration, owns only fixed
  logical state, and publishes an owner/generation-bound `HomingPreview`;
- each accepted preview publishes at most one semantic signed-step request or
  stop intent; its logical fixture coordinate advances only with its own
  accepted commit, and neither value claims an applied step, coil state,
  movement, or physical position;
- the qualified acquisition commit clears step request, establishes exactly
  `homeLogicalPosition`, and advances the session-local nonzero home epoch
  without reading or rewriting another component;
- directional occurrence-time admission rejects future and ambiguous values
  before maximum-age and pairwise/frame-skew checks, including the
  independently admissible stop path;
- release/search remain independently bounded by issued semantic requests and
  duration, with active-at-start requiring qualified release before approach;
- foreign, stale, changed, replayed, or consumed `HomingPreview` values reject
  without mutation; `canCommit()` and `commit()` define the sole state
  transition; and
- Lesson 051 alone translates an accepted one-step/stop intent into its
  separately owned Lesson 047 preview, jointly preflights both candidates, and
  commits with no fallible work remaining.

This standalone contract removes the prior rebasing and lifetime pressure
rather than repairing it with more coupling. It preserves Lesson 047
unchanged and keeps physical position unknown outside the synthetic E0 policy
claim. These closures permit E0 implementation. They do not constitute implemented
behavior, measured resource evidence, powered sensing, physical homing, or
canonical lesson promotion.

## Stress disposition: natural fit for implementation admission

The standalone homing responsibility is a natural pure-policy fit. The final
plan closes copied-observation, directional-time/skew, bounded-protocol,
collision, epoch, preview identity, storage/ownership, and quantitative-budget
design gaps without changing a published dependency. The E0 public shape is
sufficiently specified to begin implementation.

Stop and discuss architectural remediation before adding Lesson 047, coil,
endpoint, storage, callback, hidden clock, or physical-position meaning to
Lesson 050; changing shared status/lifecycle conventions; adding persistent
position; introducing a generic motor/homing framework; or allowing automatic
movement after reset.

## Gate result

- Disposition: natural fit for E0 implementation admission
- Open risks: implementation proof of preview owner/generation binding,
  one-request-per-commit behavior, signed arithmetic at both extremes,
  directional time/skew and collision precedence; Lesson 051 proof of joint
  homing/sequencer preflight; measured Lesson 050 and Lesson 051
  object/stack/flash/SRAM evidence; exact E1 sensor and E2 motor/fixture safety
  evidence
- Required discussion or decision IDs: none for E0 implementation; a durable
  decision is required before any shared-contract, persistent-position, or
  physical-support change
- Remediation owner and next action: Lesson 050 implementation owner must
  implement the fixed contract, execute its exhaustive deterministic matrix,
  measure the declared budgets, then rerun this pass against code and evidence
- Verification commands and results: repaired implementation-depth plan,
  canonical development, curriculum, testing, safety, packaging, PDF,
  work-queue, cadence, Lesson 047 plan, published Lesson 047 header, and
  Lesson 047 stress pass inspected;
  `git diff --check -- docs/design/LESSON_050_HOMING_STRESS_PASS.md` passed; no
  build or hardware command run because this is a pre-implementation design
  pass
- Maximum-composition scenario and proof: copied home/stop evidence through
  standalone Lesson 050 `HomingPreview`, then Lesson 051's separately owned
  Lesson 047 preview and inert coordinator, is fixed; collision and
  deterministic matrices are specified, while implementation replay and
  measured aggregate proof remain open
- E0 implementation permitted: yes
- Promotion permitted: no
