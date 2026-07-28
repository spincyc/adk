# Course marshal design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the hardware-independent Lesson 042 `CourseMarshal` and
`CourseMarshalPresenter` before implementation. It evaluates the maximum
authorized tabletop composition. It does not qualify an optical module,
button, PIR, ultrasonic ranger, display, or physical course.

## Boundary

- Name and lesson/project: `CourseMarshal` and `CourseMarshalPresenter`,
  Lesson 042
- Reviewer and date: preimplementation architecture stress review, 2026-07-28
- Public types and operations: the fixed-capacity checkpoint bindings, copied
  checkpoint and start evidence, run dispositions and immutable terminal
  record, `CourseMarshal::{initialize,reset,acknowledgeRecord,update,snapshot,
  initialized}`, presentation intent, and
  `CourseMarshalPresenter::{initialize,reset,update,intent,initialized}`
- Direct dependencies: Lesson 040 optical provenance and quality values,
  Lesson 041 copied presence states and snapshot, `Status`, `TimePoint`,
  `Duration`, fixed-width integer types, and project-owned copied values
- Existing decisions and interfaces reconsidered: semantic evidence rather
  than pin identity, explicit unsigned time, source/calibration provenance,
  fixed volatile capacity, immutable terminal evidence, presentation
  separation, explicit operator authority, and endpoint-owned electrical
  lifetime

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural after bounded local remediation.** The marshal consumes already-qualified semantic checkpoint events and one copied presence snapshot; it owns course order, terminal disposition, and a fixed volatile record. It does not own pins, endpoints, sampling, optical/PIR/range qualification, display transport, or rendering. The presenter consumes only one immutable marshal snapshot and emits intent. The planning draft still exposes a `ProvisionalStartSource` alternative under which PIR could start a run. Decision `3251b219-1a4a-41b7-8497-d8c6b5a72a98` closes that alternative: implementation must use an explicit qualified button request as the sole start authority, with PIR only as eligibility evidence. The provisional enum and names must be narrowed before the public header is fixed. |
| Ownership and lifecycle | **Natural.** Both policies are inert copied-state owners, non-copyable and non-movable, with no endpoint, callback, timer, bus, heap, or external storage. Initialization validates complete configuration before mutation. Reset clears active run/presentation state but does not silently reuse a sequence identity; record acknowledgment is explicit. Sensor, button, indicator, and display adapters retain acquisition, rollback, shutdown, and electrical safe-state responsibility. The presenter cannot acknowledge, reset, start, or otherwise mutate the marshal. |
| Time and ordering | **Natural if the planned rules are implemented exactly.** Every input supplies time. Whole-frame validation precedes mutation; same-time identical frames are idempotent and changed frames fault. Event age, simultaneity, finish agreement, run timeout, presenter cell cadence, heartbeat cadence, natural rollover, and the half-range exclusion are explicit. Equal-time checkpoint events sort by configured semantic slot, never array or pin order. The start request retains its own observation epoch, while PIR eligibility retains its distinct epoch and age; neither may be restamped to imply simultaneity. |
| Errors and status | **Natural.** Existing `Status` carries malformed configuration, copied source failure, and timing failure. `RunDisposition` carries bounded domain outcomes without converting skip, reverse, duplicate, simultaneous, early finish, or timeout into transport errors. Trigger evidence identifies the winning cause under documented precedence. Presenter faults cannot reclassify course evidence or feed back into course state. Canonical absent and unused values are required so replay identity does not depend on uninitialized bytes. |
| Resource budget | **Open promotion blocker, bounded by design.** Core storage is four checkpoint bindings, four input events, four accepted records, one copied presence snapshot, one immutable terminal record, and one presenter intent; there is no allocation or unbounded work. The authorized maximum physical composition is four intermediate optical boundaries, one reflective finish guard, PIR, HC-SR04 trigger/echo, one existing debounced button, four checkpoint LEDs, all-red/ready/heartbeat presentation, and one existing display. The final Mega map, roughly 18--20 claims, ADC channel/settling schedule, display transport, claim-registry occupancy, rollback order, flash, SRAM, stack, per-pin/port/aggregate current, and exact specimen currents must be measured before promotion. |
| Deterministic proof | **Specified but not yet executed.** Host fixtures must cover zero, one, and four checkpoint configurations; input capacity below, at, and above four; every ordering rejection; simultaneity and finish-agreement edges; explicit request with PIR ineligible, eligible, stale, stuck, and faulted; bounce and repeated request identity; timeout and rollover; source-fault collisions; immutable record acknowledgment; sequence exhaustion; presenter phase/digit/heartbeat boundaries; reset/restart; and fieldwise golden replay. The maximum-collision fixture below is mandatory. Hardware evidence remains separate. |
| Packaging and public surface | **Natural in the planned repository shape.** Standalone headers and out-of-line implementations can use the ordinary host, native package, Arduino archive, umbrella-header, example, and size inventories without a special source path. The public contract must remove the unresolved/PIR-authority alternative before registration. Exact adapters and renderer remain example-local compositions until separately qualified; they do not enter the pure engine API. |
| Example and documentation fit | **Natural.** The canonical sketch can retain acquire sensors and explicit button, configure policies, start endpoints, then observe all sources, freeze one evidence frame, decide course state, and render one bounded intent. Local checkpoint LEDs, all-red, heartbeat, and the display provide non-Serial evidence. The course layout, identity map, timing, state graph, and staged build are pencil drawings; only exact qualified conventional electrical circuits may be marked as formal schematics. The same terms must appear in code, HTML, PDF, tests, and acceptance records. |
| Downstream effects | **Contained before promotion.** The bounded start-authority repair affects the provisional Lesson 042 plan, new header, tests, example, replay fixture, HTML/PDF, and stress re-review. It does not change `Button`, PIR qualification, `PresenceModel`, optical policies, `UltrasonicRanger`, existing displays, or Lessons 001--039. Future consumers may copy a terminal record but cannot infer occupancy, security, navigation, or persistent history from it. |

