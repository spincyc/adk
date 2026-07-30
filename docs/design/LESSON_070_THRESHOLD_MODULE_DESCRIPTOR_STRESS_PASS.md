# Lesson 070 threshold-module-descriptor architecture stress pass

Status: initial E0 architecture disposition. A declared copied descriptor is
a natural fit for implementation, but implementation, exact resource
evidence, independent review, publication gates, and every powered-adapter
claim remain open.

This pass reviews the Lesson 070 subject fixed by the
[extended component/project cadence](../projects/component_project_cadence.md):
descriptor-driven threshold modules. The descriptor describes the observable
electrical boundary claimed by one copied declared-fixture reference.
It is not a universal module driver, an alias-to-pinout database, a substitute
for an exact-module record, or authorization to energize a seller label.

Structural validity proves only that a copied descriptor is internally
coherent under its declared schema. It does not prove that the descriptor is
true of a physical specimen. Exact identity, primary sources, traced topology,
measurements, and bench acceptance remain separate E1 evidence.

The controlling public contract, ordering, budgets, and downstream seams are
fixed by the
[Lessons 070--072 module characterization plan](LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md).
This pass interprets that plan under composition pressure; it does not offer
alternate API spellings or optional fields.

## Boundary

- Name and lesson: `ModuleThresholdDescriptor` and
  `ModuleThresholdFrame`, Lesson 070
- Review state: initial pre-implementation stress pass
- Public responsibility: statelessly validate one declared copied fixture and
  its attributable copied analog/comparator frame, and derive comparator
  assertion only when polarity is specified
- Direct dependencies: `Status`, fixed-width integers, explicit duration
  values, and closed descriptor enums
- Existing decisions reconsidered: existing `AnalogInput`, `DigitalInput`,
  and `ThresholdInput` keep their current ownership and behavior; Lesson 070
  does not retrofit physical-module meaning into them

The exact public operations are
`validateModuleThresholdDescriptor(descriptor)`,
`validateModuleThresholdFrame(descriptor, frame)`, and
`moduleComparatorAsserted(descriptor, comparatorLevelHigh)`, plus the explicit
`moduleDescriptorDeclarationsComplete(descriptor)` query. The query reports
structural declaration completeness only and grants no E1 or power authority.
There is no stateful Lesson 070 policy or codec.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | Natural only as a copied value and structural validator. Circuit meaning belongs in the descriptor; electrical acquisition and lifetime remain in future exact endpoints. Lesson 071 consumes the descriptor with supplied evidence and owns empirical characterization. A descriptor may not select an adapter, own a pin, or infer a specimen from an alias. |
| Ownership and lifecycle | Natural as caller-owned fixed data with stateless bounded functions. No object lifecycle, heap, callback, retained pointer, clock, endpoint, registry, transport, or hidden acquisition is permitted. |
| Time and ordering | Warm-up and settling are declared durations. Lesson 070 reads no clock and never measures or independently determines that a source is warmed, settled, fresh, or sampled. Lesson 071 applies supplied time, cadence, rollover, and restart semantics. A descriptor and its revisions remain immutable for one characterization run. |
| Errors and status | Malformed and contradictory descriptors or frames use `Status`; comparator assertion uses `Result<bool>`. There is no physical outcome at this layer. `Unspecified` is a valid declared E0 value, while invalid underlying enum encodings reject. An unspecified declaration never selects a plausible default and blocks E1 admission. |
| Resource budget | E0 declares zero pins, ADC channels or references, timers, interrupts, buses, rails, endpoints, power resources, and registry entries. Storage and validation work are fixed and bounded. Exact ordinary/no-LTO flash, static SRAM, stack, descriptor/object size, aggregate Lessons 070--072 use, and residual Mega SRAM are promotion gates. |
| Deterministic proof | Every enum and optional-field combination, range and duration boundary, revision and identity boundary, collision, canonical unused field, replay, and output-nonmutation case is finite and host reproducible. Tests additionally prove that accepting a descriptor touches no fake endpoint or resource. |
| Packaging and public surface | One standalone component header and mostly out-of-line implementation when needed, umbrella export, strict host target, compile-only Mega replay, exact resource probe, HTML reference, and complementary pencil-drawing PDF. No adapter, wiring table, pin assignment, schematic, or sensor-specific class enters E0. |
| Example and documentation fit | The canonical sketch acquires copied descriptor and frame fixtures, configures and validates them, then publishes named volatile channel, assertion, declared-duration-satisfaction, and status cells. It never tells the learner to upload the replay to an unidentified module. Predict/observe/interpret distinguishes “structurally accepted declaration/frame” from “physically qualified specimen.” |
| Downstream effects | Lesson 071 binds supplied analog/digital observations to one immutable descriptor and owns sweeps, thresholds, hysteresis, chatter, rails, and disagreement. Lesson 072 owns the one-declared-fixture session, presentation, and volatile characterization-record boundary; persistence remains outside this arc. Lessons 037--042 retain their existing acoustic, magnetic, and optical semantics; `ThresholdInput` remains unchanged. |

