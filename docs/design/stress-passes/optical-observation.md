# Optical observation design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the hardware-independent Lesson 040 policies. It records the implementation
checkpoint for the E0 copied-value boundary; it is not evidence that an
electrical adapter, canonical lesson, or sensor specimen has passed promotion.

## Boundary

- Name and lesson/project: `ReflectiveObservationPolicy` and
  `BeamObservationPolicy`, Lesson 040
- Reviewer and date: independent architecture stress review, 2026-07-28
- Public types and operations: `OpticalSourceKind`, `OpticalQuality`,
  `OpticalProvenance`, `ReflectiveSample`, `BeamSample`,
  `ReflectiveObservation`, `BeamObservation`,
  `ReflectiveObservationConfig`, `BeamObservationConfig`, and each policy's
  `initialize`, `reset`, `update`, `initialized`, and `snapshot`
- Direct dependencies: `Level`, `Status`, `TimePoint`, `Duration`, and fixed
  width integer types
- Existing decisions and interfaces reconsidered: pure copied-sample
  behavior, explicit unsigned time, closed status values, inert lifecycle,
  fixed Mega 2560 storage, endpoint ownership, source identity and
  calibration provenance, one-update events, and the planned Lessons 041 and
  042 consumers

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural.** The two policies share provenance and quality vocabulary but keep scalar reflectance qualification separate from interrupted-level qualification. They consume copied values and own no endpoint, pin, emitter, clock, callback, bus, claim, or retail-module interpretation. This avoids both a false universal optical driver and an optional-field mega-record. Electrical acquisition and AO/DO transactional ownership remain adapter responsibilities. |
| Ownership and lifecycle | **Natural and verified at the pure boundary.** Construction is inert and both types are non-copyable and non-movable. `initialize()` validates policy only and is idempotent; `reset()` clears temporal, candidate, event, and latched-fault history while preserving configuration and initialized state. No `shutdown()` belongs on a pure policy. Tests cover pre-initialize rejection, repeated initialize, reset, and reset recovery. Adapter partial acquisition, reverse rollback, shutdown, destruction, and safe electrical state remain separate gates and cannot be credited to these policies. |
| Time and ordering | **Natural after bounded correction.** Supplied `observedAt` preserves explicit time and deterministic replay. The implementation validates modular time before source interpretation, accepts an identical complete same-time sample idempotently, and latches changed same-time, backward, and exact-or-greater half-range input as `TimingFault` with `InvalidArgument`. The correction retains the preceding provenance, raw/level, qualified state, stability, and event evidence while changing only quality/status to expose the timing fault; it performs no partial application of the rejected sample. Reset establishes a fresh epoch. Tests cover those retention rules, exact half-range, backward time, and natural rollover for both policies. |
| Errors and status | **Natural after bounded clarification.** `OpticalQuality` separates domain interpretation from shared `Status`. Malformed level, source-ID mismatch, revision mismatch, and impossible ADC evidence latch `SourceFault` with `InvalidArgument`; exact non-OK upstream statuses are retained and latch; timing rejection uses `TimingFault` with `InvalidArgument`; qualified out-of-range evidence remains an Ok domain quality and clears a candidate. Reset is the explicit recovery path. No new status code or component-local status convention was introduced. |
| Resource budget | **Natural for the isolated policies; aggregate gate open.** State is fixed and update work is bounded, with no heap, interrupt, timer, bus, ADC mode, endpoint, or claim-registry entry. AVR measurements report all public value types at or below 24 bytes, configurations at 20 and 12 bytes, and `ReflectiveObservationPolicy`/`BeamObservationPolicy` at 67 and 51 bytes, all below the 128-byte largest-object ceiling. Widened normalization is bounded by `(1023 * 1000)` in `uint32_t`. The implementation object reports 3,186 bytes of AVR text and zero data/BSS; that diagnostic object size is not a linked sketch baseline. Lesson 042's full aggregate remains independently open. |
| Deterministic proof | **Natural and passed for the implemented E0 boundary.** Focused fixtures cover lifecycle and configuration, both reference orderings and active polarities, range/threshold/normalization boundaries, dwell and hysteresis, candidate clearing, beam bounce and edges, malformed evidence, provenance mismatches, exact upstream-status latching, complete same-time identity, timing-fault retention, half-range, backward time, rollover, reset, and field-stable replay. Strict host compilation and the ASan/UBSan inventory pass. Ambient traces still cannot prove ambient compensation, electrical crosstalk, pin rollback, ADC settling, or specimen failure modes. |
| Packaging and public surface | **Natural at the core checkpoint.** The standalone public header and out-of-line implementation compile through the normal host target, `Adk.h`, and the standalone-header inventory without a component-specific exception. The optical target is included in the normal host-test inventory. Arduino archive discovery, canonical example compilation, linked sketch size, and release packaging remain later promotion gates. |
| Example and documentation fit | **Natural only after specimen qualification.** The planned narrative is acquire an exact adapter and indicators, observe one source, qualify copied evidence, and present it through non-Serial indicators. The PDF plan correctly classifies orientation, placement, traces, timing, and state flow as pencil drawings; only an explicitly marked, electrically authoritative conventional circuit may be a formal schematic. Pin tables, powered wiring, current claims, and physical acceptance remain prohibited until an exact specimen closes its electrical gates. |
| Downstream effects | Lesson 041 can copy the qualified optical output while retaining source kind, source ID, calibration revision, timestamp, quality, and status. Lesson 042 can bind semantic checkpoints without taking pin ownership. The established `AnalogInput`, `DigitalInput`, `Level`, `Status`, and time APIs need no migration. Lessons 070--072 retain module characterization ownership. The durable button-start decision for Lesson 042 does not change these policies: PIR supplies eligibility and an explicit button supplies authorization. |

