# Lesson 061 resistive-probe architecture stress pass

Status: terminal E0 review passed; powered specimen remains E1a-open.

This pass reviews the
[Lessons 061--063 implementation plan](LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md).

`ResistiveProbeObservationPolicy` is a natural E0 component only as copied
qualification policy. A switched-power `AnalogInput` adapter is not authorized
until one exact water-probe circuit establishes electrical identity and
measured de-energization.

| Pressure | Disposition |
|---|---|
| API and layering | Natural: copied acquisition evidence enters a pure component policy; ADC and excitation lifetime remain a future endpoint composition. No generic liquid sensor is introduced. |
| Ownership and lifecycle | Fixed configuration and one copied observation only; inert construction, explicit initialize/reset, no pins, clocks, pointers, callbacks, heap, or copy/move ownership ambiguity. |
| Time and ordering | Supplied time, producer sequence, cycle time, on-time, age, duplicate, rollover, regression, and half-range behavior are explicit and bounded. Per-cycle duty is not misrepresented as cumulative corrosion history. |
| Errors and status | `Status` retains lifecycle/structural/producer failures. `ProbeQuality` retains dry/damp/wet, saturation, disconnected, excitation fault, and stale outcomes without calling them transport success. |
| Resources | E0 claims no hardware resource. The canonical replay measures 5,662 B flash and 624 B static SRAM. The exact no-LTO probe measures 3,566 B flash, 169 B static SRAM, 123 B synchronous stack, and a 69 B policy object; its 27 B input plus 37 B output buffers are counted once, leaving 7,772 B after the 128 B ISR reserve. |
| Deterministic proof | Host tests pass both calibration slopes, the declared ADC maximum, exact two-low-values disconnected predicate, full-scale saturation, threshold neighbors, a contamination-drift ramp, named stuck/backfeed traces, per-cycle duty, stale, producer fault, ordering, reset, sequence exhaustion, and a literal 37-byte canonical replay witness. |
| Packaging/public surface | One standalone header/implementation/test/example/HTML/PDF/probe inventory; no powered sketch, wiring, or schematic at E0. |
| Downstream effects | Lesson 063 consumes the copied observation. Lesson 070 may later characterize the authorized probe, but must not reinterpret this E0 quality as electrical qualification. |

## Composition pressure

The maximum named consumer is Lesson 063 with one probe policy, one
thermal/radiant policy, one monitor, fixed copied frames, and one outstanding
audit intent. A simultaneous dry-to-wet drift, incomplete discharge,
temperature threshold, radiant pulse, open reed, and failed audit receipt must
remain one bounded update with source-specific attribution.

| Pressure | Applicability and evidence |
|---|---|
| Scheduler/time | Applicable. Bound one probe update per museum frame; no catch-up or hidden sampling loop. Prove exact duty/age boundaries and rollover. |
| Memory/resources | Applicable. Measure isolated and canonical linked aggregate flash/static/stack/object/caller-owned-once buffers, enum/structure ABI, noncopy/nonmove ownership, hidden returns/temporaries, three-byte retained-return edges, fingerprinted tool/core/flags, per-buffer limits, and formula residual against the plan. Stale reviewed markers fail; hard/residual-hard failures are non-reviewable. E0 hardware claims are exactly zero. |
| Bus/transport | Not applicable at E0 because samples are copied values and the policy invokes no transport. Future ADC/excitation ownership is E1a. |
| Persistence | Not applicable: policy state is intentionally volatile. Museum audit intent does not make probe observations durable. |
| Motion/power/energy | Not applicable at E0 because there is no actuation path. Probe excitation and spill/corrosion energy are applicable E1a blockers. |
| Identity/provenance | Applicable. Source/configuration/calibration/sequence/time and complete excitation-cycle evidence are mandatory and replayed. |
| Diagnostic interference | Applicable to the aggregate: result cells and optional Serial cannot affect classification, timing, or duty evidence. Physical indicators remain E1c. |
| Failure collision | Applicable. Excitation-off failure dominates liquid classification; saturation/disconnection, staleness, and producer failure remain attributable and cannot yield healthy. |

## Prior-decision impact

- The four-layer architecture is **preserved**: policy stays above future
  platform/resource/endpoint ownership.
- The authorized-inventory admission rule is **preserved**: family listing
  permits planning only.
- The water/corrosion safety taxonomy is **preserved**: off evidence, duty,
  current, spills, drying, and corrosion remain E1a.
- Existing `AnalogInput` is **preserved**; no switched-power behavior is added
  to it before a concrete qualified adapter exists.
- Lesson 063 composition and Lesson 070 characterization are **extended** only
  by consuming the explicit observation, not by inheriting physical claims.

## Gate result

- Disposition: `natural fit` after fixing the declared ADC domain, exact
  disconnected predicate, and per-cycle-only duty claim
- Open risks: exact probe topology, switched excitation, ADC reference/source
  impedance, off/backfeed behavior, current, corrosion, spill containment,
  drying, calibration stability, and physical observation
- Required discussion or decision IDs: none for E0; any shared switched-power
  endpoint proposal requires a second concrete consumer and separate review
- Remediation owner and next action: no E0 remediation remains; E1a remains a
  separate exact-specimen task
- Verification commands and results: focused strict host and sanitizer tests,
  standalone-header checks, canonical Mega compilation,
  `make museum-case-resource-check`, PDF policy checks, and independent
  core/example/publication/resource reviews pass
- Maximum-composition scenario and proof: the Lesson 061 boundary passes its
  exact isolated and canonical replay gates; the full Lesson 063 composition
  remains a later gate and is not implied by these measurements
- Promotion permitted: yes for the E0 Lesson 061 component and learner
  artifacts; no for powered work or physical support claims
