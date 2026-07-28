# Kinetic-sculpture design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the planned Lesson 048 kinetic light sculpture. It is a
pre-implementation review of the project boundary and its maximum authorized
composition. It does not qualify a touch, tilt, joystick, stepper, ULN2003,
power supply, wiring table, schematic, powered motion, or physical acceptance
result.

## Boundary

- Name and lesson/project: kinetic light sculpture, Lesson 048
- Reviewer and date: pre-implementation architecture review, 2026-07-28
- Public types and operations: planned copied project input, bounded motion
  and light intent, explicit stop/fault state, controlled asymmetric
  transaction, lifecycle operations, and an inspectable snapshot without
  exposing pins or coil writes
- Direct dependencies: Lesson 046 authorized tactile/directional
  observations, Lesson 047 bounded stepper-motion policy, existing joystick,
  RGB LED and shift-register intent vocabulary, `Status`, explicit
  `TimePoint`/`Duration`, and fixed-width values
- Existing decisions and interfaces reconsidered: semantic evidence rather
  than pin identity, explicit time, deterministic replay, bounded work and
  storage, endpoint-owned electrical lifetime, stop dominance, de-energized
  motion safe state, presentation isolation, and the separation of E0 policy
  evidence from E2 powered acceptance

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural if the detailed plan preserves a pure composition boundary.** The project may interpret already-copied authorized input observations, select one bounded pattern, request direction and travel, and produce synchronized stepper/light intent. Lesson 047 owns coil-frame and travel mechanics; endpoint adapters own pins, claims, driver writes, and electrical shutdown. The project must not reinterpret raw chatter, invent proximity or physiological meaning, expose ULN2003 channels as its public API, or make indicator success a prerequisite for stop behavior. |
| Ownership and lifecycle | **Natural after the planned controlled asymmetry.** Construction must be inert. The project owns bounded policy state and retains no update-only input by address. Initialization validates every configuration and dependency before publishing readiness. Structural preflight rejects remain fully nonmutating. For a structurally admitted frame, the project preflights both candidate paths, commits interaction first, and immediately applies a post-commit interaction-health gate. If interaction remains healthy, the already-preflighted motion candidate may commit. If interaction becomes unhealthy, the project cancels/stops the sequence without consuming travel or accepted-motif count and publishes only the canonical inhibited intent; no intermediate energized intent is observable. A separately valid stop or copied fail-closed adapter fault follows the same fail-closed publication rule while retaining prior evidence as provenance. Shutdown, destruction, partial initialization failure, resource loss, and restart invalidate pending travel and request the canonical de-energized coil frame. Physical endpoint owners remain responsible for actually applying and proving that state. |
| Time and ordering | **Natural and fixed in the plan.** Every update receives supplied time. Input age, simultaneous events, pattern cadence, step deadline, cancellation, command expiry, direction reversal, rollover, same-time identity, missed updates, and large elapsed jumps have explicit rules. Preflight, interaction commit, post-commit health gate, and either motion commit or cancellation occur within one project update and publish one final snapshot. Lesson 047 advances at most one due step per call and retains its next logical deadline, so repeated calls catch up without skipping a coil frame or performing unbounded work. |
| Errors and status | **Natural with fixed precedence.** Existing `Status` carries malformed configuration, source, timing, and endpoint failures; project mode carries ordinary idle/running/stopped/fault outcomes. The detailed contract must distinguish operator stop, invalid or stale input, exhausted travel, motion-policy failure, driver/resource loss, and presentation failure. Stop and any motion-invalidating fault dominate new pattern/direction requests and force zero pending travel plus de-energized intent. A failed RGB or shift-register mirror cannot reclassify motion evidence or delay inhibition. |
| Resource budget | **Open at the applicable evidence class.** E0 publication requires the pure project and Lesson 047 objects, copied observations/intents, replay storage, flash, static SRAM, worst-live stack, and bounded update work. Future E1/E2 claims additionally require the applicable endpoint objects, four stepper outputs, RGB channels, shift-register pins and storage, stop input/LED, timers, interrupts, claim entries, currents, and power domains. Exact Mega measurements and margin are required for each claimed composition; isolated object sizes cannot establish fit. |
| Deterministic proof | **Planned and feasible.** Host fixtures must cover chatter and holds, valid/invalid directional sources, every coil vector, forward/reverse transitions, rate and travel edges, stop at every state and phase, changed same-time input, rollover, missed cadence, cancellation, copied adapter resource/driver/power-loss faults, shutdown/restart, and frame-for-frame motion/light replay. A rejected malformed request leaves the complete last admitted snapshot unchanged. A separately valid stop or admitted fail-closed fault retains prior evidence fields for provenance while atomically committing an inhibited phase, zero pending travel, and de-energized intent. Host evidence cannot close E2. |
| Packaging and public surface | **Natural if no special build path is introduced.** The project needs a standalone header, out-of-line implementation, ordinary archive/native inventories, umbrella export, deterministic tests, canonical Mega example, size baseline, HTML reference, and rich pencil-drawing PDF. The public contract should express semantic pattern, direction, travel, stop, fault, phase and intent values; electrical channel order belongs in the qualified adapter and formal schematic. |
| Example and documentation fit | **Natural as staged E0, E1, then E2 work.** The canonical narrative remains acquire, configure, start; then observe, decide, actuate. E0 first replays copied inputs and writes motion/light intent to fixed result cells with no powered endpoint. A later qualified E1 fixture may mirror the commanded phase and independent stop intent on resistor-limited indicators with the motor and load supply absent. All non-schematic PDF visuals are pencil drawings. Only a qualified, electrically authoritative circuit may be labeled a formal schematic. The later E2 stage may energize only a restrained lightweight paper element after every physical gate closes. |
| Downstream effects | **Contained if the surface remains project-local.** Lessons 046 and 047 remain the authorities for source qualification and bounded motion. Existing joystick, RGB, shift-register, output, timing, claim, and status contracts must not change merely to simplify this project. Lessons 049--051 may consume the stepper boundary later, but Lesson 048 must not anticipate homing, durable position, or generic motion-control abstractions. Any such shared-contract pressure requires architectural remediation before implementation. |

