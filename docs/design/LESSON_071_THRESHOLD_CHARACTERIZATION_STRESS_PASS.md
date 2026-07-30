# Lesson 071 threshold-characterization architecture stress pass

Status: initial E0 architecture review against the controlling
[Lessons 070--072 module-characterization plan](LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md).
The earlier terminology and boundary remediation is satisfied by the
three-leg streaming contract below. Implementation, exact resource evidence,
and publication gates remain open. Powered characterization remains E1-open.

This pass reviews the Lesson 071 subject fixed by the
[extended component/project cadence](../projects/component_project_cadence.md):
a deterministic sweep recorder and classifier comparison over learner-supplied
samples.

`ModuleCharacterizationPolicy` is a natural component-layer boundary only when
it analyzes copied, structurally admitted `ModuleCharacterizationPoint` values. It
owns no ADC, comparator input, pin, module power, delay, clock, stimulus,
potentiometer, storage, display, control, or endpoint.

## Boundary

- Name and lesson/project: `ModuleCharacterizationPolicy`, Lesson 071
- Reviewer and date: initial architecture review, 2026-07-29
- Public responsibility: stream one descriptor-bound ascending leg, one
  descending leg, and one verification leg, then publish bounded transition
  brackets, interval-valued hysteresis, witnesses, and conservative
  analog/comparator relation evidence
- Direct dependencies: `Status`, `TimePoint`, fixed-width values,
  `ModuleThresholdDescriptor`, `ModuleThresholdFrame`, and their stateless
  Lesson 070 validation
- Existing decisions reconsidered: explicit supplied time and provenance,
  fixed storage and bounded work, structural failures versus domain findings,
  sampled thresholds and hysteresis vocabulary, E0 copied evidence versus E1
  electrical claims, and the Lesson 072 immutable-envelope boundary

The controlling plan closes the earlier cadence ambiguity:

1. a sampled transition is a `ModuleTransitionBracket`, never an exact
   comparator voltage or ADC code;
2. a finite copied sweep reports `NoObservedTransitionActive` or
   `NoObservedTransitionInactive`, not proof of a physically stuck output;
3. hysteresis is represented by `ModuleAnalogInterval` values, never a
   midpoint or negative scalar;
4. analog/comparator disagreement is evaluated only in conservative
   guaranteed intervals during a separate verification leg; and
5. a healthy raw-rail point is ordinary accepted evidence;
   `AtLowerRail` or `AtUpperRail` is derived only when finalizing an
   endpoint-only no-transition leg, and never means generic open/short
   diagnosis without exact-fixture electrical evidence.

These corrections are now normative in the controlling plan and satisfy the
earlier bounded local remediation without changing an existing public API.

## Frozen public values

All public names are module-prefixed:

- `ModuleCharacterizationLeg`: `Ascending`, `Descending`, `Verification`;
- `ModuleSweepDirection`: `Increasing`, `Decreasing`, `Unordered`;
- `ModuleCharacterizationState`: `Idle`, `Collecting`, `Complete`,
  `Rejected`, `Shutdown`;
- `ModuleCharacterizationReason`: `None`, `WarmupUnsatisfied`,
  `SettlingUnsatisfied`, `ProducerFault`, `Stale`,
  `SequenceDiscontinuity`, `TimestampDiscontinuity`,
  `DirectionViolation`, `Chatter`, `NoObservedTransitionActive`,
  `NoObservedTransitionInactive`, `AtLowerRail`, `AtUpperRail`,
  `TransitionOrientationMismatch`, `AnalogComparatorDisagreement`;
- `ModuleCompactWitness`, which retains only presence, control ordinal, raw
  code, canonical comparator assertion, sequence, and observation time;
- `ModuleComparatorRelation`: `Unverified`, `Consistent`, `Ambiguous`,
  `Disagrees`;
- `ModuleCharacterizationPoint`, which retains nonzero session/run/leg IDs,
  control ordinal, leg and direction, source/configuration identity, complete
  `ModuleThresholdFrame`;