## Composition pressure scenario

The maximum authorized course has four ordered intermediate optical
checkpoints, one separate reflective finish guard, one PIR eligibility source,
one HC-SR04 approach source, and one existing debounced button whose qualified
press event is the sole start authority. Its observation paths are four local
checkpoint LEDs, all-red, ready, heartbeat, and one existing display. The
course moves only a hand-held card or unpowered model.

One scheduler iteration acquires each source in a documented fixed order,
finishes any bounded ultrasonic update, captures an immutable copied evidence
frame, updates the marshal once, updates the presenter once, and renders at
most one display cell plus bounded LEDs. The proof fixture injects a second
resource-claim failure during partial initialization, optical saturation, PIR
warm-up or stuck motion, HC-SR04 no echo, button bounce/stuck/source fault,
four checkpoint events with a simultaneous pair, display failure, reset, and
restart while faults remain.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable.** Measure the worst source-acquisition and render iteration, including ADC channel settling, HC-SR04 polling/latency, button qualification, five optical updates, PIR, one marshal update, one presenter update, and one display cell. Replay simultaneous timestamps, one-tick edges, rollover, missed updates, and faults. No source may be starved, and presentation failure or a large elapsed jump may not cause catch-up loops. |
| Total memory and hardware resources | **Applicable.** Enumerate all live endpoints, policies, copied frames, records, presenter state, display state, stack peak, flash, SRAM, claim entries, GPIO/analog pins, ADC reference/mode, display transport, LEDs, test points, and current. Test one and four configured checkpoints plus an attempted fifth event; measure the aggregate Mega example rather than summing isolated guesses. |
| Shared bus or transport | **Conditionally applicable.** The pure policies own no bus. If the selected existing display uses a bus, the example composition must name its endpoint owner, borrowing and address rules, bounded one-cell transaction, acquisition rollback, failure isolation, and restart behavior. A direct display instead requires an explicit pin and cadence budget. |
| Persistence and recovery | **Not applicable.** Course state, sequence epoch, and terminal records are intentionally volatile; no RTC, EEPROM, removable storage, schema, commit, wear, or power-loss recovery owner exists. Shutdown/reinitialize behavior is in-object inspection, not persistence. |
| Motion, external power, or stored energy | **Not applicable.** The project has no actuator, motor, gate drive, switched external load, launcher, or stored-energy command path. The button authorizes only starting the inert timing policy. Adapters must still de-energize indicators and release claims on partial acquisition, reset, shutdown, and destruction. |
| Observation identity and provenance | **Applicable.** Every checkpoint retains checkpoint ID, optical kind, source ID, calibration revision, source epoch, quality, and status. Start evidence retains the explicit button event identity and the separately aged copied PIR eligibility state. Finish evidence retains reflective provenance and timed range evidence. Same-time replay compares canonical public fields; delayed evidence cannot be combined as if simultaneous. |
| Diagnostic interference | **Applicable.** Four checkpoint LEDs, ready, all-red, heartbeat, display, optional Serial, and test points belong in the same pin/current/time/memory budget. A disabled, busy, failed, or stalled renderer cannot alter checkpoint acceptance, start authority, elapsed time, terminal record, or safe endpoint shutdown. Heartbeat must expose a stalled presentation loop without becoming an input to course correctness. |
| Failure collision and recovery | **Applicable.** Whole-frame malformed/time/source precedence selects one recorded trigger without partially accepting a checkpoint. The collision fixture combines resource rollback, PIR ineligibility, no echo, simultaneous optical events, a faulty explicit start request, and display failure. It must prove deterministic attribution, canonical inactive evidence, retained accepted prefix, renderer isolation, reset/restart with faults present, and claim reuse. |

