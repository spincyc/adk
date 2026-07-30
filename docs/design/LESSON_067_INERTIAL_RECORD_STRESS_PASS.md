# Lesson 067 inertial-record architecture stress pass

Status: initial E0 architecture disposition. The record boundary is approved
for implementation only as a pure copied-value normalizer with a normative
fixed byte image. Implementation, exact resource evidence, independent
review, publication gates, and every powered-adapter claim remain open.

This pass reviews the Lesson 067 subject fixed by the
[extended component/project cadence](../projects/component_project_cadence.md):
inertial record normalization. The cadence's phrase “Lesson 043 adapters” is
not an implementation fact: Lessons 043--045 published copied synthetic
inertial evidence and deliberately left exact MPU6050 and QMI8658 adapters
open. Lesson 067 therefore consumes copied `InertialSample` values and
revision-specific fixtures. It neither reaches through
`InertialObservationPolicy` nor invents either missing adapter.

The component is a natural fit only as a source-frame record-envelope
boundary. It preserves source identity, configured ranges, calibration and
configuration revisions, observation identity, ready state, saturation, and
producer status in one canonical representation. Lesson 068 owns source
qualification and configured axis mapping. Lesson 069 owns session recording
and comparison.

## Boundary

- Name and lesson: `NormalizedInertialRecord`, Lesson 067
- Review state: initial pre-implementation stress pass
- Public responsibility: validate one complete copied `InertialSample`,
  classify its producer outcome, and encode or decode one canonical inertial
  record without losing provenance
- Direct dependencies: the published Lesson 043 `InertialSample`, `Status`,
  `TimePoint`, fixed-width integer values, and a local record codec
- Existing decisions reconsidered: Lesson 043 source-frame units and
  provenance remain authoritative; Lesson 044 board-frame orientation remains
  presentation policy; no old lesson API changes merely to make Lesson 067
  convenient

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | Natural only over the complete copied `InertialSample`. Consuming `InertialObservationPolicy` would import freshness filtering, accepted-sample retention, and its deliberately narrow source policy, obscuring the ready/fault events that a record must preserve. Axis mapping, stationary bias, qualification, voting, failover, presentation, and storage remain outside Lesson 067. |
| Ownership and lifecycle | Prefer a stateless codec/normalizer. Every operation consumes a copied value and fills caller-owned fixed storage synchronously. It retains no source, buffer, clock, callback, transport, or endpoint and uses no heap, virtual dispatch, recursion, or hidden initialization. If implementation introduces mutable lifecycle state, this stress pass must reopen. |
| Time and ordering | `observedAt` and `sequence` are preserved evidence, not a clock read by the component. Lesson 067 validates local field domains but does not infer freshness, repair gaps, reorder records, compare epochs, or establish a session. Duplicate, regression, rollover, and sample-age policy belong to Lessons 068--069. |
| Errors and status | Malformed structure and invalid enum/domain combinations return `Status` and leave output byte-identical. A well-formed producer failure is recordable domain evidence: it becomes `SourceFault` while retaining producer `Status`; it is not returned as a codec failure. `dataReady == false` with a successful producer becomes `NotReady`, not an all-zero sample. |
| Determinism | A normative fixed-width, explicit-endian image is the cross-build identity. Raw structs, padding, native endianness, `bool` representation, `memcmp` over C++ objects, and compiler ABI are never persistence formats. Reserved bytes are zero and covered by the checksum. |
| Resources | Fixed-size records and bounded single-record work are mandatory. Encoding and decoding traverse the image once, perform no search or retry, and use no dynamically sized staging. Exact ordinary/no-LTO flash, static SRAM, stack, public-value size, image size, and residual Mega SRAM are promotion gates. |
| Packaging and public surface | One standalone header/source, umbrella export, strict host target, compile-only Mega replay, exact resource probe, HTML reference, and complementary pencil-drawing PDF. E0 includes no I2C library, register address, sensor driver, pin assignment, wiring table, or formal schematic. |
| Example and documentation fit | The example follows acquire, configure, start and observe, decide, actuate over copied fixtures. Named volatile cells expose record state, identity, readiness, saturation, status, checksum outcome, and round-trip equality without making Serial the only evidence. |
| Downstream effects | Lesson 068 may qualify one explicitly configured source and apply one explicit source-to-qualification-frame mapping. Lesson 069 may bind exact normalized records to a recording session. Neither may reinterpret a digest as provenance, infer an adapter from a model label, or silently discard fault records. |

## Frozen E0 information boundary

The normalized value retains, field by field:

- a nonzero record schema revision and normalization-contract revision;
- `InertialSourceKind`, `InertialModel`, nonzero source identity,
  configuration revision, and calibration revision;
- acceleration range in micro-g and angular-rate range in
  milli-degrees-per-second;
