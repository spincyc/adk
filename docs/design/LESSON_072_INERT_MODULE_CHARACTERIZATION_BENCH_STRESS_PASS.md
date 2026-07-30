# Lesson 072 inert-module-characterization-bench architecture stress pass

Status: initial E0 design review. The copied-evidence composition is a natural
fit; implementation, exact resource evidence, publication gates, and all
powered acceptance remain open.

This pass reviews the controlling
[Lessons 070--072 module-characterization plan](LESSONS_070_072_MODULE_CHARACTERIZATION_PLAN.md).

`InertModuleCharacterizationBench` is an E0 composition only when it consumes
one immutable `ModuleCharacterizationEnvelope` containing a Lesson 070
descriptor and the terminal Lesson 071 evidence for that exact descriptor and
run, guides a bounded inert review script, and prepares one caller-owned
volatile characterization image. It is not a generic sensor adapter, module
detector, ADC or comparator owner, electrical identity prover, calibration
laboratory, endpoint owner, acceptance authority, or safety instrument.

## Boundary

- Name and lesson/project: `InertModuleCharacterizationBench`, Lesson 072
- Review state: initial stress pass before implementation
- Public responsibility: validate one atomic `ModuleCharacterizationEnvelope`,
  bind one exact declared fixture for one session, advance the exact review
  script, expose the Lesson 071 relation without inventing physical units, and
  prepare one canonical 192-byte caller-owned volatile record image
- Direct dependencies: the promoted Lesson 070 threshold-module descriptor,
  promoted Lesson 071 characterization-run result, `Status`, `TimePoint`,
  fixed-width value types, and no hardware endpoint
- Existing decisions reconsidered: none of Lessons 061--063 or earlier sensor
  policies is widened; actual acquisition, power, and presentation remain
  separately gated physical work

## Fit review

| Pressure | Evidence and initial disposition |
|---|---|
| API and layering | Natural only if Lesson 072 consumes one complete copied envelope binding one Lesson 070 descriptor to the Lesson 071 result produced for that exact descriptor and run. Reading mutable child objects separately would permit a torn decision. A descriptor records a declared fixture contract; it does not identify a physical board or prove its electrical construction. |
| Ownership and lifecycle | Inert construction, noncopyable/nonmovable coordinator, fixed configuration, explicit `initialize`/`reset`/`shutdown`, and no heap, callback, retained caller pointer, child reference, or hidden clock. One lifecycle session fixes one declared fixture and descriptor identity. Changing either requires reset and a new generation. |
| Time and ordering | Descriptor revision, run generation, run sequence, sample sequence, ramp index, observation time, and script step remain distinct. Duplicate, regression, ordinary rollover, exact half-range ambiguity, future evidence, stale evidence, and exhaustion rules are explicit. A same-sequence replay is idempotent only when the complete envelope is byte-identical. |
| Errors and status | Structural or lifecycle failure rejects atomically through `Status`. Well-formed chatter, no-transition, endpoint-only rail, stale, disagreement, or other terminal Lesson 071 evidence remains typed characterization state. A healthy rail sample is ordinary evidence and is never upgraded into a terminal outcome, `OpenLike`, or `ShortLike` diagnosis. Unknown or mismatched declared-fixture identity dominates plausible numeric evidence. |
| Resources | E0 claims zero pins, ADC channels, timers, interrupts, buses, power switches, displays, LEDs, supplies, or media. Promotion requires exact ordinary and isolated no-LTO measurement of the linked Lessons 070--072 maximum composition, including caller-owned maximum run evidence, staged record image, complete result, hidden returns, stack, and ISR reserve. |
| Deterministic proof | Host fixtures cover both polarities, both ramp directions, transition brackets, conservative intervals, chatter, no-transition/stale/disagreement outcomes, endpoint-only rail finalization, proof that ordinary rails do not become terminal or `OpenLike`/`ShortLike` diagnoses, unknown descriptors, chronology boundaries, canonical record corruption, reset, and shutdown. No fixture is reported as a powered specimen. |
| Packaging/public surface | One project header/source, one codec header/source, strict host tests, compile-only Mega replay, exact linked resource probe, HTML reference, and complementary pencil-drawing PDF. No `AnalogInput`, `DigitalInput`, endpoint, display, or power-driver ownership enters E0. |
| Example and documentation fit | The canonical replay follows acquire/configure/start and observe/decide/actuate over one copied envelope. Named result cells expose bench state, exact script step, relation, fault dominance, and `recordPrepared` without making Serial the only evidence. |
| Downstream effects | Earlier application policies do not inherit physical qualification from this lesson. Lesson 079 retains low-side load-driver ownership and Lesson 081 retains later component-qualification scope. A Lesson 072 characterization record is not a hardware acceptance record. |

