# Tactile and directional observation design stress pass

This is the pre-implementation architecture stress pass for the Lesson 046
boundary retained in
[the component/project cadence](../../projects/component_project_cadence.md).
It evaluates only copied, hardware-independent E0 observations. The later
implementation-depth plan must fix the exact public names and layouts before
code begins. This record does not qualify a Metal Touch, tilt/ball, contact, or
joystick specimen and does not claim powered or physical acceptance.

## Boundary

- Name and lesson/project: tactile and directional observation, Lesson 046
- Reviewer and date: pre-implementation design pass, 2026-07-28
- Proposed public responsibility: preserve copied tactile/contact level,
  polarity, chatter qualification, published qualified pulse-width and
  stuck-active evidence, event identity, source validity, provenance, and
  copied joystick direction without claiming
  proximity, capacitive sensing, gesture recognition, or physiology
- Direct dependencies: the published `ContactDynamics` copied-level policy,
  existing `Status`, `Level`, `TimePoint`, `Duration`, and a bounded
  project-facing translation from the published `AnalogJoystickSnapshot`
- Existing decisions and interfaces reconsidered: endpoint ownership,
  copied-observation identity, explicit supplied time, source-status
  attribution, `ContactDynamics` qualification semantics, joystick
  calibration ownership, fixed storage, authorized-specimen gating, and E0/E1
  evidence separation

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural only as a provenance facade over existing behaviors.** `ContactDynamics` already owns copied-level polarity, chatter qualification, `qualifiedPulseWidth`, one-update events, stuck-active classification, source status, and explicit-time ordering. Lesson 046 must reuse it rather than fork a second qualification/pulse-width state machine. A thin facade may add source identity and Lesson 048 vocabulary, but it must not weaken or reinterpret the underlying observation or infer a new long-touch semantic from its clocks. Joystick direction is a separate copied input translated from an already-calibrated `AnalogJoystickSnapshot`; it is not another contact channel and must not be pushed through `ContactDynamics`. |
| Ownership and lifecycle | **Natural.** The E0 boundary owns copied policy/configuration/history only. It owns no `DigitalInput`, `AnalogInput`, `Button`, `AnalogJoystick`, pin, resource claim, callback, clock, or powered module. Construction remains inert, initialization validates the complete configuration, and reset clears event/history state without fabricating endpoint shutdown. Exact electrical adapters retain acquisition, rollback, and safe-state ownership. |
| Time and ordering | **Natural if the facade preserves published semantics.** Tactile/contact samples retain `ContactDynamics` same-time identity, half-range rejection, rollover, qualification, release, refractory, and stuck-active boundaries. Copied joystick direction needs an explicit observation time and sequence or an implementation-plan proof that frame identity supplies both; a held direction must not become a repeated event merely because the facade is polled. A frame combining tactile and direction evidence must define skew and failure precedence before implementation. |
| Errors and status | **Natural with independent attribution.** Tactile producer status, joystick producer status, structural validity, and timing validity remain distinguishable. A malformed or failed source cannot be relabeled as inactivity or neutral direction. Existing `Status` is sufficient; contact quality/disposition and a bounded directional enum describe domain state. The facade must not invent `Gesture`, `Near`, `TouchStrength`, heartbeat, or physiological status from a binary level. |
| Resource budget | **Applicable and open.** Reuse should avoid duplicating contact history. The E0 boundary consumes zero pins, ADC channels, timers, interrupts, buses, claims, heap, or powered diagnostics. The implementation-depth plan must set AVR object, stack, flash, and aggregate Lesson 048 thresholds, then measure one tactile facade plus copied direction at minimum and the full sculpture composition at maximum. Duplicate retained contact snapshots or a second debounce engine are remediation triggers. |
| Deterministic proof | **Naturally expressible but not yet specified at implementation depth.** Required fixtures include both polarities, chatter below/at/above qualification, exact qualified-pulse-width and stuck-active boundaries, release, refractory, identical replay, changed same-time evidence, rollover, half-range ambiguity, every source status, malformed enums, source-identity changes, and reset/recovery. Direction cases include neutral, four cardinals, diagonals, saturation, contradictory or out-of-range copied values, held input, forward sequence, replay, and source fault. Pairwise tactile/direction collisions and byte-stable replay must be fixed in the plan. |
| Packaging and public surface | **Natural if the plan keeps one small lesson surface.** Public declarations must be standalone and implementations out of line, with ordinary host, archive, umbrella, example, size, HTML, and PDF integration. Re-exporting the full endpoint-owning `AnalogJoystick` through a supposedly pure component would buckle the layer boundary. A generic universal input, gesture hierarchy, variant container, callback API, or runtime-polymorphic source is not justified. |
| Example and documentation fit | **Natural for E0 replay.** The canonical example can replay copied contact and copied direction fixtures, then expose qualified state through bounded LED/RGB intent. It must explicitly say E0 synthetic replay and cannot show powered Metal Touch/tilt wiring, a formal electrical schematic, or a bench claim. All E0 visuals are pencil drawings. Future E1 material must name the exact specimen, voltage/polarity, test point, acquisition proof, independent safe-state proof, and non-Serial observation path. |
| Downstream effects | **Contained if reuse remains explicit.** Lesson 048 can consume source-labelled tactile events and copied direction while Lesson 047 owns bounded motion. Lessons 031 and 037 retain their public contracts and remain the behavioral authorities for joystick calibration and contact qualification. Earlier `Button`, `MagneticContact`, and `PulseInput` interfaces need no migration. Lesson 055 may consume the values later but does not justify generalizing them now. Any change to the published joystick or `ContactDynamics` contracts is architectural remediation. |