## Composition pressure scenario

The maximum E0 composition is one copied Lesson 046 tactile fixture, one
copied directional fixture selected by the detailed plan, one copied
independent stop observation, one Lesson 047 bounded stepper policy, and
compact motion, RGB, shift-register, and stop-intent result cells. E0 owns no
pin or endpoint and the canonical Mega sketch is compile-only. A separate E1
fixture may use qualified source and inert indicator endpoints while the motor,
driver load, and load supply remain absent. E2, only after qualification, adds
four owned driver outputs, one restrained 28BYJ-48/ULN2003 assembly, and a
lightweight blunt paper element inside a guarded envelope.

The maximum-collision trace begins during a direction reversal at the travel
boundary. It injects a simultaneous stop event, stale or invalid source,
missed step deadline, presentation failure, driver/resource loss, and external
motor-power loss, then attempts new motion, reset, acknowledgement, shutdown,
and restart while faults remain. Stop/fault precedence must prevent any
queued, catch-up, or replayed motion from surviving the collision.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open.** Bound acquisition, policy, one transactional project update, stepper sequencing, RGB intent, shift-register intent, and stop indication at the fastest supported cadence. Exercise simultaneous timestamps, reversal, a one-tick deadline edge, rollover, missed calls, a large elapsed jump, and diagnostic failure. No catch-up loop, light animation, or input chatter may starve stop processing or create scheduler-dependent travel. |
| Total memory and hardware resources | **Applicable; open.** Measure the complete canonical Mega ELF and worst-live composition, including all retained objects, candidate values, endpoint state, replay storage, stack, core ISR allowance, pins, timers, interrupts, claim entries, and observation paths. Separately total logic-supply current and external motor/driver current. Four coil outputs, RGB, shift-register control, source inputs, stop input and independent stop LED must fit without undocumented multiplexing or a weakened margin. |
| Shared bus or transport | **Not applicable unless the detailed plan selects one.** The authorized baseline uses direct copied inputs and GPIO-backed intent and introduces no bus or transport. If implementation selects a bus-backed display, expander, or source, this row becomes applicable and must name its owner, address, borrowing, bounded transaction, rollback, contention, failure and restart behavior before promotion. |
| Persistence and recovery | **Not applicable.** Pattern phase, travel budget, direction, stop/fault state and replay identity are intentionally volatile. No position survives reset or power loss; no EEPROM, RTC, removable storage, schema, wear, torn-write recovery, or homing claim belongs to Lesson 048. Restart begins inhibited with no queued motion. |
| Motion, external power, or stored energy | **Applicable; E2 independently blocks only powered-motion support and claims, not E0 host-verified publication.** E0 emits intent only and owns no powered endpoint. E1 may power only qualified inert indicators while the motor, driver load, and load supply remain absent. E2 requires exact specimen identity and primary ratings, separate current-limited load supply, qualified ULN2003 protection, common-ground evidence where required, measured coil/stall behavior, guarded travel, stable restraint, low-energy first motion, command expiry, de-energized startup/reset/fault/shutdown, and an independent physical means to remove load power. A software stop or stop LED is not an emergency stop. E2 remains draft until a named person records bench acceptance. |
| Observation identity and provenance | **Applicable; open.** Each admitted project frame must preserve source kind and ID, configuration/calibration revision where defined, timestamp/sequence, raw interpreted state distinction, validity and status. Motion intent must retain the accepted pattern, direction, travel epoch and coil phase that produced the matching light intent. Delayed or invalid inputs cannot be restamped or mixed as simultaneous, and fieldwise replay must use the same identity rules without comparing padding. |
| Diagnostic interference | **Applicable; open.** RGB and shift-register lights, independent stop LED, four coil-intent observations, optional Serial, and named test points share the same time, pin, memory and current budgets. Filling, disabling, disconnecting or failing a diagnostic must not alter travel accounting, phase order, stop/fault precedence, command expiry, or de-energization. The independent stop indication mirrors inhibition but provides neither stop authority nor proof that motor power is absent. |
| Failure collision and recovery | **Applicable; open.** Structural validation first identifies which independently copied control fields are trustworthy; it does not grant a malformed pattern or direction request authority to suppress a separately valid stop. A valid stop or already-latched inhibition dominates every motion request, including when an unrelated field is malformed. Next come admitted fail-closed motion/source faults, including copied adapter resource, driver, or power-loss status; then travel exhaustion/cancellation, ordinary pattern/direction change, and presentation. A malformed request with no independently valid stop/fault is rejected without partial mutation. An admitted stop/fault commits zero pending travel and de-energized intent before light presentation while retaining prior accepted evidence as provenance. Recovery requires faults to be absent, explicit policy authorization where specified, and a fresh baseline; it cannot replay a held event or resume an interrupted move. |