## Frozen E0 information rules

### Exact identity and admission

`ModuleBenchConfig` fixes `benchRevision`, `envelopeRevision`,
`recordSchemaRevision`, `expectedDescriptorId`,
`expectedDescriptorRevision`, `expectedDescriptorSchemaRevision`,
`expectedDeclaredSpecimenRevision`,
`expectedDeclaredElectricalEvidenceRevision`, `expectedDescriptorDigest`,
`expectedControlSourceId`,
`expectedControlSourceConfigurationRevision`, and `maximumControlAge`. It does
not duplicate the complete descriptor. The
authoritative descriptor exists exactly once inside
`ModuleCharacterizationEvidence`, which exists exactly once inside
`ModuleCharacterizationEnvelope`. Invalid encodings or inconsistent
cross-fields reject before a session begins. Valid `Unspecified` declarations
remain E0-readable but block later E1 admission.

The descriptor stays fixed for the complete session. Lesson 072 does not
hot-swap, autodetect, vote among aliases, or accept whichever trace resembles
a known family. Marketplace aliases remain documentation metadata and never
become C++ type names or electrical identity.

The E0 family allowlist is limited to copied evidence shaped for declared
low-voltage analog/comparator fixture contracts in the light, sound, Hall,
thermal, flame/radiant, Metal Touch, vibration, and obstacle families. That
list permits planning, not identity, connection, or power. Generic
analog-temperature and
capacitive-touch boards, gas/heater modules, register devices, emitters,
physiological modules, lasers, and unidentified boards are not admitted by
this boundary. A flame/radiant-family fixture authorizes only inert replay;
it never authorizes a flame, heater, ignition source, or hot target.

### Honest Lesson 070 to Lesson 071 to Lesson 072 execution

The architecture preserves a strict evidence flow:

1. Lesson 070 validates and publishes a compact descriptor for one declared
   fixture reference; it does not identify physical hardware.
2. Lesson 071 consumes that descriptor and copied samples to produce a
   bounded characterization-run result.
3. Lesson 072 atomically consumes one immutable
   `ModuleCharacterizationEnvelope` and validates that its evidence belongs to
   that exact descriptor, source, session, and run.

Lesson 072 never reconstructs a descriptor from trace shape, recomputes a
private characterization with different rules, repairs malformed child
evidence, or treats a digest as a replacement for authoritative fields.

The envelope contains `envelopeRevision`, the complete evidence-owned
descriptor and terminal evidence, `descriptorDigest`, and `evidenceDigest`.
It therefore binds:

- descriptor schema/identity/revision, declared specimen reference/revision,
  and electrical-evidence revision;
- envelope and characterization revisions;
- declared ADC domain, active polarity, pull requirement, warm-up,
  settling, and threshold-pot direction;
- Lesson 071 lifecycle generation, session/run/leg identities, leg counts,
  first/last sample sequences, and first/last observation times;
- ascending and descending crossing evidence, conservative intervals,
  chatter, no-transition, endpoint-only rail, stale, and comparator-relation
  outcomes;
- complete producer status plus the domain-separated `descriptorDigest` and
  `evidenceDigest`.

