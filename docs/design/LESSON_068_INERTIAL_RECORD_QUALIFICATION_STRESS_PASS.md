# Lesson 068 inertial-record qualification architecture stress pass

Status: published and host verified at E0. The axis-mapping remediation,
one-source record qualifier, deterministic replay, measured Mega example,
HTML, and pencil-drawing PDF pass their non-hardware gates. Powered specimens
and physical qualification remain E1-open.

This pass reviews the Lesson 068 subject fixed by the
[extended component/project cadence](../projects/component_project_cadence.md):
qualifying one explicitly configured inertial-record source from copied
stationary evidence. The cadence's source-selection language means
configuration, never voting, probing, fallback, or automatic failover.

The component is a natural E0 policy only when it consumes complete copied
Lesson 067 records, applies one explicit proper-rotation mapping, evaluates one
bounded stationary window, and publishes an attributable terminal
qualification result. It owns no sensor, adapter, register map, I2C bus,
address probe, interrupt, clock, calibration store, display, or recorder.

## Boundary

- Name and lesson/project: inertial-record source qualification, Lesson 068
- Reviewer and date: initial architecture review, 2026-07-29
- Public responsibility: determine whether one configured source and mapping
  produce a bounded stationary qualification window, then publish the complete
  evidence and reason
- Direct dependencies: `Status`, explicit time values, fixed-width values,
  the complete copied Lesson 067 normalized-record envelope, and a
  foundational signed-axis mapping value
- Existing decisions reconsidered: Lesson 043 source provenance, Lesson 044
  board-frame mapping, copied-evidence layering, explicit supplied time,
  fixed-capacity windows, reset-safe attempts, and one-source-per-session
  composition for Lesson 069

Lesson 068 qualifies the configured interpretation of one copied source. The
only positive E0 domain is `SyntheticFixture`/`Synthetic`; `initialize()`
returns `Unsupported` for an otherwise well-formed physical source
configuration without changing lifecycle state. It
does not certify sensor accuracy, prove a device marking, discover which
module is attached, estimate arbitrary orientation, or calibrate a physical
unit. E0 supports synthetic record fixtures. `Mpu6050Adapter` and
`Qmi8658Adapter` records are negative unsupported seams until an independent
E1 adapter and exact specimen have passed their own primary-source and bench
gates.

## Required mapping remediation

The existing Lesson 044 `BoardFrame` is presentation vocabulary:
`right`, `forward`, and `up`. Lesson 068 needs the lower-level operation of
mapping source X/Y/Z into a qualification frame for both acceleration and
angular rate. Depending on `OrientationPresentationPolicy` would invert the
layering, while privately reimplementing its determinant rule would create two
authorities for the same safety-relevant transform.

The implementation factors the existing six-value `SignedAxis` and
proper-rotation validation/application rules into
`signed_axis_mapping.h`. The published Lesson 044 spellings and behavior are
preserved: its header includes the foundational declaration and its
implementation delegates to `validSignedAxisMapping()` and
`mapSignedAxes()`. Lesson 068 supplies the semantic `SourceAxisMapping` alias
and uses the same helpers. This remediation does not change angle math,
`BoardFrame` meaning, orientation thresholds, or any Lesson 043--045 public
result.

A valid mapping uses each source axis exactly once and has determinant `+1`.
Exactly 24 of the 216 possible signed-axis triples are valid. Duplicate axes,
opposite signs of one reused axis, and all 24 left-handed reflections reject
as invalid configuration. Mapping applies identically to acceleration and
angular-rate vectors. Negating `INT32_MIN` is not representable and rejects
the candidate atomically; implementation must not invoke signed overflow or
saturate it into apparently valid evidence.

## Frozen E0 values and lifecycle

The complete Lessons 067--069 plan may refine spelling, but it must retain:

- one value-owned configuration containing the exact source identity and
  source/configuration/calibration/range revisions, accepted Lesson 067
  schema and normalization revisions, `SourceAxisMapping`, qualification
  contract revision, window size, maximum sample age, maximum inter-sample
  gap, expected stationary acceleration in micro-g, per-axis acceleration
  deviation limits,
  and per-axis angular-rate limits;
- one nonzero qualification-attempt identity and one nonzero lifecycle
  generation, both retained in evidence;
- a fixed aggregate window, with configured count in the inclusive range
  `2..32`;
