# Balance-table instrument design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the planned hardware-independent Lesson 045 `BalanceInstrument`. It is a
pre-implementation review of the E0 copied-frame contract, not evidence that
the public API, tests, Mega example, size limits, or publication gates pass.
It does not qualify an inertial module, I2C transaction, live control,
indicator, sounder, wiring table, schematic, powered board, or E1 result.

## Boundary

- Name and lesson/project: stationary balance-table instrument, Lesson 045
- Reviewer and date: pre-implementation architecture review, 2026-07-28
- Public types and operations: planned copied joystick/button observations,
  atomic project input, compact live/frozen evidence, caller-owned replay
  storage, project output, and
  `BalanceInstrument::{initialize,shutdown,acknowledgeFault,update,snapshot,
  initialized}`
- Direct dependencies: Lesson 043 copied inertial observations, Lesson 044
  orientation and presentation policies, `Status`, `TimePoint`, `Duration`,
  fixed-width integers, and project-local copied control observations
- Existing decisions and interfaces reconsidered: explicit supplied time,
  semantic evidence rather than pin identity, fixed volatile storage,
  caller-owned replay state, explicit button freeze authority, presentation
  separation, provenance retention, endpoint-owned electrical lifetime, and
  the retained Lessons 067--069 normalization boundary

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural in the plan.** The project consumes already-copied inertial, joystick, and button observations; applies project admission, freeze, sensitivity, recovery, and precedence; and emits presentation intent. It does not own sampling, conversion, pins, endpoints, buses, sensor registers, or physical rendering. The project-local control observations avoid inventing repository-wide joystick or button snapshot contracts. A caller may translate existing accessors, but that adapter remains outside the pure policy. |
| Ownership and lifecycle | **Natural with one local design check open.** The planned instrument owns no endpoint or heap object. It borrows the orientation and presentation policies for its initialized lifetime and borrows a caller-owned `BalanceFrameStorage` that must remain alive and unmodified. Candidate inputs are update-only values and are never retained by address. Initialization validates before mutation; failed frames do not alter replay storage; shutdown clears replay availability, frozen evidence, and current intent. Implementation must prove partial policy initialization rollback and alias safety. If borrowed mutable policies cause partial-update or lifetime strain, the bounded remedy is to own the small policies by value or construct them from copied configuration, not add allocation, runtime polymorphism, or a service locator. |
| Time and ordering | **Natural if implemented exactly.** Every observation and complete frame carries explicit time and sequence identity. Replay-first fieldwise comparison, modular forward deltas, the half-range exclusion, future-time rejection, freshness, input skew, diagnostic phase, and same-time identity are specified without a clock, delay, or retry loop. One atomic frame is admitted before freeze, sensitivity, orientation, and presentation mutations. An unchanged inertial sample may age in a later frame only through canonical recomputation of its derived fields. |
| Errors and status | **Natural.** Existing `Status` describes configuration, producer, and malformed-frame failures; mode and inertial quality describe project-domain state. Fixed producer precedence is inertial, freeze button, then joystick. Ineligible stale, saturated, unsteady, or beyond-range evidence disables tone without being mislabeled as a producer failure. Fault is latched; recovery requires a fully healthy latest frame, explicit out-of-band acknowledgement, and one later healthy re-priming frame that cannot replay controls. Independent status fields preserve attribution even if presentation also fails. |
| Resource budget | **Bounded estimates; measurement is a promotion blocker.** The plan sets AVR hard thresholds of 192 bytes for the instrument, 128 for input, 144 for output, 132 for caller replay storage, 660 for the resident four-policy composition plus replay storage, and 932 for the worst live composition with candidate input and returned snapshot. The complete E0 Mega sketch is limited to 2,048 bytes static SRAM, at least 1,024 bytes measured stack margin, and 28 KiB flash, with no heap, pins, timers, interrupts, ADC, or I2C claims. Implementation must publish every object size, coexistence total, stack peak, update-work bound, and sketch measurement. Provenance may not be removed to meet a target. |
| Deterministic proof | **Planned and sufficient in scope; not yet executed.** The matrix covers lifecycle, all directions, freeze authority, sensitivity clamps, atomic admission, exact replay, reused-sample aging, every sequence/time boundary, pairwise and credible triple fault collisions, recovery, permutations, capacity, shutdown, and tone-off safety. A versioned golden trace must reproduce startup, live and frozen states, changed live evidence, faults, acknowledgement, rollover, and shutdown. Host replay cannot close a hardware gate. |
| Packaging and public surface | **Natural if the compact surface survives implementation.** The planned standalone header and out-of-line implementation fit the ordinary archive and host inventories. Caller-owned replay storage keeps the full atomic frame out of the instrument and public output while compact evidence retains interpretation identity. Umbrella registration, build inventories, exact AVR layout, size baseline, example, HTML, PDF, indexes, and newest-lesson checks remain open integration work. |
| Example and documentation fit | **Natural at E0.** The canonical compile-only Mega sketch can initialize pure policies, configure the board frame and project, replay one atomic frame, decide validation/orientation/freeze/sensitivity, and copy complete RGB, diagnostic, and tone intents into named host result cells. It must never read a live control or actuate an endpoint under E0. Code, tests, HTML, and PDF use the same live, frozen, recovering, provenance, sensitivity, and fault vocabulary. Every E0 PDF visual is a classified pencil drawing; no electrically authoritative formal schematic is available. |
| Downstream effects | **Contained if the proposed surface holds.** The new project composes Lessons 043--044 and local copied controls without changing published joystick, button, display, sound, `Status`, `TimePoint`, or I2C contracts. Lessons 022 and 031 retain their configuration/calibration responsibilities; Lesson 033 remains the source of joystick behavior rather than a new shared observation type; Lessons 040--042 establish copied-evidence and compact-storage precedent but are not dependencies to be generalized; Lessons 067--069 retain normalization, qualification, and recorded cross-device comparison. Any required shared-contract change is architectural remediation, not a Lesson 045 cleanup. |