Full fields remain authoritative. Digests are collision detectors and
correlation aids only. Every correlation field is validated before any bench
state or caller-owned output changes. A new run paired with an old descriptor,
a changed descriptor paired with a prior result, or two independently read
child snapshots rejects atomically.

Descriptor, evidence, and compact-witness digests use CRC-32/ISO-HDLC over
canonical little-endian, field-by-field encoding with domain tags
`ADK-MOD-DESC-1`, `ADK-MOD-EVID-1`, and `ADK-MOD-WIT-1`. Parameters are
polynomial `0x04c11db7` (reflected `0xedb88320`), initial `0xffffffff`,
reflected input/output, and final XOR `0xffffffff`. Every 32-bit result,
including zero, is valid; digests have no sentinel value.

Canonical encoding uses one byte for enums, Boolean values, and `StatusCode`;
two little-endian bytes for `uint16_t`; and four little-endian bytes for
`uint32_t`, `Duration::milliseconds()`, and `TimePoint::milliseconds()`. A
false `present` flag requires every following field in that optional value to
encode as zero. Descriptor and evidence order follow their public declaration
order recursively. Compact-witness order is `present`, `controlOrdinal`,
`analogRaw`, `comparatorAsserted`, `sequence`, and `observedAt`; bracket
points use complete `ModuleCharacterizationPoint` declaration order.

The domain tag bytes begin each digest. Tag-only seed vectors are
`0xcb58c79d`, `0x1daf5814`, and `0xb1b83fae` for descriptor, evidence, and
witness respectively. Tests freeze full valid descriptor/evidence and
present/absent-witness golden byte vectors. Field order, width, tag, or
canonical-absence changes are schema changes. Raw structure bytes, padding,
addresses, and pointers are never hashed.

`beginSession()` accepts only terminal, fully correlated Lesson 071 evidence.
A terminal unhealthy envelope produces fault-dominant presentation rather
than replaying Lesson 071. Nonterminal or malformed evidence rejects without
committing a session.

### Exact state, script, and control surface

The public `ModuleBenchState` values are exactly `Inert`, `Ready`,
`ScriptActive`, `RecordPrepared`, `Fault`, and `Shutdown`. The review script
uses exactly these `ModuleBenchScriptStep` values, in order:

1. `InspectDeclaration`;
2. `ReviewAscending`;
3. `ReviewDescending`;
4. `ReviewVerification`;
5. `PrepareRecord`.

`ModuleBenchCommand` is exactly `None` or `Advance`. Each
`ModuleBenchControl` binds source/configuration identity, session ID, sequence,
observation time, command, and `producerStatus`.
Changed duplicates, regressions, gaps where required, future/stale controls,
and exact half-range ambiguity reject without mutation.

`beginSession()` is the sole envelope admission point and moves a valid
session from `Ready` to `ScriptActive`. `Advance` moves only to the next named
review step. Direct `prepareRecord()` is valid only at the final
`PrepareRecord` step and calls the single codec path; `Advance` reaches that
step but never writes caller bytes.
`RecordPrepared` makes repeated direct preparation idempotent and
byte-identical. The direct `reset()` method is the sole reset mutation path:
it discards the saved compact snapshot, advances the nonzero lifecycle
generation, and returns `Ready`. Session IDs are nonzero forward modular and
are not reused. `Shutdown` is terminal and inert.

These are semantic review controls at E0. The project does not wait for real
time, sample a pin, adjust a potentiometer, switch a rail, drive a display, or
assert that any physical action occurred. There is no duplicated
begin/finish-leg control surface because Lesson 071 has already terminalized
the run.

| State | command `None` | command `Advance` | direct `prepareRecord()` | direct `reset()` |
|---|---|---|---|---|
| `Ready` | idempotent | invalid | invalid | `Ready`, generation + 1 |
| `ScriptActive` before final | idempotent | next named step | invalid | `Ready`, generation + 1 |
| `ScriptActive` at final | idempotent | remains final | prepare canonical record | `Ready`, generation + 1 |
| `RecordPrepared` | idempotent | invalid | idempotent and byte-identical | `Ready`, generation + 1 |
| `Fault` | invalid | invalid | invalid | `Ready`, generation + 1 |
| `Shutdown` | not initialized | not initialized | not initialized | not initialized |

