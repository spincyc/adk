# Inertial observation design stress pass

This is the pre-implementation architecture stress pass for the Lesson 043
copied-value boundary in
[the Lessons 043--045 plan](../LESSONS_043_045_BALANCE_TABLE_PLAN.md).
It evaluates only the authorized E0 synthetic-replay design. No implementation,
Mega execution, powered sensor, I2C transaction, physical measurement, or
promotion is claimed.

## Boundary

- Name and lesson/project: `InertialObservationPolicy`, Lesson 043
- Reviewer and date: pre-implementation design pass, 2026-07-28
- Proposed public types and operations: `InertialSourceKind`,
  `InertialModel`, `InertialSaturation`, `InertialSampleQuality`,
  `InertialSource`, `InertialVector`, `InertialSample`,
  `InertialObservationConfig`, `InertialObservation`, and
  `InertialObservationPolicy::{initialize,reset,update,snapshot,initialized}`
- Direct dependencies: existing `Status`, `TimePoint`, `Duration`, fixed-width
  integer types, and caller-owned copied synthetic fixtures
- Existing decisions and interfaces reconsidered: explicit supplied time,
  unsigned half-range ordering, status-versus-quality separation, copied
  observation identity, source-owned conversion and qualification, fixed
  storage, deterministic replay, E0/E1 evidence separation, and the retained
  Lessons 067--069 normalization boundary

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural in the proposed E0 boundary.** The policy validates a complete copied six-axis value and derives freshness, saturation, and sequence-gap evidence. It owns no endpoint, resource, transport, register codec, conversion, calibration, filtering, rotation, or integration. Exact MPU6050 and QMI8658 adapters remain separate future producers rather than subclasses or hidden modes of this policy. |
| Ownership and lifecycle | **Natural by design.** The proposed object owns only configuration, sequence history, and a stable copied snapshot. It is inert until `initialize()`, has no borrowed lifetime, callback, heap allocation, claim, or cleanup side effect, and `reset()` returns canonical invalid/not-initialized evidence while preserving configuration. Implementation must prove repeated initialization and reset without partial mutation. |
| Time and ordering | **Natural, with exhaustive boundary proof required.** Policy time and sample time enter explicitly. The design defines future samples, apparent regression, wrap-safe progression, exact-age freshness, semantic delta-zero replay, modular forward sequence, gaps, the ambiguous half range, regression, and identity-domain changes. An exact repeated sample may age from `Current` to `Stale` without becoming a new sample. No hidden cadence or clock is introduced. |
| Errors and status | **Natural.** Existing `Status` carries producer and structural failures. `Current`, `Stale`, and `Saturated` remain value qualities; stale and saturated evidence retain OK status. Structural invalidity and producer failure yield `Invalid`, with the plan fixing collision precedence instead of inventing inertial-specific status codes or silently accepting a payload. |
| Resource budget | **Applicable and open.** The policy is fixed-storage and O(1), with zero E0 pins, timers, interrupts, ADC channels, I2C claims, or heap. AVR targets are `InertialSample <= 52 B` and policy including snapshot/history `<= 96 B`; 64 B and 128 B are hard review thresholds. The full Lesson 045 composition has separate 516 B target/660 B hard resident thresholds and 756 B target/932 B hard worst-live thresholds. Exact `sizeof`, stack, flash, static SRAM, and aggregate coexistence must be measured after implementation. |
| Deterministic proof | **Naturally expressible but not yet run.** Configuration, supplied times, complete samples, and producer statuses fully determine output. Required tests cover lifecycle, every source/configuration identity member, signed extrema, saturation agreement, every producer status, age and sequence boundaries, rollover, semantic replay, byte-stable snapshots, reset, and repeated byte-stable traces. Padding bytes and `memcmp` may not define semantic equality. |
| Packaging and public surface | **Natural if implemented as planned.** One declarative standalone header and one out-of-line source can enter the ordinary host source inventory, umbrella header, Arduino archive, and size baseline without a transport exception. The public physical source enum values are negative-test provenance in E0; validation must return `Unsupported` for unqualified MPU/QMI producers rather than imply an adapter exists. |
| Example and documentation fit | **Natural for E0 replay.** The canonical sketch can acquire no endpoint, configure and initialize one pure policy, then observe a bounded constant fixture, decide its quality, and copy health results. It must say `E0 SYNTHETIC REPLAY`, contain no `Wire`, address, sensor wiring, GPIO/timer endpoint, or powered wording, and remain compile-only. HTML and PDF must preserve the same units/provenance vocabulary; all E0 visuals are pencil drawings because there is no electrically authoritative schematic. |
| Downstream effects | **Contained if provenance remains complete.** Lesson 044 consumes one current, nonsaturated observation; Lesson 045 composes it with copied joystick/button evidence and intent-only presentation. Lessons 022, 031, 033, and 040--042 retain their public contracts. Lessons 067--069 still own record/schema normalization, calibration provenance qualification, cross-device comparison, and recording; Lesson 043 must preserve the fields they need and must not claim equivalence between sources. |

