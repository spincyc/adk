# Lesson 069 qualified-motion-recorder architecture stress pass

Status: terminal E0 promotion review. The bounded copied-evidence composition
is a natural fit, implementation and exact resource gates pass with one
independently reviewed flash-target miss, and powered acceptance remains open.

This pass records the initial and terminal dispositions for Lesson 069 in the
[extended component/project cadence](../projects/component_project_cadence.md).

The qualified motion recorder is an E0 composition only when it consumes one
saved terminal Lesson 068 qualification decision, accepts only Lesson 067
records and controls that reproduce its correlation identity, advances one
fixed hand-motion script, and emits caller-owned record and presentation
intent. It is not an
inertial adapter, source selector, axis qualifier, clock, filesystem, display
driver, or durable logger.

## Boundary

- Name and lesson/project: `QualifiedMotionRecorder`, Lesson 069
- Review state: terminal implementation and promotion stress pass
- Public responsibility: validate one atomic qualification envelope, admit
  records from one explicitly configured source for one recorder session,
  append a bounded normalized trace without silent loss, advance an explicit
  script step, and fill inert health/orientation/presentation intent
- Direct dependencies: promoted Lessons 067 and 068 copied values, the
  existing Lesson 044 orientation-presentation policy, `Status`, `TimePoint`,
  fixed-width value types, and no hardware endpoint
- Existing decisions reconsidered: none of Lessons 043--045 is widened;
  source-frame normalization remains Lesson 067 work, qualification-frame
  mapping remains Lesson 068 work, and RTC/SD durability remains deferred

## Fit review

| Pressure | Evidence and initial disposition |
|---|---|
| API and layering | Natural only if the project receives a complete copied envelope binding one normalized record to its qualification witness. Reading mutable Lesson 067 and Lesson 068 snapshots separately would permit a torn decision and is rejected as architectural buckling. The recorder never chooses, votes, probes, or fails over between MPU6050, QMI8658, or synthetic sources. |
| Ownership and lifecycle | Inert construction, noncopyable/nonmovable coordinator, fixed configuration, explicit `initialize`/`reset`/`shutdown`, no heap, callback, retained caller pointer, child reference, or hidden clock. A session fixes one source identity and all schema, normalization, mapping, qualification, script, and presentation revisions until reset. |
| Time and ordering | All time enters as copied evidence or supplied `now`. Record sequence, sample sequence, qualification attempt, and script step remain distinct. Same-sequence byte-identical envelopes are idempotent; changed duplicates, regressions, exact half-range ambiguity, future evidence, invalid age, and exhaustion reject atomically. |
| Errors and status | Structural and lifecycle invalidity use `Status`. Well-formed unqualified, stale, not-ready, saturated, or producer-fault evidence remains explicit domain state. Source or qualification fault dominates orientation and presentation. Capacity exhaustion is visible and never overwrites an earlier record. |
| Resources | E0 claims zero pins, timers, interrupts, buses, endpoints, supplies, displays, LEDs, buttons, clocks, or media. The linked Lessons 043, 044, 067, 068, and 069 maximum composition includes caller-owned trace storage and complete result/intent values. The exact ABI/no-LTO probe passes every hard gate; its sole flash-target miss is independently reviewed below. |
| Deterministic proof | Host fixtures can replay both named physical-family record shapes without claiming their adapters, every proper axis mapping, every qualification outcome, script boundaries, capacity boundaries, sequence/time rollover, reset, and shutdown. Golden traces compare normative record images, not C++ object bytes. |
| Packaging/public surface | One project header/source, strict host test, compile-only Mega replay, exact linked resource probe, HTML reference, and complementary pencil-drawing PDF. No I2C, Wire, RTC, SD, LCD, RGB, button, MPU6050, or QMI8658 driver include enters E0. |
| Example and documentation fit | The canonical replay follows acquire/configure/start and observe/decide/actuate over copied fixtures. Named result cells expose source health, script progress, orientation intent, trace count, capacity state, and record outcome without making Serial the only observation path. |
| Downstream effects | Lessons 043--045 retain their public contracts. Lessons 067--068 retain normalization and qualification ownership. A future powered implementation is split into independently qualified sensor acquisition, presentation, and RTC/SD persistence rather than hidden behind this project. |

## Frozen E0 information rules

### One configured source per session

“Interchangeable” means that separate recorder sessions can consume the same
public normalized contract from separately configured and independently
qualified sources. It does not mean hot swapping, fallback, voting, blending,
or accepting whichever device produces data first.