### Characterization semantics

Raw analog evidence remains in the descriptor's declared integer ADC domain.
Lesson 072 never converts counts into temperature, illuminance, sound level,
gas concentration, distance, touch capacitance, or another physical unit
without a separately justified calibration contract.

Comparator physical level and descriptor-interpreted threshold state remain
separate. Active-low is not a fault and cannot be inverted twice. Ascending
and descending crossings retain ramp direction and sample attribution.
Hysteresis is a bounded interval or categorical result from Lesson 071, not an
invented precision claim.

Chatter, `NoObservedTransitionActive`, `NoObservedTransitionInactive`, stale,
and analog/comparator disagreement evidence stays explicit. Healthy raw values
at either rail remain ordinary points. `AtLowerRail` or `AtUpperRail` is
terminal only when `finalizeLeg()` finds an endpoint-only run; even then it is
not an `OpenLike` or `ShortLike` diagnosis. The copied results describe the
supplied run under its declared fixture assumptions; they do not prove the
physical root cause. A plausible threshold transition
cannot erase a simultaneous stale, identity, producer, rail, or disagreement
fault.

Raw diagnostic evidence may remain visible beside an unhealthy classification
only when it is marked invalid or diagnostic. It must never appear as a
healthy reading, accepted calibration, or proof that a module is safe to
connect.

### Volatile canonical characterization record

One complete qualifying run may prepare one
`ModuleCharacterizationRecordImage`. Its size is exactly 192 bytes. The image
describes the copied descriptor, terminal Lesson 071 evidence, and
deterministic E0 review state. It is not a hardware acceptance record,
calibration certificate, conformance report, safety approval, or evidence that
a physical module was powered.

`ModuleCharacterizationRecordCodec` is the only encoder and decoder.
`prepareRecord()` stages exactly 192 bytes and then copies them atomically into
caller-owned output. Before encoding, the bench derives one compact semantic
`ModuleCharacterizationRecord` from its admitted evidence and compact result.
That record contains only the fields represented by the frozen image; it is
not another copy of the descriptor or envelope. The coordinator never
hand-encodes a second format.
Multibyte fields are little-endian; reserved bytes are zero; bytes 0--189 are
covered by CRC-16 in bytes 190--191. Magic is ASCII `ADMC`, bytes
`0x41 0x44 0x4d 0x43`. The fixed layout retains magic, version,
length, record/bench revisions, lifecycle/session identity, complete compact
descriptor declarations, run and Lesson 071 generation/revisions/counts,
transition brackets, guaranteed/ambiguity intervals, relation,
witness/provenance digests, envelope digests, terminal state/reason/status,
and script state.

