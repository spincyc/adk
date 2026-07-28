# Course marshal design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the hardware-independent Lesson 042 `CourseStartPolicy`, `CourseMarshal`,
and `CourseMarshalPresenter`. This checkpoint reviews the compact implemented
E0 core and its maximum authorized tabletop composition. It does not qualify
an optical module, button, PIR, ultrasonic ranger, display, or physical course.

## Boundary

- Name and lesson/project: `CourseMarshal` and `CourseMarshalPresenter`,
  Lesson 042
- Reviewer and date: compact-code architecture review, 2026-07-28
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
| API and layering | **Natural after completed bounded remediation.** The marshal consumes already-qualified semantic checkpoint events and one borrowed presence snapshot; it owns course order, terminal disposition, and bounded volatile evidence. It does not own pins, endpoints, sampling, optical/PIR/range qualification, display transport, or rendering. The presenter consumes only a compact immutable marshal projection and emits intent. The implemented `CourseStartPolicy` has only `None` and `ExplicitButtonWithPirEligibility`; the rejected provisional PIR-start authority is absent. Decision `3251b219-1a4a-41b7-8497-d8c6b5a72a98` is therefore enforced by the type surface rather than left as runtime configuration. |
| Ownership and lifecycle | **Natural and explicit.** All three policies are inert, non-copyable, and non-movable, with no endpoint, callback, timer, bus, heap, or external storage transport. The marshal retains references to six caller-owned fixed storage objects for its complete lifetime; those objects must neither move nor be destroyed first. In contrast, `CourseMarshalInputView` presence and event pointers are borrowed only for `update()` and are never retained. Initialization validates before use, reset preserves sequence identity, and record acknowledgment is explicit. Sensor, button, indicator, and display adapters retain acquisition, rollback, shutdown, and electrical safe-state responsibility. |
| Time and ordering | **Natural if the planned rules are implemented exactly.** Every input supplies time. Whole-frame validation precedes mutation; same-time identical frames are idempotent and changed frames fault. Event age, simultaneity, finish agreement, run timeout, presenter cell cadence, heartbeat cadence, natural rollover, and the half-range exclusion are explicit. Equal-time checkpoint events sort by configured semantic slot, never array or pin order. The start request retains its own observation epoch, while PIR eligibility retains its distinct epoch and age; neither may be restamped to imply simultaneity. |
| Errors and status | **Natural.** Existing `Status` carries malformed configuration, copied source failure, and timing failure. `RunDisposition` carries bounded domain outcomes without converting skip, reverse, duplicate, simultaneous, early finish, or timeout into transport errors. Compact `CourseTriggerRecord` preserves the selected cause and checkpoint/start projection; a separate `CourseTriggerPresenceStorage` losslessly retains the complete presence projection only for finish, range, or presence causes. Presenter faults cannot reclassify course evidence or feed back into course state. Canonical absent and unused values keep fieldwise replay deterministic. |
| Resource budget | **E0 core measured and bounded; aggregate promotion evidence remains open.** Exact AVR sizes are: `CourseStartPolicy` 59 bytes; caller storages `CourseRunStorage` 93, `CourseTriggerStorage` 80, `CourseTriggerPresenceStorage` 102, `CourseReplayFrameStorage` 35, `CourseReplayPresenceStorage` 102, and `CourseReplayEventStorage` 44 bytes, 456 bytes total; `CourseMarshalInputView` 38; `CourseMarshalSnapshot` 16; `CourseMarshal` 69; `CoursePresentationIntent` 21; and `CourseMarshalPresenter` 55. Marshal plus retained storage is 525 bytes, and start plus marshal/storage plus presenter is 639 bytes before `PresenceModel`, endpoints, renderer state, and stack. There is no allocation or unbounded work. The final Mega map, ADC schedule, display transport, flash, full SRAM/stack, claims, and electrical current remain aggregate-example gates. |
| Deterministic proof | **Core PASS.** Strict, custom-style, sanitizer, and focused fixtures cover configurations and capacity, all order rejections, simultaneity and agreement edges, explicit-button authority with PIR eligibility/faults, same-time identity, timeout and rollover, source-fault trigger projection, acknowledgment, sequence behavior, presenter mapping/cadence/fault isolation, and lossless fieldwise replay across the split caller-owned storages. Failed preflight leaves replay storage unchanged, proving frame acceptance is atomic. The powered maximum-collision and aggregate example remain separate promotion gates. |
| Packaging and public surface | **Natural for the E0 code checkpoint.** The standalone header and out-of-line implementation use the ordinary source shape without a special abstraction. Compact snapshots and caller-owned storage keep large evidence out of return-by-value and stack paths while preserving bounded typed access. Exact adapters, renderer, umbrella/archive registration, canonical example, and aggregate size evidence remain integration work. |
| Example and documentation fit | **Natural.** The canonical sketch can retain acquire sensors and explicit button, configure policies, start endpoints, then observe all sources, freeze one evidence frame, decide course state, and render one bounded intent. Local checkpoint LEDs, all-red, heartbeat, and the display provide non-Serial evidence. The course layout, identity map, timing, state graph, and staged build are pencil drawings; only exact qualified conventional electrical circuits may be marked as formal schematics. The same terms must appear in code, HTML, PDF, tests, and acceptance records. |
| Downstream effects | **Contained.** The completed start-authority and compact-storage remediation affects only the unpromoted Lesson 042 header, implementation, tests, future example, replay fixture, HTML/PDF, and promotion re-review. It does not change `Button`, PIR qualification, `PresenceModel`, optical policies, `UltrasonicRanger`, existing displays, or Lessons 001--039. Future consumers receive compact snapshots or explicit record/trigger access and cannot infer occupancy, security, navigation, or persistent history. |

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
| Total memory and hardware resources | **Applicable; E0 subtotal measured.** The exact AVR compact-core subtotal is 639 bytes: 59-byte start policy, 69-byte marshal, 456 bytes across six retained storages, and 55-byte presenter. The 38-byte borrowed input view, 16-byte snapshot, and 21-byte intent are transfer values rather than additional retained core storage, though caller stack residency must still be measured. Enumerate `PresenceModel`, all endpoints, renderer/display state, stack peak, flash, claims, GPIO/analog pins, ADC reference/mode, LEDs, test points, and current in the aggregate Mega example. |
| Shared bus or transport | **Conditionally applicable.** The pure policies own no bus. If the selected existing display uses a bus, the example composition must name its endpoint owner, borrowing and address rules, bounded one-cell transaction, acquisition rollback, failure isolation, and restart behavior. A direct display instead requires an explicit pin and cadence budget. |
| Persistence and recovery | **Not applicable.** Course state, sequence epoch, and terminal records are intentionally volatile; no RTC, EEPROM, removable storage, schema, commit, wear, or power-loss recovery owner exists. Shutdown/reinitialize behavior is in-object inspection, not persistence. |
| Motion, external power, or stored energy | **Not applicable.** The project has no actuator, motor, gate drive, switched external load, launcher, or stored-energy command path. The button authorizes only starting the inert timing policy. Adapters must still de-energize indicators and release claims on partial acquisition, reset, shutdown, and destruction. |
| Observation identity and provenance | **Applicable; core PASS.** Every checkpoint retains checkpoint ID, optical kind, source ID, calibration revision, source epoch, quality, and status. Start evidence retains explicit-button identity and separately aged PIR eligibility. The trigger record stores the selected compact cause projection while its dedicated caller-owned presence storage preserves the complete selected presence value. Run, trigger, frame, presence, and event storages together retain a lossless fieldwise replay identity without retaining either input pointer. Delayed evidence cannot be combined as if simultaneous. |
| Diagnostic interference | **Applicable.** Four checkpoint LEDs, ready, all-red, heartbeat, display, optional Serial, and test points belong in the same pin/current/time/memory budget. A disabled, busy, failed, or stalled renderer cannot alter checkpoint acceptance, start authority, elapsed time, terminal record, or safe endpoint shutdown. Heartbeat must expose a stalled presentation loop without becoming an input to course correctness. |
| Failure collision and recovery | **Applicable.** Whole-frame malformed/time/source precedence selects one recorded trigger without partially accepting a checkpoint. The collision fixture combines resource rollback, PIR ineligibility, no echo, simultaneous optical events, a faulty explicit start request, and display failure. It must prove deterministic attribution, canonical inactive evidence, retained accepted prefix, renderer isolation, reset/restart with faults present, and claim reuse. |

