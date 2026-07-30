# Lessons 073--075 RTC integrity rescope decision

Status: curriculum subject selected; implementation not authorized.

## Decision and provenance

Lessons 073--075 are reassigned from the unauthorized DS1302, BMP180, and
PCF8591 subjects to one RTC-integrity arc:

| Lesson | Selected subject | Learner result |
|---:|---|---|
| 073 | Copied RTC Transaction Evidence | Inspect one attributable, bounded DS1307-family register transaction and distinguish framing, integrity, semantic, and source faults without owning a bus |
| 074 | Qualified Clock Observation | Decide whether copied clock observations from one declared source are coherent, fresh, monotonic under the stated policy, and suitable for a named use |
| 075 | Inert Time-Warp Detective Desk | Reconcile copied transaction and qualification evidence into an explainable case record that identifies unset, stopped, stale, discontinuous, rollback, and transport-fault scenarios |

This selection is authorized by the deduplicated inventory union in
[`AUTHORIZED_ELEGOO_SET.md`](../inventory/AUTHORIZED_ELEGOO_SET.md): the Mega
manifest and the Upgraded 37-in-1 manifest both list the DS1307 RTC family.
That listing authorizes curriculum planning only. It does not identify an
exact PCB, charging circuit, cell, pull-up network, voltage limit, bus address,
or powered behavior. The repository already publishes an owned `I2cBus`
transport model and the abstract `Rtc`, `ClockReading`, and `ClockState`
contracts from Lessons 022--024, so this arc teaches evidence integrity before
adding any physical adapter.

The engagement order is deliberate. Lesson 073 begins with the concrete
forensic artifact learners can inspect byte by byte. Lesson 074 generalizes
that evidence into a clock-source qualification rule without teaching one
chip as universal. Lesson 075 supplies the visible payoff: a detective desk
that explains why a clock cannot be trusted instead of merely printing a
timestamp. The project remains inert and replayable while making failure
diagnosis the central activity.

## Alternatives rejected

| Alternative | Disposition |
|---|---|
| Retain DS1302 in Lesson 073 | Rejected: DS1302 is absent from the authorized inventory union and is not interchangeable with the listed DS1307 family. |
| Retain BMP180 or PCF8591 in Lesson 074 | Rejected: neither family is present in the authorized inventory union. |
| Treat a documented DS3231 shipping alternative as a DS1307 | Rejected: the inventory explicitly preserves DS1307 and DS3231 as distinct variants. Their register maps, status semantics, oscillator behavior, electrical topology, and module hazards require separate identities and adapters. |
| Begin with a powered DS1307 adapter | Rejected for this arc: exact specimen and primary electrical evidence are not ready, and listing authorization is not permission to energize hardware. |
| Make Lesson 074 DS1307-specific | Rejected: qualification concerns source identity, provenance, sequence, freshness, and clock behavior. Keeping that policy source-neutral permits later DS1307 or separately qualified DS3231 evidence without flattening their transports. |
| Use the existing `Rtc` result as sufficient transaction evidence | Rejected: `ClockReading` intentionally hides register framing and provenance, while Lesson 073 must preserve the copied evidence needed to explain acquisition and interpretation failures. |

## Evidence and safety boundaries

The promoted software scope, if a later implementation-depth plan passes
review, is E0 only. Tests and examples may consume deterministic, copied
DS1307-shaped register fixtures and caller-supplied time. No component in this
arc may claim a bus, address, pin, timer, interrupt, battery, cell, charging
path, oscillator, display, storage medium, or physical RTC. Register-shaped
bytes are evidence fixtures, not proof that a DS1307 produced them.

Lesson 073 must retain the copied transaction's declared source identity,
register start, requested length, returned prefix, transaction status,
sequence/correlation fields, and observation time before decoding calendar
fields or status bits. The implementation plan must obtain the exact DS1307
register rules from a named primary datasheet, freeze its decoding and fault
precedence, and test malformed BCD, invalid ranges, short reads, extra bytes,
transport failure, stale and mismatched correlation, rollover, and every
retained source/status distinction. It must not imply that host replay
performed I2C.

Lesson 074 must qualify observations from exactly one declared source per
qualification session. Its policy must be source-neutral: it consumes copied
clock observations plus explicit provenance and supplied time, not DS1307
registers or a concrete `Rtc`. The implementation plan must define bounded
sample storage, admission and terminal states, freshness, duplicate and gap
handling, ordering and wrap rules, rollback/discontinuity classification,
failure precedence, restart semantics, and the evidence needed to justify
each outcome. Qualification is fitness for one named educational use, not
accuracy, calibration, traceability, or wall-clock truth.

