# Lesson 049 identity-record implementation stress pass

This record applies the
[component design stress pass](../templates/component-design-stress-pass.md)
to the implemented Lesson 049 identity-record and bounded-enrollment boundary.
It is the terminal post-implementation E0 architecture review, not an RFID
adapter, storage device, authentication system, wiring, or physical-acceptance
claim.

## Boundary

- Name and lesson/project: local identity records and bounded enrollment,
  Lesson 049
- Reviewer and date: post-implementation architecture review, 2026-07-28
- Public types and operations: `LocalIdentity`, `IdentityEvidence`,
  `IdentityBinding`, two caller-owned 160-byte canonical slots,
  `IdentityImageView`, compact `EnrollmentCandidate`,
  `IdentityDurableCommitEvidence`, enrollment/query outcomes, lifecycle
  operations, and an inspectable snapshot
- Direct dependencies: fixed-width values, `Status`/`Result<T>`, explicit
  `TimePoint`/`Duration`, fieldwise copied observations, and the existing
  versioned-record/checksum conventions
- Existing decisions and interfaces reconsidered: identity is evidence rather
  than authentication; keypad events and RFID frames are qualified outside
  this policy; storage lifetime and commit are explicit; no heap, hidden I/O,
  hidden clock, plaintext credential, or unbounded collection is introduced

The proposed E0 boundary is a pure policy over copied observations and imported
or exported byte images. It owns no RFID reader, keypad, SPI bus, EEPROM, SD
card, pin, timer, interrupt, or endpoint. An identifier is a bounded opaque
byte sequence plus source kind, source ID, source configuration revision,
observation sequence, and observation time. It is not a person's identity, a
secret, a credential, or proof of possession. A local bin ID is mapping
metadata and never participates in identity equality.

The directory has a compile-time maximum number of entries and identifier
bytes. Empty, known, unknown, duplicate, full, locked-out, malformed-source,
stale-source, corrupt-image, unsupported-version, and storage-pending outcomes
remain distinct. Enrollment requires an explicit `previewEnrollment()` call
with a valid bin; merely observing an RFID UID can never enroll it. A candidate
allows at most one matching externally acknowledged commit and is cleared by
cancellation or a non-reconciling lifecycle transition. A failed or
indeterminate external commit instead preserves its exact retry identity
across shutdown so reinitialization can reconcile it.

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural with a strict copied-evidence boundary.** RFID framing, CRC, anti-collision, transport errors, electrical lifetime, and SPI ownership belong to a future qualified adapter. Key scanning remains in `Keypad` for the later project. Lesson 049 compares copied identifier bytes, stages an explicitly requested identity-to-bin enrollment, and publishes local policy outcomes. It must not expose MFRC522 registers, card-family semantics, keypad pins, or storage-sector details. |
| Ownership and lifecycle | **Natural for E0; fixed by the detailed plan.** Construction copies the small configuration and borrows only explicitly sized storage. `initialize()` validates capacity `1..8`, two 160-byte slots with extent `320` and stride `160`, and one nonoverlapping candidate buffer with capacity exactly `160`. The registry is the exclusive candidate-byte writer. Compact handles and bounded read-only views expire on cancellation, reset, shutdown, successful reconciliation, or replacement. `reset()` preserves committed bindings and source replay identity but rejects atomically while reconciliation is required. `shutdown()` always publishes inactive state while preserving a retryable reconciliation latch/scratch identity; reinitialization reconciles supplied slots before new work. |
| Time and ordering | **Natural and fixed at design depth.** Every observation and enrollment preview receives supplied time. The plan defines maximum evidence age, saturating unknown-attempt lockout, exact-boundary expiry, source-sequence forward/equal/ambiguous/regressing rules, rollover, and changed same-sequence failure. One call performs bounded validation and at most one table mutation; repeats create no event, and there is no scan-until-empty, retry loop, or hidden debounce. |
| Errors and status | **Natural with domain outcomes separated from operation failure.** The fixed disposition set distinguishes known, unknown, duplicate, lockout, directory full, authorization required, enrollment pending, malformed, corrupt/unsupported image, indeterminate commit, and storage fault. `IdentityDurableCommitEvidence` separately carries synchronization, reread-validation, and durable status instead of collapsing them to a boolean. Corruption is never silently treated as empty, an unknown identifier is not authentication failure, and reconciliation failure cannot be overwritten by ordinary observation. |
| Resource budget | **Measured and passing.** The implementation retains 4--10 identifier bytes, eight bindings/bins, two 160-byte canonical image slots, and one 160-byte candidate image. `sizeof(LocalIdentityRegistry)` is `121 B`, below the `128 B` reusable-component hard ceiling. The clean Mega replay measures 7,026 B flash against 16,384 B and 1,113 B static SRAM against 1,280 B. Its conservative path is 357 B plus 128 B ISR, within the reviewed 512 B allowance. The measured Lesson 051 aggregate separately proves the consuming composition fits. |
| Deterministic proof | **Executed and passing at E0.** Tests inject copied identities, timestamps, all 160 image bytes, extents/strides/overlaps, views, and rich commit evidence. They cover every field offset and CRC boundary, endian vectors, configuration mismatch, flags/reserved/unused canonicality, erased/malformed slots, handle binding, view invalidation, reconciliation failures/latch/idempotence, time boundaries, reset/shutdown/restart, fieldwise replay, traits, and AVR sizes. They also prove known snapshots expose the exact committed nonzero binding revision/image generation, every other disposition zeros revision, downstream audit retains that admitted pair despite later storage changes, and noncanonical absent identity/key payloads reject atomically. Strict native and sanitizer executions pass. |
| Packaging and public surface | **Implemented as ordinary first-class E0 code.** The standalone header, out-of-line implementation, native/archive inventories, umbrella export, tests, canonical Mega replay, measured size baseline, HTML reference, and pencil-drawing PDF are present. The public API uses semantic records and byte-image import/export; it adds no RFID library, EEPROM singleton, template container framework, or lesson-only build path. |
| Example and documentation fit | **Published as an E0 replay.** The canonical example acquires fixed fixture storage, configures bounded policy, starts, then observes copied identity evidence, decides enrollment/query, and exposes result cells. The HTML and PDF preserve the same vocabulary and limitations. A future E1 fixture may add a qualified reader, status LEDs/display, and named bus/test points; keypad input belongs to the Lesson 051 composition. Every non-schematic PDF visual is a pencil drawing; only an electrically authoritative qualified circuit may be marked as a formal schematic. |
| Downstream effects | **Contained only if the directory remains local policy.** Lesson 050 consumes no identity details. A `Known` Lesson 049 snapshot publishes the exact nonzero matched binding revision and image generation; every other disposition publishes zero revision. Lesson 051 copies that admitted pair into authorization/audit candidates rather than rereading mutable bindings, and still may not reinterpret raw UID bytes as authorization. Its absent identity/key fields are canonical zero/status/source values, not ignored storage. Lessons 016--018 retain keypad and inert access-policy authority; Lessons 022--024 retain bus/storage authority. |