- `ModuleTransitionBracket`, which retains a presence flag and the complete
  adjacent `before` and `after` points;
- `ModuleAnalogInterval`, which retains a presence flag and inclusive lower
  and upper raw codes;
- `ModuleCharacterizationConfig`, which copies characterization revision,
  complete Lesson 070 descriptor, required points per leg, maximum age, and
  maximum gap; and
- `ModuleCharacterizationEvidence`, which copies lifecycle/session/run/leg
  correlation, characterization revision, complete descriptor,
  source/configuration identity, state/reason/terminal leg, all three counts,
  both full brackets, guaranteed inactive,
  guaranteed active, and ambiguity intervals, comparator relation, full
  bracket points, four compact first/last/offending witnesses, and `Status`.

`requiredPointsPerLeg` is configurable only in the inclusive range `[2, 16]`.
Every leg completes at exactly that configured count. Sixteen is the hard
maximum for each leg and 48 is therefore the maximum accepted point count only
when the configured requirement is 16.

The policy uses streaming aggregates. It retains no point array. It keeps only
counts, correlation anchors, adjacent transition points, first/last witnesses,
the compact first offending witness pair, frozen learned brackets, intervals,
and relation state. The only full points retained are the four points owned by
the two transition brackets. The first and last compact witnesses are the
single canonical owners of first/last sequence and observation time; those
values are not duplicated at evidence top level. Caller-owned fixture arrays may be replayed one
point at a time without becoming component storage.

## Lifecycle and correlation

The frozen operations are:

```text
initialize(now)
beginSession(now, sessionId, runId)
beginLeg(now, legId, Ascending, Increasing)
observe(now, point)...
finalizeLeg(now)
beginLeg(now, legId, Descending, Decreasing)
observe(now, point)...
finalizeLeg(now)
beginLeg(now, legId, Verification, Unordered)
observe(now, point)...
finalizeLeg(now)
evidence(output)
reset(now) / shutdown(now)
```

Construction is inert. Configuration is copied. The policy is noncopyable and
nonmovable, allocates no heap memory, invokes no callback, reads no clock, and
performs no I/O.

Configuration accepts only a complete Lesson 070 descriptor with
`ModuleChannelTopology::AnalogAndComparator` and explicit
`ModuleComparatorPolarity::ActiveHigh` or
`ModuleComparatorPolarity::ActiveLow`. Analog-only, comparator-only, and
unspecified-polarity descriptors fail configuration validation. `maximumAge`
and `maximumGap` must each be nonzero and strictly below the modular half
range. Lesson 070's known-zero readiness declaration does not extend to these
Lesson 071 freshness and continuity bounds. Every
accepted point requires both channels present, both channel statuses
`ModuleChannelStatus::Current`, both producer statuses OK, and canonical
`comparatorAsserted`.

The complete `ModuleThresholdFrame` is the sole warm-up and settling authority.
Its `declaredWarmupSatisfied` and `declaredSettlingSatisfied` fields are bound
to frame provenance and configuration by Lesson 070 validation. Lesson 071
does not add, accept, or reconcile duplicate readiness booleans in
`ModuleCharacterizationPoint`.

Initialize and reset increment a nonzero lifecycle generation and reject with
`CapacityExceeded` at `UINT32_MAX` without mutation or wrap. Session and run
IDs are nonzero and forward modular. The three leg IDs are nonzero, distinct,
forward modular, and occur only in ascending, descending, verification order.
Ascending accepts only `Increasing`; descending only `Decreasing`;
verification only `Unordered`.

Every successful lifecycle operation retains its supplied `now`. A subsequent
`beginSession`, `beginLeg`, `finalizeLeg`, `reset`, or `shutdown` requires
forward-or-equal modular time relative to that retained value; backward and
exact-half-range ambiguous values return `InvalidArgument` atomically.
`initialize()` after `shutdown()` returns `NotInitialized`; only construction of
a new policy object begins a new object lifecycle.

