# Lesson 062 thermal-radiant architecture stress pass

Status: initial pre-implementation review; all powered specimens remain
E1b-open.

This pass reviews the
[Lessons 061--063 implementation plan](LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md).

`ThermalRadiantObservationPolicy` is a natural E0 copied-evidence policy when
it preserves three distinct source contracts. It must not turn a marketplace
label into a driver or erase the difference between converted temperature and
categorical threshold/radiant evidence.

| Pressure | Disposition |
|---|---|
| API and layering | Natural with role-specific copied values. Thermistor milli-Celsius, Digital Temperature categorical state, and radiant raw/threshold state remain distinct; no universal sensor or hardware adapter appears. |
| Ownership and lifecycle | One fixed configuration and copied envelope; inert construction, initialize/reset, no caller pointers, transport, endpoint, callback, heap, or hidden acquisition. |
| Time and ordering | Each source retains its own sequence/time/age. Delayed values are never presented as simultaneous. Radiant active/candidate state has total pulse/sustained boundaries; duplicates do not extend it and reset clears it. |
| Errors and status | Producer/lifecycle/structural failures use `Status`; uncertainty, disagreement, saturation, stale, pulse, sustained, and ambient saturation remain domain quality. |
| Resources | E0 claims zero hardware. The plan's canonical Lesson 062 row controls; exhaustive enum/struct/ownership/caller-buffer/hidden-return probes are promotion gates. |
| Deterministic proof | Signed extremes, widened upper-bound uncertainty arithmetic plus an independent explanatory lower-bound oracle, fixed Digital Temperature categorical mapping, every equality/crossing and disagreement combination, complete pulse/sustained/duplicate/reset edges, saturation, source collision, lifecycle, and replay are required. |
| Packaging/public surface | One policy package and synthetic Mega replay. No thermistor curve adapter, single-wire transport, module pin map, flame stimulus, wiring, or schematic at E0. |
| Downstream effects | Lesson 063 consumes this observation. Lessons 064--066 retain 18B20/single-wire scope; Lessons 070--072 retain module characterization. |

## Composition pressure

The maximum trace combines a thermistor uncertainty interval crossing alarm,
a contrary Digital Temperature threshold, a short radiant pulse becoming
sustained then rail-saturated, stale companion evidence, rollover, reset, and
the Lesson 063 liquid/reed/audit collision.

| Pressure | Applicability and evidence |
|---|---|
| Scheduler/time | Applicable. One bounded update, no polling/retry/catch-up, exact independent ages and radiant duration across rollover. |
| Memory/resources | Applicable. Measure policy and canonical linked Lesson 063 aggregate including copied-once envelopes/buffers, outstanding/dirty slots, fingerprinted tool/core/flags, compiler callgraph plus three-byte retained-return edges, per-buffer limits, and formula residual. Stale reviewed markers fail; hard/residual-hard failures are non-reviewable. E0 pin/timer/bus/ADC/power claims are zero. |
| Bus/transport | Not applicable at E0: all values are copied and no transport is invoked. Exact modules may require different future endpoints. |
| Persistence | Not applicable: calibration identifiers are provenance, not stored calibration. All policy state is volatile. |
| Motion/power/energy | Not applicable at E0 because no output or stimulus path exists. Powered modules and an owned harmless low-energy IR target/source remain E1b gates; exposure is at most 5 seconds followed by at least 30 seconds inactive. Lasers, flame, heater, ignition, eyes, continuous/unattended exposure, unknown targets, and replay are excluded. |
| Identity/provenance | Applicable. The three noninterchangeable roles require distinct IDs plus configuration/calibration/sequence/time/status. |
| Diagnostic interference | Applicable in composition. Result cells/Serial cannot alter hazard classification. Future RGB/LCD evidence is separately budgeted. |
| Failure collision | Applicable. Invalidity/staleness cannot become normal through voting; disagreement, saturation, and producer faults retain side-specific evidence. |

## Prior-decision impact

- Inventory identity gating is **preserved**: `Digital Temperature` remains
  unidentified and is not treated as 18B20.
- Lessons 064--066 are **preserved**: no single-wire transport or DS18B20
  adapter is introduced.
- Lessons 070--072 are **preserved**: raw module characterization is not
  absorbed here.
- The heating/flame/laser exclusions are **preserved**: E0 is synthetic;
  future radiant stimulus is owned, harmless, bounded to 5 seconds with a
  30-second inactive interval, attended, kept from eyes, and never replayed.
- Existing observation provenance and supplied-time decisions are
  **extended** with role-specific copied evidence, not a shared generic type.

## Gate result

- Disposition: `natural fit` after fixing conservative uncertainty thresholds,
  categorical Digital Temperature semantics, and complete radiant state edges
- Open risks: exact thermistor curve and divider, Digital Temperature
  identity/protocol/topology, radiant sensor response/polarity, supply/output
  rails, pull-ups, current, ambient behavior, stimulus geometry, uncertainty,
  and physical acceptance
- Required discussion or decision IDs: none for E0; a common adapter or
  calibrated radiant unit would require separate evidence and review
- Remediation owner and next action: Lesson 062 implementation lane supplies
  exhaustive arithmetic/order tests and exact resource probes
- Verification commands and results: document review only; implementation
  commands are pending
- Maximum-composition scenario and proof: specified above; deterministic
  aggregate fixture pending
- Promotion permitted: yes for E0 implementation; no for powered adapters,
  calibrated physical claims, or flame-related work