## Exact declared E0 values

`ModuleThresholdDescriptor` carries exactly:

- nonzero `schemaRevision`, `descriptorId`, `descriptorRevision`,
  `declaredSpecimenReference`, `declaredSpecimenRevision`, and
  `declaredElectricalEvidenceRevision`;
- `ModuleChannelTopology`: `AnalogOnly`, `ComparatorOnly`, or
  `AnalogAndComparator`;
- `ModuleComparatorOutputStage`: `Unspecified`, `PushPull`, `OpenDrain`, or
  `OpenCollector`;
- `ModulePullRequirement`: `Unspecified`, `None`, `PullUp`, or `PullDown`;
- `ModuleDeclaredRail`: `Unspecified`, `Ground`, `LogicSupply`, or
  `ModuleSupply`;
- ordered `ModuleMillivoltRange` values for declared supply and signal;
- an ordered `ModuleRawDomain`;
- `ModuleComparatorPolarity`: `Unspecified`, `ActiveHigh`, or `ActiveLow`;
- `ModuleThresholdControlKind`: `Unspecified`, `Fixed`, or `Potentiometer`;
- `ModuleThresholdDirection`: `Unspecified`, `IncreasingClockwise`, or
  `IncreasingCounterclockwise`; and
- `ModuleDeclaredDuration` values for warm-up and settling, each pairing
  `ModuleDurationDeclaration::Known` or `Unknown` with a `Duration`.

`Unspecified` is a supported E0 declaration, not an invalid enum and not a
guessed default. Any required `Unspecified` field makes the declaration
E1-inadmissible. Invalid underlying enum encodings always reject.

Pull rail is `Unspecified` unless a pull is declared and must be specified
when a pull is declared. Output stage, polarity, threshold-control kind, and
direction use the controlling plan's topology-specific canonical
cross-fields. A descriptor does not choose a resistance, rail implementation,
pin mode, or endpoint. Marketplace aliases, PCB color, connector count,
bundle position, and `KY-` numbers remain documentation metadata rather than
electrical identity.

Known zero warm-up or settling means explicitly that none is required. An
unknown duration uses `ModuleDurationDeclaration::Unknown` with a canonically
zero numeric value and blocks E1 admission. Lesson 070 reads no clock and
never waits internally.

`ModuleThresholdFrame` binds `schemaRevision`, `descriptorId`,
`descriptorRevision`, `declaredSpecimenReference`,
`declaredSpecimenRevision`, and `declaredElectricalEvidenceRevision` to a
`ModuleFrameProvenance` containing nonzero source identity,
source-configuration revision, sequence, and supplied `observedAt`. It then
carries:

- `analogRaw`, `analogStatus`, and `analogProducerStatus`;
- copied comparator level, status, presence, asserted value, and producer
  status; and