- states `Idle`, `Collecting`, `Qualified`, and `Rejected`, plus exact terminal
  reasons `None`, `ConfigurationMismatch`, `ProducerFault`, `NotReady`,
  `Saturated`, `Stale`, `SequenceDiscontinuity`,
  `TimestampDiscontinuity`, `AccelerationOutsideWindow`,
  `AngularRateOutsideWindow`, and `ArithmeticOverflow`;
- complete result evidence: mapping, attempt/generation, qualification
  revision, accepted count, first and last record sequence/time, maximum
  observed age and gap, mapped per-axis acceleration and angular-rate
  minima/maxima, signed 64-bit per-axis sums and reported means, the first
  failing record's provenance and producer status, terminal reason, and
  operation status; and
- an inert construction followed by `initialize()`, explicit
  `begin(now, attemptId)`, one-record-at-a-time `observe(now, record)`,
  `reset()`, `shutdown()`, `evidence()`, and lifecycle queries.

Configuration is copied. The policy is noncopyable and nonmovable, allocates
no heap memory, invokes no callback, reads no clock, and performs no
acquisition. A qualification attempt is terminal after either qualification
or rejection. Further records are rejected without mutation until an explicit
new attempt. Reset clears the window, terminal evidence, history, and attempt
authority while retaining validated configuration; shutdown leaves an
inactive, unqualified safe state. `initialize()` advances lifecycle generation
only after configuration and synthetic-source validation succeed. `reset()`
advances it and atomically clears attempt aggregates. Either operation returns
`CapacityExceeded` without mutation at `UINT32_MAX`; generation never wraps to
zero.

There is exactly one configured source per policy and attempt. Source identity,
model, ranges, configuration revision, calibration revision, record schema,
normalization contract, and mapping are part of that domain. A source change
requires a reset and new explicitly configured policy; it cannot be inferred
from an incoming record. Lesson 069 may compare separately completed sessions
on the host, but it must not ask this policy to select, vote, or fail over.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural after completed mapping remediation.** The policy consumes complete copied Lesson 067 records and publishes qualification evidence. Source adapters and register interpretation remain below Lesson 067; orientation and presentation remain in Lesson 044; recording and session comparison remain in Lesson 069. |
| Ownership and lifecycle | **Natural with one copied configuration and bounded accumulators.** No borrowed source, record buffer, endpoint, or storage lifetime exists. Attempt and lifecycle generations prevent a pre-reset or foreign record stream from completing a later qualification. |
| Time and ordering | **Natural with supplied `now`, record observation time, sequence, and bounded gaps.** Future, stale, regressing, ambiguous, duplicate-changed, and excessive-gap evidence are explicit. No hidden polling cadence or catch-up loop exists. |
| Errors and status | **Natural if structural rejection and valid unhealthy evidence remain separate.** Malformed enums/configuration reject without mutation. A well-formed producer fault, not-ready record, saturation, source mismatch, or stationary-bound failure terminalizes the attempt with its own reason and retained evidence. |
| Resource budget | **Natural with running extrema and widened sums rather than 32 retained records.** Promotion measures exact object/evidence sizes, ordinary and no-LTO Mega replay, synchronous stack, aggregate Lesson 069 composition, and residual SRAM. E0 owns zero pins, timers, interrupts, buses, ADC channels, storage, or power resources. |
| Deterministic proof | **Natural.** All 216 mappings, both vector applications, every exact bound, window size, ordering class, producer-state collision, reset point, rollover, and arithmetic extreme have finite host fixtures. |
| Packaging and public surface | One standalone header/implementation, shared foundational mapping helper, umbrella export, host/archive inventory, strict tests, compile-only Mega replay, exact resource probe, HTML reference, and pencil-drawing PDF. No adapter library, wire protocol, or lesson-only framework is introduced. |
| Example and documentation fit | The Mega sketch begins one synthetic attempt, feeds copied normalized records in acquire/configure/start then observe/decide form, and exposes terminal qualification/reason/result cells. These volatile cells are the E0 non-Serial observation path; they do not prove a physical stationary sensor. |
| Downstream effects | Lesson 069 consumes one immutable terminal qualification envelope bound to the exact normalized record domain. Lesson 044 retains orientation ownership. Future MPU6050 and QMI8658 adapters remain independent, and one passing source cannot qualify another model, range, revision, or mapping. |