- acceleration in micro-g and angular rate in milli-degrees-per-second for
  all three source-frame axes;
- supplied observation time and producer sequence, including zero after
  modular rollover;
- data-ready state, saturation classification, complete producer `Status`,
  and normalized record state.

The closed record states are:

```cpp
enum struct InertialRecordState : uint8_t
{
    Recorded,
    NotReady,
    SourceFault
};
```

A successful producer with `dataReady == true` produces `Recorded`. A
successful producer with `dataReady == false` produces `NotReady`. A
non-`Ok` producer status produces `SourceFault` regardless of ready or
saturation bits. `SourceFault` and `NotReady` canonicalize readiness false,
saturation `None`, and both vectors to zero so stale or untrusted payload
cannot masquerade as a current sample. State is derived, not trusted as a
second caller assertion.

Source, model, units, ranges, revisions, and source-frame axes are copied
facts. The component does not calibrate values, subtract bias, rotate axes,
convert to pitch or roll, infer gravity, or claim that an enum tag
authenticates a physical device. Defined MPU6050 and QMI8658 source/model
pairs may survive copied fixtures as attribution labels, but E0 support is
still synthetic replay only. An exact powered source remains unsupported
until its separately reviewed adapter proves identity and configuration.

All public enum values and source/model combinations are validated
exhaustively. Zero source identity, sequence, schema revision, normalization
revision, configuration revision, calibration revision, or configured range
is rejected before output mutation. A future schema may relax a field only
through an explicit versioned decoder; zero is not a wildcard.

Vector values are preserved exactly as signed 32-bit normalized units,
including extrema. Lesson 067 performs no negation, magnitude calculation, or
range clamp, so `INT32_MIN` cannot trigger signed-overflow repair. Saturation
is explicit evidence rather than a request to clip a value. Range consistency
checks use widened arithmetic and exact inclusive boundaries.

## Canonical record image

The E0 cross-build image is exactly 64 bytes. Its layout is frozen before
implementation as:

| Bytes | Field |
|---:|---|
| 0--3 | magic |
| 4 | image format version |
| 5 | image length |
| 6--7 | record schema revision |
| 8--9 | normalization-contract revision |
| 10 | record state |
| 11 | source kind |
| 12 | source model |
| 13 | source identity |
| 14--15 | configuration revision |
| 16--17 | calibration revision |
| 18--19 | reserved zero |
| 20--27 | acceleration and angular-rate ranges |
| 28--39 | acceleration vector |
| 40--51 | angular-rate vector |
| 52--55 | observation time |
| 56--59 | producer sequence |
| 60 | flags, including readiness and saturation |
| 61 | producer status |
| 62--63 | CRC-16 over bytes 0--61 |

Every multi-byte integer uses little-endian byte order. Framing validation
checks magic, format version, and length first. CRC validation follows
framing and precedes semantic validation of reserved-zero fields, revisions,
enums, source/model pairing, identity, ranges, state, flags, and status. The
checksum detects
accidental image corruption; it is not authentication, source identity,
durability, or hostile-input protection.

Encoding stages a complete zero-filled candidate image, writes every field,
computes the checksum, and copies it to caller storage only after all checks
pass. Decoding stages a complete candidate value and publishes it only after
the full image validates. On any failure the caller's complete output,
including canaries around it, remains byte-identical. Encoding the decoded
value must reproduce the same 64 bytes; accepting aliases, nonzero reserved
bytes, alternative enum spellings, or noncanonical flags is forbidden.

The C++ value remains field-defined. Its padding, `sizeof`, and object bytes
are neither compared nor persisted. Semantic equality compares every defined
field. A checksum or later digest never substitutes for those fields.

## Failure precedence and atomicity

The deterministic rejection order is:

1. null or incorrectly sized caller storage where the API permits such a
   representation;
2. framing: magic, image version, and image length;
3. CRC mismatch;
4. semantic reserved bytes, schema, and normalization-contract revisions;
5. enum domains and source/model pairing;
6. zero or invalid identity, revision, and range fields;
7. contradictory flags, state, status, readiness, saturation, or range
   evidence; and
8. an internal canonical round-trip invariant.

Framing, then CRC, then semantics is externally visible locked precedence and
may not be reordered as an optimization. A structurally valid producer
`Status` never causes codec rejection. Unknown future enum or revision values
return `Unsupported` without mutation; malformed current values return
`InvalidArgument` or `InvalidConfiguration` according to the public contract.
No failure returns a partially decoded identity or vector.

Range evidence cannot be weakened by saturation. An axis at the configured
positive or negative range boundary is valid; one normalized unit beyond it
is contradictory and rejects. Saturation describes a producer-reported
condition and does not authorize an out-of-domain normalized value.