| Bytes | Content |
|---:|---|
| 0--3 | ASCII `ADMC` |
| 4 | version |
| 5--6 | length = 192 |
| 7--8 | record schema revision |
| 9--10 | bench revision |
| 11--14 | lifecycle generation |
| 15--18 | session ID |
| 19--22 | descriptor ID |
| 23--24 | descriptor revision |
| 25--28 | specimen reference |
| 29--30 | specimen revision |
| 31--32 | declared electrical-evidence revision |
| 33 | channel topology |
| 34 | comparator output stage |
| 35 | pull requirement |
| 36 | declared pull rail |
| 37 | comparator polarity |
| 38 | threshold-control kind |
| 39 | threshold direction |
| 40--41 | declared supply minimum mV |
| 42--43 | declared supply maximum mV |
| 44--45 | declared signal minimum mV |
| 46--47 | declared signal maximum mV |
| 48--49 | raw-domain minimum |
| 50--51 | raw-domain maximum |
| 52 | warm-up declaration |
| 53--56 | warm-up milliseconds |
| 57 | settling declaration |
| 58--61 | settling milliseconds |
| 62--65 | run ID |
| 66--69 | characterization lifecycle generation |
| 70--71 | characterization revision |
| 72 | ascending count |
| 73 | descending count |
| 74 | verification count |
| 75--89 | ascending bracket: presence, raw pair, assertion flags, sequence pair |
| 90--104 | descending bracket: presence, raw pair, assertion flags, sequence pair |
| 105--109 | guaranteed-inactive interval: presence/lower/upper |
| 110--114 | guaranteed-active interval: presence/lower/upper |
| 115--119 | ambiguity interval: presence/lower/upper |
| 120 | comparator relation |
| 121--124 | first-witness digest |
| 125--128 | last-witness digest |
| 129--132 | offending-before digest |
| 133--136 | offending-after digest |
| 137--140 | first sequence |
| 141--144 | last sequence |
| 145--148 | descriptor digest |
| 149--152 | evidence digest |
| 153 | terminal characterization state |
| 154 | terminal reason |
| 155 | terminal status code |
| 156 | script step |
| 157--158 | descriptor schema revision |
| 159--160 | envelope revision |
| 161 | source ID |
| 162--163 | source-configuration revision |
| 164--189 | reserved zero |
| 190--191 | CRC-16/CCITT-FALSE over bytes 0--189 |

CRC-16/CCITT-FALSE uses polynomial `0x1021`, initial `0xffff`, no reflection,
and final XOR `0x0000`. The compact record intentionally cannot reconstruct
the full envelope or its points; the domain-tagged digests bind that
authoritative evidence while compact brackets and summaries support review.
Raw C++ structure layout, padding, object addresses, and `memcmp` of public
values never define the record format. Decode returns
`ModuleCharacterizationRecordValidity::{Valid,BadLength,BadFraming,
BadIntegrity,BadSemanticValue}` and leaves output unchanged on failure.
Failed or early preparation leaves bench state and the caller image
byte-identical. Repeated preparation in `RecordPrepared` returns the same
192-byte image without advancing state.

Decode order is exact: span size, framing, CRC, semantic/canonical fields,
then staged assignment. Only a span size other than 192 is `BadLength`.
Wrong `ADMC` magic, image version, or encoded length is `BadFraming`; a CRC
mismatch is `BadIntegrity`; and a CRC-valid semantic defect is
`BadSemanticValue`. Encode validates semantics before capacity, stages and
zeros all 192 bytes, calculates CRC last, and copies only on success. Invalid
input returns `InvalidArgument`; short output returns `CapacityExceeded`; bytes
after 191 in a larger output span remain untouched.

All reachable encode failures therefore occur before staged construction.
After semantic and capacity validation succeeds, staging consists only of
fixed-memory field writes, CRC calculation, and the final copy; it performs no
fallible operation, allocation, callback, or injected dependency. A
mid-construction failure is consequently non-applicable rather than an
injectable test case. Tests still prove failure-before-staging atomicity,
canaries around the complete destination and result objects, construction in
the private staging image, and copy-only-on-success. Decode retains its
independent staged-assignment and unchanged-output-on-failure obligations.

Codec semantics require valid enum/status encodings; required nonzero
revisions and identities; a structurally valid reconstructed descriptor;
counts in 0--16; canonical absent brackets and intervals; in-domain bracket
raw values with differing assertion flags and unambiguously forward nonzero
sequences; ordered in-domain intervals; and nonoverlapping guaranteed
intervals. When all counts are zero, first/last sequence are zero; otherwise
they are nonzero and equal or unambiguously forward. Terminal state is exactly
`Complete` with `None`/`Ok`, or `Rejected` with a non-`None` reason and its
retained status. Script step is exactly `PrepareRecord`; reserved bytes are
zero. Digests, including zero, are opaque exact values and are not recomputed
from compact fields.