## Composition pressure scenario

The maximum authorized design composition is:

```text
one synthetic inertial fixture
  -> one copied InertialObservationPolicy
  -> one OrientationPolicy
  -> one BalancePresentationPolicy
  -> one BalanceInstrument
plus copied joystick/button observations
plus copied RGB + diagnostic + tone intent records
at the fastest documented replay cadence
with stale + producer fault + button event + diagnostic failure colliding
```

Lesson 043 must remain constant-time within that chain. The fixture cursor is
the only input-sized loop, outside the policy. The maximum replay must include
identity-domain changes, sequence wrap and gaps, an exact repeated sample
aging through the freshness boundary, simultaneous consumer inputs, reset
with faults present, and deterministic restart.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open implementation gate.** Each update must use a fixed number of field checks and widened arithmetic operations, with no retry, blocking, transport wait, or input-sized search. Measure or inspect the complete 043--045 worst-tick path at the fastest documented replay cadence. Replay simultaneous timestamps, exact freshness, one tick stale, wrap, half-range ambiguity, and missed updates; prove the inertial policy cannot starve joystick/button processing or turn cadence into implicit state. |
| Total memory and hardware resources | **Applicable; open implementation gate.** Publish `sizeof` for every 043 type and the later aggregate rows. Measure the candidate input, owned snapshot/history, returned snapshot copy, caller replay storage, global/static SRAM, worst update stack, and flash together. E0 consumes zero claims, pins, timers, interrupts, ADC channels, buses, and powered diagnostics. Crossing a target requires a bounded reduction attempt; crossing a hard threshold blocks promotion. Identity, status, ranges, timestamp, and calibration evidence may not be discarded to save bytes. |
| Shared bus or transport | **Not applicable to E0.** The copied policy and synthetic producer perform no I2C or other transport operation and own no bus or device. A future exact adapter must borrow an initialized `I2cDevice` from one owner and separately prove bounded transactions, address and variant identity, pull-ups and levels, rollback, NACK/stuck-bus behavior, and restart. That seam does not authorize speculative bus fields in Lesson 043. |
| Persistence and recovery | **Not applicable.** Configuration, last sample/domain, gap history, and snapshot are intentionally volatile. No EEPROM, removable storage, schema, wear, torn-write, or reset-survival promise exists. `reset()` deliberately clears sequence history. Durable inertial records and their schema/calibration provenance remain Lessons 067--069 work. |
| Motion, external power, or stored energy | **Not applicable.** E0 replays copied numbers and produces records/intents only. It owns no motor, servo, relay, launcher, external supply switch, moving apparatus, or stored-energy actuation path. The balance table is a stationary synthetic instrument, not navigation, stabilization, collision avoidance, or safety control. |
| Observation identity and provenance | **Applicable and structurally central.** Every sample retains source kind/model/ID, configuration and calibration revisions, declared acceleration/rate ranges, data-ready state, explicit saturation, producer status, timestamp, and sequence. All identity members define the sequence domain. Consumers may freeze presentation, but must not relabel the sample, erase health, or combine delayed values as simultaneous. E0 accepts only `SyntheticFixture`/`Synthetic`; later 067--069 work may qualify and normalize retained records without reconstructing provenance. |
| Diagnostic interference | **Applicable at composition level; E0 policy has no diagnostic owner.** Numeric result cells and optional Serial are presentation evidence outside Lesson 043 correctness. RGB, diagnostic, and tone intents must be included in later aggregate memory/time/failure accounting. Enabling, filling, or failing a diagnostic must not alter validation, freshness, sequence history, or producer-fault attribution. Powered LEDs, tone endpoints, and hardware test points are E1 and cannot be cited as E0 proof. |
| Failure collision and recovery | **Applicable; table and replay proof required.** Structural invalidity dominates staleness; producer failure dominates numeric classification; structurally valid saturation dominates staleness; exact semantic replay recomputes age without a new event. The maximum collision adds a stale sample, producer fault, button event, and diagnostic failure. Inertial health must remain independently attributable, invalid payload must not mutate accepted history, presentation failure must not rewrite source status, and reset/restart with faults present must remain deterministic. |