Lesson 075 must compose the preceding evidence without silently re-decoding
the source or weakening its qualification result. The inert detective desk
must produce bounded semantic presentation and volatile record intent with
attributable explanations for at least valid, not-set, oscillator-stopped,
transport-fault, stale, rollback, forward-jump, source-change, and evidence-
integrity cases. Any display or durable record remains intent or caller-owned
memory at E0. Serial may supplement the replay but cannot be described as a
physical observation path.

E1 remains wholly open. A future powered adapter requires an inventoried exact
specimen, primary datasheets for the IC and relevant module circuitry,
verified cell chemistry and polarity, charging-path disposition, pull-up
rails, voltage compatibility, address and register identity, bus ownership,
authoritative schematic, current and safe-state evidence, non-Serial
observation, fault injection, and a recorded Mega 2560 bench result. DS3231
work requires its own source and specimen gate; a passing DS1307 bench card
cannot accept it.

## Existing RTC migration seam

The current public `Rtc::read()` returns `Result<ClockReading>`, and
`ClockState` distinguishes `Valid`, `NotSet`, `OscillatorStopped`, and
`TransportFault`. Existing consumers include the greenhouse and magnetic
passage work. This is a real migration seam: Lesson 073 needs richer copied
transaction evidence, while Lesson 074 needs provenance and qualification
history that the compact `ClockReading` does not carry.

This decision does **not** choose whether to preserve `Rtc` unchanged behind
an adapter, extend it, add a parallel evidence-producing contract, or migrate
consumers. The implementation-depth plan must inventory every current
consumer and serialization dependency, exercise the most demanding
composition, compare bounded alternatives, and record compatibility and
migration consequences before fixing a public shape. No new fields or states
may be opportunistically inserted into `Rtc`, `ClockReading`, or
`ClockState`.

## Source-readiness matrix

| Evidence source | Ready for this decision | Required before implementation or E1 |
|---|---|---|
| Official Elegoo manifests preserved in `AUTHORIZED_ELEGOO_SET.md` | Yes: establishes DS1307 family listing and DS3231 alternative provenance | Exact shipped specimen still required |
| Existing Lessons 022--024 design and `src/rtc.h` | Yes: establishes current transport and clock abstraction seam | Full consumer/migration analysis required before API change |
| DS1307 primary IC datasheet | Not yet accepted by this decision | Named edition, durable citation, register map, BCD/range rules, clock-halt behavior, address and electrical limits required before Lesson 073 implementation |
| Exact DS1307 module schematic or equivalent primary module evidence | No | Required before wiring, charging-path, pull-up, cell, voltage, or E1 claims |
| Exact specimen inspection and bench record | No | Required for electrical identity and every E1 support claim |
| DS3231 primary and exact-module evidence | No | Required for any DS3231-specific fixture, adapter, electrical claim, or interchangeability assessment |

## Dependencies and activation gates

The implementation-depth plan must depend explicitly on the published
Lessons 022--024 bus/RTC contracts and their current consumers, the copied-
evidence and provenance patterns used in Lessons 064--072, the fixed-capacity
and no-heap rules, the architecture stress-pass template, the safety model,
the PDF visual policy, and exact Mega 2560 resource budgets. It must specify
public types, lifecycle, status and failure precedence, record images if any,
resource probes, deterministic golden fixtures, every-byte corruption where
an encoded image exists, example narrative, HTML and pencil-drawing PDF
outcomes, and open physical acceptance cards.

Activation requires all of the following:

1. a primary-source-backed DS1307 register interpretation suitable for copied
   fixtures;
2. a reviewed decision for the `Rtc`/`ClockState` migration seam;
3. pre-implementation architecture stress passes for Lessons 073, 074, and
   075, including aggregate SRAM/stack/flash pressure and existing consumers;
4. frozen E0 provenance, correlation, freshness, discontinuity, and failure
   semantics with deterministic acceptance vectors;
5. explicit separation of DS1307 and DS3231 identities throughout code,
   tests, examples, and prose; and
6. an implementation-depth three-lesson plan reviewed before first-class
   code, examples, lessons, navigation, or release metadata change.

Acceptance of this rescope means only that the lesson numbers, subjects,
ordering, authorization rationale, and planning boundaries are durable. It
does not claim an implementation, a supported DS1307 or DS3231 adapter,
electrical qualification, powered observation, clock accuracy, persistence,
or physical acceptance.