Capacity tests cover every configured pattern and travel limit immediately
below, at, and above its supported boundary. The transactional proof must
inject failure at preflight, after interaction commit, and before motion
publication. It must prove structural rejection mutates nothing; a healthy
post-commit interaction permits exactly the preflighted motion commit; and a
newly unhealthy interaction cancels/stops sequence state without consuming
travel or accepted-motif count. Result cells must expose only the final
committed or inhibited snapshot, never an intermediate energized intent.
Replay, stop, fault, reset, and shutdown must obey the same rules and preserve
byte-stable semantic replay for admitted frames. The E0 fixture must present
both the exact
commanded coil vector and the independent inhibited indication without
energizing a driver. None of those results is physical motion, current,
thermal, safe-state, or power-loss evidence.

## Prior-decision impact

- Semantic observations and intent below endpoint/electrical ownership:
  **preserved**.
- Explicit supplied time, bounded work, deterministic precedence, rollover,
  and same-time identity: **preserved**. The plan fixes at most one due step
  per call, retains the next logical deadline, and forbids skipped frames.
- Fixed storage, no heap, no exceptions, and measured aggregate Mega capacity:
  **preserved**, with measurement open.
- Stop dominance and no queued motion after shutdown: **extended** to the
  transactional project composition; the stop input is not called an
  emergency stop or physical interlock.