- copied `declaredWarmupSatisfied` and `declaredSettlingSatisfied`
  declarations.

`ModuleChannelStatus` is exactly `NotPresent`, `Current`, `Stale`, or
`ProducerFault`. Comparator-only frames use the canonical analog-absent
representation; analog-only frames use the canonical comparator-absent
representation. Presence, statuses, values, producer statuses, derived
assertion, and descriptor topology must agree. Warm-up and settling flags are
copied declarations bound to the frame provenance/configuration; they are not
measured evidence and are not derived from an internal timer.
When the descriptor declares an unknown warm-up or settling duration, the
corresponding copied satisfaction flag must be false.

Rails and raw extrema remain copied endpoint codes. Lesson 070 does not call
them open circuits, shorts, saturation, or faults and defines no generic
quality enum.

## Structural validity is not authority

The validator answers only whether a descriptor is:

1. expressed in a supported schema;
2. composed from valid closed values;
3. internally consistent;
4. canonical in every unused field; and
5. within fixed numeric and timing domains.

It does not answer whether the specimen exists, whether its identity token
matches the connected board, whether a traced circuit is correct, whether a
pull-up reaches the stated rail, whether an output is voltage-safe, or whether
the declared warm-up and control direction were measured.

No codec belongs to Lesson 070. Fieldwise public values remain authoritative.
Lesson 072 owns stable characterization-record serialization and must bind
the complete relevant descriptor facts and revisions. A future digest or
checksum cannot substitute for physical identity, provenance, or the
fieldwise descriptor.

## Validation and failure precedence

The controlling arc precedence applies:

1. invalid encoding or descriptor/frame configuration returns
   `InvalidConfiguration` or `InvalidArgument`;
2. descriptor/frame correlation mismatch returns `InvalidArgument`;
3. attributable channel failure remains a valid copied frame with
   `ModuleChannelStatus::ProducerFault`;
4. stale, rail, and declared warm-up/settling-unsatisfied values remain named
   copied machine values for Lesson 071 rather than an invented Lesson 070
   quality; and
5. a complete coherent declaration/frame returns `Ok`.

Invalid input is not a domain observation. Domain rejection in later lessons
is valid attributable evidence. Descriptor and frame validation are
stateless, do not publish a replacement value, and do not repair input.
`moduleComparatorAsserted()` returns failure when polarity is `Unspecified`
or invalid; it never guesses. The implementation uses no diagnostic strings,
exceptions, hidden repair, defaults, or component-specific status convention.

## Deterministic proof matrix

Host tests must cover:

- every valid and invalid encoding of `ModuleChannelTopology`,
  `ModuleComparatorOutputStage`, `ModulePullRequirement`,
  `ModuleDeclaredRail`, `ModuleComparatorPolarity`,
  `ModuleThresholdControlKind`, `ModuleThresholdDirection`,
  `ModuleDurationDeclaration`, and `ModuleChannelStatus`;
- every topology/output/pull/rail/polarity/control/direction cross-product;
- `Unspecified` accepted structurally at E0 and reported incomplete by
  `moduleDescriptorDeclarationsComplete()`;
- zero, one, and maximum identity/revision values; ordered, equal, and
  inverted declared ranges; exact raw lower/upper rail;
- known-zero, known-nonzero, and unknown-with-canonical-zero durations;
- analog-only, comparator-only, and dual-channel presence/status/value
  canonicality;
- active-high and active-low assertion, plus `Unspecified` assertion failure;
- separate analog and comparator producer faults and their collision
  precedence;
- copied declared warm-up and settling satisfaction bound to exact frame
  provenance/configuration;
- aliases that share behavior but cannot collapse distinct declared specimen
  references or declared electrical-evidence revisions;
- byte-identical copied replay and fieldwise equality without a raw-struct
  persistence claim; and
- proof that accepted and rejected descriptors make zero fake endpoint,
  registry, clock, transport, and power calls.