The E0 core proof exercises capacity immediately below, at, and above the
four-event input limit, simultaneous work, malformed-frame atomicity, restart,
sequence behavior, and lossless fieldwise replay. The aggregate composition
must still add failure at every endpoint-acquisition step, measured scheduler
and stack pressure, renderer/display failure, and shutdown/restart with powered
faults. Passing host replay cannot close the exact-specimen or E1 bench gates.

## Prior-decision impact

- Decision `3251b219-1a4a-41b7-8497-d8c6b5a72a98`, explicit button start with
  PIR eligibility only: **preserved** by `CourseStartPolicy`; the provisional
  PIR-start alternative is absent.
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
  **preserved**. The exact compact E0 retained subtotal is 639 AVR bytes;
  aggregate example and stack measurement remain required.
- Caller-owned bounded storage with explicit retained lifetimes, update-only
  borrowed input pointers, and atomic replay publication: **extended** for the
  large fixed evidence record without introducing allocation.
- Complete provenance with compact trigger projections: **preserved**. Split
  storage reduces individual AVR object sizes without dropping the selected
  presence evidence or fieldwise replay identity.
- Presentation intent cannot change primary policy or evidence:
  **preserved**.
- Volatile records rather than RTC or storage reuse: **preserved**.
- No motor, gate, launcher, security, occupancy, navigation, or life-safety
  claim: **preserved**.