- De-energized startup, reset, fault, cancellation, shutdown and resource
  loss: **preserved** as required intent at E0 and independently proven
  electrical behavior at E2.
- Presentation cannot change primary evidence, travel, phase, or safe state:
  **preserved**.
- Exact specimen, polarity, ratings, supply and protection before electrical
  claims: **preserved**.
- E0 simulation and result-cell intent before E1 inert indicators or E2
  powered motion: **preserved**. Compilation, replay, indicators and an agent
  review do not close the named-person physical acceptance gate.
- Physically separate logic and motor power, a guarded low-energy envelope,
  and independent load-power removal: **preserved**.
- Volatile position with no homing or durable-position claim:
  **preserved**; those concerns remain in Lessons 049--051.
- Pencil drawings for every non-schematic PDF visual and formal-schematic
  classification only for an electrically authoritative qualified circuit:
  **preserved**.

No published interface is yet challenged. The main local strain was the
transaction boundary across opaque interaction state, stepper preview, travel
accounting, replay identity and light intent. The plan resolves it with a
controlled asymmetric protocol: structural preflight, interaction commit,
post-commit health gate, then either the already-preflighted motion commit or
fail-closed sequence cancellation. Implementation must still prove that the
unhealthy branch consumes no travel or motif count and never publishes
intermediate energized intent.

If implementation requires rollback after physical coil writes, mutable
borrowing between policies, an unbounded catch-up queue, timer ownership that
conflicts with existing consumers, weakening stop/fault dominance, changing a
published status/time/endpoint contract, or adding homing/persistence, the
disposition becomes **architectural remediation required**. Promotion stops;
affected consumers, compatibility and migration cost, safety and resource
consequences, alternatives, and a bounded experiment must be discussed and
recorded in a durable decision.

## Stress disposition

**Bounded local remediation.** The project is a natural consumer of copied
tactile/directional evidence and bounded stepper intent. The plan resolves the
opaque-preview coupling locally with the controlled asymmetric protocol and
fixes stop/fault/recovery precedence. Implementation proof remains open:
structural rejects must be nonmutating; healthy interaction must commit only
the preflighted motion candidate; and newly unhealthy interaction must
cancel/stop without travel/count consumption or intermediate energized intent.

E0 result cells can establish deterministic composition without a circuit. A
later qualified E1 indicator fixture supplies the circuit-native non-Serial
observation path while the motor and load supply remain absent. Neither can establish
driver correctness, de-energization, current, restraint, power-loss behavior
or physical safety. Those remain independent gates for powered-motion support,
not blockers to honest E0 host-verified publication.

## Gate result

- Disposition: bounded local remediation
- Open risks: implementation proof of controlled asymmetric commit,
  nonmutating structural rejection, and fail-closed post-commit health gate;
  aggregate cadence, flash, SRAM, stack, pin, timer,
  claim and current budgets; exact input, stepper, driver and supply qualification;
  authoritative schematic; guarded motion and named-person E2 acceptance
- Required discussion or decision IDs: none yet; a durable consequential
  decision is required if the detailed plan changes any published contract,
  introduces timer/resource conflicts, or broadens Lesson 048 into homing,
  persistence, generic motion control, or safety-control behavior
- Remediation owner and next action: Lessons 046--048 implementation owner
  proves the plan-fixed preflight, interaction commit, post-commit health gate,
  and healthy-motion-commit/fail-closed-cancellation branches, plus fixed
  precedence and recovery, capacity/resource limits, E0 trace cells, and the
  complete E2 acceptance matrix; an independent reviewer repeats this pass
  before promotion
- Verification commands and results:
  - canonical queue, cadence, curriculum, project and safety review: completed
  - implementation, aggregate resource measurement, tests and replay: not yet
    available
  - E2 specimen, electrical and physical acceptance: open
  - `git diff --check -- docs/design/stress-passes/kinetic-sculpture.md`:
    passed
- Maximum-composition scenario and proof: scenario fixed above; deterministic
  E0 fixture, measured aggregate resources, fault collision and all E2
  physical evidence remain open
- Promotion permitted: no