Configuration fixes the exact source identity and identity domain, normalized
record schema and normalization revision, qualification revision, axis
mapping revision, hand-motion script revision, orientation configuration
revision, maximum accepted age, and bounded trace capacity. Zero, unknown,
reserved, or internally inconsistent values reject initialization. A valid
reset advances the lifecycle generation and begins a new empty session; it
does not silently retain qualification or trace records from the previous
session.

Physical-family tags are provenance, not proof that a physical MPU6050 or
QMI8658 was attached. E0 positive replay uses authorized copied fixtures.
Unidentified revisions remain unpowered and cannot be made acceptable by a
matching enum value.

### Saved qualification and atomic stream correlation

`qualify()` saves one complete terminal Lesson 068 witness. Each subsequent
`update()` receives one copied normalized source record plus closed command
evidence that must reproduce the saved qualification identity and the
configured caller-owned trace token. Together they bind at least:

- source identity and identity domain;
- source, schema, normalization, mapping, calibration, and qualification
  revisions;
- source observation time and sample sequence;
- qualification lifecycle generation and attempt sequence;
- qualification state, reason, status, window bounds, and evidence digest;
- the normalized acceleration, angular-rate, ready, saturation, producer
  status, and transport-status evidence required by the published seams.

The exact authoritative qualification fields remain present in recorder
state. Qualification and record digests are collision detectors and
correlation aids, never substitutes for comparing or preserving the
authoritative fields. The trace token binds commands to the configured
caller-owned buffer contract without retaining its address.

The recorder validates the saved witness and complete update correlation
before changing session state. The control must name the saved qualification
revision, lifecycle generation, attempt, digest, and configured trace token;
it must also match the record source, sequence, and time. A record attached to
a different qualification or trace rejects without changing the recorder,
caller-owned result, trace buffer, or presentation intent.

Well-formed `NotQualified` and `Rejected` envelopes may update explicit health
and fault presentation, but they cannot append a motion record or advance the
hand-motion script. Malformed evidence is not converted into a semantic
rejection.

### Bounded trace and record image

The caller owns fixed-capacity trace storage and supplies its exact capacity
at initialization or through a fixed public value type. The recorder retains
no hidden allocation and no pointer beyond the synchronous call boundary.
Accepted qualified samples append in order until full. Full capacity produces
an explicit terminal-or-recoverable `TraceFull` outcome according to the
frozen lifecycle; it never wraps, overwrites the oldest sample, coalesces
records, or pretends that a dropped sample was saved.

Each accepted trace cell has a normative fixed-width byte image with explicit
magic, format version, encoded length, source and revision identity, session
generation, recorder and source sequences, observation time, qualification
attempt, script step, normalized values and quality, orientation result,
health/fault fields, reserved-zero bytes, and integrity check. Encoding fixes
field order, width, signed representation, and byte order. Raw struct layout,
padding, `memcmp` of public objects, compiler ABI, and an in-memory address
are never record identity.

Appending is transactional:

1. validate configuration, lifecycle, envelope, chronology, and capacity;
2. derive qualification-frame sample and orientation from copied evidence;
3. build and validate a staged canonical cell;
4. write the complete destination cell;
5. commit count, sequence, script progress, and result together.

Any failure before the commit leaves all observable state and every trace byte
unchanged. Record-sequence exhaustion faults before zero. A same-sequence
byte-identical replay does not append twice; a changed duplicate rejects.

E0 trace storage is volatile evidence only. “Recorded” means copied into the
caller-owned bounded trace for this process lifetime. E0 does not open a file,
issue an SD write, read an RTC, acknowledge media, retry storage, recover a
torn write, or claim power-loss durability.

### Script and presentation

The hand-motion script is a fixed, versioned sequence of bounded learner
prompts. Each accepted record is attributed to the current step before that
step advances. A step advances only on its explicit deterministic completion
predicate; neither source substitution nor a diagnostic/display event can
advance it. The terminal step remains terminal until reset.

Orientation is derived through the existing Lesson 044 presentation seam from
the qualification-frame sample. Lesson 069 does not introduce a second
pitch/roll algorithm or reinterpret source axes. Presentation output is inert
semantic intent:

- explicit source and recorder health;
- current script prompt and progress;
- valid orientation value-or-fault content;
- trace count and full indication;
- character-display content tokens;
- RGB color and blink-code tokens.

Fault presentation dominates valid orientation. A valid-looking angle cannot
remain visible as current when the bound envelope is stale, unqualified,
saturated where disallowed, or producer-faulted. Trace-full and storage-not-
implemented states are distinguishable from sensor fault. Display self-test
intent is separate from changing orientation and cannot qualify a source,
append a record, clear a fault, or advance the script.