## Composition pressure scenario

The maximum authorized E0 scenario uses a full eight-binding directory, a
ten-byte admitted identifier, one copied synthetic-identity fixture, one
explicit enrollment preview, two canonical versioned directory images, one
candidate scratch image, and compact status/result cells. It then runs
inside the implemented Lesson 051 host composition with bounded homing, stepper
intent, gate intent, display/LED intent, and one audit-record retry.

The maximum collision begins at the lockout-expiry boundary with a full
directory. It injects a repeated known identifier, a different identifier with
the same source sequence, a corrupt newer image beside a valid older image, a
foreign acknowledgement, and a failed persistence commit. It then crosses
timestamp rollover, restarts, replays the held identifier, queries the
recovered directory, and attempts a carousel request. The result must retain
the last valid directory, consume no candidate after a rejected
acknowledgement, publish no gate or motion authority, and attribute corruption
separately from the ordinary full/duplicate outcome.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Applicable; bounded replay passes.** One call performs at most one eight-entry comparison, one complete candidate/image validation, lockout checks, and snapshot publication. Tests replay exact expiry, one tick on each side, rollover, missed calls, rapid repeats, and the Lesson 051 collision. No image checksum, diagnostic, or persistence retry may starve stop handling. |
| Total memory and hardware resources | **Applicable; E0 measurements pass.** The Lesson 049 replay and maximum Lesson 051 E0 ELF account for retained objects, two recovery slots, candidate scratch, snapshots, fixed buffers, stack, and the core/ISR allowance against their numeric gates. E0 owns zero hardware resources. Future reader/keypad/storage/display compositions require a separate SPI/GPIO/timer/interrupt/claim/current and observation-path budget. |
| Shared bus or transport | **Not applicable at E0.** Copied fixtures and imported byte images perform no transport transaction. A future MFRC522 adapter makes this row applicable: one `SpiBus` owner, explicit borrower lifetime and chip select, bounded framed operations, CRC/timeout behavior, contention, rollback, failure, and restart must be designed and qualified independently before wiring or E1 claims. Storage sharing requires the same explicit owner/order analysis. |
| Persistence and recovery | **Applicable; supplied-image proof passes and physical backing remains open.** The implemented 160-byte codec fixes little-endian fields, magic `0x4944`, version 1, zero flags/reserved bytes, nonzero registry-configuration ID, generation, entry count, eight 16-byte entries, and CRC-16/CCITT-FALSE coverage. Tests prove extents, stride, nonoverlap, foreign-configuration rejection, canonical unused entries, erased-slot treatment, generation ordering, and fail-closed recovery. Rich evidence must match the inactive slot, metadata, and reconciled bytes and prove synchronization, reread validation, and durable success. `acknowledgeExternalCommit()` is the sole atomic install boundary; there is no later fallible commit. E0 proves supplied-image behavior only, not physical-media atomicity, wear, retention, or power loss. |
| Motion, external power, or stored energy | **Not applicable to Lesson 049.** The policy has no actuation path and cannot emit motion, coil, servo, or gate-open intent. In Lesson 051, an identity match is only a request prerequisite; independent confirmation, successful homing/position, stop/fault checks, and command expiry remain mandatory before any inert actuator intent. |
| Observation identity and provenance | **Applicable and fixed at design depth.** Equality uses length plus 4--10 bytes only within `SyntheticIdentity`; bin, table position, SPI address, card hints, checksum, and presentation are not identity. Each observation preserves source kind/ID, configuration revision, sequence, timestamp, identity, and status. Padding beyond length is canonical zero; all-zero and all-`0xff` sentinels are invalid. A known result binds bin, nonzero binding revision, and image generation in one snapshot; non-known results canonically zero the revision. Lesson 051 copies that snapshot provenance once and requires canonical zero/status/source forms when identity or key evidence is absent. Changed content at one sequence faults atomically, delayed values cannot be restamped, and replay compares fields rather than padding. |
| Diagnostic interference | **Applicable in the consuming project; open.** Result cells, LEDs, LCD, optional Serial, trace bytes, and persistence/audit status share time and memory budgets but provide no enrollment or carousel authority. Filling, disabling, or failing diagnostics cannot change equality, consume a candidate, alter the directory, convert unknown to known, or suppress stop/fault behavior. An enrollment LED reports policy state; it does not prove a durable write. |
| Failure collision and recovery | **Applicable; precedence replay passes.** Structural/source/time/image failures dominate lockout and never increment it. Known evidence clears attempts only after lockout expiry. Preview rejects known identity, occupied bin, stale evidence, full table, or active lockout without mutation. Negative, indeterminate, foreign, stale, or mismatched acknowledgement preserves the prior table and retryable candidate; only a matching acknowledged candidate commits. Ordinary reset/restart does not resume a volatile preview or carousel request; required reconciliation preserves only the exact retry identity needed to resolve an exposed commit. |