## Admission and failure precedence

Each update first validates the call and complete copied record before it
updates a count, sum, extrema, or retained result. The required precedence is:

1. lifecycle and attempt authority;
2. complete enum, status, canonical-record, schema, and normalization
   structure;
3. exact configured source, model, source ID, configuration/calibration
   revision, and declared ranges;
4. observation chronology relative to supplied `now`, then maximum age;
5. sequence ordering and same-sequence fieldwise identity;
6. inter-sample time and sequence continuity;
7. producer/transport status;
8. data-ready state;
9. saturation;
10. mapping representability;
11. acceleration per-axis stationary bounds;
12. angular-rate per-axis stationary bounds; and
13. bounded accumulation and window completion.

Invalid lifecycle or malformed structure returns `Status` failure and leaves
the policy byte-for-byte unchanged. Once a complete record is structurally
admissible, a source mismatch or valid unhealthy record is terminal domain
evidence: it is retained with its exact provenance and reason rather than
being silently skipped until enough convenient samples arrive. The returned
operation status may remain `Ok` for such a semantic rejection. No average,
majority, later good sample, or alternate source can erase that outcome.

A non-OK producer status dominates not-ready and saturation because its
payload is not trusted. For an OK producer, not-ready dominates saturation
and numeric bounds because it is not a new admissible sample. Saturation
dominates stationary arithmetic. Acceleration failure precedes angular-rate
failure only to choose the scalar returned reason; the evidence retains all
per-axis comparisons available from that same structurally valid record.

Records must advance by exactly one within the repository's modular
half-range rules. `UINT32_MAX` to `0` is the valid one-step sequence rollover.
Equal sequence
accepts only a fieldwise identical full record and is idempotent: it cannot
increase the window, extend an attempt, or change extrema. Changed content at
the same sequence, a gap greater than one, regression, or exact-half-range
ambiguity terminalizes as `SequenceDiscontinuity`. Zero is an ordinary
sequence value after natural wrap, not a sentinel. A zero observation gap,
backward observation time, exact-half-range time ambiguity, or gap above the
configured maximum terminalizes as `TimestampDiscontinuity`. A future or
half-range-ambiguous first record is also `TimestampDiscontinuity`. An
inter-sample gap at the configured maximum is accepted; one tick larger
fails. Age at the maximum is accepted; one tick older is `Stale`.

## Stationary qualification semantics

Mapping occurs before stationary comparison. For destination axis `i`, each
mapped acceleration is retained and compared in micro-g, without milli-g
conversion or a vector-magnitude shortcut, and must lie in the inclusive
configured interval centered on that axis's expected stationary acceleration
component. Each mapped angular-rate
absolute value must be at or below that axis's inclusive limit. Arithmetic
widens before subtraction, absolute value, sum, or comparison. No calculation
may negate `INT32_MIN`, overflow a 32-bit accumulator, or narrow before a
checked result is known.

Every individual record must satisfy every configured bound. A window whose
mean appears stationary cannot hide one excursion. At exactly `N-1` accepted
records the result remains `Collecting`; the `N`th accepted record publishes
one terminal `Qualified` result. There is no sliding window, automatic retry,
warm-up discard, adaptive threshold, or replacement of a failed sample.

The terminal evidence reports extrema, signed 64-bit
`accelerationSumsMicroG` and
`angularRateSumsMilliDegreesPerSecond`, and corresponding means from exactly
the accepted records. Signed division truncates toward zero under C++11 and
is tested on positive and negative values. A reported mean is descriptive
evidence for this attempt, not a calibration write and not permission to
alter later samples.

Qualification applies only to the exact complete domain in its result.
Changing source identity, either configured range, calibration or
configuration revision, mapping, record schema, normalization contract, or
qualification contract invalidates reuse. Timestamp epochs are not invented:
record occurrence time and supplied policy time are compared only under their
declared shared-domain contract. If Lesson 067 cannot preserve that contract,
qualification is blocked rather than restamping the record.

## Composition pressure