Every point must match lifecycle, session, run, active leg, direction,
descriptor schema/identity/revision, declared specimen reference/revision,
electrical-evidence revision, source/configuration identity, and expected
control ordinal. Control ordinals are exactly contiguous `1..N` independently
for each leg, where `N == requiredPointsPerLeg`. Source sequences are strictly
contiguous forward across accepted points under the plan's modular rule.
Correlation mismatch is an API-level `InvalidArgument` and
leaves evidence unchanged; it is not a characterization observation.

Equal sequence/time/ordinal is idempotent only for fieldwise-identical complete
evidence and consumes neither an ordinal nor a point. A changed duplicate,
regression, sequence gap, ordinal gap, future or backward time, half-range
ambiguity, or identity drift follows the controlling precedence without
partial mutation. `finalizeLeg()` before exactly N accepted points rejects
atomically. Any point beyond N, including a seventeenth when N is 16, returns
API `CapacityExceeded` atomically; capacity is not a
`ModuleCharacterizationReason` and does not terminalize domain evidence.

A completed or rejected session is terminal. Evidence is immutable after the
verification leg completes or a valid domain rejection terminalizes the run.
Reset begins a new lifecycle generation; old session, run, leg, and point
authority cannot enter it. Shutdown publishes `Shutdown` and prohibits work
until a new object lifecycle.

## Learning-leg semantics

“Increasing” and “Decreasing” describe only raw analog-code order. They do not
mean increasing light, heat, sound, magnetic field, distance, hazard, or any
other physical quantity. Lesson 070 descriptor declarations may document a
semantic or threshold-control direction, but Lesson 071 does not invent
physical units.

Each learning leg admits exactly one clean comparator transition:

- on the first canonical state change, the immediately adjacent accepted
  points become the complete `ModuleTransitionBracket`;
- a second reversal is `Chatter`;
- a comparator toggle at equal raw code is `Chatter`;
- equal raw codes without a toggle do not extend a learned interval;
- an increasing/decreasing violation is `DirectionViolation`; and
- no transition yields `NoObservedTransitionActive` or
  `NoObservedTransitionInactive` according to the one canonical comparator
  state observed throughout the leg.

The policy does not average, interpolate, sort, reverse, or discard points.
The crossing is known only to lie in the inclusive bracket. Healthy points at
either declared raw endpoint are accepted normally and may participate in a
transition bracket. Only `finalizeLeg()` derives `AtLowerRail` or
`AtUpperRail`, and only for an endpoint-only leg that observed no transition.
That result does not invent a threshold.

Every learning leg covers the complete declared raw domain. Ascending begins
at the exact raw lower bound and ends at the exact raw upper bound; descending
begins at the upper bound and ends at the lower bound. Finalization records
`DirectionViolation` when either endpoint is missing, except when every
accepted point is the same exact declared endpoint. That endpoint-only special
case records `AtLowerRail` or `AtUpperRail` before the coverage check,
terminalizes the run, and cannot produce intervals or advance to another leg.
Consequently the outer guaranteed intervals below are bounded by observed
sweep coverage rather than extrapolated beyond it.

The two learned brackets freeze three conservative
`ModuleAnalogInterval` values:

- `guaranteedInactiveInterval`;
- `guaranteedActiveInterval`; and
- `ambiguityInterval`.

The intervals express what both sampled transition brackets prove. Brackets
may be disjoint, touching, overlapping, or identical. An overlap is not
automatically a fault; it may mean only that sampling is too coarse to resolve
hysteresis. No exact threshold scalar, bracket midpoint, or signed hysteresis
scalar exists in the public contract.

The bracket orientations must be complementary: ascending-before equals
descending-after, ascending-after equals descending-before, and those two
states differ. Otherwise descending finalization records
`TransitionOrientationMismatch`.