Capacity proof covers zero, one, maximum-minus-one, maximum, and
maximum-plus-one entries; identifier widths immediately below, at, and above
their limits; empty and all-`0xff` images; every-byte corruption;
truncation and trailing bytes; unsupported versions; generation equality and
wrap; duplicate bytes under different bins; and hash/checksum collisions
where applicable. Equality remains an exact bounded byte comparison, never a
checksum comparison.

The persistence transaction remains intentionally split:

1. policy preflights a candidate directory without changing live state;
2. it exports a bounded view of the complete 160-byte canonical candidate;
3. an external persistence coordinator attempts durable commit;
4. the coordinator synchronizes, rereads, validates, and returns rich evidence
   identifying the exact operation, generation, slot, and reconciled bytes;
5. `acknowledgeExternalCommit()` validates every binding and byte, then
   atomically installs that exact table, advances generation, consumes the
   candidate, and clears reconciliation-required;
6. exact duplicate success evidence is idempotent without mutation; and
7. negative, indeterminate, foreign, stale, wrong-slot, byte-mismatched, or
   failed evidence retains the prior table and retryable candidate, latches
   reconciliation-required, and prohibits unrelated work.

This is a Lesson 049-local two-phase protocol, not a change to `Storage`.
Retrofitting random access, rollback, or read/recovery semantics into the
published append/sync interface merely to simplify this lesson would challenge
Lessons 022--024 and is prohibited without a separate architectural decision.

## Prior-decision impact

- Four-layer dependency direction and endpoint-owned electrical lifetime:
  **preserved**; Lesson 049 is pure component policy at E0.
- `Status`/`Result<T>`, no heap/exceptions/RTTI, fixed storage, and explicit
  lifecycle: **preserved**.