## Composition pressure scenario

The maximum authorized E0 design composition is:

```text
copied listed-source level + provenance
  -> one tactile facade reusing one ContactDynamics instance
copied calibrated joystick snapshot + frame identity
  -> bounded directional observation
both observations + explicit time + explicit stop
  -> Lesson 048 motion/light intent
  -> copied stepper-phase, RGB, shift-register, and stop-LED intent
```

The stress replay must collide tactile chatter, an exact stuck-active
boundary, held joystick direction, source replay, explicit stop, and
downstream motion inhibition. Stop remains authoritative in Lesson 048;
Lesson 046 only reports observations and cannot queue motion.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; open implementation gate.** Each input update must be O(1), nonblocking, and use supplied time. No retry, gesture window search, sample queue, transport wait, or hidden polling loop is permitted. Replay at the fastest documented Lesson 048 cadence must prove that chatter and held direction cannot starve stop handling. |
| Total memory and hardware resources | **Applicable; open implementation gate.** Measure the facade, its reused `ContactDynamics`, copied direction input/output, transient frame, caller fixtures, aggregate static SRAM, worst-live stack, and full 046--048 flash together. E0 consumes no hardware resources. Crossing a plan threshold first requires removal of duplicate snapshots/history; erasing provenance or replacing fixed storage with allocation is not an acceptable reduction. |
| Shared bus or transport | **Not applicable to E0.** All evidence is copied. Future exact specimens may use GPIO or analog endpoints, but their owners and electrical qualification remain outside the pure policy. Lesson 046 must not add speculative bus or register fields. |
| Persistence and recovery | **Not applicable.** Qualification history and direction identity are volatile. Reset deliberately clears events and candidates. No EEPROM, storage schema, wear, or restart-survival promise exists. |
| Motion, external power, or stored energy | **Not owned by Lesson 046.** This boundary emits observations only. Lesson 047 owns bounded stepper intent and de-energized shutdown; Lesson 048 owns stop dominance and composition. A tactile or joystick event must never directly energize a coil. E0 contains no motor power, moving element, or stored-energy proof. |
| Observation identity and provenance | **Applicable and central.** A tactile record must distinguish the synthetic fixture from only the source values currently authorized by the implementation-depth plan and retain a stable kind, identifier, and revision. Do not reserve speculative enum variants for every future family. A source change starts a fresh qualification domain; history cannot cross devices. Direction must retain that it was copied from an already-calibrated joystick observation rather than imply a raw axis or tilt specimen. Source identity, time, sequence/frame identity, status, polarity, and qualified state may not be reconstructed downstream. |
| Diagnostic interference | **Applicable at composition level.** LED/RGB/shift-register/tone or Serial presentation cannot affect qualification, event identity, provenance, direction, or fault recovery. E0 intent records are not physical observations. Later powered diagnostics need separate resource and failure evidence and cannot be the sole proof of acquisition or shutdown. |
| Failure collision and recovery | **Applicable; matrix required.** Malformed structure and time identity are checked before behavioral mutation; exact producer status remains attributable; a tactile fault cannot silently neutralize valid joystick evidence, and a joystick fault cannot fabricate tactile release. Invalid frames do not mutate accepted history. Reset/reinitialize with faults present is deterministic, and a source-domain change requires fresh qualification rather than inheriting a held contact. |