`beginSession()` precedence is lifecycle/configuration, structural envelope
and descriptor/evidence validation, exact identity/revision/session/run/source
correlation, descriptor/evidence/configured digest equality, then terminal
admissibility. Structural, correlation, or digest failure is an atomic API
rejection even when fields resemble a producer fault. A fully correlated,
digest-correct attributable terminal rejection is admitted as fault-dominant
domain evidence.

The image retains only listed compact review fields. `terminalLeg`, current
`legId`, observation times, control ordinals, directions, complete points, and
full producer statuses are intentionally omitted. They remain authoritative
in Lesson 071 evidence and are indirectly bound by `evidenceDigest`; decode
cannot reconstruct a Lesson 071 result.

`recordPrepared` means only that these bytes exist in caller memory. The arc
defines no endpoint workflow beyond that volatile image.

### Inert presentation intent

`ModuleBenchPresentationIntent` contains exactly the current
`ModuleBenchScriptStep`, `ModuleBenchState`, `faultDominant`, and the
`ModuleComparatorRelation` copied from terminal Lesson 071 evidence.
`ModuleBenchResult` is deliberately compact: lifecycle generation, session
ID, state, step, run ID, descriptor and evidence digests, relation,
presentation, `recordPrepared`, and `Status`. It does not duplicate the
descriptor, evidence, or envelope.

Malformed evidence, producer failure, or a terminal unhealthy relation makes
presentation fault-dominant even when a bracket or raw witness looks
plausible. Presentation cannot alter the relation, create an image, or advance
the script. E0 does not define a physical display, RGB endpoint, button, power
switch, or safe-off output. Repeated shutdown is idempotent and prepares no
record.

## Composition pressure

The maximum E0 fixture uses the largest valid descriptor and Lesson 071 run,
both polarities, ascending and descending boundary crossings, maximum chatter
evidence, one simultaneous rail/disagreement/stale collision, a final direct
`prepareRecord()` call, ordinary sequence/time rollover, failed codec
validation, reset, and shutdown. The descriptor, complete run envelope,
staged record, committed
record, complete result, and coordinator are live together exactly where the
public call graph requires them.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time | Applicable. One bounded envelope validation, one script decision, one presentation fill, and at most one fixed-size codec call occur per command. There is no replay of Lesson 071, polling, warm-up wait, acquisition loop, endpoint refresh, retry loop, or unbounded catch-up. |
| Total memory and hardware resources | Applicable. Measure ordinary and isolated no-LTO linked Lessons 070--072 with maximum envelope, both 192-byte staged and destination images, result, 512-byte-target coordinator, hidden-return allowances, conservative stack, ISR reserve, and caller-owned buffers counted once. The exact provisional 072 tuple is 24/32 KiB flash target/hard, 2,048/3,072 B static SRAM, 768/1,024 B stack, 512/768 B coordinator, exactly 192 B per image, exactly 384 B simultaneous images, and 4,096/3,072 B residual SRAM. A target miss requires an independent design pass; a hard or residual-hard miss blocks. E0 hardware claims remain exactly zero. |
| Shared bus or transport | Not applicable at E0. Future acquisition or presentation endpoints require exact ownership, borrower lifetime, pin/address identity, arbitration, bounded transaction, rollback, congestion, and restart evidence. |
| Persistence and recovery | Not applicable. The sole 192-byte caller image is volatile output; the arc defines no persistence state machine. Atomic staging and typed codec validation are local value semantics. |
| Motion, external power, or stored energy | Not applicable at E0. Future E1 work is limited to one exact identified low-voltage module and must establish supply, pinout, inactive state, current, pull rail, source impedance, backfeed, and safe stimulus before connection. No mains, gas exposure, heater, flame, ignition, laser, physiological use, or safety-alarm claim is authorized. |
| Observation identity and provenance | Applicable. Preserve inventory/specimen identity, descriptor/profile/algorithm revisions, run generation/sequence, ramp direction/index, sample chronology, raw and comparator evidence, and every transformation into presentation and record fields. |
| Diagnostic interference | Applicable. Optional Serial and result/image inspection cannot alter identity, relation, script admission, state, or record content. Future physical diagnostics enter the aggregate pin/timer/current/bus/memory budget. |
| Failure collision and recovery | Applicable. Exercise unknown identity plus a plausible trace; stale evidence plus a crossing; chatter plus rail evidence; disagreement plus direct `prepareRecord()`; codec rejection plus direct `reset()`; and shutdown plus a control. Identity and structural faults dominate characterization, direct reset is the sole reset path, and every rejected path preserves prior complete output. |

