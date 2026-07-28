# Bounded stepper motion design stress pass

This is the pre-implementation architecture stress pass for the Lesson 047
bounded stepper-motion policy in the Lessons 046--048 tactile kinetic-sculpture
arc. It evaluates a pure logical boundary first. Exact 28BYJ-48 and ULN2003
identity, an electrical endpoint, powered motion, and physical acceptance are
separate later gates. No implementation, energized coil, motor movement, or
physical-performance claim is recorded here.

## Boundary

- Name and lesson/project: bounded stepper-motion policy, Lesson 047
- Reviewer and date: pre-implementation design pass, 2026-07-28
- Proposed public responsibility: accept explicit supplied time and a bounded
  motion request; publish one complete four-channel logical coil vector plus
  motion state, direction, phase, remaining travel, and fault evidence
- Direct dependencies: existing `Status`, explicit-time types, fixed-width
  integer types, and caller-owned copied requests; no endpoint or transport
- Existing decisions and interfaces reconsidered: pure policy before physical
  rendering, explicit time, fixed storage, finite travel, stop dominance,
  inactive construction/failed initialization/reset, intent-versus-endpoint ownership,
  E0/E1/E2 evidence separation, and future Lesson 050 homing

The public names and exact record layout remain an implementation-plan
decision. The shape must preserve these semantics without prematurely making
the ULN2003, its LEDs, pin polarity, or a particular half-step table part of a
generic motion abstraction.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural if the first boundary stays pure.** The policy owns legal logical phase progression, rate, finite travel, cancellation, and fault-safe all-off intent. It does not claim pins, drive coils, know ULN2003 electrical polarity, infer shaft position, or report that motion occurred. A later exact-specimen renderer may translate a complete logical vector into endpoint writes only after separate qualification. |
| Ownership and lifecycle | **Natural with a strict inactive invariant.** Construction, failed initialization, reset, cancellation, and fault publish the all-off logical vector and no pending step. The pure policy follows the established `initialize()`/`reset()` lifecycle, owns only configuration and bounded logical state, borrows nothing, and allocates no heap. The implementation plan must not add policy `shutdown()` unless a distinct pure-policy semantic need is demonstrated; physical shutdown belongs to a later endpoint that separately owns all four outputs and acquires or rolls them back atomically. |
| Time and ordering | **Applicable and central.** Time enters on each update; there is no hidden clock, delay, blocking loop, or timer ISR. The contract must define zero rate, minimum and maximum interval, exact due time, late updates, simultaneous request/cancel/fault, unsigned rollover, and whether one call advances at most one phase. The conservative pre-implementation rule is one phase per call with no catch-up burst: lateness remains observable and cannot silently spend several units of travel. Reversal starts from the currently published phase and changes only the next legal neighbor. |
| Errors and status | **Natural using existing status plus semantic motion state.** Invalid configuration or malformed requests return `Status` without mutation. Idle, running, complete, cancelled, and inhibited/faulted are semantic states, not transport errors. Stop/cancel and caller-owned copied source or downstream-failure evidence dominate a simultaneously due step and publish all-off intent. The implementation plan must define exact attribution and collision precedence. A later endpoint failure may be fed back only as explicit copied evidence and may never be relabeled as successful travel. |
| Resource budget | **Applicable and open.** The pure policy must be fixed-storage, O(1), and own zero pins, timers, interrupts, buses, ADC channels, or heap. The implementation plan must set measured AVR object, stack, flash, and aggregate Lesson 048 budgets before code. Lesson 048 owns any E1 phase-mirror presentation; a separate future Lesson 047 E2 motor endpoint owns the four driver outputs. Each needs a distinct pin/claim/current budget, and four coil indicators remain evidence channels rather than proof of motor movement. |
| Deterministic proof | **Naturally expressible but not yet run.** Supplied time, configuration, request, stop/fault inputs, and prior state fully determine the next snapshot. Tests must exhaust every legal vector and neighbor in both directions, exact due boundaries, late calls, travel values below/at/above capacity, cancellation at every phase, reversal, rollover, invalid input without mutation, reset, restart with faults present, and byte-stable replay. Endpoint shutdown is a separate E2 test. |
| Packaging and public surface | **Natural if separated into policy and qualified renderer.** Lesson 047 may first publish one standalone declarative policy header and out-of-line source through the ordinary native/Arduino inventories. Exact electrical rendering must not be smuggled into the pure type via Arduino calls, pin numbers, conditional builds, or callbacks. No shared generic motion interface is justified by one stepper consumer. |
| Example and documentation fit | **Natural at E0.** The canonical first example replays bounded requests and exposes complete logical coil vectors using inert result cells or a simulator view; Mega compilation is packaging evidence only. All non-schematic PDF visuals use pencil presentation. A wiring diagram becomes an electrically authoritative formal schematic only after exact E2 qualification; until then there is no schematic, powered wiring table, or motor-observation claim. |
| Downstream effects | **Contained if logical travel is not called position.** Lesson 048 may mirror intent on existing qualified lights before motor power and must preserve independent stop indication. Lesson 050 may consume requested/issued logical steps, but still begins with unknown physical position until bounded homing succeeds. Existing `DigitalOutput`, resource claims, explicit-time rules, motor safety, and project stop semantics remain unchanged. Reusing the policy for DC motors, servos, launchers, stabilization, or a generic actuator hierarchy would challenge established scope. |