Shutdown produces canonical inert presentation and appends nothing.
Repeated shutdown is idempotent. Reset clears volatile trace/session progress
only through the documented caller-owned-buffer contract; it must not erase
or rewrite bytes outside the declared capacity.

## Composition pressure

The maximum E0 fixture alternates two separately configured sessions using
copied MPU6050-shaped and QMI8658-shaped golden records, while never claiming
either powered adapter. Each session reaches its trace limit with the largest
qualification window, orientation policy, result, staging image, and
caller-owned trace live. It injects one source fault, one qualification
rejection, a changed duplicate, a stale record at the age boundary, ordinary
sequence/time rollover, the last valid script transition, full capacity,
shutdown, and reset.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time | Applicable. One bounded envelope validation, fixed orientation calculation, script decision, presentation fill, and at most one fixed-size append per update. No polling, retry, acquisition wait, display refresh loop, storage retry, or unbounded catch-up. Prove a constant work bound at empty, one-before-full, and full capacity. |
| Total memory and hardware resources | Applicable. Measure ordinary and isolated no-LTO linked Lessons 043/044/067/068/069 with recorder object, both candidate/staged cells, qualification envelope, orientation result, presentation result, hidden-return allowance, conservative stack, ISR reserve, and caller-owned maximum trace counted exactly once. Target/hard gates are recorder object 512/768 bytes, canonical record image 128/160 bytes, synchronous stack 768/1,024 bytes, static SRAM 2,048/3,072 bytes, at least 4,096/3,072 bytes residual SRAM, and flash 32/40 KiB. Any target miss requires an independent written disposition; no hard miss promotes. |
| Shared bus or transport | Not applicable at E0. A future source adapter, LCD, RTC, or SD endpoint needs explicit ownership, borrower lifetime, address/chip-select identity, arbitration, bounded transaction, rollback, congestion, restart, and combined-resource proof. |
| Persistence and recovery | Applicable only to canonical volatile trace semantics. Prove fixed encoding, capacity, transactional append, corruption detection for serialized cells, reset/shutdown behavior, and no durability language. RTC timestamp domain, media schema, commit protocol, torn-write recovery, wear, capacity, removal, and power-loss behavior remain E1-open. |
| Motion, external power, or stored energy | Not applicable at E0. The script describes hand motion of an inert identified board only; it authorizes no motor, launcher, ignition, vehicle control, or unattended moving load. |
| Observation identity and provenance | Applicable. Preserve exact source, range, calibration, mapping, normalization, qualification, session, script, and sample identity through envelope, orientation, trace cell, and presentation result. Values from distinct source times are never presented as one simultaneous record. |
| Diagnostic interference | Applicable. Serial, display/RGB intent, self-test, result inspection, and trace serialization cannot alter qualification, orientation, script completion, capacity, or append order. Future powered diagnostics enter the aggregate pin/timer/current/bus/memory budget. |
| Failure collision and recovery | Applicable. Exercise source fault plus valid-looking orientation, qualification rejection plus script completion candidate, full capacity plus a fresh qualified record, changed duplicate plus reset request, and shutdown plus pending presentation. Lifecycle/reset dominance must be explicit, and every rejected path preserves the prior complete trace and state. |

The canonical Mega replay honestly runs Lessons 067, 068, and 069 and measures
39,428 B flash and 2,347 B static SRAM. The exact no-LTO maximum composition
measures 35,144 B flash, 2,347 B static SRAM, 861 B synchronous stack, a 509 B
recorder object, a 128 B canonical record image, and 4,856 B residual SRAM.
Exact flash, static SRAM, and stack exceed their targets by 2,376 B (7.25%),
299 B (14.60%), and 93 B (12.11%) respectively. Object, image, and residual
targets pass, and every hard gate passes. Independent review accepted the
three target misses after the earlier fabricated-witness example was replaced
with the genuine Lesson 068 qualifier and structural duplication was removed.
The review is bound to boundary-scoped resource fingerprint
`7be0c300acc4f0e93d9cb5fa2e9b5a0ced5771458f9749e38eb8e507e61b30c6`
and the measured source, configuration, layouts, no-LTO toolchain and flags,
probe, fixture, and call graph; any change makes it stale. It authorizes no
hard-limit miss or powered/persistent E1 composition.

## Required deterministic proof

Tests must cover:

- invalid, reserved, mismatched, and exact configured source identities and
  every schema/normalization/mapping/calibration/qualification/script revision;
