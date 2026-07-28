# Presence model design stress pass

This pre-implementation record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the proposed Lesson 041 PIR qualification and copied-evidence presence
model. It evaluates the planned public boundary; implementation, measured
composition, exact-fixture, publication, and physical gates remain open.

## Boundary

- Name and lesson/project: `PirObservationPolicy` and `PresenceModel`,
  Lesson 041
- Reviewer and date: pre-implementation architecture review, 2026-07-28
- Public types and operations: `PirPhase`, `PresenceQuality`, `PirSample`,
  `PirObservation`, `PirObservationConfig`,
  `PirObservationPolicy::{initialize,reset,update,snapshot,initialized}`,
  `TimedRangeEvidence`, the four explicit optional observation wrappers,
  `PresenceInput`, `PresenceSourceBit`, `PresenceModelConfig`, the copied
  per-source state values, `PresenceSnapshot`, and
  `PresenceModel::{initialize,reset,update,snapshot,initialized}`
- Direct dependencies: Lesson 040 copied `BeamObservation` and
  `ReflectiveObservation`, existing copied `RangeReading`, `Status`,
  `TimePoint`, `Duration`, microsecond time values, `Level`, and fixed-width
  integer types
- Existing decisions and interfaces reconsidered: source-owned qualification,
  copied observation provenance, explicit time domains, same-time identity,
  optional-source absence, status versus value quality, disagreement,
  event ownership, fixed storage, and deterministic replay

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural in the planned boundary.** `PirObservationPolicy` alone qualifies PIR warm-up, polarity, motion/clear dwell, retrigger, stuck motion, and recovery. `PresenceModel` consumes qualified copied PIR and optical evidence plus copied terminal range evidence; it neither owns endpoints nor requalifies those sources. A passage event remains a beam restoration semantic event with retained context, not a generic vote or reuse of magnetic passage policy. |
| Ownership and lifecycle | **Natural, subject to implementation proof.** Both values are planned as inert, fixed-storage pure policies with explicit initialization and reset and no pin, endpoint, callback, bus, timer, heap, or borrowed lifetime. Reset returns PIR warm-up and aggregate passage/disagreement candidates to an unqualified state without synthesizing events. Tests must prove failed initialization and malformed updates leave no partial mutation. |
| Time and ordering | **Natural but proof remains open.** All course time and microsecond acquisition timing enter in copied values. Source epochs may precede the frame and retain measured age; future, half-range, and apparent backward epochs fault before mutation. `TimedRangeEvidence` preserves course-clock start/completion and the distinct microsecond measurement interval rather than pretending delayed range and current optical/PIR samples were simultaneous. Same-time identity, rollover, dwell, freshness, agreement, and passage-window boundaries are specified for deterministic implementation. |
| Errors and status | **Natural.** Existing `Status` represents malformed evidence and source/timing failure. `PirPhase`, `OpticalQuality`, range states, and `PresenceQuality` retain semantic state such as warm-up, stale, disagreement, timeout, and out-of-range. The planned precedence is malformed timing/tuple, source fault, stale, disagreement, then ordinary transition; absence of a required source is `Unqualified` with Ok status rather than an invented failure. |
| Resource budget | **Architecturally bounded; measured gate open.** Both policies use fixed copied records and bounded constant-time source evaluation. The provisional composition reserves PIR D23, beam D22, optional finish guard A0, HC-SR04 D24/D25, and evidence LEDs D30--D32, with no bus, timer, interrupt, storage, actuator, or external-power owner. Final object/stack/flash/SRAM measurements, claim occupancy, exact pins, current, and coexistence margin on Mega 2560 remain required before promotion. |
| Deterministic proof | **Specified but not yet executed.** The matrix covers PIR warm-up/retrigger/stuck/recovery, optical passage, every legal and crossed-invalid range tuple, per-source freshness, simultaneous changes, disagreement, same-time identity, rollover, half-range, collision precedence, reset/restart, and byte-stable replay. Host fixtures must demonstrate no partial mutation and capacity boundaries before this row can close. |
| Packaging and public surface | **Natural if implemented conventionally.** The plan names one standalone declarative header, one out-of-line source, focused host tests, ordinary umbrella exposure, Arduino source discovery, and no special native source path. Archive inventory, exception/sanitizer coverage, and measured size baselines remain open implementation gates. |
| Example and documentation fit | **Natural at the copied-policy layer.** The Lesson 041 narrative can observe each source alone, copy an immutable multi-source frame, decide presence/passage quality, and actuate D30--D32 evidence LEDs. The Mega sketch, HTML, and PDF must use identical warm-up, freshness, disagreement, passage, and provenance vocabulary. All non-schematic visuals must use the required pencil presentation; an authoritative schematic requires an exact qualified circuit. |
| Downstream effects | **Contained if the narrow contract is preserved.** Lesson 042 may consume the stable semantic passage event and copied context. Lesson 040 policies, `UltrasonicRanger`, `RangeReading`, magnetic passage components, telemetry observation tracking, storage, and RTC contracts remain unchanged. A generic timestamped-observation framework, changed upstream observation type, implicit source substitution, or persistence would challenge this result and require architectural remediation. |