- Pencil drawings for every non-schematic PDF visual and exact electrical
  qualification before a formal schematic: **preserved**.

No existing published interface is challenged. The provisional Lesson 042
start and oversized-object concerns were confined to this unpromoted boundary
and are resolved in the reviewed E0 code.

## Stress disposition

**Natural fit after completed bounded local remediation.** The implemented
start seam makes a qualified explicit button event the sole authority and
uses copied PIR evidence only for eligibility. The compact marshal retains six
explicit caller-owned bounded storages, borrows input pointers for one update,
publishes compact snapshots, and exposes selected trigger projections without
losing replay evidence. Preflight validation is atomic with respect to all
retained replay state.

The custom-style, strict, sanitizer, focused deterministic, exact-layout, and
diff checks pass for this E0 code checkpoint. No shared start abstraction or
change to earlier button, PIR, presence, optical, range, or display contracts
is required. Canonical promotion remains blocked on the aggregate Mega example,
endpoint/resource measurements, documentation/publication, powered specimen
qualification, and the repeated pre-promotion stress pass.

## Gate result

- Disposition: natural fit after completed bounded local remediation; E0 core
  architecture gate passed
- Open risks: unresolved exact optical/PIR/range/button/display specimens;
  unmeasured aggregate cadence, ADC settling, pin/claim/current, flash, full
  SRAM, and stack budgets; aggregate renderer/endpoint collision fixture;
  canonical example, documentation, packaging, publication, and open E1 bench
  acceptance
- Required discussion or decision IDs:
  `3251b219-1a4a-41b7-8497-d8c6b5a72a98` resolves start authority; no further
  user decision is required for this stress finding
- Remediation owner and next action: local core remediation is complete; the
  Lessons 040--042 integration owner builds and measures the maximum Mega
  composition, completes public registration and lessons, and repeats this
  pass before promotion
- Verification commands and results:
  - focused strict host build and deterministic test: passed
  - custom-style check for the component header, implementation, and test:
    passed
  - focused sanitizer build and deterministic test: passed
  - exact AVR layout probe: passed with start policy 59 bytes; retained
    storages 93/80/102/35/102/44 bytes (456 total); input view 38; snapshot 16;
    marshal 69; intent 21; presenter 55; marshal plus storage 525; start,
    marshal/storage, and presenter subtotal 639
  - lossless fieldwise replay, trigger projection, borrowed-pointer
    non-retention, and malformed-preflight atomicity review: passed
  - `git diff --check -- docs/design/stress-passes/course-marshal.md`: passed
- Maximum-composition scenario and proof: compact E0 policy/storage replay
  passed; aggregate endpoint/example timing, stack, resource, electrical, and
  collision evidence remains open
- Promotion permitted: no
