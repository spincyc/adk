# Contact dynamics design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the hardware-independent Lesson 037 behavior. It evaluates the public
boundary before publication; it does not qualify a particular contact module
or claim physical acceptance.

## Boundary

- Name and lesson/project: `ContactDynamics`, Lesson 037
- Reviewer and date: independent read-only architecture review, 2026-07-28
- Public types and operations: `ContactQuality`, `ContactDisposition`,
  `ContactDynamicsConfig`, `ContactSample`, `ContactObservation`, and
  `ContactDynamics::{initialize,reset,update,initialized,snapshot}`
- Direct dependencies: `Level`, `Status`, `TimePoint`, `Duration`, and fixed
  width integer types
- Existing decisions and interfaces reconsidered: pure copied-sample
  behaviors; explicit unsigned time; source-fault latching; construction,
  initialization, and reset; fixed Mega 2560 memory; endpoint ownership in the
  lesson adapter; and earlier `Button`, `MagneticContact`, and `PulseInput`
  boundaries

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural after local repair.** The component turns copied level evidence into qualified contact events, refractory disposition, pulse width, and stuck-contact quality. It owns no endpoint, pin, clock, callback, or output. The adapter retains electrical acquisition and lifetime, and downstream consumers receive only the observation value. Dependencies continue downward to existing value types. |
| Ownership and lifecycle | **Natural.** Construction is inert and the type is non-copyable and non-movable. `initialize()` validates policy without acquiring resources and is idempotent. `reset()` clears copied history, candidates, counts, events, and latched source fault while preserving initialized state. The plan explicitly classifies this as a non-owning pure policy: endpoint shutdown, claim release, destruction, and safe electrical state remain adapter responsibilities. |
| Time and ordering | **Natural.** Every update carries one explicit `TimePoint`. Identical complete samples at the same time are idempotent; changed same-time evidence and apparent advances at or beyond half-range produce a timing fault before lower-priority interpretation. Natural rollover is valid. Qualification, release, stuck, refractory, and accepted-attack boundaries have explicit precedence, including exact-edge behavior. One-update events and disposition are stable until the next later update. |
| Errors and status | **Natural after local repair.** Existing `Status` values express invalid configuration, invalid runtime evidence, upstream failure, and timing failure. Non-OK source evidence latches its exact status without manufacturing an event. The initial review found that an invalid `ContactSample::rawLevel` could be interpreted as inactive. The unpromoted implementation now validates the enum after time identity and before source status or raw-state mutation, latches `SourceFault` with `InvalidArgument`, preserves prior qualified/raw evidence, and requires reset. Focused tests prove precedence and recovery. |
| Resource budget | **Natural.** State and work are fixed and bounded: no heap, RTTI, exception, interrupt, timer, bus, endpoint, claim-registry entry, or hidden sample buffer is added. Counters saturate at `UINT32_MAX`. Four instances are planned for Lesson 039, while their endpoint, LED, display, and sound resources remain explicit in the composing sketch. Exact AVR flash, SRAM, stack, and pin evidence remains an ordinary example/promotion gate. |
| Deterministic proof | **Natural.** Host fixtures provide complete copied samples and cover lifecycle, invalid configuration, both active polarities, qualification and release edges, bounce, refractory suppression and exact completion, stuck activity and recovery, source and malformed-evidence latching, same-time identity, half-range, backward time, rollover, saturation, reset, and replay equivalence. Tests inspect public state except for bounded counter-saturation setup. |
| Packaging and public surface | **Natural.** The public declaration is standalone and implementation is out of line. The ordinary host inventory has a focused target; native and Arduino source discovery need no component exception. Inclusion through `Adk.h`, the canonical example, measured AVR size, and full archive gates remain promotion work rather than architectural exceptions. |
| Example and documentation fit | **Natural.** The planned sketch is the explicit adapter owner and retains acquire/configure/start followed by observe/qualify/present. One endpoint update feeds one copied sample. The plan correctly limits injected non-OK status to the pure seam: current `DigitalInput::update()` returns `void`, so the sketch cannot claim a runtime read failure it cannot observe. A later electrically specific adapter must provide qualified status evidence or narrow its claim. Pencil orientation drawings, formal schematic classification, named test points, non-Serial indicators, shutdown evidence, and deferred bench acceptance remain explicit. |
| Downstream effects | Lesson 039 can compose four independent observations without callbacks, allocation, or shared mutable policy. Earlier `Button`, `MagneticContact`, and `PulseInput` APIs need no migration: they own different electrical or pulse-capture responsibilities and publish different histories. No existing example, lesson, status table, packaging promise, or physical record changes to accommodate this boundary. |

## Prior-decision impact

- Hardware-neutral policy over copied observations: **preserved**. Endpoint
  ownership and electrical lifetime do not move into the component.
- Existing `Status`, `TimePoint`, `Duration`, replay-identity, rollover, and
  half-range conventions: **preserved**.
- Inert construction, explicit initialization, non-copyability, and reset
  after a latched behavior fault: **preserved**.
- Fixed-memory, no-exception, no-heap Mega 2560 policy: **preserved**.
- Stable-level qualification and release gating: **extended** with explicitly
  bounded refractory, width, count, and stuck-contact policy rather than
  retrofitted into `Button` or `MagneticContact`.
- Lesson 039 copied-observation composition: **preserved**. It consumes the
  published value without taking endpoint ownership.
- Electrical specimen qualification, non-Serial evidence, pencil-visual
  policy, and open physical acceptance: **preserved**.

No prior decision is challenged. The review does reveal useful future audit
candidates: `Button` and `MagneticContact` predate the complete copied-frame
identity and source-status conventions, while `PulseInput` has a distinct
microsecond armed-capture contract without changed-same-time rejection.
Those differences do not invalidate their current consumers and are not
prerequisites for Lesson 037. A shared edge-qualification base would migrate
several public APIs without a second proven pure-policy consumer, so generic
consolidation is explicitly deferred rather than manufactured here.

## Stress disposition

**Bounded local remediation completed.** The malformed-level validation defect
was confined to the unpromoted `ContactDynamics` boundary. Its correction
preserves the planned public API, prior decisions, and downstream behavior.
After that repair, the component is a natural architectural fit; no
cross-component remediation or user decision is required.

## Gate result

- Disposition: bounded local remediation completed; natural fit after repair
- Open risks: exact contact specimen behavior and E1 evidence remain gated;
  polling can miss transitions between updates; current `DigitalInput` cannot
  originate runtime read-failure status; ordinary AVR size and complete
  lesson publication gates remain open
- Required discussion or decision IDs: none for the component architecture
- Remediation owner and next action: no architecture remediation remains;
  the Lesson 037 integration owner must complete the adapter, canonical
  example, measured size, HTML, pencil-visual PDF, and deferred physical
  acceptance record without overstating endpoint fault evidence
- Verification commands and results:
  - `make build/host/test_contact_dynamics`: passed
  - `build/host/test_contact_dynamics`: passed
  - focused strict host compilation of `contact_dynamics.cpp`: passed
  - host object inspection: 4,236 bytes text and zero data/BSS; this is not an
    AVR size measurement
  - `git diff --check`: passed after this record
- Promotion permitted: yes by the architecture stress gate; ordinary build,
  lesson, size, packaging, and publication gates remain independently
  controlling