The compile-only Mega replay uses copied descriptor and frame fixtures only.
Named volatile cells expose topology, polarity, raw value, comparator
assertion, channel status, declared warm-up/settling satisfaction, identity,
declaration-completeness result, and returned status. Compilation and those cells
prove deterministic software behavior, not physical-module conformance.

## Composition pressure: Lesson 072 E0 characterization bench

The maximum authorized E0 composition contains one active copied descriptor,
one fixed-capacity Lesson 071 ascending/descending characterization run,
copied analog and comparator evidence at the maximum supported cadence, one
bounded presentation intent, and one caller-owned volatile record candidate.
It exercises the maximum descriptor/run/result buffers, exact boundary
samples, warm-up and settling transitions, comparator chatter, analog rail,
digital disagreement, reset, and a colliding descriptor-revision change.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | Applicable to Lessons 071--072, not to L070 validation itself. L070 performs one bounded validation without polling, delay, retry, or catch-up. The aggregate fixture proves one-sample bounded work, simultaneous timestamps, duration equality, rollover, and missed-update behavior. |
| Total memory and hardware resources | Applicable. Measure descriptor, maximum run, result, presentation, record, caller buffers, stack peaks, flash, static SRAM, and residual Mega margin. E0 continues to claim zero pins, ADC modes, timers, interrupts, buses, rails, or registry entries. |
| Shared bus or transport | Not applicable at E0 because all inputs are copied values and the contract contains no bus, endpoint, or transport. A future physical adapter reopens arbitration, ownership, voltage, pin, and rollback review. |
| Persistence and recovery | L070 is intentionally volatile and has no codec. Lesson 072 owns one 192-byte caller-owned volatile record codec, including framing, integrity, and semantic validation, but no commit, retry, recovery, capacity, wear, or durability claim. Its image binds the descriptor and evidence revisions rather than inferring them later. Durable storage is outside this arc. |
| Motion, external power, or stored energy | Not applicable at E0 because no actuation or power path exists. The cadence's switched sensor power is E1-only and requires exact low-voltage identity, inactive default, back-power analysis, current budget, and observed physical removal. |
| Observation identity and provenance | Applicable downstream. Every Lesson 071 sample binds the declared specimen reference/revision, descriptor and declared-electrical-evidence revisions, source sequence/time, analog and digital statuses, and common acquisition epoch. Lesson 070 supplies immutable declared meaning but manufactures no observation provenance or physical identity. |
| Diagnostic interference | E0 uses caller-owned result cells or semantic intents only. A future onboard comparator LED cannot prove sampled analog/digital values or the external stimulus. LCD, seven-segment, RGB, and test-point costs belong in the aggregate E1 budget. |
| Failure collision and recovery | Malformed descriptor prevents run start. After start, descriptor/revision change faults the run rather than switching meaning. Sample structure/time/source faults retain precedence over characterization outcomes; rails, stuck output, chatter, and disagreement remain typed evidence. Presentation or persistence failure cannot reclassify the run. |

The aggregate proof covers capacity immediately below, at, and above every
fixed Lesson 071/072 bound, worst-case simultaneous evidence, reset and
restart with faults still present, descriptor revision collision, and
byte-stable replay where the Lesson 072 record contract requires it.

## Initial resource gates

These provisional plan gates control over earlier estimates and may be
tightened, but no hard limit may be raised without another stress review.

| Metric | Target | Hard limit |
|---|---:|---:|
| ordinary Lesson 070 sketch flash | 10 KiB | 14 KiB |
| ordinary static SRAM | 768 B | 1,024 B |
| isolated synchronous stack | 320 B | 448 B |
| Lesson 070 object composition | 192 B | 256 B |
| descriptor value | 64 B | 96 B |
| residual Mega SRAM | 4,096 B | 3,072 B |