The bounded proof must exercise capacity immediately below, at, and above the
four-event input limit, worst-case simultaneous work, failure at every
acquisition step, shutdown and restart with the same faults, sequence
exhaustion, and byte-stable fieldwise replay. Passing host replay cannot close
the exact-specimen or E1 bench gates.

## Prior-decision impact

- Decision `3251b219-1a4a-41b7-8497-d8c6b5a72a98`, explicit button start with
  PIR eligibility only: **preserved**, after removing the provisional
  PIR-start alternative before API registration.
- Qualified semantic evidence rather than pin or array identity:
  **preserved**.
- Source kind, source ID, calibration revision, epoch, quality, and status
  provenance: **preserved and extended** into fixed course bindings and frozen
  terminal evidence.
- Explicit supplied time, unsigned rollover, half-range exclusion, and
  same-time identity: **preserved**.
- Existing `Status`/`Result` conventions and separate domain dispositions:
  **preserved**.
- Pure copied-state policy below endpoint ownership and presentation adapters:
  **preserved**.
- Fixed memory, bounded work, no heap, no exceptions, and no hidden clocks:
  **preserved**, with aggregate measurement still required.
- Presentation intent cannot change primary policy or evidence:
  **preserved**.
- Volatile records rather than RTC or storage reuse: **preserved**.
- No motor, gate, launcher, security, occupancy, navigation, or life-safety
  claim: **preserved**.
- Pencil drawings for every non-schematic PDF visual and exact electrical
  qualification before a formal schematic: **preserved**.

No existing published interface is challenged. The provisional Lesson 042
start types are challenged only inside this unpromoted boundary and have a
recorded resolution.

## Stress disposition

**Bounded local remediation required.** The architecture fits naturally once
the planning-only start seam is narrowed to the durable decision: a qualified
explicit button event is the sole authority to start, and copied PIR evidence
can only make that request eligible. Keeping `PirEligibilityStartsRun` or an
`Unresolved` runtime value would preserve an invalid authority path and is not
permitted.

The remediation is confined to the unpromoted Lesson 042 API, implementation,
tests, fixture, example, and documentation. It does not require a shared start
abstraction or changes to earlier button, PIR, presence, optical, range, or
display contracts. Promotion also remains blocked on aggregate resource
measurements, deterministic maximum-composition replay, exact specimen
qualification, and the repeated pre-promotion stress pass.

## Gate result

- Disposition: bounded local remediation required before implementation shape
  is registered
- Open risks: unresolved exact optical/PIR/range/button/display specimens;
  unmeasured aggregate cadence, ADC settling, pin/claim/current, flash, SRAM,
  and stack budgets; unexecuted collision and replay fixtures; open E1 bench
  acceptance
- Required discussion or decision IDs:
  `3251b219-1a4a-41b7-8497-d8c6b5a72a98` resolves start authority; no further
  user decision is required for this stress finding
- Remediation owner and next action: Lessons 040--042 integration owner narrows
  the start evidence to explicit-button authority with PIR eligibility, then
  implements focused host fixtures and measures the maximum Mega composition
- Verification commands and results:
  - document review against the live Lesson 042 plan, stress template, active
    task, and start-authority decision: passed
  - implementation, host replay, AVR size, archive, lesson, and hardware
    checks: not yet run because this is a preimplementation record
  - `git diff --check -- docs/design/stress-passes/course-marshal.md`: passed
- Maximum-composition scenario and proof: specified above; deterministic
  replay and measured aggregate resource evidence remain open promotion gates
- Promotion permitted: no
