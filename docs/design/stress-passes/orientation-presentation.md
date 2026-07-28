# Orientation and presentation design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the planned Lesson 044 board-frame orientation and presentation-intent
boundary. It is a pre-implementation review: the public shape and required
proof are specified, but no implementation, test, size, packaging, or
publication result is claimed.

## Boundary

- Name and lesson/project: `OrientationPolicy` and
  `BalancePresentationPolicy`, Lesson 044
- Reviewer and date: pre-implementation architecture review, 2026-07-28
- Public types and operations: `SignedAxis`, `BoardFrame`,
  `OrientationQuality`, `BalanceDirection`, `OrientationConfig`,
  `OrientationEstimate`,
  `OrientationPolicy::{initialize,reset,update,snapshot,initialized}`,
  `BalanceLightIntent`, `BalanceToneIntent`, `BalancePresentationConfig`,
  `BalancePresentation`, and
  `BalancePresentationPolicy::{initialize,reset,update,snapshot,initialized}`
- Direct dependencies: the planned Lesson 043 copied
  `InertialObservation`, existing `Status`, and fixed-width integer types
- Existing decisions and interfaces reconsidered: pure policy over copied
  evidence, explicit mounting configuration, fixed-point units, conservative
  error classification, status-versus-quality separation, intent-versus-
  endpoint ownership, supplied diagnostic phase, fixed storage, E0 synthetic
  replay, and the later Lessons 067--069 normalization boundary

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural in the planned boundary.** `OrientationPolicy` maps one already-qualified copied observation into board-relative pitch, roll, and quality. `BalancePresentationPolicy` maps that estimate plus caller-supplied sensitivity and diagnostic phase into complete light/tone intent. Neither policy owns a sensor, transport, endpoint, RGB LED, sounder, pin, timer, bus, clock, or scheduler. Physical acquisition remains in future exact adapters; physical actuation remains in existing presentation components. |
| Ownership and lifecycle | **Natural by design; proof open.** Both policies are inert, fixed-storage value owners with explicit initialization and reset. They borrow nothing and acquire no hardware resources. Invalid configuration must fail without publishing a partly configured state; repeated initialization and reset must be deterministic; the uninitialized and reset snapshots must be canonical, fault-safe values. Tests must prove these properties before promotion. |
| Time and ordering | **Natural.** Lesson 043 owns freshness and observation time. Lesson 044 neither advances time nor integrates angular rate. It consumes one immutable observation per update; angular rate is used only as a stationarity guard. The caller supplies the complete diagnostic phase, so the presentation policy cannot hide blinking time. Repeated identical inputs must be byte-stable, while a direction-change tone depends only on the immediately preceding accepted presentation direction. Reset must erase that direction history without emitting a tone. |
| Errors and status | **Natural if the planned precedence is retained.** Non-OK upstream status is preserved in `Invalid`; stale, saturated, malformed, or otherwise non-current input never leaks angles into presentation. Gravity or rate guard rejection is semantic `Unsteady`, not a fabricated transport failure. Valid angles classify as `Level`, `Tilted`, or `BeyondPresentationRange`. Presentation configuration or sensitivity errors use existing `Status`; invalid/non-OK estimates yield red fault intent and no tone. No new shared status convention is needed. |
| Resource budget | **Applicable and open.** The AVR target/hard thresholds are 80/112 bytes for `OrientationPolicy` and 64/96 bytes for `BalancePresentationPolicy`. The controlling resident Lesson 045 composition target/hard threshold is 516/660 bytes including four policies and caller replay storage; worst live composition plus candidate and returned snapshot is 756/932 bytes. The complete E0 sketch must remain at or below 2,048 bytes static SRAM, retain at least 1,024 bytes measured stack margin, use at most 28 KiB flash, allocate no heap, and claim zero pins, timers, interrupts, buses, or ADC channels. Every object and aggregate requires AVR measurement; crossing a target requires a bounded reduction attempt, and crossing a hard threshold blocks promotion. |
| Deterministic proof | **Specified, not yet run.** The implementation must use 64-bit widened products, sums, absolute values, and sign changes; floor integer square root; and the exact 16-step integer CORDIC recurrence and immutable microdegree table in the plan. C++ signed division by powers of two, not signed right shift, fixes negative rounding. Output rounds half away from zero. An offline high-precision oracle sweep over accepted-domain boundaries, threshold-adjacent values, and randomized vectors must prove maximum angular error no greater than 4 millidegrees. All 24 right-handed frame mappings, every quadrant and special axis case, overflow edges, guards, lifecycle, replay, and intent transitions remain required. |
| Packaging and public surface | **Natural if kept as one declarative header and one out-of-line source.** The policies belong in ordinary native/Arduino inventories and the umbrella header without a device driver, generated lookup dependency, floating-point runtime, or source duplication. Standalone-header, strict host, sanitizer, Mega compile, archive-consumer, and measured-size gates are open. The CORDIC table and recurrence must have one production definition used by host and Arduino builds. |
| Example and documentation fit | **Natural under the E0 boundary.** The canonical sketch replays synthetic stationary poses and copies complete intent records; it does not touch physical LEDs, a sounder, a sensor, or a debug transport. Its stable host/simulator intent cells are the E0 non-Serial inspection path, while Mega compilation is packaging evidence only. HTML and PDF must explicitly call both input and output synthetic. Every axes, pose, vector, threshold, state-flow, and timing visual is a pencil drawing; no formal schematic is justified without an exact qualified powered circuit. |
| Downstream effects | **Contained if the seam holds.** Lesson 045 may compose both policies and retain source/sequence/time/calibration identity through freeze, but it must not move freshness, freeze authority, joystick interpretation, or hardware presentation into Lesson 044. Lessons 067--069 retain cross-device normalization, calibration provenance, qualification, comparison, and recording. Existing RGB, mono-LED, and sounder ownership remains unchanged. A generic inertial driver, fused attitude, shared angle abstraction, or endpoint command surface would challenge prior boundaries and require architectural remediation. |