## Required deterministic proof

Tests must cover:

- every Lesson 070 descriptor enum and reserved value; valid and invalid ADC
  domains, both active polarities, pull requirements, threshold-pot
  directions, warm-up/settling boundary values, and descriptor digests;
- exact, zero, unknown, duplicated, and mismatched descriptor/specimen,
  envelope, characterization, expected-control source, and
  expected-control-configuration identities; control `producerStatus`
  precedence;
- every one-field descriptor/run correlation mismatch, including lifecycle,
  sequence, sample count, time bound, ramp plan, revision, and digest;
- ascending and descending active-high and active-low golden ramps, with
  threshold values immediately below, at, and above the boundary;
- every guaranteed-active, guaranteed-inactive, and ambiguity interval
  boundary; no exact threshold or signed scalar hysteresis; chatter
  count/window boundaries; no-transition active/inactive; stale; every
  analog/comparator disagreement combination; ordinary healthy rail samples;
  endpoint-only `AtLowerRail`/`AtUpperRail` finalization; and proof that rail
  codes do not become `OpenLike` or `ShortLike` diagnoses;
- proof that characterization outcomes never become calibrated physical units
  or physical root-cause claims;
- minimum and maximum run sizes, forbidden undersize/oversize, sample
  duplicate, changed duplicate, regression, ordinary rollover, exact
  half-range ambiguity, future time, age equality, and one-past stale;
- exact `InspectDeclaration`/`ReviewAscending`/`ReviewDescending`/
  `ReviewVerification`/`PrepareRecord` steps; the exact `None`/`Advance`
  command domain; direct-only `reset()` and `prepareRecord()` paths; the
  state/command table, early prepare, reset behavior, shutdown, repeated
  shutdown, reinitialize, and exhaustion;
- exact 192-byte canonical record golden images with ASCII `ADMC`,
  little-endian fields, integer extrema,
  reserved-zero enforcement, corruption in every byte class, and integrity
  mismatch;
- every reachable failure before staged construction, unchanged complete
  record/result objects and their surrounding canaries on rejection, private
  staging followed by copy-only-on-success, repeated byte-identical
  preparation, changed duplicate rejection, typed decode validity, and
  unchanged decode output on failure; no mid-construction failure injection is
  required because the validated fixed-memory write/CRC/copy sequence has no
  fallible operation, allocation, callback, or injected dependency;
- CRC-32/ISO-HDLC tag-only seed vectors, complete descriptor/evidence golden
  encodings, present/absent compact-witness encodings, canonical absent-zero
  rejection, and CRC-16/CCITT-FALSE image vectors;
- caller-owned envelope/result/record canaries, hidden-return probes,
  byte-stable replay across optimization settings, and no out-of-bounds write;
- fault-dominant `ModuleBenchPresentationIntent`, exact state/step/relation,
  and canonical inert result on reset, shutdown, and malformed input;
- absence of heap use and hardware/library dependencies, plus strict host,
  ASan/UBSan, style, header, Mega compile, resource, PDF, site, and publication
  gates.

The canonical Mega replay is compile-only and writes named result cells for
lifecycle/session identity, `ModuleBenchState`, `ModuleBenchScriptStep`,
`faultDominant`, `ModuleComparatorRelation`, `recordPrepared`, and codec
validity.

Residual SRAM uses the conservative formula
`8192 - measured static SRAM - conservative synchronous stack - 128`, where
128 bytes is reserved interrupt margin. During planning, the provisional AVR
estimate reserved 720 bytes for the coordinator and 384 bytes for simultaneous
staged and destination images. That estimate preserved the initial capacity
rationale but is superseded by terminal AVR-target evidence: the implemented
bench measures 436 bytes, while the two simultaneous images remain exactly
384 bytes.