The maximum authorized E0 composition uses a 32-record synthetic stationary
trace at maximum declared ranges, all extrema and widened sums, a complete
terminal qualification envelope, and a Lesson 069 fixed-capacity recorder
session. The collision frame arrives at the maximum age and gap while sequence
rollover, one saturated axis, one rate-bound crossing, and a changed duplicate
compete. Structural and ordering evidence must select the deterministic
outcome without partial accumulation or source switching.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | Applicable. One update performs O(1) validation, six mappings, bounded comparisons, and accumulator updates. No scan of retained samples, retry, sleep, polling, or catch-up is allowed. Tests exercise fastest cadence, delayed calls, exact boundaries, rollover, and the maximum collision. |
| Total memory and hardware resources | Applicable. The implementation retains configuration, six minima/maxima, six widened sums, attempt provenance, and one result, not 32 full records. Initial promotion targets are policy plus result at or below 384 bytes, hard ceiling 512 bytes; conservative synchronous stack at or below 384 bytes, hard ceiling 512 bytes. The complete Lesson 069 composition must separately preserve at least 4,096 bytes residual Mega SRAM. Any target miss requires independent review; a hard miss blocks promotion. |
| Shared bus or transport | Not applicable at E0. Incoming records are copied values. Future I2C adapters own address, identity, transaction, data-ready, interrupt, timeout, reset, and rollback behavior independently and cannot be qualified merely because this policy accepts their bytes. |
| Persistence and recovery | Not applicable. Qualification state and attempts are volatile. A persisted prior result cannot qualify a current source after restart. Any later stored envelope requires its own canonical format, corruption, version, provenance, and recovery proof. |
| Motion, external power, or stored energy | Not applicable. A learner may hand-move an unpowered module only as a narrative illustration; E0 executes copied traces. No output path, motor, launcher, ignition, or energizing operation exists. |
| Observation identity and provenance | Applicable and central. The complete exact source/range/revision/schema/mapping domain accompanies attempt, first/last sequence and time, count, extrema, sums, terminal reason, and offending record attribution. Labels and array positions are never source identity. |
| Diagnostic interference | Applicable in composition. Result cells, optional Serial, LEDs, LCD, host export, or trace storage cannot change qualification, supply missing evidence, hide an unhealthy record, or extend a window. RGB/display health in Lesson 069 is presentation, not qualification authority. |
| Failure collision and recovery | Applicable. Structural invalidity rejects atomically. Valid source mismatch, producer fault, not-ready, saturation, ordering/timing fault, acceleration excursion, then rate excursion have the stated deterministic precedence and terminal evidence. Recovery requires explicit reset/new attempt; no alternate source is tried. |

## Deterministic proof matrix

| Family | Required cases |
|---|---|
| Lifecycle | construction, initialize success/failure/unsupported physical domain, begin before initialize, duplicate/zero-ID begin, observe without attempt, terminal observe, reset/shutdown at every count, attempt and evidence generation, initialize/reset generation exhaustion |
| Configuration | zero IDs/revisions/ranges/window/age/gap, window 1/2/31/32/33, invalid enum, half-range durations, impossible or overflowing expected-stationary-acceleration intervals |
| Mapping classification | all 216 signed-axis triples with an independent determinant oracle: exactly 24 proper rotations accepted and 192 rejected |
| Mapping application | each accepted transform on asymmetric positive/negative vectors for both units, permutation/sign correctness, both signed extremes, every `INT32_MIN` negation path, no partial output |
| Source domain | every source kind/model pair, exact synthetic identity, each single-field mismatch, `Unsupported` physical configurations before lifecycle mutation, changed range/configuration/calibration/schema/normalization revision |
| Producer quality | every `Status`, ready/not-ready, every saturation enum, producer fault plus not-ready/saturation/numeric collision, malformed values, retained offending provenance |
| Time and sequence | first record, ordinary exactly-one progress, equal identical idempotence, equal changed, gap, regression, exact-half ambiguity, `UINT32_MAX` to `0`, future time, zero/backward/ambiguous observation gap, exact age/gap, one tick each side; exact `SequenceDiscontinuity`, `TimestampDiscontinuity`, and `Stale` reasons |
| Stationary bounds | every mapped axis at lower/upper acceleration bounds and one beyond; every positive/negative rate at limit and one beyond; simultaneous acceleration/rate collision; widened arithmetic extremes |
| Window | N=2 and N=32, N-1 collecting, N qualified, no N+1 mutation, failed first/middle/final sample, duplicate does not count, reset and explicit retry |
| Evidence | exact first/last attribution, count, extrema, sums, signed mean rounding if exposed, terminal reason/status, canonical zero fields while unqualified, byte-stable replay without padding-based equality |
| Composition | terminal envelope matches the exact Lesson 067 record domain consumed by Lesson 069; foreign/stale/changed envelopes reject; presentation or recorder capacity cannot alter qualification |
| Tooling | strict C++11 warnings, ASan/UBSan, traits, host/archive inventory, Mega compile, object/result sizes, no-LTO stack, aggregate residual SRAM, lesson/PDF/site gates |