## Composition pressure scenario

The controlling authorized composition is one synthetic producer feeding one
Lesson 043 copied-observation policy, one `OrientationPolicy`, one
`BalancePresentationPolicy`, and one Lesson 045 `BalanceInstrument`, plus
copied joystick/button observations and copied RGB, diagnostic, and tone intent
records. Replay runs at the fastest documented cadence and collides stale
inertial evidence, producer failure, a button event, and diagnostic failure.
The Lesson 044-specific worst step also combines a gravity-band edge, an
angular-rate edge, a diagonal pitch/roll error-band tie, maximum sensitivity,
and a diagnostic-phase change in one update.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open.** Each orientation update must have a fixed operation count: three-axis remap, bounded magnitude calculation, at most two 16-step CORDIC evaluations, classification, and one bounded presentation transform. There is no retry, wait, hidden clock, or input-sized production loop. Measure worst update work and replay simultaneous inputs at the fastest documented cadence, including identical timestamps, rollover handled upstream, missed frames, fault recovery, and direction-change history. Prove Lesson 044 cannot starve Lesson 043 validation, Lesson 045 input assembly, or copied presentation. |
| Total memory and hardware resources | **Applicable; open.** Measure both policy objects, configs, estimates, presentation values, CORDIC locals/table placement, caller copies, returned snapshots, aggregate globals, peak stack, flash, and the complete four-policy composition against the stated targets and hard thresholds. E0 must retain zero digital pins, timers, interrupts, I2C claims, ADC inputs, and heap allocation. Diagnostic intent records count in SRAM even though they own no hardware. |
| Shared bus or transport | **Not applicable to Lesson 044 E0.** Its direct input is a copied value and its output is copied intent; neither policy can transact on or borrow an I2C bus. A future exact MPU6050 or QMI8658 adapter owns its separate transport seam. Introducing bus state, register identity, polling, or adapter fallback here would invalidate this disposition. |
| Persistence and recovery | **Not applicable.** Board frame and presentation configuration are supplied at construction; accepted direction history is intentionally volatile. The policies own no EEPROM, schema, calibration store, wear policy, or reset-surviving promise. Reset returns to canonical invalid/fault-safe snapshots and clears tone-change history. Lessons 067--069 retain persisted provenance and normalized recording concerns. |
| Motion, external power, or stored energy | **Not applicable.** These policies observe copied gravity-relative evidence and emit inert light/tone intent records. They provide no motor, load balancing, stabilization, vehicle, launcher, relay, external-power switch, or stored-energy actuation path. “Balance” names a stationary tabletop presentation, not control authority. Any endpoint-owning or actuating consumer requires a separate safety and resource pass. |
| Observation identity and provenance | **Applicable; downstream proof open.** Lesson 044 must not rewrite the Lesson 043 source, model, sequence, timestamp, configuration revision, calibration revision, ranges, readiness, saturation, or status. Its estimate is derived from exactly one accepted observation, never a mixture of axes from different samples. Lesson 045 must retain that source identity through freeze. Byte-identical copied input, configuration, sensitivity, and phase must replay byte-identical estimates and intents. If the proposed estimate cannot preserve the identity needed by freeze without hidden coupling, stop and locally extend the unpromoted output or record architectural remediation; do not recover provenance by reading mutable upstream state. |
| Diagnostic interference | **Applicable; E0 intent proof open.** The caller-supplied phase chooses between two complete unsteady frames; it cannot affect orientation calculation, direction, quality, status, or tone history. Invalid, unsteady, and beyond-range frames suppress tone. Filling, dropping, or failing a later physical diagnostic must not change policy behavior. Intent values, host inspection cells, and any future endpoints count in their applicable aggregate budgets; Serial is never required for correctness or evidence. |
| Failure collision and recovery | **Applicable; open.** Inject malformed configuration, invalid frame mapping, upstream non-OK status, stale and saturated input, gravity and rate guard failures, CORDIC boundary vectors, invalid sensitivity, and diagnostic-phase changes in pairwise and credible multi-fault collisions. Structural/input failure must dominate angle and direction presentation; `Unsteady` must dominate otherwise-valid tilt; `BeyondPresentationRange` must dominate ordinary direction intent; all three suppress tone. Recovery must require a later accepted update, preserve exact upstream status where applicable, avoid stale angle leakage, and avoid a synthetic direction-change tone after reset or fault. |