- Supplied time, bounded work, stable snapshots, rollover, and fieldwise
  same-time identity: **preserved**.
- RFID UID-shaped values as copied evidence, not authentication:
  **preserved**. No security, access-control, uniqueness, privacy, or personal
  identity claim is added. Lesson 051 keypad events remain independent copied
  confirmation evidence.
- Existing `Keypad` semantics and the Lesson 018 inert access trainer:
  **preserved**. Lesson 049 consumes no key events and does not duplicate
  scanning, debounce, chord, or credential policy; Lesson 051 owns its copied
  confirmation-event interpretation.
- Existing `Storage::append()`/`sync()` and fixed-storage fake:
  **preserved** by keeping image recovery and commit coordination outside the
  storage interface. Adding read/random-write/transaction methods would be
  **challenged** and requires separate discussion and migration analysis.
- Existing versioned fixed-record precedent:
  **extended** to a bounded multi-entry image with canonical encoding,
  generation, and checksum. The format is lesson-local until a second concrete
  consumer demonstrates a shared abstraction.
- Presentation isolation and circuit-native evidence:
  **preserved**. LEDs/LCD/Serial never decide equality or durability.
- No hidden persistent plaintext credential:
  **preserved**. UIDs and bin mappings are local identifiers, nevertheless
  documentation must warn that exported images disclose them.
- Lessons 047--048 volatile position and lack of homing:
  **preserved**. Lesson 049 introduces neither position nor motion and does not
  make a UID sufficient carousel authority.
- Exact specimen and primary-source qualification before powered adapter,
  wiring, schematic, or physical claims: **preserved**.
- Pencil presentation for every non-schematic PDF visual and formal-schematic
  classification only for qualified authoritative circuits: **preserved**.

## Rejected alternatives

1. **Treat UID as authentication or a secret.** Rejected because ordinary RFID
   identifiers may be copied or observed and the curriculum expressly limits
   them to bounded local identifiers.
2. **Enroll on first sight or on a held/repeated observation.** Rejected
   because it bypasses explicit `previewEnrollment()`, makes transport
   repetition mutate policy, and creates unsafe downstream ambiguity.
3. **Use labels, card-family hints, reader address, array order, or checksum as
   identity.** Rejected because each is metadata, location, or lossy evidence.
4. **Silently replace a corrupt directory with empty state.** Rejected because
   it converts lost provenance into successful initialization and may remap
   bins unexpectedly.
5. **Mutate the live directory before persistence succeeds.** Rejected because
   restart could disagree with the visible enrollment result and Lesson 051
   could act on an identity that never became durable.
6. **Expand `Storage` with lesson-specific read, overwrite, or rollback.**
   Rejected at this boundary because it changes a published resource contract
   and several consumers before a second concrete need is designed.
7. **Create a generic identity framework.** Rejected because one RFID/keypad
   lesson does not justify a cross-domain abstraction and product identity,
   sensor provenance, and local UID lookup have materially different rules.

## Stress disposition

### Post-implementation review

The repaired implementation-depth plan closes the pre-implementation
architectural strain without changing a published dependency:

- exact public values, capacities, source domain, identifier validity and
  equality are fixed;
- caller-owned live storage, two 160-byte recovery slots, and one exclusively
  written 160-byte candidate buffer have explicit lifetimes;
- canonical image fields, checksums, unused bytes, generation ordering,
  corrupt/torn/erased handling, and fail-closed recovery are fixed;
- compact preview handles, bounded export-view lifetime, rich external durable
  evidence, atomic acknowledgement/install, cancellation,
  foreign/stale/mutated candidate rejection, reconciled-byte comparison, and
  retry behavior have one owner and exact ordering;
- lockout counting, exact expiry, source replay, changed same-sequence,
  rollover, future/ambiguous/regressing time, reset, shutdown, and restart are
  fixed;
- Lesson 051 receives only a stable local bin mapping and must independently
  satisfy confirmation, durable admission, current-session home, exact
  position, stop, freshness, and health before gate intent;
- object, flash, static SRAM, stack/ISR, and aggregate remaining-SRAM gates are
  numeric; and
- the exhaustive host, packaging, example, HTML, pencil-PDF, and E1/E2
  separation gates are enumerated.

The subsequent normative byte-contract repair extends, rather than replaces,
that closure:

- the 160-byte image and 16-byte entry layouts now fix every offset, width,
  unsigned little-endian encoding, flag, reserved byte, unused entry, and
  erased-slot representation;