Let `lowState` be the shared ascending-before/descending-after assertion state,
`lowProved` the lesser raw code of those two points, and `highProved` the
greater raw code of ascending-after and descending-before. The closed interval
from the declared raw lower bound through `lowProved` is guaranteed
`lowState`; the closed interval from `highProved` through the declared upper
bound is guaranteed the opposite state. The closed middle interval
`[lowProved + 1, highProved - 1]` is ambiguity when its lower endpoint does
not exceed its upper endpoint, and is absent otherwise. Widen before endpoint
arithmetic. Assign the two outer intervals to
`guaranteedInactiveInterval` and `guaranteedActiveInterval` according to
`lowState`.

## Verification-leg semantics

Verification starts only after both learning brackets and their intervals are
frozen. It streams up to 16 `Unordered` points and never remaps either bracket
from verification data.

Each verification point is classified against only the guaranteed intervals:

- a matching comparator state inside a guaranteed region contributes
  `Consistent`;
- a contradictory state inside a guaranteed region produces
  `AnalogComparatorDisagreement`, relation `Disagrees`, and retains the
  offending evidence;
- a point in the ambiguity interval contributes `Ambiguous`; and
- an ambiguity point can never become disagreement merely because it is close
  to one sampled transition.

This separate third leg prevents circular reasoning. The policy does not learn
a classifier from a point and immediately use that same point as independent
proof of agreement. If all verification points lie in ambiguity, relation is
`Ambiguous`, not `Consistent`.

## Rails and electrical diagnosis

A healthy frame exactly at the Lesson 070 descriptor's lower or upper raw
endpoint is ordinary accepted evidence, not a terminal observation or
electrical fault. It contributes to count, order, transition, and witness
evidence like any other healthy point. A rail with producer failure remains
`ProducerFault`.

Lesson 071 does not diagnose open circuit, short circuit, reversed supply,
missing pull, wrong ADC reference, damaged module, or incorrect potentiometer
from a raw endpoint. Those names require exact E1 fixture topology, voltage,
pull, test-point, stimulus, and bench evidence. Without that evidence the
machine result remains the ordinary accepted point or, when `finalizeLeg()`
finds an endpoint-only no-transition leg, `AtLowerRail` or `AtUpperRail`.

## Compact evidence ABI

`ModuleCharacterizationEvidence` owns the complete descriptor and correlation
fields but avoids repeated full points. Its exact retained shape includes:

- two `ModuleTransitionBracket` values containing exactly four full
  `ModuleCharacterizationPoint` values in total;
- one `ModuleCompactWitness` each for first, last, offending-before, and
  offending-after evidence;
- first/last sequence and observation time within their respective compact
  witnesses, plus all three leg counts;
- all three `ModuleAnalogInterval` values and one
  `ModuleComparatorRelation`; and
- terminal state, reason, leg, and `Status`.

Compact witnesses are diagnostic correlation evidence, not substitutes for
the authoritative full bracket points. No hidden full first, last, or
offending point and no retained leg array may enlarge the ABI. Measured AVR
shapes are 57 B for the caller-local point, 375 B for evidence, and 498 B for
the policy.

## Shared precedence and atomicity

Lesson 071 adopts the controlling arc precedence:

1. invalid configuration or encoding returns `InvalidConfiguration` or
   `InvalidArgument`, preserving previous evidence;
2. lifecycle, session, run, leg, ordinal, or correlation mismatch returns
   `InvalidArgument`, preserving previous evidence;
3. attributable channel/producer failure returns API `Ok` with terminal
   `ProducerFault`;
4. attributable time or sequence discontinuity returns API `Ok` with the
   named terminal reason;
5. attributable unsettled/unwarmed evidence returns API `Ok` with its named
   machine observation, while a healthy raw-rail point remains accepted;
6. transition, chatter, interval, or relation outcome returns API `Ok`; and
7. a complete verification leg publishes complete evidence.

Invalid input is not a domain observation. A valid domain rejection is
attributable evidence. All mutation uses staged candidates; a failed API call
leaves object state, caller output, and any caller image unchanged.