## Composition pressure scenario

The maximum currently authorized Lesson 041 composition is one qualified PIR
source, one qualified interrupted-beam source, one optional qualified
reflective finish guard, one HC-SR04 range source bridged into copied
`TimedRangeEvidence`, and three evidence LEDs. Acquisition completes into one
immutable `PresenceInput` frame before policy evaluation. The replay must
exercise simultaneous PIR motion, beam interruption/restoration, finish-guard
activation, and terminal range completion; delayed range versus newer source
epochs; disagreement at its threshold; stale and malformed evidence
collisions; reset with faults present; and recovery without a synthetic
passage event.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open composition gate.** Bound PIR cadence, optical dwell updates, HC-SR04 trigger/echo polling and adapter latency, immutable-frame assembly, policy update, and three LED writes. Replay worst-case simultaneous completion, missed updates, rollover, and one-tick-before/at/after thresholds; prove range polling cannot starve source qualification and frame order cannot change the result. |
| Total memory and hardware resources | **Applicable; open composition and exact-fixture gate.** Measure all source objects, policy objects, copied frame/snapshot values, stack peaks, flash, SRAM, six provisional source pins, three LED pins, claim-registry entries, and current margin on the Mega 2560. Exact AO/DO and PIR specimens may change pin, pull-up, or current facts and therefore must be resolved before the final map is authoritative. |
| Shared bus or transport | **Not applicable to the planned component.** The pure policies and provisional PIR, beam, reflective, HC-SR04, and LED fixture own no shared bus or transport. Adding a display or bus-backed diagnostic reopens scheduling, ownership, rollback, and resource analysis. |
| Persistence and recovery | **Not applicable.** PIR, disagreement, passage-candidate, and copied observation state are intentionally volatile; no EEPROM, RTC, removable storage, schema, wear, or reset-recovery promise is introduced. Reset explicitly returns to unqualified/warm-up state. |
| Motion, external power, or stored energy | **Not applicable.** The project observes a hand-moved card or unpowered model and produces indicator evidence only. It owns no motor, launcher, relay, external supply switch, or stored-energy actuation path. |
| Observation identity and provenance | **Applicable; implementation proof open.** Preserve source ID, source epoch, status, PIR phase, optical provenance and calibration revision, complete range state, course-clock start/completion, and microsecond measurement epoch/latency. Same-time comparison must cover every public input field; repeated identical events are idempotent while changed evidence with the same identity faults. |
| Diagnostic interference | **Applicable; open composition gate.** D30--D32 evidence LEDs and named HC-SR04 trigger/echo test points share the final pin/current/time budget. Serial remains optional. Prove LED or diagnostic failure cannot alter source qualification, aggregate precedence, event identity, or reset behavior, and that non-Serial evidence remains available. |
| Failure collision and recovery | **Applicable; implementation proof open.** Inject a malformed range tuple and timing fault while PIR changes and the beam restores, plus stale/disagreeing required evidence and reset. Structural/timing failure must suppress the event without partially mutating stable state; source attribution and copied evidence must remain inspectable, and recovery must require new valid evidence rather than replaying the rejected transition. |