The maximum-composition proof must cover capacity immediately below, at, and
above each supported threshold; worst-case simultaneous work; reset and
restart while faults remain; object and stack coexistence; and byte-stable
replay. Host proof cannot establish powered-sensor accuracy, electrical
compatibility, physical presentation, or E1 acceptance.

## Exact math and classification gate

The following are promotion requirements, not implementation results:

1. Board-frame initialization accepts exactly the 24 right-handed signed-axis
   orientations and rejects duplicate axes, opposite reuse, and left-handed
   frames.
2. Gravity magnitude uses widened squares and sums. `(0,0,0)` fails the
   gravity guard and never reaches `atan2`; no narrow negation of
   `INT32_MIN` is permitted.
3. Pitch is
   `atan2MilliDegrees(F, isqrt(R*R + U*U))`; roll is
   `atan2MilliDegrees(R, U)`. The plan's axis and quadrant special cases are
   exact.
4. The production and host builds use the same 16-step CORDIC recurrence,
   table, truncation-toward-zero division, normalization, and half-away-from-
   zero final rounding.
5. The high-precision oracle establishes maximum error `E <= 4`
   millidegrees. Failure blocks promotion.
6. Classification is conservative: `Level` and in-range require
   `abs(angle) + E` to remain within their thresholds; error-band cases do not
   inherit the optimistic class.
7. Dominant directions differing by at most `2*E` resolve pitch-first.
   Otherwise the larger absolute angle wins, with signs mapping only to the
   documented forward/backward/right/left meanings.
8. Presentation scales and clamps the dominant angle without wrap. A short
   tone is an intent emitted only on an accepted direction change; repeated
   direction, level, unsteady, beyond-range, invalid, and fault frames are
   silent.