Future physical acceptance is separate. It must record the exact specimen,
authoritative or verified circuit evidence, pinout, supply, pull rail, ADC
reference/source impedance, current, inactive and backfeed behavior,
prediction, named analog/digital/power/ground test-point observations,
interpretation, resource acquisition, and measured safe state. Passing the E0
record cannot pre-fill or satisfy that bench record.

## Prior-decision impact

- Lesson 070 descriptor ownership is **preserved**: Lesson 072 validates and
  presents the descriptor but never redetects module type from sample shape.
- Lesson 071 characterization ownership is **preserved**: Lesson 072 consumes
  its complete result and does not run a private classifier.
- Earlier light, sound, Hall, thermal/radiant, touch, vibration, obstacle,
  resistive-probe, and monitoring policies are **preserved**: a generic
  characterization does not replace their application semantics or inherit
  their physical claims.
- The authorized-inventory admission rule is **preserved**: a family listing,
  seller alias, or plausible trace is not electrical identity or permission to
  energize a board.
- Lessons 022/024 are **not entered**: Lesson 072 ends at one caller-owned
  volatile 192-byte value and defines no persistence workflow.
- Lesson 079 low-side-driver ownership is **preserved**: Lesson 072 publishes
  no sensor-power output or switched-power implementation.
- Lesson 081 qualification scope is **preserved**: Lesson 072 describes one
  copied characterization run and does not issue an acceptance certificate.
- Circuit-native observation policy is **preserved** through typed result and
  image cells at E0. Exact display/indicator/control endpoints and measured
  acquisition/safe state remain separate E1a/E1b gates.

## Terminal gate result

- Disposition: `natural fit after bounded remediation` for an E0 composition
  that consumes one atomic Lesson 070/071 envelope, fixes one declared fixture
  per session, advances the exact five-step review script, preserves
  conservative categorical faults, and prepares exactly one canonical
  volatile 192-byte
  characterization record through the shared codec
- Exact resource evidence: fingerprint
  `b56bd8ef7a12328b80ad613b2a8b41f2cbde8e6fbd5ff0aa408208f50e3b6679`;
  27,354 B flash, 2,002 B static SRAM, 740 B synchronous stack, 436 B bench,
  exact 192 B record and 384 B simultaneous images, and 5,322 B residual SRAM.
  Flash is an independently accepted target miss with 5,414 B hard-limit
  margin; every other target and hard gate passes.
- Open risks: exact admitted specimens, ADC reference/source impedance, pull and output
  levels, switched-power current/backfeed/safe state, and powered presentation
- Required discussion or decision IDs: none for the bounded E0 shape;
  automatic detection, runtime module switching, generic arbitrary-module
  admission, calibrated physical units, actual endpoint ownership, any
  workflow beyond the caller image, or acceptance certification requires a
  new architecture decision
- Completed remediation: coordinator retains a compact snapshot rather than a
  full envelope; duplicate evidence storage was removed; point/decode phases
  are lifetime-isolated; decode/publish is an out-of-line stage. The stabilized
  semantics keep static SRAM and stack below target while retaining exhaustive
  corruption and repaired-CRC semantic rejection evidence.
- Verification results: strict host runtime and style pass; production digest
  goldens, zero digest, every-byte corruption, CRC-repaired semantic
  boundaries, digest-correct forgery, correlated terminal fault, lifecycle,
  chronology, atomicity, canary, and exhaustion tests pass; independent
  semantic review passes.
- Maximum-composition scenario and proof: canonical Mega replay executes real
  Lessons 070 and 071, admits one atomic envelope, advances all five steps,
  and retains two simultaneous 192-byte images for encode/decode comparison.
- Promotion permitted: yes for copied E0 software/documentation only; no
  physical support, storage, calibration, qualification, or acceptance claim