The exact probe records ordinary and no-LTO flash/static SRAM, descriptor and
object sizes, caller-owned buffers instantiated once, compiler-callgraph
synchronous stack including retained return-address edges, aggregate Lessons
070--072 lifetime/phase storage, residual Mega SRAM, and the complete
compiler/core/flag fingerprint. Heap use, recursion, indirect calls, unknown
callgraph edges, dynamic stack, or stale reviewed target markers fails the
gate. Hard-limit and residual-hard-floor failures are not reviewable.

## Prior-decision impact

- `preserved`: exact-module admission and PX quarantine; aliases remain
  metadata; authorized listing is not energization; P1/P2 distinctions;
  fixed storage; explicit supplied time; current endpoint ownership; Serial
  is never sole evidence; and pencil drawings are required except for a
  formally identified authoritative schematic
- `preserved`: no gas/heater exposure or concentration claim, created flame,
  laser, physiological/life-safety use, mains, ignition, pyrotechnics, or
  unidentified module
- `preserved`: Lessons 037--042 keep their acoustic, magnetic, and optical
  policy meaning; Lesson 071 owns empirical characterization; Lesson 072 owns
  session records and presentation
- `extended`: the existing canonical taxonomy's conditioned-threshold and
  analog-plus-threshold behaviors gain one explicit copied descriptor
  vocabulary
- `challenged and prohibited`: universal pinout/power configuration, generic
  internal-pull authorization, “sensitivity” direction, alias-as-identity,
  exact electrical claims without acceptance, or mutation of the existing
  `ThresholdInput` contract

## E1 reopen triggers

Any of the following requires exact-specimen review and recorded physical
acceptance:

- choosing a physical module, connector pin order, supply, logic voltage, ADC
  reference, source impedance, output stage, pull rail/resistance, comparator
  IC, or onboard indicator loading;
- claiming a potentiometer orientation, warm-up, settling, polarity, raw
  range, AO/DO relationship, or stimulus meaning;
- owning or switching a rail, ADC channel, digital pin, endpoint, timer,
  interrupt, or registry entry;
- publishing a wiring table, formal schematic, or powered experiment;
- treating an accepted descriptor, seller alias, or similar-looking board as
  proof of specimen identity; or
- adding generic analog-temperature, capacitive-touch, heated gas-response,
  physiological, laser, emitter, or other excluded families.

Physical acceptance requires both-face photographs, markings, exact pinout,
primary schematic/datasheet or verified trace, rail/current/output-high/low
measurement, comparator pull rail, inactive outputs, threshold-control
direction, AO/DO correlation, rail/open/short evidence, named AO/DO/VCC/GND
test points, current-limited first energization, and shutdown plus physical
power-removal observations.

## Terminal gate result

- Disposition: `natural fit` for a pure declared copied E0 descriptor and
  structural validator
- Promotion status: implemented, independently reviewed, and published at E0
- Closed scope: explicit descriptor vocabulary, structural/canonical
  validation, fixed copied values, deterministic nonmutation, exhaustive host
  matrices, compile-only Mega replay, and exact resource evidence
- Explicitly open: empirical characterization, session recording,
  presentation, persistence, endpoint ownership, exact specimens, power, and
  physical acceptance
- Measured evidence: 4,564 B ordinary flash, 684 B ordinary static SRAM;
  4,546 B exact no-LTO probe flash, 690 B static SRAM, 75 B synchronous stack,
  45 B descriptor, 38 B frame, and 83 B descriptor-plus-frame composition.
  The link/source dependency scan contains no endpoint, registry, clock,
  transport, power, or hardware-acquisition dependency.
- Reopen rule: any material departure from the public fields, canonical
  cross-field rules, stateless ownership, resource fingerprint, or
  zero-resource boundary reopens this pass
- Maximum-composition scenario: one-descriptor Lessons 071--072 E0
  characterization bench with maximum fixed run and record buffers
- Promotion permitted: yes for copied E0 software/documentation only; all
  exact-specimen, electrical, powered, and physical-acceptance gates remain
  open