## Logical vector and transition contract

The implementation plan must enumerate one finite legal-vector table and one
explicit adjacency rule. Every active output is a complete four-bit logical
vector, never a partial pin mutation. Forward and reverse traversal use
opposite neighboring orders over the same table. All-off is a safe-state
vector outside the energizing cycle; it is not counted as a completed step or
as a remembered physical position.

Required pre-promotion evidence includes:

- every table entry is unique, bounded to four channels, and permitted by the
  selected logical sequence;
- every forward and reverse transition is an adjacent table transition;
- request start, reversal, completion, cancellation, fault, and reset have an
  exact vector and phase-history result;
- remaining travel decrements only when one new legal vector is published;
- zero travel never energizes, arithmetic cannot wrap into more travel, and
  no cancellation leaves a latent request;
- a late update cannot emit an unobservable catch-up burst; and
- logical vector publication is never described as coil current, shaft angle,
  torque, accuracy, or completed physical travel.

Choosing full-step versus half-step sequencing, gearbox ratio, direction
labeling, holding behavior, and electrical active polarity requires an
explicit plan decision. None may be inferred from a retail product name.

## Composition pressure scenario

The maximum currently authorized design composition is:

```text
copied touch/contact + joystick/directional observations
  -> Lesson 048 bounded sculpture policy
  -> one Lesson 047 logical motion policy
  -> copied four-channel coil intent
  -> inert RGB/shift-register phase mirror
plus independent stop input and stop indicator
with a due phase + reversal + stop + copied downstream-failure report colliding
```