## Composition pressure scenario

The maximum authorized E0 composition is one synthetic inertial producer,
one copied `InertialObservationPolicy`, one `OrientationPolicy`, one
`BalancePresentationPolicy`, and one `BalanceInstrument`, plus project-local
copied joystick and button observations and copied RGB, diagnostic, and tone
intent cells. It runs at the fastest documented finite fixture cadence.

The collision frame combines stale inertial evidence, a producer fault, a
qualified freeze-button event, a sensitivity event, and a diagnostic-output
failure. The fixture then supplies healthy evidence, attempts premature and
valid acknowledgement, re-primes without applying controls, freezes a valid
estimate, changes live tilt while frozen, unfreezes, shuts down, and restarts
with the fault still present in the source trace.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open.** Each update is fixed O(1) work with no hidden sampling, retry, catch-up, or input-sized loop. Measure the fastest documented replay cadence, worst collision-frame path, diagnostic phase edge, same-time replay, rollover, and a large elapsed jump. Prove validation and presentation cannot starve another component and that diagnostics do not create scheduler authority. |
| Total memory and hardware resources | **Applicable; open.** Measure all four policy objects, caller replay storage, candidate frame, returned snapshot, copied result cells, fixture cursor, globals, stack peak, and flash together. Exercise below, at, and above any replay-fixture capacity. E0 must retain zero pin, timer, interrupt, ADC, I2C, and heap use. Crossing a target requires a bounded reduction attempt; crossing a hard threshold blocks promotion and triggers a stress decision. |
| Shared bus or transport | **Not applicable to E0.** No E0 type owns or borrows a bus or transport, and the canonical path consumes already-copied synthetic values. A future exact inertial adapter is a separately qualified E1 boundary with one bus owner, explicit borrower lifetime, address and pull-up evidence, bounded transactions, rollback, NACK/stuck-bus recovery, test points, and aggregate measurement. It cannot be hidden inside this project. |
| Persistence and recovery | **Not applicable.** Freeze, sensitivity, sequence baselines, and fault/recovery mode are intentionally volatile. There is no EEPROM, RTC, removable storage, schema, commit, wear, or power-loss retry. Shutdown clears frozen and replay state, and reinitialize begins `AwaitingFrame`; this lifecycle behavior is not persistence. |
| Motion, external power, or stored energy | **Not applicable.** The E0 project emits copied intent only and has no actuator, motor, external-load switch, launcher, balancing mechanism, or stored-energy path. Its authorized subject is a stationary, lightweight, hand-tilted inert platform, not a vehicle, wearable, load balancer, or safety instrument. E0 contains no powered endpoint at all. A future E1 renderer must still prove de-energized startup, fault, shutdown, reset, and power-removal states. |
| Observation identity and provenance | **Applicable; structurally preserved, proof open.** Live and frozen evidence retain inertial kind, model, source ID, configuration and calibration revisions, declared ranges, sample timestamp and sequence, quality, saturation, data-ready state, and status. The frozen estimate keeps the exact evidence that produced it while new live evidence advances independently. Full-frame fieldwise replay storage retains all axes and local control identity; no `memcmp`, padding, pointer retention, restamping, or mixing a new control with over-skew inertial evidence is permitted. Golden replay must prove these rules byte-stably at the serialization boundary. |
| Diagnostic interference | **Applicable; open.** Copied RGB, mono-diagnostic, and tone intents and stable host result cells belong in the same memory, time, and failure budget. Serial is optional. A diagnostic-output failure must not change freeze authority, sensitivity, evidence admission, fault attribution, or recovery; every invalid or ineligible frame still carries no-tone intent. The E0 host cells are observation paths, not physical evidence. |
| Failure collision and recovery | **Applicable; open.** Malformed/future input precedes producer status; producer faults follow fixed field order; skew follows producer faults; an existing latch suppresses controls; ineligible orientation forces tone off; freeze precedes sensitivity; ordinary presentation is last. The maximum-collision fixture must show atomic rejection, unchanged replay storage on failure, retained frozen evidence, independent producer and diagnostic attribution, no automatic clear, acknowledgement only after a healthy latest frame, control-free re-priming, canonical shutdown, and restart with faults still present. |