## Deterministic proof matrix

Host tests must cover:

- all valid and invalid enum values and every source-kind/model pairing;
- synthetic, copied MPU6050-labeled, and copied QMI8658-labeled fixtures
  without claiming either physical adapter;
- zero and maximum valid revisions, ranges, source identities, observation
  times, and sequences;
- all three record states derived from ready and every `StatusCode`;
- all four saturation classifications, each axis, both vector families, and
  exact inclusive range boundaries;
- `INT32_MIN`, `INT32_MAX`, zero, and signed values near each configured
  range without overflow or clamping;
- distinct configuration, calibration, schema, and normalization revisions
  that cannot collide into equal records;
- encode/decode round trips and two independent encoders producing the exact
  same 64-byte image;
- every individual byte corrupted in turn, including header, provenance,
  vectors, reserved byte, and checksum;
- wrong magic, version, length, byte order, reserved values, flags, state,
  and checksum, including collisions that prove framing-before-CRC-before-
  semantics precedence;
- complete output and surrounding canaries unchanged after every rejected
  encode and decode;
- proof that source faults and not-ready records survive round trip rather
  than reusing the last successful vector;
- proof that raw C++ object layout is not used by the codec; and
- the maximum linked Lessons 043--045 plus Lesson 067 E0 replay under ordinary
  and exact no-LTO resource probes.

The compile-only Mega replay uses copied fixtures and a caller-owned
64-byte image. It exposes volatile state, source identity, vector, producer
status, checksum acceptance, and round-trip-equality cells. It owns no pin and
must not be presented or uploaded as an inertial-device driver.

## Initial resource gates

These are pre-implementation ceilings. The Lessons 067--069 plan may tighten
targets but may not raise a hard limit without reopening this stress pass.

| Metric | Target | Hard limit |
|---|---:|---:|
| ordinary Lesson 067 sketch flash | 10 KiB | 14 KiB |
| ordinary static SRAM | 768 B | 1,024 B |
| isolated synchronous stack | 320 B | 448 B |
| normalized public value | 96 B | 128 B |
| canonical image | 64 B | 64 B |
| codec mutable object state | 0 B | 0 B |

The exact probe records ordinary and no-LTO flash/static SRAM, public value
and image sizes, one caller-owned image instantiated once, compiler-callgraph
synchronous stack including retained return-address edges, residual Mega
SRAM, and the complete compiler/core/flag fingerprint. Heap use, recursion,
indirect calls, unknown callgraph edges, dynamic stack, stale reviewed target
markers, an image other than 64 bytes, or mutable codec state fails the gate.
Hard-limit and residual-hard-floor failures are not reviewable.

## E1 reopen triggers

Any of the following requires a separate exact-specimen or composition review:

- naming an exact MPU6050, QMI8658, carrier PCB, address, register-map
  revision, range encoding, calibration procedure, or conversion scale;
- owning SDA, SCL, interrupt, reset, address-strap, sensor-power, timer, or
  bus resources;
- probing an address, writing configuration, reading identity or sample
  registers, retrying transport, or selecting a source at runtime;
- claiming that a source/model enum, inventory label, or copied fixture proves
  physical identity;
- adding axis mapping, bias removal, gravity/orientation inference,
  qualification, voting, fallback, or failover;
- retaining caller storage, buffering multiple records, opening media,
  acknowledging persistence, repairing torn writes, or claiming durability;
- changing normalized units, image layout, checksum algorithm, schema
  compatibility, or the meaning of a revision; or
- using the checksum as authentication or safety evidence.

An initial E1 adapter must first establish the exact chip and carrier
revision, primary register map, supply and logic levels, regulator and
level-shifter population, pull-ups and address straps, identity/reset
behavior, configured ranges, byte order, scale conversion, data-ready
semantics, saturation behavior, axis convention, cadence, and named physical
observation points. MPU6050 evidence cannot qualify a QMI8658 variant, and the
reverse is equally prohibited.

## Initial gate result

- Disposition: `natural fit` as a stateless E0 source-frame record normalizer
  and canonical codec
- Promotion status: architecture only; implementation and publication remain
  open
- Closed scope: copied-value validation, lossless provenance, derived record
  state, exact canonical image, CRC corruption detection, and atomic
  encode/decode
- Explicitly open: source qualification, axis mapping, comparison, session
  recording, persistence, presentation, every exact powered adapter, and
  bench acceptance
- Required remediation: the Lessons 067--069 implementation plan and cadence
  must stop referring to nonexistent Lesson 043 adapters and instead name
  copied revision-specific fixtures plus future independently gated adapters
- Next action: freeze the companion plan/API against this boundary, implement
  the deterministic proof matrix, measure the exact resource tuple, and run a
  terminal post-implementation stress pass before publication