E0 runs this chain as deterministic replay with no endpoint. E1 may render the
logical vectors only to already qualified resistor-limited indicators. E2 may
add the exact stepper/driver/load-supply system only after its independent
electrical and mechanical gates close.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open implementation gate.** Bound the work of one update independent of lateness and requested travel. Replay exact due time, one tick early/late, the maximum representable late interval, rollover, repeated timestamps, missed calls, simultaneous controls, and fastest documented cadence. One call emits at most one transition; the consumer must remain responsive to stop and cannot be starved by catch-up. |
| Total memory and hardware resources | **Applicable; open implementation gate.** Measure policy state, request/snapshot copies, full Lesson 048 resident SRAM, worst live stack, and flash. E0 has zero hardware claims. E1 includes every mirror LED/output and diagnostic in pin/current/timing budgets. E2 separately accounts for four owned outputs, claim-registry capacity, load supply, motor and driver currents, dissipation, clamp path, and physical power removal. |
| Shared bus or transport | **Not applicable to the pure policy.** Inputs and outputs are copied values and no bus or transport is owned or borrowed. A shift-register mirror in Lesson 048 is a separate renderer with its own already-established transport ownership. Adding pin writes, SPI, or an endpoint callback to the policy invalidates this result. |
| Persistence and recovery | **Not applicable to Lesson 047.** Phase, remaining travel, and request state are intentionally volatile. Reset loses any inference about physical position and publishes all-off. Lesson 050 owns the later homing rule; persistence may not turn issued logical steps into known physical position after reset or power interruption. |
| Motion, external power, or stored energy | **Applicable at E2 and a promotion blocker for physical claims.** The exact motor voltage/current, ULN2003 variant and COM clamp connection, separate current-limited load supply, common reference, thermal/sequence limits, restrained lightweight fixture, independent physical disconnect, and all-off behavior must be reviewed as one system. Startup, stale command, stop, reversal, resource loss, logic reset/unplug, driver fault, stall, shutdown, and load-power removal require recorded observations. Firmware all-off is not physical isolation. |
| Observation identity and provenance | **Applicable.** Each snapshot must retain request identity or sequence, supplied time, phase/vector, direction, remaining travel, semantic state, and fault/status source. The sculpture may mirror only the same snapshot it presents as motion intent; delayed light and motion records cannot be combined as though simultaneous. E0 replay preserves this identity without claiming endpoint execution. |
| Diagnostic interference | **Applicable.** Phase-mirror lights, the independent stop LED, optional Serial, and future ULN2003 channel LEDs have separate evidence meanings and budgets. Filling or failing diagnostics cannot delay stop, advance phase, decrement travel, clear a fault, or become required for correctness. ULN2003 LEDs show commanded channel state only; they do not prove coil current or shaft movement. |
| Failure collision and recovery | **Applicable; exhaustive table required.** The maximum collision is a due phase plus direction reversal, stop assertion, malformed new request, and an explicit caller-owned copied downstream-failure report. The implementation plan must fix precedence and retained attribution for every colliding item. Stop or accepted fault evidence must prevent transition, publish all-off intent, consume no travel, and leave no request for automatic restart. Recovery requires faults cleared plus a new explicit bounded request; it never resumes from an inferred physical position. |

## Evidence gates and renderer deferral

| Gate | Permitted evidence | Explicitly not established |
|---|---|---|
| E0 pure policy | Host tests, sanitizer replay, deterministic fixture traces, Mega compile-only packaging, object/stack/flash measurements, and inert logical-vector result cells | GPIO operation, ULN2003 support, electrical polarity, coil current, motor movement, direction, travel, torque, timing performance, or safe powered shutdown |
| E1 inert Lesson 048 presentation | Lesson 048 uses an independently qualified resistor-limited LED or display to present the copied logical vector and stop state; its pin/resource acquisition, rollback, and all-off can be observed without a motor supply | A Lesson 047 electrical renderer, stepper-driver or motor behavior, clamp path, load-current safety, physical direction, completed travel, stall response, or motion acceptance |
| E2 powered motion | Exact-specimen records, primary sources, authoritative schematic, all-power-off inspection, current-limited separate supply, measured rails/coil current/thermal behavior, restrained first motion, fault injection, physical disconnect, and signed bench acceptance | Support for another 28BYJ-48 winding/voltage, another ULN2003 board revision, payload, speed, torque, accuracy, continuous duty, or unattended use beyond the measured fixture |

The renderer remains deferred from the pure policy even if a candidate
ULN2003 board appears to use the expected four inputs. Qualification must
resolve exact motor winding and rated voltage/current; connector order; driver
part and board topology; logic/load rails; active polarity; COM/clamp wiring;
indicator meaning; idle and worst-case current; dissipation; power sequencing;
and no-backfeed behavior. Only then may one renderer own four endpoints and
perform a fail-all-off complete-frame update. If atomic electrical switching
is impossible, the plan must bound the break-before-make transition and prove
that each intermediate vector is legal; the policy must not pretend four GPIO
writes occur simultaneously.

## Prior-decision impact

- Endpoint/resource ownership and constructor-inert lifecycle:
  **preserved in design**; the pure policy owns no endpoint, while a future
  renderer must own and atomically roll back all four claims.