Analog producer fault dominates comparator interpretation. Comparator producer
fault dominates chatter and relation analysis. Unwarmed or unsettled evidence
cannot establish a transition. Time/sequence discontinuity dominates
direction or comparator meaning in the same point. Direction violation
dominates an apparent crossing. Chatter prevents a clean learned bracket or
verification relation from being manufactured by selecting convenient
points.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | Natural as a streaming copied-evidence policy above Lesson 070. Lesson 071 owns neither acquisition nor Lesson 072 envelope/record/presentation behavior. |
| Ownership and lifecycle | Natural with copied configuration, fixed streaming witnesses, no retained point array or caller pointer, explicit generation, and three ordered legs. |
| Time and ordering | Natural with supplied `now`, point observation time, exact session/run/leg/ordinal correlation, modular ordering, and no hidden delay, sampling cadence, retry, sorting, or clock. |
| Errors and status | Natural with the shared structural/correlation API failures and attributable terminal domain reasons. Healthy raw endpoints remain accepted; endpoint-only no-transition finalization, producer failure, chatter, ambiguity, and disagreement remain distinct. |
| Resource budget | Natural if implementation retains only the frozen compact evidence shape. Exact target/hard gates are 12/16 KiB flash, 1,024/1,536 B static SRAM, 448/640 B stack, 512/768 B policy, 320/384 B evidence, 96/128 B phase-local point, and 4,096/3,072 B residual SRAM. Provisional AVR estimates are 736 B policy, 376 B evidence, and 72 B point: target misses for policy/evidence but within hard limits, pending measurement and review. |
| Deterministic proof | Natural because every result derives from complete descriptor configuration, copied points, explicit ordering, and supplied time. All transition, bracket, interval, relation, collision, and capacity cases are finite. |
| Packaging and public surface | One `module_characterization.h/.cpp` boundary, host test, ordinary Mega replay, umbrella/archive/native inventories, exact resource probe, HTML/API lesson, and pencil-drawing PDF. No endpoint or storage exception is introduced. |
| Example and documentation fit | `Lesson071Characterization` streams ascending, descending, then verification fixture points and exposes interval/witness evidence. It invokes no ADC, digital input, delay, module power, display, or storage API. Typed evidence is the non-Serial observation path. |
| Downstream effects | Lesson 072 consumes one immutable `ModuleCharacterizationEnvelope`; it owns no Lesson 071 child and does not replay characterization. Earlier sensor-specific policies retain their calibrated semantics. |

## Composition pressure scenario

The maximum authorized E0 composition uses one complete Lesson 070 descriptor,
16 accepted ascending points, 16 accepted descending points, 16 accepted
verification points, both raw endpoints, coarse overlapping brackets, an
equal-code comparator toggle, age/gap/order boundaries, four full bracket
points, four compact witnesses, and one Lesson 072 immutable envelope and
192-byte volatile record image.
Presentation intent simultaneously faults.