## Composition pressure scenario

The maximum named consumer is the planned Lesson 042 tabletop course marshal:
four ordered intermediate optical checkpoints, one separate reflective finish
guard, PIR eligibility, HC-SR04 range evidence, an explicit qualified button
start, four checkpoint LEDs, all-red/ready/heartbeat presentation, and one
existing display. The hand-moved card or unpowered model creates no actuation
path. One loop must finish source acquisition into an immutable evidence frame
before the presence and marshal policies consume it.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open promotion blocker.** Measure the acquisition and update order for as many as five optical channels, PIR, button, HC-SR04 completion, one presence frame, marshal update, and one bounded display cell. Bound ADC settling after channel changes, range latency, optical dwell, simultaneous checkpoint edges, display work, worst-case burst time, rollover, and missed-update behavior. A host policy replay alone cannot establish acquisition cadence or starvation margin. |
| Total memory and hardware resources | **Applicable; open promotion blocker.** Measure isolated policy objects and the aggregate sketch's flash, static SRAM, stack peak, copied frames, four-slot records, presenter state, claim occupancy, and pins. Enumerate below, at, and above the supported four-checkpoint capacity. The plan's approximate 18--20 claims and blank module/display/current totals are planning bounds, not acceptance evidence. |
| Shared bus or transport | **Conditionally applicable; open until display selection.** The optical policies participate in no bus or transport. The maximum composition may use one for the selected existing display; promotion must name the owner, borrower lifetime, bounded update, address/pins, partial-init rollback, failure behavior, and restart behavior, or prove that the selected display has no shared transport. |
| Persistence and recovery | **Not applicable.** Optical calibration references are supplied configuration and all policy, run, and record state is intentionally volatile. No storage or RTC owner exists, and reset begins a new in-memory qualification history. |
| Motion, external power, or stored energy | **Not applicable.** The authorized project has no motor, launcher, gate actuator, external-load switch, or stored-energy command; the learner moves a card or unpowered model by hand. Powered sensor emitters, if any, still require ordinary exact-specimen current and shutdown qualification but do not create an actuation authority in these policies. |
| Observation identity and provenance | **Applicable and verified at the Lesson 040 seam; aggregate proof open.** Each observation retains typed-local source ID, calibration revision, sample epoch, raw evidence, quality, and status; a containing boundary supplies source kind when kinds can mix. Focused tests reject source and revision mismatches and compare complete fields for same-time identity and replay. Cross-kind alias prevention and delayed range/optical composition remain Lesson 041/042 proofs. |
| Diagnostic interference | **Applicable; open promotion blocker.** Include raw/event/fault LEDs, four checkpoint LEDs, all-red/ready/heartbeat outputs, display, optional Serial, and named test points in pin, current, flash, SRAM, stack, and loop-time totals. Disabled, busy, or failed presentation must not alter qualification, event lifetime, run ordering, or retained fault evidence. |
| Failure collision and recovery | **Applicable; pure-policy subset passed and aggregate proof open.** The focused suite covers timing versus changed evidence, source/revision/malformed evidence, exact upstream-status latching, candidate clearing, and reset recovery with deterministic retained state. PIR warm-up/stuck state, no range echo, simultaneous checkpoints, button faults, display failure, and resource rollback belong to the not-yet-built aggregate. Adapter partial acquisition and reverse rollback require exact endpoints. |

## Prior-decision impact

- Pure hardware-neutral policy over copied observations: **preserved**.
  Electrical acquisition and endpoint lifetime remain outside both policies.
- Separate responsibilities rather than a universal optical abstraction:
  **preserved**. Reflective magnitude and interrupted level share values, not
  a driver hierarchy.