- Explicit supplied time, rollover-safe ordering, and nonblocking updates:
  **preserved**, subject to exhaustive boundary replay.
- Existing `Status` plus semantic state: **preserved**; physical completion is
  never fabricated from an issued vector.
- Fixed storage, deterministic replay, and no heap: **preserved in design**,
  with measured AVR and aggregate gates open.
- Intent-versus-electrical rendering: **preserved and made safety-critical**;
  logical vectors are complete copied intent, not pin state or motion proof.
- E0/E1/E2 evidence separation and no physical claim without a bench record:
  **preserved**.
- Motion/inductive-load safety and independent power removal:
  **preserved**, but exact E2 evidence remains a blocker.
- Lesson 048 stop dominance and light-before-motion staging: **preserved**.
- Lesson 050 unknown-until-homed position: **preserved**; Lesson 047 exposes
  no absolute-position promise.
- Pencil drawings for non-schematic visuals: **preserved**; only a qualified
  exact E2 circuit may justify a formal schematic.

## Stress disposition: natural fit

Finite legal vectors, explicit time,
bounded travel, cancellation, and all-off intent extend established copied
policy conventions without changing shared ownership or status contracts.

The following remediation triggers stop implementation or promotion:

1. If late-update requirements demand multi-step catch-up or blocking motion,
   retain one-transition-per-call and revise the supported rate/cadence. A
   scheduler, interrupt-owned sequencer, or hidden timer is architectural
   remediation requiring discussion.
2. If direction, phase, or travel cannot be defined without exact gearbox or
   energized-motion assumptions, narrow the E0 names to logical transitions.
   Do not encode unmeasured steps-per-revolution, speed, position, or accuracy.
3. If a renderer needs partial output ownership, callback borrowing, or
   policy-visible pins/polarity, keep it separate and redesign it around one
   owner of all four endpoints. Changing shared endpoint/claim contracts is
   architectural remediation.
4. If the selected electrical sequence permits a hazardous intermediate
   during sequential GPIO writes, block E2. Resolve with a bounded
   break-before-make renderer and exact trace evidence; never weaken the legal
   vector table to match incidental write order.
5. If memory, stack, flash, pins, current, timing, or thermal measurements
   cross a declared hard threshold, narrow supported rate/travel/diagnostics
   or the physical fixture. Erasing status, fault provenance, stop dominance,
   or all-off state is not an acceptable optimization.
6. If Lesson 048 or Lesson 050 needs absolute position, automatic restart, or
   persistence from issued steps, preserve Lesson 050's unknown-until-homed
   rule. A broader position contract requires an explicit cross-lesson
   decision and migration review.

Any proposal to generalize this boundary across servos, DC motors, homing,
stabilization, safety interlocks, or launcher control is outside this local
pass. Stop, enumerate affected consumers and hazards, discuss alternatives,
and record a consequential decision before proceeding.

## Gate result

- Disposition: natural fit
- Open risks: exact vector/hold contract; cadence and travel arithmetic;
  measured AVR and Lesson 048 aggregate resources; complete collision replay;
  exact 28BYJ-48/ULN2003 identity; renderer ownership and intermediate writes;
  supply, clamp, current, thermal, fixture, disconnect, and E2 bench evidence
- Required discussion or decision IDs: none for E0 planning; required if a
  remediation trigger changes shared timing, endpoint, status, position, or
  safety contracts
- Remediation owner and next action: Lesson 047 planning owner must fix the
  logical table and boundary semantics, declare quantitative budgets, and
  build the exhaustive pure replay before any renderer owner begins E1/E2 work
- Verification commands and results: canonical queue, cadence, stress
  template, safety taxonomy, development, testing, and safety contracts
  inspected; implementation, compile, size, package, lesson, site, and
  hardware commands not run because this is a pre-implementation pass
- Maximum-composition scenario and proof: scenario and collision precedence
  specified above; deterministic full-chain replay and measured aggregate
  evidence remain open
- Promotion permitted: no