Capacity proof must cover values immediately below, at, and above configured
ranges and freshness limits; sequence deltas below, at, and above the modular
half range; source-domain changes; and object sizes below, at, or above each
design threshold. Host replay cannot close exact-specimen, electrical, or
bench gates.

## Prior-decision impact

- Layered resource/endpoint/component ownership: **preserved**; Lesson 043 is a
  pure copied-value component with no electrical lifetime.
- Explicit supplied time and half-range ordering: **preserved**; no clock or
  cadence is hidden.
- Existing `Status` plus semantic quality: **preserved**; stale and saturation
  are not recoded as transport failures.
- Source-owned conversion and qualification: **preserved**; E0 fixtures state
  already-converted fixed-point values and physical adapters remain separate.
- Fixed memory, no heap, no RTTI/exceptions, and deterministic replay:
  **preserved in design**, with measured AVR and replay evidence open.
- Circuit-native non-Serial evidence and separate acquisition/safe-state
  proof: **preserved by scope**; E0 produces bounded result records and claims
  no powered circuit. Any E1 adapter must add independent bus/resource and
  electrical safe-state evidence.
- Lesson 022 timing/tone, Lessons 031/033 joystick/button, and Lessons
  040--042 optical/presence/marshal contracts: **preserved**; no public change
  or source substitution is proposed.
- Lessons 067--069 record normalization and source qualification:
  **preserved**; Lesson 043 retains raw producer identity and revisions but
  does not normalize, compare devices, persist records, or certify
  calibration.
- Exact-specimen and physical-evidence gates: **preserved**; physical enum
  values are unsupported negative cases until separate adapter review.
- Pencil presentation for every non-authoritative-schematic visual:
  **preserved**; E0 has no formal schematic.

## Stress disposition

**Natural fit at the pre-implementation design boundary, with quantitative
and replay gates open.** The policy extends the existing copied-observation
pattern without changing ownership, time, status, packaging, or transport
contracts.

Two local buckling risks require deliberate implementation checks:

1. Reusing unit-free `InertialVector` for acceleration and angular rate may
   permit field swaps. If compile/test review shows that named containing
   fields are insufficient, replace it before promotion with two strongly
   named fixed-layout structs. This is bounded local remediation; do not add a
   template, hierarchy, runtime polymorphism, or generic vector framework.
2. A complete copied sample plus snapshot and history may exceed AVR targets.
   First remove duplicate internal representation or use caller-owned
   transient input while preserving every public identity/provenance field.
   Crossing the 128 B policy hard threshold, or needing to erase provenance,
   changes the disposition and blocks promotion.

Any need to change `Status`, `TimePoint`, `I2cBus`, joystick/button contracts,
presentation ownership, or the retained 067--069 scope is architectural
remediation. The owner must stop, enumerate affected consumers and migration
cost, discuss alternatives with the user, and record a consequential decision
before implementation continues.

## Gate result

- Disposition: natural fit at pre-implementation design review
- Open risks: unit-field misuse; exact AVR layout, stack, flash, and aggregate
  SRAM; complete boundary/collision replay; standalone/header/archive
  integration; example and publication audits; exact inertial specimen,
  adapter, electrical schematic, E1 fixture, and bench acceptance
- Required discussion or decision IDs: none at this design boundary; required
  if either bounded risk crosses its hard threshold or a shared contract must
  change
- Remediation owner and next action: Lesson 043 implementation owner must
  implement the smallest copied policy, run the full deterministic matrix,
  publish exact AVR measurements, and update this record after independent
  post-implementation review
- Verification commands and results: plan and canonical-contract inspection
  completed; implementation, tests, compilation, size, package, lesson, site,
  and hardware commands not run because this is a pre-implementation pass
- Maximum-composition scenario and proof: scenario specified above;
  deterministic full-chain replay and measured aggregate resource proof remain
  open
- Promotion permitted: no