## Prior-decision impact

- `ContactDynamics` as the canonical copied contact qualification policy:
  **preserved and reused**, not cloned or retrofitted.
- `AnalogJoystick` ownership of raw axes, calibration, dead zone, saturation,
  and select-button behavior: **preserved**. Lesson 046 receives a copied,
  already-calibrated snapshot and derives only bounded directional vocabulary.
- Hardware endpoints own resources and electrical lifetime: **preserved**.
  The Lesson 046 E0 component owns no pin or shutdown action.
- Existing `Status`, explicit supplied time, modular ordering, fixed storage,
  and deterministic replay: **preserved**.
- Complete provenance and independent producer attribution: **extended
  locally** by the facade without changing the underlying published policies.
- Listed-family authorization versus exact-specimen support: **preserved**.
  `Metal Touch` is planned coverage, not an electrical identity; contact or
  joystick fallback is described honestly and does not certify the named
  module.
- Circuit-native evidence, separate acquisition/safe-state evidence, and
  physical acceptance: **preserved as future E1 gates for powered Lesson 046
  input adapters**. Stepper motion and external motor power remain the
  separately controlled E2 scope of Lessons 047--048.
- Pencil presentation for every non-authoritative-schematic visual:
  **preserved**; the E0 lesson has no formal schematic.

## Stress disposition

**Natural fit.** The behavior fits existing
architecture when Lesson 046 is a small provenance facade that composes the
published `ContactDynamics` policy and a separate copied-direction translator.
No new generic input abstraction or shared-contract migration is warranted.
The remaining planning obligations are ordinary promotion gates rather than a
different disposition.

The following conditions trigger remediation before public implementation:

1. If adding provenance requires changing `ContactObservation`, preserve the
   published type and keep provenance in a Lesson 046 wrapper. A proposed
   change to `ContactDynamics` or its consumers is architectural remediation
   and requires an affected-consumer/migration decision.
2. If direction cannot be derived without borrowing or owning
   `AnalogJoystick`, introduce a small complete copied value at the adapter
   seam. Do not move endpoint ownership into Lesson 046. Changing the
   published joystick contract is architectural remediation.
3. If the wrapper duplicates qualification/history or cannot retain source
   identity within the eventual AVR hard budget, first remove redundant
   storage locally. Crossing the hard budget or dropping provenance changes
   this disposition and blocks promotion.
4. If exact specimen research shows the listed Metal Touch family is not a
   binary contact compatible with the facade, keep that specimen unsupported
   and use the authorized contact/joystick fallback. Do not broaden the API
   into guessed capacitive, proximity, gesture, or physiological semantics.

Any need to change `Status`, `TimePoint`, modular ordering, endpoint ownership,
published Lessons 031/037 behavior, or Lesson 047 stop/shutdown authority is
architectural remediation. Enumerate affected consumers, alternatives,
migration cost, and safety/resource consequences; discuss the materially
different outcome with the user and record a durable decision before code.

## Gate result

- Disposition: natural fit
- Open risks: exact public facade/value layout; provenance-domain members;
  copied-direction identity and diagonal precedence; exact AVR and aggregate
  budgets; complete collision replay; exact Metal Touch/contact/tilt specimen
  qualification; powered Lesson 046 input adapter, schematic, E1 fixture, and
  bench acceptance; separate E2 stepper/motion evidence in Lessons 047--048
- Required discussion or decision IDs: none at this pre-plan boundary;
  required if a published contract must change, a hard resource threshold is
  crossed, or exact specimen evidence contradicts binary-contact reuse
- Remediation owner and next action: the Lessons 046--048 planning owner must
  define the smallest complete wrapper and copied-direction values, numeric
  resource thresholds, deterministic matrices, the Lesson 046 E0/E1 division,
  and the separate Lessons 047--048 E2 boundary; the Lesson 046 implementation
  owner must then prove reuse rather than parallel contact state
- Verification commands and results: canonical queue, cadence, existing
  `ContactDynamics` stress record/header, and `AnalogJoystick` header
  inspected; implementation, tests, compilation, size, package, lesson, site,
  and hardware commands not run because this is a pre-implementation pass
- Maximum-composition scenario and proof: scenario specified above; complete
  046--048 deterministic replay and measured aggregate proof remain open
- Promotion permitted: no; implementation-depth plan, implementation,
  independent post-implementation stress review, and ordinary gates remain
  controlling