Changing the approximation, weakening the oracle, widening the accepted error,
using floating-point output without a new measured contract, or relabeling a
linear ratio as degrees is not a local cleanup. It reopens the public units,
threshold semantics, tests, lesson explanations, and downstream freeze
behavior.

## Prior-decision impact

- Pure hardware-neutral policy over copied observations: **preserved**.
  Electrical acquisition and physical presentation remain outside Lesson 044.
- Explicit supplied time and deterministic replay: **preserved**. Freshness is
  upstream; diagnostic phase is caller-supplied; angular rate is not
  integrated.
- Existing `Status` plus domain quality: **preserved**. Transport/structural
  failure is not collapsed into stationary-state classification.
- Fixed-width named units and widened deterministic math: **extended** by a
  bounded integer angle approximation with an explicit oracle and error band.
- Honest sensor interpretation: **preserved**. The output is gravity-relative
  pitch/roll for stationary hand tilt, not yaw, heading, navigation,
  displacement, velocity, fall, gesture, vibration, or fused attitude.
- Explicit mounting instead of a device convention: **preserved** through a
  validated right-handed `BoardFrame`.
- Endpoint/resource ownership: **preserved**. Light and tone values are intent,
  not hardware commands.
- Lessons 067--069 normalization and provenance scope: **preserved**. Lesson
  044 consumes one already-converted source and does not compare device
  families or persist calibration.
- Zero-resource E0 and separately gated exact adapters/E1 evidence:
  **preserved**. No powered module, wiring table, formal schematic, or
  physical observation is authorized.
- Pencil presentation for every non-schematic PDF visual: **preserved**.
  Lesson 044 has no electrically authoritative circuit to exempt.

No prior decision is presently challenged. Promotion must stop if
implementation needs hidden time, axis data from multiple observations,
mutable upstream access, a device-specific convention, transport ownership,
endpoint ownership, fused dynamics, lost provenance, relaxed error semantics,
or a shared angle/status migration.

## Stress disposition

**Natural fit at the pre-implementation design boundary; proof and size gates
remain open.** Gravity-only orientation and separate presentation intent fit
the existing architecture without a shared-contract migration. The exact
math, conservative error bands, explicit mounting, supplied diagnostic phase,
and upstream/downstream separation bound the local implementation.

The first sign of buckling is any need to hide time or source state, drop
identity to meet SRAM, weaken the 4-millidegree contract, duplicate physical
presentation ownership, or absorb device normalization. The bounded response
is to stop Lesson 044 promotion, preserve the existing public contracts, and
measure or prototype a smaller local representation or approximation. If the
result changes units, error semantics, copied-observation identity, layering,
or downstream Lesson 045 behavior, record architectural remediation and a
durable decision before implementation proceeds.

## Gate result

- Disposition: natural fit at pre-implementation review
- Open risks: implementation correctness; full CORDIC oracle and threshold
  sweep; all frame mappings and collision fixtures; AVR object, aggregate,
  stack, and flash measurements; standalone/header/archive/Mega gates;
  example, HTML, pencil-visual PDF, site, and publication review; exact
  specimen adapters, powered presentation, and E1 acceptance
- Required discussion or decision IDs: none at this boundary; discussion is
  required if a remediation changes shared units, error/status semantics,
  identity/provenance, layer ownership, or downstream behavior
- Remediation owner and next action: Lesson 044 implementation owner must
  implement the two pure policies and exact integer recurrence, run the
  deterministic and oracle matrices, publish individual and aggregate AVR
  measurements, and rerun this pass against the measured implementation
- Verification commands and results: none claimed; this is a design-only
  pre-implementation record
- Maximum-composition scenario and proof: scenario specified above; focused
  replay, oracle, collision, aggregate resource, example, packaging, and
  publication evidence remain open
- Promotion permitted: no; implementation and every non-hardware gate remain
  outstanding, while exact powered adapters and E1 acceptance remain
  independently gated