## Prior-decision impact

- Lesson 043 copied sample provenance and lack of adapters: **preserved**.
  Lesson 068 consumes normalized records and cannot turn physical-family enum
  tags into powered support.
- Lesson 044 ownership of pitch/roll and presentation intent: **preserved**.
  Only the general proper-rotation mechanism moves downward; presentation
  vocabulary and results remain unchanged.
- Existing `SignedAxis` spelling and accepted right-handed frames:
  **preserved through remediation**. A second validator or a dependency from
  qualification to presentation is rejected.
- Explicit supplied time, modular half-range ordering, fixed storage, no heap,
  and bounded work: **preserved**.
- Producer faults, not-ready evidence, saturation, stale data, and source
  mismatch remain distinct: **preserved and strengthened** by a terminal
  reason plus complete attribution.
- One source per configured session: **preserved**. Voting, silent fallback,
  runtime probing, and automatic failover are prohibited.
- Calibration revision as provenance rather than a calibration algorithm:
  **preserved**. Stationary means or biases do not mutate calibration.
- Lesson 069 may compare separately configured traces but cannot merge source
  authority: **extended** by requiring an immutable exact-domain
  qualification envelope.
- Exact specimen and primary-source gates before powered adapters, wiring, or
  formal schematics: **preserved**.
- Every non-schematic lesson visual uses pencil-drawing presentation:
  **preserved**.

## Rejected alternatives

1. **Reuse `OrientationPresentationPolicy` as the qualifier.** Rejected
   because pitch/roll presentation, stationary source qualification, and
   source mapping have different owners and failure vocabularies.
2. **Duplicate the private Lesson 044 frame validator.** Rejected because two
   proper-rotation authorities can drift. The small foundational helper is the
   narrower remediation.
3. **Average away individual excursions.** Rejected because one saturated,
   moving, stale, or faulty record is material evidence and must not disappear
   inside a plausible mean.
4. **Keep collecting after a bad record.** Rejected because it selects a
   convenient subset and makes completion depend on hidden retry policy.
5. **Probe both device families and accept whichever responds.** Rejected
   because unidentified-address writes and runtime source selection violate
   the cadence and specimen gates.
6. **Treat a passing synthetic trace as physical calibration.** Rejected
   because E0 proves only deterministic copied-record policy.
7. **Retain all 32 full records inside the component.** Rejected because
   running evidence is sufficient for qualification and the full trace belongs
   to the Lesson 069 recorder or host fixture.
8. **Persist and reuse a prior qualification after restart.** Rejected because
   source attachment, configuration, mounting, and session continuity are not
   proved.

## Gate result

- Disposition: `natural fit; mapping remediation implemented`
- Open risks: final Lesson 067 record schema and time-domain contract, exact
  public result spelling, signed-mean rounding if exposed, measured AVR
  object/stack/aggregate sizes, exact physical specimen identity, axis
  mounting, register interpretation, electrical topology, stationary fixture,
  bias/accuracy thresholds, and bench acceptance
- Required discussion or decision IDs: the Lessons 067--069 implementation
  plan records the shared mapping extraction and corrects the cadence's
  inaccurate reference to existing Lesson 043 adapters
- Remediation result: the shared mapping primitive preserves Lesson 044
  semantics; exhaustive mapping, precedence, lifecycle-generation, arithmetic,
  strict host, style, Mega compile, measured size, lesson, monochrome,
  pencil-policy, and site gates pass
- Remaining composition verification: the Lessons 067--069 exact aggregate
  resource probe and maximum-recorder fixture remain part of Lesson 069's
  promotion boundary
- Maximum-composition proof: the 32-record rollover/boundary/failure collision
  and complete Lesson 069 envelope integration must pass without partial
  accumulation, hidden retry, or source switching
- Promotion result: E0 published and host verified; no powered adapters,
  physical calibration, wiring, formal schematics, or bench claims