The correct outcome remains attributable and bounded. It publishes no exact
threshold, fabricated hysteresis scalar, or electrical open/short diagnosis.
Lesson 072 presentation or record preparation cannot rewrite Lesson 071
evidence.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | Applicable. One `observe()` and each `finalizeLeg()` perform bounded O(1) work. No point-array scan, delay, retry, catch-up, sorting, or hidden sampling loop exists. |
| Total memory and hardware resources | Applicable. Promotion measures the exact target/hard tuple: flash 12/16 KiB, static SRAM 1,024/1,536 B, stack 448/640 B, policy 512/768 B, evidence 320/384 B, phase-local point 96/128 B, and residual SRAM 4,096/3,072 B. Residual uses `8192 - static - stack - 128`. E0 hardware ownership is zero. |
| Shared bus or transport | Not applicable at E0 because all frames and points are copied and no method invokes an endpoint. Future acquisition, display, and controls require separate E1 owners and acceptance. |
| Persistence and recovery | Not applicable. Lesson 071 state is intentionally volatile. Lesson 072 prepares one caller-owned volatile image and explicitly makes no durability claim. |
| Motion, external power, or stored energy | Not applicable at E0 because no actuation or power-control path exists. Exact E1 specimen acquisition owns inactive-default supply and harmless family-specific stimulus. |
| Observation identity and provenance | Applicable and central. Every accepted point binds descriptor/specimen/electrical revisions, source/configuration, session/run/leg/ordinal, sequence/time, warm-up/settling, channel statuses, and producer statuses. |
| Diagnostic interference | Applicable in Lesson 072 composition. Result cells, optional Serial, future display/indicator intent, and volatile record preparation cannot affect brackets, intervals, witnesses, or relation. |
| Failure collision and recovery | Applicable. Shared precedence selects structural/correlation failure, producer failure, ordering failure, unwarmed/unsettled evidence, then transition/relation outcome. Healthy rails remain accepted; endpoint-only rail classification occurs only at finalization. Reset changes generation and invalidates all pre-reset authority. |

## Deterministic proof matrix

| Family | Required cases |
|---|---|
| Lifecycle | construction, initialize success/failure, every call before initialize, duplicate initialize, zero/duplicate/regressing session/run/leg IDs, wrong leg order/direction, finalize incomplete leg, reset/shutdown at every phase, generation exhaustion |
| Descriptor/correlation | every descriptor and frame encoding, each descriptor/specimen/electrical/source/configuration mismatch, wrong session/run/leg/ordinal, changed and identical duplicate |
| Capacity | `requiredPointsPerLeg` 1/2/15/16/17; finalize at N-1 rejects atomically; exactly N completes; N+1 returns API `CapacityExceeded` without terminal domain mutation; attempted seventeenth at N=16; no retained point array |
| Learning direction | increasing/decreasing exact steps, equal raw code, violation by one, lower/upper endpoints, active-high and active-low symmetry |
| Transitions | clean transition at every adjacent location, equal-code toggle, second and third reversal, no-transition active/inactive, endpoint-only run |
| Brackets/intervals | every ascending/descending bracket ordering, disjoint/touching/overlapping/identical brackets, every guaranteed and ambiguity interval boundary |
| Verification | consistent, ambiguous, and disagreeing points; all-ambiguous leg; contradiction at each guaranteed boundary; verification cannot alter learned brackets |
| Sequence/time | identical/changed duplicate, gap, regression, natural wrap, exact-half ambiguity, future/backward time, exact maximum age/gap and one tick each side |
| Producer/qualification | `AnalogAndComparator`-only configuration; explicit active-high/low polarity; both channels `Current` and producer-OK; canonical assertion; separate analog/comparator producer failure, stale, frame-owned declared warm-up/settling readiness, healthy endpoint acceptance, endpoint-only no-transition finalization, and every precedence pair |
| Atomicity/evidence | API rejection leaves policy/output unchanged; full descriptor/correlation/count/time/interval/relation fields; exactly four full bracket points and four compact witnesses; terminal immutability; reset invalidation |
| Composition | exact immutable evidence enters Lesson 072 envelope; wrong descriptor/session/run/source/digest rejects there; presentation and volatile record failures cannot change Lesson 071 |
| Tooling | strict C++11 warnings, ASan/UBSan, traits, native/archive inventories, Mega compile, exact resource fingerprints, object-layout proof, lesson/PDF/site gates |

## Prior-decision impact

- Endpoint-owned electrical lifetime: **preserved**.
- Explicit supplied time, modular ordering, and deterministic replay:
  **preserved**.
- Fixed storage, no heap, exceptions, RTTI, sorting, or unbounded work:
  **preserved**.
- Lesson 070 descriptor/frame authority: **preserved** and consumed without
  reinterpretation.
- Earlier calibrated light, sound, magnetic, thermal, radiant, touch, and
  optical semantics: **preserved**. Lesson 071 is not their replacement.