- separate complete sessions for copied MPU6050-shaped, QMI8658-shaped, and
  synthetic fixtures without automatic selection or mid-session substitution;
- a qualification witness bound to the exact normalized record, plus every
  one-field mismatch in source, revision, sequence, time, attempt, and digest;
- each Lesson 067 ready, not-ready, saturation, producer-status,
  transport-status, and malformed-record outcome;
- each Lesson 068 collecting, qualified, rejected, reset, and terminal-fault
  outcome, including a valid-looking sample that remains unqualified;
- every proper axis permutation used by golden traces and explicit rejection
  of a mismatched or improper mapping revision;
- orientation boundaries inherited from Lesson 044, with fault dominance and
  no cached healthy angle after unhealthy evidence;
- script step zero, every transition, last step, terminal replay, reset, and
  proof that rejected/unqualified inputs do not advance;
- capacity zero if forbidden, one, one-before-maximum, maximum, and full;
  verify no wrap, overwrite, coalescing, partial append, or out-of-bounds write;
- record sequence first/last/exhaustion, source sequence duplicate/changed
  duplicate/regression/wrap/half-range, time future/age equality/one-past/
  wrap/half-range, and lifecycle generation exhaustion;
- canonical encode/decode golden images, explicit endianness, signed extrema,
  reserved-zero enforcement, corruption at every byte class, and integrity
  mismatch;
- caller-owned envelope/result/trace canaries, hidden-return probes,
  unchanged-output rejection, unchanged-trace rejection, and byte-identical
  replay across optimization settings;
- simultaneous source fault, qualification rejection, diagnostic failure,
  trace-full, reset, and shutdown collisions under the documented precedence;
- initialization, reset, shutdown, repeated shutdown, reinitialize, and
  recovery while copied source evidence remains faulted;
- absence of heap use and hardware/library dependencies, plus strict host,
  ASan/UBSan, style, header, Mega compile, resource, PDF, site, and publication
  gates.

The canonical Mega replay is compile-only and writes named result cells for
health, script progress, orientation validity, trace count, capacity state,
RGB intent, and character-display intent. Future physical acceptance must
separately record prediction, non-Serial observation, interpretation,
resource-acquisition evidence, and safe-state evidence.

## Prior-decision impact

- Lessons 043--045 are **preserved**: Lesson 069 consumes copied inertial
  evidence and the existing orientation seam without changing their APIs or
  claiming a physical adapter.
- Lesson 067 is **preserved** as source-frame normalization and record
  provenance; Lesson 069 does not repair or reinterpret malformed records.
- Lesson 068 is **preserved** as explicit one-source qualification and
  qualification-frame mapping; Lesson 069 does not vote, fail over, or
  requalify.
- “Interchangeable” is **narrowed for architectural consistency** to the same
  consumer contract across separately configured sessions. Runtime source
  switching would require a new design decision and new discontinuity rules.
- Lessons 022/024 persistence decisions are **preserved**: a caller-owned
  trace and canonical image are not RTC/SD support or durable storage.
- Circuit-native observation policy is **extended** only as inert RGB and
  character-display intent at E0; powered endpoints and display self-test
  evidence remain separate acceptance gates.
- The sensor inventory rule is **preserved**: no unidentified board is
  powered, and copied family tags do not establish exact silicon, carrier,
  address, voltage, register map, or calibration.

## Gate result

- Disposition: `natural fit with required remediation` for an E0 composition
  using one atomic qualification envelope, one configured source per session,
  fixed caller-owned capacity, canonical record images, and fault-dominant
  inert presentation
- Open risks: persistence state machine; exact MPU6050 or QMI8658 specimen;
  powered I2C, display/RGB/button, RTC, and SD acceptance
- Required discussion or decision IDs: none for the bounded E0 shape; runtime
  source switching, automatic failover, hidden child reads, silent ring
  overwrite, or durability claims require an explicit architecture decision
- Remediation owner and next action: the E0 implementation is complete; future
  owners must remeasure and obtain a new independent disposition whenever the
  resource fingerprint changes
- Verification commands and results: strict host tests, public-header
  isolation, canonical Mega compilation, style, lessons, site, and exact
  resource gates pass; release workflow status is recorded in the work queue
- Maximum-composition scenario and proof: exact tuple and fingerprint recorded
  above; the sole flash-target miss is independently accepted and every hard
  limit passes
- Promotion permitted: yes for the bounded host-verified E0 project only; no
  powered source, presentation, clock, media, persistence, or bench claim is
  permitted