The required deterministic maximum-composition fixture must cover optional
sources absent and present, required masks and agreement masks below/at/above
their legal limits, worst-case simultaneous work, partial frame rejection,
shutdown/reset and restart, and byte-stable replay. Host proof cannot replace
the exact-specimen electrical record or bench acceptance.

## Prior-decision impact

- Hardware-neutral policy with endpoint ownership in adapters: **preserved**.
- Source-specific qualification rather than a universal sensor abstraction:
  **preserved**; PIR and the two optical policies keep distinct semantics.
- Qualified copied evidence and retained provenance: **extended** by the
  arc-local `TimedRangeEvidence` bridge without changing `RangeReading`.
- Explicit supplied time and unsigned half-range ordering: **preserved** in
  both clock domains; no conversion claims simultaneity.
- Existing `Status` plus domain quality/state values: **preserved**.
- Optical ownership of the passage transition: **preserved**; contextual
  sources never vote or silently substitute.
- Fixed memory, no heap, and deterministic bounded updates: **preserved in
  design**, with measurement still required.
- Non-Serial observation and separate acquisition/safe-state evidence:
  **extended** by three evidence LEDs and named trigger/echo test points,
  pending final fixture qualification.
- No persistence, motion actuation, occupancy, navigation, collision
  avoidance, traffic enforcement, identity, or life-safety claim:
  **preserved**.
- Lesson 042 explicit button authorization with PIR eligibility only:
  **preserved**. Lesson 041 publishes evidence and no start authority.
- Pencil presentation for every non-authoritative-schematic PDF visual:
  **preserved**, pending publication audit.

## Stress disposition

**Natural fit at the planned architecture boundary, with implementation and
composition evidence open.** The separation between PIR qualification,
source-owned optical/range semantics, and aggregate copied-evidence policy
fits existing layering without changing a supported public contract.

Promotion must stop if implementation requires revalidating upstream optical
or range policy, hiding source age/status, converting clocks into a false
common epoch, treating absence as failure, voting away disagreement, adding a
generic observation framework, or deriving start authority from PIR. Those
are architectural changes rather than local implementation details.

## Gate result

- Disposition: natural fit in pre-implementation review
- Open risks: public-header implementation drift; deterministic transition and
  collision coverage; measured host/AVR object, stack, flash, and SRAM costs;
  final pin/claim/current/timing coexistence; exact PIR, beam, reflective, and
  HC-SR04 specimen qualification; authoritative circuit; pencil-visual audit;
  and physical acceptance
- Required discussion or decision IDs: the recorded Lesson 042 explicit-button
  authorization decision is preserved and does not alter this component;
  no additional discussion is required unless a listed architectural boundary
  is challenged
- Remediation owner and next action: Lesson 041 implementation owner must keep
  the proposed separation, build the deterministic source and maximum-
  composition fixtures, measure aggregate resources, and reopen this record
  if implementation pressure changes any public or upstream contract
- Verification commands and results:
  - template-section coverage and plan-contract review: passed
  - implementation, focused host tests, exception/sanitizer tests, trace
    replay, Mega compile/size, package, and publication checks: not yet run
- Maximum-composition scenario and proof: scenario specified above; bounded
  deterministic replay, aggregate size/resource evidence, exact-specimen
  electrical evidence, and bench acceptance remain open
- Promotion permitted: no; architecture fit is provisionally accepted, but
  implementation, measured composition, exact-fixture, publication, and
  physical gates remain independently controlling