- Exact transition scalar prohibition: **preserved and satisfied** by full
  brackets.
- Conservative hysteresis/disagreement requirement: **preserved and
  satisfied** by frozen intervals plus an independent verification leg.
- Unknown modules remain unpowered: **preserved**.
- E0 host evidence is not physical verification: **preserved**.
- No durability claim: **preserved**; Lesson 071 owns no record image.
- Pencil drawings for non-schematic PDF visuals and formal-schematic status
  only for an electrically authoritative qualified circuit: **preserved**.
- No flame, combustion, gas, medical, physiological, body-contact,
  capacitive-touch, immersion, mains, high-voltage, or unattended-stimulus
  claims: **preserved**.

## Rejected alternatives

1. **Retain any point array.** Rejected because streaming witnesses and
   intervals are sufficient and the controlling resource contract forbids it.
2. **Use only ascending and descending legs.** Rejected because comparing a
   point against a classifier learned from that point is circular.
3. **Call ADC or comparator endpoints from Lesson 071.** Rejected because
   electrical acquisition and lifetime belong to E1.
4. **Average transition samples into an exact threshold.** Rejected because
   the crossing is proved only inside the bracket.
5. **Publish a signed scalar hysteresis value.** Rejected because coarse or
   overlapping brackets require interval-valued ambiguity.
6. **Call no-transition evidence a proved stuck pin.** Rejected because a
   finite sweep cannot distinguish stimulus, adjustment, range, wiring, and
   device causes.
7. **Diagnose open or short from a raw endpoint.** Rejected absent explicit
   exact-fixture electrical evidence.
8. **Treat ambiguity as disagreement.** Rejected because the learned brackets
   do not prove comparator state inside that interval.
9. **Sort, reverse, discard, or retry inconvenient points.** Rejected because
   doing so erases ordering, chatter, and copied evidence.
10. **Treat one descriptor/run as physical module qualification.** Rejected
    because descriptor declarations and E0 fixtures are not power or specimen
    authority.

## Gate result

- Disposition: `natural fit after satisfied bounded local remediation`
- Exact no-LTO evidence: 11,562 B flash, 1,160 B static SRAM, 339 B
  synchronous stack, 498 B policy, 375 B evidence, 57 B caller point, and
  6,565 B residual SRAM at fingerprint
  `e1cfc001cc95937e25337eb53a7a4c8b3602d6a046b23b0a8b94fe7a342d562b`.
- Reviewed target misses: static SRAM is 136 B above its 1,024 B target and
  376 B below its 1,536 B hard limit; evidence is 55 B above its 320 B target
  and 9 B below its 384 B hard limit. Both are independently accepted only
  for that exact fingerprint. Any ABI growth or fingerprint change requires
  fresh measurement and review.
- Open risks: Lesson 072 envelope/codec integration and all exact E1
  specimen/stimulus evidence
- Required discussion or decision IDs: none remaining for the E0 boundary;
  the controlling Lessons 070--072 plan records the consequential decisions
- Completed remediation: redundant top-level first/last sequence and time
  fields were removed; canonical values remain in the first/last witnesses.
  Full evidence candidates moved off synchronous stack, reducing the measured
  path from 1,459 B to 339 B without weakening atomicity or provenance.
- Verification required at promotion: strict C++11 host tests, ASan/UBSan,
  style and archive inventories, Mega compile, exact fingerprint-bound
  resource gates, aggregate Lesson 072 composition, lesson build, monochrome
  and pencil-policy checks, strict site build, and independent architecture
  review
- Maximum-composition scenario and proof: all three 16-point legs, overlapping
  brackets, equal-code chatter, verification ambiguity/disagreement, exact
  ordering boundaries, and Lesson 072 presentation/record collision must
  replay without retained arrays, circular classification, invented
  precision, or partial mutation
- Promotion permitted: yes for copied E0 implementation after the listed
  gates pass; no for powered characterization, wiring, electrical diagnosis,
  persistence/durability, or bench claims