- nonzero `registryConfigurationId` binds recovered and candidate images to
  one configuration, so a checksum-valid foreign image cannot be admitted;
- entry and whole-image CRC-16/CCITT-FALSE parameters and exact coverage are
  normative and remain corruption detection rather than security;
- slot extent `320`, stride `160`, count two, candidate capacity `160`, and
  nonoverlap reject undersized, oversized, aliased, or ambiguous storage;
- rich acknowledgement reconciliation is now the single atomic install
  boundary; no public `canCommit()` or later `commit()` seam remains;
- failed or indeterminate reconciliation latches required recovery and blocks
  observe, preview, cancel, reset, and replacement while preserving the exact
  retry identity across shutdown; and
- exact duplicate successful acknowledgement is idempotent only when all
  handle fields, slot/view metadata, checksum, configuration, and reconciled
  bytes identify the already installed image; and
- matched binding revision plus image generation now cross the Lesson 049/051
  boundary as one admitted provenance pair, while non-known snapshots and
  absent project evidence use canonical zero forms.

The implementation matches that closure. The external coordinator contract
requires synchronization and validation of the durable reread before
acknowledgement; the reconciled view identifies the exact inactive slot,
configuration, candidate metadata, and bytes; expired views are unusable;
acknowledgement validation and install form one infallible policy mutation;
canonical encoding persists no struct padding; and all three 160-byte buffers
fit the measured Mega margins. Actual media atomicity, retention, wear, and
power-loss survival remain E1 and do not block the honest E0 publication.

**Natural fit after bounded local remediation.** The repaired plan makes the
copied-identifier directory a natural component-layer extension and confines
durable-enrollment semantics to compact handles and bounded views over
caller-owned canonical bytes with rich acknowledged evidence. No public value
embeds the 160-byte image, so the public `128 B` ceiling and explicit SRAM
accounting can coexist. It neither changes `Storage` nor introduces a generic
identity, transaction, credential, or durability abstraction. No prior public
decision is challenged.

The implemented architecture preserves the retryable candidate without
dynamic memory or ambiguous ownership, makes acknowledgement/install one
bounded mutation, and leaves `Storage`, `Keypad`, `Status`, time, endpoint,
and published record contracts unchanged. The post-implementation pass
therefore permits Lesson 049 E0 promotion. Any future change that weakens those
properties requires a new stress pass and, where shared contracts are
affected, durable architectural discussion before integration.

## Gate result

- Disposition: natural fit after bounded local remediation; terminal
  post-implementation E0 stress pass
- Closed and verified: exact record fields and capacities; source and
  equality rules; lockout and same-sequence/time rules; two-slot canonical
  recovery; normative byte offsets/little-endian encoding/CRC coverage/
  reserved and erased forms/configuration binding; exact extents, stride, and
  nonoverlap; compact handle and bounded-view lifetimes; owner/generation/
  operation/scratch/checksum-bound rich atomic acknowledgement; reconciliation
  latch and exact duplicate idempotence; no complete image in a public value;
  matched-binding-revision/image-generation projection and canonical absent
  evidence; Lesson 051 invariant; deterministic replay; strict native and
  sanitizer execution; measured AVR object/flash/static-SRAM and stack/ISR
  fit; archive/package/umbrella registration; canonical Mega replay; HTML
  reference; and pencil-drawing PDF
- Residual gates: none for Lesson 049 E0 promotion
- Open future gates that do not block E0: exact RFID/keypad/storage specimens,
  transport/electrical qualification, actual nonvolatile atomicity/retention/
  wear/power-loss behavior, circuit-native observation, and all physical
  acceptance
- Residual risks: privacy handling for exported local identifiers and the
  future physical-media and powered-endpoint behavior explicitly retained at
  E1/E2; the E0 tests fail closed on the reviewed codec, ownership,
  reconciliation, canonicality, and SRAM hazards
- Required discussion or decision IDs: none if the local image and explicit
  commit protocol is retained; a durable decision is required before any
  published storage-interface change or authentication/security claim
- Remediation owner and next action: no E0 remediation; retain exact powered
  reader/keypad/storage/display qualification and physical acceptance work in
  the E1/E2 queue
- Verification commands and results: strict native and sanitizer matrices,
  style/format/diff checks, canonical Mega compile, archive/package/umbrella
  registration, size baseline, HTML reference, pencil-drawing PDF gates, and
  the consuming Lesson 051 aggregate review pass
- Maximum-composition scenario and proof: the exact scenario above has
  deterministic fixture coverage and measured Lesson 051 aggregate evidence
- Promotion permitted: yes, for the explicitly inert E0 boundary only