- Existing `Status`, `Level`, `TimePoint`, `Duration`, rollover, and half-range
  conventions: **preserved**. Local precedence maps rejected structural and
  timing evidence to `InvalidArgument` without widening shared status.
- Inert construction, explicit initialization, non-copyability, reset, fixed
  storage, no exceptions, and no heap: **preserved**.
- Calibration-bound provenance: **extended** to optical observations through
  typed-local source identity and a caller-supplied revision; no persisted
  calibration system is introduced.
- Honest sensor interpretation: **preserved**. Scalar evidence does not become
  lux, distance, electrical-fault, ambient-rejection, or crosstalk proof.
- Lesson 041 copied-source composition and Lesson 042 semantic checkpoint
  bindings: **preserved**. Neither downstream owner acquires optical pins or
  rewrites source evidence.
- Explicit button authorization with PIR eligibility, decision
  `3251b219-1a4a-41b7-8497-d8c6b5a72a98`: **preserved** and outside the
  Lesson 040 API.
- Lessons 070--072 electrical characterization ownership: **preserved**.
  Lesson 040 consumes qualified configuration and does not absorb topology,
  comparator, pull-up, or module descriptor work.
- Exact-specimen qualification, Mega resource evidence, non-Serial
  observation, pencil-visual classification, and separate physical acceptance:
  **preserved and still controlling**.

No prior decision is challenged. The status/precedence questions were resolved
inside the unpromoted boundary without changing shared types or earlier
consumers. If later composition requires a new shared
status code, generic timestamped-observation retrofit, universal endpoint,
hidden emitter scheduler, or a change to `AnalogInput`/`DigitalInput`, the
disposition becomes architectural remediation required and promotion must
stop for a durable decision.

## Stress disposition

**Bounded local remediation complete; E0 core API fit verified, lesson
promotion not yet permitted.** The implementation preserves the planned
separation, dependencies, ownership, explicit-time rules, and fixed resource
model. The pre-implementation gate is passed: timing/source/malformed-evidence
precedence, retained snapshot behavior, status mapping, reversed reference
normalization, candidate clearing, upstream-status latching, and replay now
have focused tests. Independent review found no shared API migration or layer
inversion; its timing-retention and fixture-coverage findings were incorporated
before this checkpoint.

Exact-specimen identity and aggregate resource gates do not invalidate the
pure-policy architecture, but they do prohibit claims about powered adapters,
wiring, ambient compensation, crosstalk, current, or complete Lesson 040/042
promotion.

## Gate result

- Disposition: bounded local remediation complete; E0 core API and
  pre-implementation architecture gate passed
- Open risks: exact optical specimen identity, pin order, supply,
  rails, polarity, pull-ups, emitter behavior, and current remain unqualified;
  no canonical Mega example, linked sketch size, HTML reference, pencil PDF,
  site route, package baseline, or physical acceptance exists;
  aggregate cadence, ADC settling, claim, current, memory, display, diagnostic,
  and collision budgets remain open; physical acceptance is unperformed
- Required discussion or decision IDs:
  `3251b219-1a4a-41b7-8497-d8c6b5a72a98` is controlling for downstream start
  authority; no new user discussion is required for the bounded Lesson 040
  clarification unless it would change a shared public convention
- Remediation owner and next action: local core remediation is complete. The
  Lesson 040 integration owner must build the canonical Mega example, measure
  its linked size, publish HTML and a pencil-visual PDF, integrate site and
  package inventories, and keep powered wiring and acceptance gated until an
  exact specimen is qualified
- Verification commands and results:
  - `make build/host/test_optical_observation`: passed with the strict normal
    C++17 host flags
  - `build/host/test_optical_observation`: passed
  - `make headers-check`: passed, including `optical_observation.h` and
    `Adk.h`
  - `make host-test-sanitize`: passed with AddressSanitizer and
    UndefinedBehaviorSanitizer, including the normal optical test target
  - AVR public-layout measurement: values at most 24 bytes; reflective/beam
    configurations 20/12 bytes; reflective/beam policies 67/51 bytes, measured
    from emitted global `.size` directives with Arduino-bundled
    `avr-g++ (GCC) 7.3.0`
  - AVR implementation-object diagnostic: 3,186 bytes text, zero data/BSS;
    measured with that toolchain for ATmega2560 and GNU `avr-size` 2.26;
    constructor aliases were counted once by emitted section; this is not a
    linked sketch baseline
  - no archive, PDF, site, linked-sketch, or hardware result is claimed
- Maximum-composition scenario and proof: Lesson 042's four checkpoints plus
  reflective finish guard, PIR, HC-SR04, explicit button start, indicators,
  and display is the controlling scenario; its deterministic collision replay
  and measured aggregate resource evidence remain open promotion blockers
- Promotion permitted: no