The composition proof must include exact semantic replay, changed equal-time
rejection, reused inertial identity aging from current to stale, partial
initialization rollback, failure before replay-storage commit, and candidate
plus resident plus returned-snapshot stack pressure. No result may be described
as physical observation.

## Prior-decision impact

- Pure copied semantic policy below endpoint and transport ownership:
  **preserved**.
- Explicit supplied time, modular rollover, half-range exclusion, and
  same-time semantic identity: **preserved**.
- Fixed storage, bounded work, no heap, no exceptions, no RTTI, and no hidden
  clock: **preserved**, subject to measured aggregate proof.
- Caller-owned bounded replay storage with an explicit lifetime and
  commit-last atomic publication: **extended locally** from the compact
  copied-evidence pattern; no shared storage abstraction is proposed.
- Complete source, sequence, time, range, status, and calibration provenance:
  **preserved and extended** through distinct live and frozen evidence.
- Explicit debounced button event as sole freeze authority: **preserved**;
  tilt, joystick thresholds, held state, and replayed events cannot freeze.
- Joystick sensitivity as volatile presentation control: **preserved**;
  no potentiometer or second authority is introduced.
- Presentation failure cannot reclassify primary evidence or controls:
  **preserved**.
- Endpoint owners retain acquisition, rollback, and electrical safe-state
  duties: **preserved**; E0 has no endpoint.
- Lessons 067--069 retain device qualification, normalization, durable
  records, and cross-device comparison: **preserved**.
- Stationary, no-actuator, no external-load, no stored-energy, no navigation,
  no wearable, no balancing-control, and no safety claim: **preserved**.
- Pencil presentation for every non-schematic PDF visual and exact electrical
  qualification before a formal schematic: **preserved**.

No published interface is challenged by the planned project. The two known
strain points are local and unpromoted: mutable borrowed policies may complicate
atomic rollback, and the complete copied frame may buckle AVR SRAM or stack
budgets. Their bounded remedies are, respectively, value-owned small policies
or internally constructed policy state, and compact caller-owned replay plus
measured field-level representation reduction. Neither remedy may discard
provenance, weaken replay identity, or alter shared contracts.

If those remedies cannot meet the hard limits, or implementation requires a
change to `Status`, `TimePoint`, `I2cBus`, published joystick/button behavior,
presentation ownership, or Lessons 067--069 scope, the disposition becomes
**architectural remediation required**. Promotion stops while affected
consumers, compatibility and migration costs, safety/resource consequences,
alternatives, and a bounded experiment are recorded and discussed. A durable
decision must precede any shared-contract change.

## Stress disposition

**Bounded local remediation pending measurement.** The planned responsibility,
layering, explicit control authority, volatile recovery, provenance, and
stationary no-actuator safety boundary fit the existing architecture. The
caller-owned replay seam and compact output are the correct local direction,
but implementation must prove atomicity and the hard AVR object, aggregate,
and stack thresholds before this can become a natural fit.

## Gate result

- Disposition: bounded local remediation pending implementation and
  measurement
- Open risks: borrowed-policy rollback/alias behavior; exact object padding;
  candidate/resident/snapshot stack coexistence; worst-path update work;
  diagnostic-failure isolation; replay-storage commit atomicity; complete
  failure-collision and recovery replay; packaging and publication integration;
  all exact-specimen, powered-adapter, schematic, and E1 gates
- Required discussion or decision IDs: none for the planned local experiment;
  a consequential decision is required before any shared-contract or
  curriculum-scope change
- Remediation owner and next action: Lesson 045 implementation owner builds
  the compact caller-storage version first, measures every AVR object and the
  aggregate stack/flash/SRAM composition, exercises collision and recovery
  fixtures, and repeats this pass before promotion
- Verification commands and results:
  - plan and canonical-contract review: completed
  - implementation, focused strict tests, sanitizer replay, style,
    standalone-header, Mega compile, exact AVR layout, stack, size, packaging,
    PDF, site, and independent reviews: not yet run
  - `git diff --check -- docs/design/stress-passes/balance-table-instrument.md`:
    passed
- Maximum-composition scenario and proof: scenario specified above; no
  implementation or measured proof yet
- Promotion permitted: no
