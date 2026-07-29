# IR command translator design stress pass

This record applies the
[component design stress pass](../templates/component-design-stress-pass.md)
to the proposed Lesson 054 IR command translator before its public shape is
fixed. It bounds the project described by the
[extended cadence](../projects/component_project_cadence.md) and does not
authorize an IR emitter, receiver, remote, keypad, LCD, carrier timer, wiring,
or physical round-trip claim. Lesson 054 may reach E0 using copied fixture
records and logical intent only. E1 remains blocked until the exact adjacent
transmitter and receiver fixtures are independently qualified.

## Boundary

- Name and lesson/project: IR command translator, Lesson 054
- Reviewer and date: pre-implementation composition stress review, 2026-07-28
- Proposed public values and operations: one atomic copied update envelope
  carrying optional cancellation, prepared commit, receive, actual-emission,
  and fault evidence; one immutable
  valid-receive-to-different-local-symbol mapping; one bounded parent
  transaction binding an immutable Lesson 052 evidence generation to one
  Lesson 053 transmit candidate; copied transmit, receive, presentation,
  indicator, and round-trip results; explicit lifecycle and inspectable
  snapshots
- Direct dependencies: owned Lesson 052 decoder, 100-word storage and captured
  evidence policy; owned Lesson 053 known-code emission policy; copied stop and
  actual-emission evidence; `Status`; explicit microsecond time; and
  fixed-capacity storage
- Existing decisions and interfaces reconsidered: receive evidence is not
  transmit authority; unknown-protocol replay is forbidden; only documented
  learner-created harmless codes may be emitted; electrical lifetime belongs
  to endpoints; time and work are explicit and bounded; child candidates are
  owner/generation bound; diagnostics cannot control primary behavior; E0
  intent is not E1 physical evidence

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural only as a project-local coordinator.** The owned Lesson 052 child classifies copied pulse evidence and the owned Lesson 053 child validates and progresses a known local symbol. Lesson 054 may translate only an immutable allowlisted valid receive tuple to a different fixed output symbol, correlate, cancel, and present those semantic values. It exposes no independent local-selection API and must not sample receiver pins, synthesize timer edges, drive an emitter, scan a keypad, write an LCD, or infer an unknown protocol. Raw electrical work remains in separately qualified endpoints and adapters. |
| Ownership and lifecycle | **Natural if the coordinator owns its children and copies every admitted input.** Construction is inert and the coordinator is non-copyable/non-movable. `initialize()` validates configuration and initializes children in dependency order; failure rolls back in reverse order. `shutdown()` first publishes cancel, emitter-off, and inactive-indicator intent, then shuts down children. No snapshot or candidate borrows a caller buffer. Parent and child candidates are private, owner/generation bound, invalidated by reset or shutdown, and cannot outlive the coordinator. |
| Time and ordering | **Natural with one atomic copied update envelope per scheduling boundary.** One supplied microsecond timestamp carries optional cancellation, prepared commit preview, receive evidence, and actual-emission evidence together. Whole-envelope structural validation precedes matching cancellation, independent receive classification, actual-emission correlation, optional still-current commit, ordinary progress, and presentation. Same-time results therefore cannot depend on call order. There is no separate receive/cancel/commit/emission ingress, blocking carrier loop, receive wait, display wait, catch-up loop, hidden clock, or restamping of delayed evidence. |
| Errors and status | **Natural with semantic outcomes over existing `Status`.** Structural invalidity is non-mutating. Semantic results distinguish unlisted valid receive, busy, cancelled, source fault, malformed receive, unknown receive, repeat rejected, self-echo suppressed, receive timeout, attribution mismatch, translated, and diagnostic fault. Stop or cancellation dominates emission and ordinary progress; an already-latched safety/dependency fault remains attributable. Display or indicator failure cannot relabel a transport result. |
| Resource budget | **Measured E0 promotion evidence passes every hard gate with one disclosed and accepted target miss.** E0 owns zero pins, timers, interrupts, buses, ADC channels, registry claims, endpoints, emitters, displays, supplies, or heap. The standalone Lesson 054 sketch measures 16,162 B flash and 1,343 B static SRAM; the maximum composition measures 21,864 B flash and 3,531 B static SRAM, below the 28 KiB/3,584 B targets and 32 KiB/4,096 B hard gates. The coordinator is 407 B, below its 512 B target and 640 B hard ceiling, and borrows a separately reported exact 400 B caller-owned receive buffer. Conservative stack plus ISR reserve is 888 B: it misses the 800 B target but passes the 1,024 B hard gate. Maximum static plus stack leaves 3,773 B of Mega SRAM. A corrected resource decision supersedes decision `55185e25-483f-4f35-9a2a-6f6a6d206cf9` while preserving its bounded acceptance rationale. E1 must separately account for exact timer/carrier ownership, receiver interrupt or sampling resources, keypad, LCD bus/address, LEDs, claim entries, current, supply, and every observation path. |
| Deterministic proof | **Complete for E0.** Strict and ASan/UBSan replay covers mapping and digest boundaries, copied-capability binding, rejection atomicity, rollover/half-range/overflow, exact echo/response/timeout boundaries, cancellation, attribution, reset/shutdown, and fieldwise replay. The repaired 16-mask matrix exercises every combination of cancel, prepared commit, receive, and actual-emission presence with individually valid payloads and the required idle/prepared/active precondition, proving the frozen order-independent outcome for each mask. |
| Packaging and public surface | **Passed for E0.** The declarative project header and out-of-line implementation expose copied commands, transaction identity, dispositions, inert intents, and snapshots without Arduino conditionals, timer registers, GPIO polarity, carrier ISR details, LCD addresses, mutable transmit buffers, or a generic replay API. Native/umbrella registration, canonical Mega sketch, size evidence, HTML, PDF, downloads, lesson routes, and site staging passed terminal review. |
| Example and documentation fit | **Passed with staged E0/E1 claims.** The canonical E0 sketch replays copied fixtures through acquire, configure, start and observe, decide, actuate flow; actuation remains copied logical transmit/presentation intent. Its result cells demonstrate translation, receive, transmit, fault, suppression, and attributable round-trip state. HTML/PDF narrative and links agree with the sketch; rendered PDFs pass metadata, embedded-font, extraction, monochrome, pencil-visual, and policy review. E1 remains limited to future exactly qualified adjacent fixtures. |
| Downstream effects | **Contained if no generic replay or transport abstraction is introduced.** Lessons 025--027 retain receive-only evidence meaning. Lessons 052 and 053 retain their independent capture and listed-emission responsibilities. The project must not expand the safety model into unknown-device replay, access-control operation, protocol discovery, or a reusable half-duplex transport. Any demand for those capabilities challenges curriculum and safety decisions and requires separate user discussion and a durable decision. |

## Composition pressure scenario

The maximum authorized E0 composition is one immutable, deliberately
non-identity receive-to-transmit mapping, one owned Lesson 053 transmit-policy
child, one owned Lesson 052 decoder/evidence child with its 100-word storage,
one atomic copied update envelope with independent cancel, prepared commit,
receive, and synthetic actual-emission cells, one transaction-correlator state,
and copied transmit LED, receive LED, LCD, optical-test-point, and fault
presentation intents. The E0 fixture contains no pin, carrier, receiver,
emitter, timer, interrupt, bus, display, keypad, or powered hardware.

The maximum collision begins with one mapped receive generation prepared while
a prior transmission is completing. At the same supplied timestamp it injects
cancel, a dependency fault, copied receive evidence matching the outgoing
code, a second receive envelope from a foreign source, saturating diagnostic
state, and failed presentation. The coordinator must publish inactive intent,
retain the independent fault and cancellation attributions, reject or suppress
the self-echo according to qualified actual-emission evidence, avoid committing
the stale parent or Lesson 053 candidate, and never turn either received value
into authority outside the immutable mapping.

| Composition pressure | Applicability and required evidence |
|---|---|
| Scheduler and time load | **Passed for E0.** There is one active operation and no queue. One atomic update performs at most one optional Lesson 052 decode/copy with `n <= 100`, one parent transition, one direct Lesson 053 phase calculation, and fixed presentation work. There is no retry or catch-up loop. Strict, sanitizer, 16-mask collision, boundary, and measured conservative stack evidence confirm the bounded path. |
| Total memory and hardware resources | **Passed for E0.** Standalone and maximum linked measurements, the 407 B coordinator, separate 400 B borrowed storage, and the 888 B conservative stack path establish the final evidence above. The 21,864 B/3,531 B maximum and 3,773 B post-stack margin pass hard limits. The corrected resource decision accepts the disclosed 88 B stack-target miss because the 1,024 B hard ceiling and aggregate margin pass without weakening the atomic contract or evidence narrative. E1 additionally needs exact hardware-resource evidence. |
| Shared bus or transport | **Absent from E0; applicable at E1.** E0 consumes copied values and has no bus owner or transport. An exact LCD or keypad adapter may introduce I2C, SPI, or scanned-pin resources at E1. The E1 plan must name owners, borrower lifetimes, address/chip-select, arbitration, bounded transactions, partial initialization rollback, congestion, and restart. Display traffic may not delay cancel or carrier safe state. |
| Persistence and recovery | **Not applicable to the proposed support claim.** The symbolic catalog and translation mapping are immutable firmware/configuration data, and transaction state is intentionally volatile. Reset starts with no active authorization, inactive semantic intent, empty correlation state, and no recovered round-trip claim. Adding learned commands, stored captures, or durable replay tables would challenge the fixed-authority boundary and requires a separate architecture and safety decision. |
| Motion, external power, or stored energy | **Applicable at E1 only for emitted optical energy.** E0 emits no energy. E1 requires an exact IR LED/driver, current-limiting calculation, carrier duty and maximum burst bounds, adjacent low-power geometry, and inactive construction/startup/failure/cancel/shutdown state. Current limiting and bounded duty reduce exposure; they do not establish eye safety, and the lesson makes no eye-safety claim. No access-control, long-range, high-power, pyrotechnic, launcher, or unknown-device command is authorized. |
| Observation identity and provenance | **Applicable and central.** Every copied receive record retains source identity, observation time, configuration revision, validity/status, decoder family, categorical evidence strength (`None`, `ShapeRecognized`, or `IntegrityVerified`), repeat/malformed classification, and immutable Lesson 052 evidence generation. Every parent transaction receives a unique operation identity and binds the parent instance epoch/generation, mapping/configuration revisions, receive provenance and evidence generation, input digest, and exact Lesson 053 child preview. Round-trip attribution additionally requires exact qualified actual-emission evidence; matching payload bytes alone are insufficient. |
| Diagnostic interference | **Applicable.** Transmit LED, receive LED, fault pattern, LCD intent, optical test point, Serial, and trace storage share the E1 time/resource budget. Filling, disabling, or faulting any diagnostic must not alter command eligibility, carrier timing, cancel precedence, echo classification, receive admission, or emitter safe state. The transmit and receive indicators remain semantically distinct even when a half-duplex guard suppresses a received frame. |
| Failure collision and recovery | **Applicable.** Inject cancel plus dependency fault, malformed or unknown receive evidence, saturated diagnostic state, stale candidate, backward source time, and failed presentation at one timestamp. Precedence is invalid update-envelope rejection without mutation; previously latched safety/dependency fault; readable cancel/stop forcing inactive intent; independent malformed/source/timing co-input classification; mapping rejection; child readiness; ordinary progress; presentation. Independent causes remain independently visible. Restart with faults present remains inactive and requires a fresh valid receive generation and fresh candidate generations. |

## Copied provenance and fixed transmit authority

The translator must not accept a raw pointer, view, or mutable pulse buffer as
durable transaction state. The sole coordinator ingress copies one complete
bounded update envelope containing canonical presence flags and optional
cancel, prepared commit preview, receive, and actual-emission values. Invalid
absent fields, capacity overflow, inconsistent length, invalid candidate,
source, or operation identity, backward time, or an unknown configuration
revision rejects the whole envelope without mutation. A valid present receive
value is forwarded into the owned Lesson 052 seam; it may change only that
evidence generation and cannot start transmission. A prepared preview can
commit only through this envelope after cancellation, receive classification,
and actual-emission correlation have applied.

Transmit authority comes only from the initialization-time immutable fixed
mapping. It maps `StationPing` to `StationReady`, `StationReady` to
`StationAcknowledge`, and `StationAcknowledge` to `StationPing`;
`StationCancel` authorizes no transmission and cancel dominates. A mapping
input must be valid, integrity-verified, from the configured qualified source,
and match the fixed receive address/command/revision tuple. The output is
always a different local symbol. Repeat, unknown, malformed, truncated,
overflow, source-faulted, self-echo, stale, and unlisted valid evidence never
select output. Pulse records and decoded/raw values may enter only as receive
evidence; none is accepted as transmit authority. The surface exposes no raw
address/command, raw-duration, arbitrary-byte, or independent local-selection
transmit operation.

## Atomic parent and child candidates

Lesson 054 requires a parent `preview`/`canCommit`/`commit` transaction or an
equivalent bounded internal protocol. Lesson 052 contributes an immutable
already-admitted evidence generation, not a mutable child candidate. The
parent validates that generation and its full copied provenance, reserves one
parent candidate, then obtains the exact Lesson 053 candidate. The preview
binds owner, parent generation/instance epoch, mapping/configuration revisions,
immutable Lesson 053 emission-catalog revision/digest, operation identity,
input digest, evidence generation, receive provenance, and the complete child
preview before any commit. Every explicit preview field must match the live
retained candidate. The preview is an ordinary public value, not an
unforgeable token: any exact fieldwise copy is valid while that sole retained
parent candidate remains live. Its authority comes from matching live private
state, not from the identity or address of the preview object.

The implementation must prove that a rejection leaves the parent, immutable
Lesson 052 generation, and Lesson 053 child unchanged. Parent `canCommit()`
checks every parent and child field; at the same microsecond the already
preflighted Lesson 053 commit must be infallible, after which the parent
publishes the same attributable operation. If that property cannot be
preserved, promotion stops: sequential best-effort mutation may not be
described as atomic. A generic transaction manager or change to earlier public
component contracts is architectural remediation requiring discussion and a
durable decision.

Successful commit publishes exactly one parent snapshot with the fixed mapped
receive/output pair and every child result attributable to the same operation.
Mutated, foreign-owner, stale-generation, reused/consumed, post-reset, and
post-shutdown previews reject without mutation. An independently constructed
fieldwise-identical value is equivalent to an exact copy while the one
matching retained candidate is live; it rejects after that candidate is
cancelled, consumed, reset, or otherwise invalidated. Cancel or a dominating
fault invalidates all outstanding emission candidates before ordinary child
progress.

## Cancel and fault precedence

The sole update ingress first validates every presence flag, canonical absent
field, identity, timestamp, and operation relationship. A structurally invalid
whole envelope rejects atomically and cannot be repurposed as cancellation.
For a valid envelope, matching cancellation is applied before optional receive
classification, actual-emission correlation, prepared commit, or ordinary progress. Thus a
semantically malformed, unknown, truncated, or source-faulted receive value in
the same envelope cannot mask cancellation, and its independent disposition
remains observable. Cancel requests inactive semantic intent and invalidates
pending transmit work. An already-latched safety or dependency fault remains
attributable and is not erased merely because cancel also arrived.

After cancel, no retry, repeat frame, later receive classification, diagnostic
recovery, or late child acknowledgement may resume emission. Recovery requires the
fault-clear rule defined by the owning child, a fresh admitted valid receive
generation, and fresh parent and child candidates. Presentation failure is always subordinate
to cancel and transport state. Unknown/malformed receive evidence is observable
but never a transmit fault unless its owning receive policy explicitly reports
a source failure.

## Round-trip attribution and half-duplex self-echo

A round trip is complete only when a qualified receive observation is
correlated to one exact emitted operation. The atomic copied result publishes
its completion flag, operation ID, transmitted code, immutable emission-catalog
revision/digest, transmitter source, actual start/completion timestamps,
modular elapsed duration, exact Lesson 052 capture disposition and categorical
evidence strength, complete receive provenance, receive evidence generation,
separate translation/correlation disposition, and status. A matching command
from a different source, an older generation,
before actual completion, outside the response window, or after timeout is not
the transaction response. No mixture of fields from different updates or
operations may be published as one result.

The physical workbench uses adjacent transmitter and receiver fixtures and is
therefore exposed to its own optical output. The half-duplex policy is frozen
as a half-open actual-emission interval:
`[actual startedAt, actual completedAt + echoGuard)`. Receiver evidence whose
Lesson 052 `observedAt` lies in that interval is copied and classified
`self-echo suppressed`, never used as round-trip completion. Exactly at the
guard end it is eligible for normal evaluation. E0 uses explicitly marked
synthetic actual-emission evidence and calls this observation-time
correlation, not capture-onset or optical proof. E1 may claim optical interval
correlation only after a separately reviewed receiver adapter supplies
qualified timing. Suppression retains the latest evidence identity and a
saturating count. A policy commit or logical completion is not actual optical
start or completion.

The saturation invariant is proved directly, not by replaying `2^32`
receive events. The saturating-arithmetic helper has compile-time boundary
proof that incrementing `UINT32_MAX` remains `UINT32_MAX` and that incrementing
`UINT32_MAX - 1` reaches `UINT32_MAX`. Ordinary reachable integration cases
then prove each admitted self-echo invokes that helper and increments the
published count from normal runtime states. Together these establish both the
maximum boundary and the live coordinator path without an infeasible
event-count replay.

The response interval begins exactly where echo suppression ends:

```text
responseStart = actualCompletedAt + echoGuard
response interval = [responseStart, responseStart + responseWindow)
elapsed = receiveObservedAt - actualCompletedAt
```

One tick before `responseStart` is self-echo suppressed; exactly at
`responseStart` is response-eligible. One tick before the exclusive response
end remains eligible; exactly at the end or later is a timeout. Elapsed time
therefore includes the echo guard and preserves latency from actual completion,
not from a restamped response-window start.

Both checked additions and the subtraction obey unsigned modular half-range
rules. If either interval addition overflows into ambiguous ordering, exceeds
the unambiguous half range, or cannot form the two ordered half-open intervals,
the configuration/evidence rejects or faults without publishing a round-trip
result. The implementation must not saturate, shorten, or reinterpret an
overflowed interval as immediately due. A regressing or exact-half-range
elapsed subtraction likewise faults atomically.

## Indicators and non-Serial evidence

The project reserves separate semantic intents for transmit activity, receive
activity, and fault state. Transmit activity follows admitted burst state;
receive activity follows admitted Lesson 052 evidence even when that evidence
is unknown or self-echo suppressed. The fault pattern is distinguishable from
both and does not share their meaning. The LCD intent displays source, catalog
command or `unknown`, categorical evidence strength/disposition, and
round-trip status.

At E1, the exact qualified adjacent receiver is the authoritative optical
activity test point. A phone camera may provide an optional qualitative hint,
but it is neither qualification evidence nor proof of correct carrier or
command content. Activity at the exact receiver proves bounded optical
activity, not successful decoding or command validity; that requires separate
qualified receive evidence. The lesson states what to predict, where and when
to observe it, and how to interpret it. Resource acquisition evidence is
separate from optical inactive-state evidence. Cancel and shutdown acceptance
must show the emitter inactive at the exact receiver test point even if LEDs,
LCD, or Serial fail.

## Deterministic matrix

The promoted E0 host matrix includes:

- every fixed mapping and digest field; proof that every authorized output
  differs from its input; invalid receive tuples, sources, revisions, and
  unsupported symbols;
- valid mapped, repeat, unknown, malformed, truncated, overflow, stale,
  source-faulted, self-echo, and unlisted-valid receive evidence, plus attempts
  to use captured values or an independent local selection as transmit
  authority;
- every parent phase crossed with cancel, child fault, source fault, timeout,
  saturated diagnostics, presentation failure, reset, and shutdown, including cancel in the
  same admitted update as malformed, truncated, unknown, and source-faulted
  receive co-input;
- every meaningful permutation of simultaneous cancel, prepared commit,
  receive, and actual-emission values, proving the single-envelope precedence
  is independent of any conceptual source order;
- exact fieldwise parent/child preview copies while the sole matching retained
  candidate is live; mutated, foreign-owner, stale-generation, reused/consumed,
  reset-invalidated, and shutdown-invalidated previews; including failure
  after each preflight boundary;
- exact actual start/completion, `responseStart`, exclusive response end, and
  timeout timestamps; one tick before, exactly at, and one tick after both
  response boundaries; exact elapsed-from-actual-completion values; echo/window
  addition overflow and half-range ambiguity; absent or mismatched
  actual-emission evidence; simultaneous events; rollover; backward apparent
  source time; and large elapsed jumps;
- local self-echo during emission, at both guard boundaries, after the guard,
  repeated echo, receiver saturation, missing carrier, malformed/truncated/
  noisy frames, unknown frames, and a separately sourced matching response;
- direct compile-time `UINT32_MAX - 1` and `UINT32_MAX` saturation-helper
  boundaries plus reachable ordinary self-echo increments through the
  coordinator, explicitly not a runtime `2^32`-event replay;
- round-trip matches and mismatches by operation, catalog revision, parent
  instance epoch, parent generation, configuration revision, copied-input
  digest, transmit source, receive source, source epoch, interval, decoder
  family, repeat disposition, and categorical evidence strength;
- indicator/LCD/trace disabled, full, delayed, and failed, proving byte-stable
  primary results; and
- two independently constructed coordinators replaying the same copied inputs
  field by field, plus capacity and aggregate size evidence for the complete
  Lesson 052--054 composition.

## E0 and E1 promotion boundaries

### E0 copied-fixture boundary

E0 is promoted on the repaired public contracts, complete deterministic
matrix, canonical compile-only Mega replay, measured aggregate evidence,
clean terminal implementation review, HTML/PDF/site publication review, and
this post-implementation stress pass. It owns no hardware resources and makes
no claim about carrier frequency, receiver fidelity, optical power, range,
physical round-trip timing, display operation, keypad operation, or emitter
safe state. Its result is a deterministic policy proof over copied evidence
and logical intents.

### E1 exact-fixture boundary

E1 remains blocked on exact receiver, emitter/driver, keypad, LCD, indicator,
board, and supply identities; primary datasheets; voltage/current and
carrier/timer qualification; an authoritative schematic; complete pin,
interrupt, timer, bus, claim, memory, stack, and power budgets; current-limited
adjacent geometry with no eye-safety claim; half-duplex echo evidence;
rollback and safe-state tests;
and a signed bench acceptance record. E1 must separately prove resource
acquisition, known harmless command transmission, receive classification,
round-trip attribution, cancel/fault precedence, diagnostic isolation, and
emitter inactivity after failure, reset, cancellation, shutdown, and power
removal.

Unknown-protocol replay, access-control commands, credential capture,
unidentified emitters, high-power or long-range transmission, and any
pyrotechnic or launcher control remain prohibited regardless of E1 progress.

## Prior-decision impact

| Decision or contract | Disposition and evidence |
|---|---|
| Four-layer architecture and endpoint-owned electrical lifetime | **Preserved.** Lesson 054 coordinates semantic copied values and intent. Pins, carrier timing, receiver sampling, current limiting, and safe electrical shutdown remain endpoint/adapter responsibilities. |
| Explicit time, bounded work, no heap, and deterministic replay | **Preserved subject to measured implementation.** The proposed transaction advances a bounded amount per supplied timestamp using fixed storage. Hidden waits, callbacks, allocation, and catch-up loops are excluded. |
| Receive-only Lessons 025--027 and copied Lesson 052 provenance | **Preserved.** Receive evidence remains observation, not authority. Earlier captures do not acquire a transmit path. |
| Lesson 053 listed learner-created transmission only | **Preserved and reinforced.** The immutable local symbolic catalog is the sole transmit authority; copied or unknown captures cannot modify it. |
| Unknown-protocol replay and hazardous control prohibition | **Preserved.** The API has no learn-and-send operation or arbitrary pulse-transmit escape hatch. |
| Transactional candidate precedent | **Extended only within the promoted 052--054 E0 block.** Owner/generation binding and joint preflight follow existing policy. Any need to change an earlier public contract or add a generic transaction framework is a challenged decision. |
| Circuit-native observation and diagnostic isolation | **Preserved.** Distinct transmit, receive, fault, display, and optical evidence are budgeted without becoming correctness inputs. |
| Pencil visuals with formal-schematic exception | **Preserved.** E0 uses pencil drawings only; an E1 schematic requires exact electrically authoritative fixtures and explicit classification. |
| E0/E1 evidence separation | **Preserved.** Host replay publishes copied policy results only. Hardware support waits for exact specimens and bench evidence. |

## Remediation triggers

Stop implementation or promotion and discuss a broader remediation if any of
the following becomes necessary:

- a received or learned frame must modify transmit authority;
- an arbitrary pulse buffer or unknown protocol must pass to the emitter;
- the parent cannot preflight its immutable Lesson 052 generation and exact
  Lesson 053 candidate, or the child commit can fail after preflight;
- cancellation cannot force inactive semantic intent before ordinary work;
- matching payload bytes are the only available round-trip identity;
- self-echo cannot be distinguished or observably suppressed within a bounded
  window;
- the coordinator must own timer registers, GPIO, bus transactions, ISR
  behavior, or endpoint safe state;
- diagnostics must be enabled for primary correctness;
- fixed storage or bounded per-update work cannot fit the aggregate Mega 2560
  budget with required margin;
- exact E1 fixtures require a shared timer, bus, supply, or pin exception that
  changes an earlier component contract; or
- hardware behavior is needed to substantiate an E0 claim.

Future local remediation inside the promoted Lessons 052--054 E0 contracts
must retain fixed authority, copied provenance, deterministic time, bounded
work, and fail-closed cancellation. A generic replay facility,
shared transaction framework, new ownership convention, changed status
semantics, or compatibility change is architectural remediation. Record the
affected consumers, alternatives, migration and resource costs, discuss the
material choice with the user, and publish a durable decision before
continuing.

## Gate result

- Disposition: **E0 promotion passed; E1 remains blocked**
- Open risks: none for the bounded copied-fixture E0 claim. Exact receiver,
  emitter/driver, electrical/timer resources, optical behavior, powered
  cancellation, and bench evidence remain open for E1
- Required discussion or decision IDs: stack-target disposition
  `55185e25-483f-4f35-9a2a-6f6a6d206cf9`; a new durable decision is required
  before any remediation trigger changes shared architecture, safety,
  curriculum, or public behavior
- Remediation owner and next action: preserve the promoted E0 contract and
  carry only the separately gated exact-fixture work into E1
- Verification commands and results: focused strict and ASan/UBSan tests,
  complete host/sanitizer inventories, style and diff checks, the repaired
  valid-payload 16-mask matrix, canonical Mega/size checks, terminal
  implementation review, `make lessons-check`, `make site`, lesson-route/PDF/
  sketch-download checks, and PDF metadata/font/text/monochrome/pencil-policy
  review all pass. Resource evidence records 16,162 B/1,343 B standalone,
  21,864 B/3,531 B maximum, 407 B coordinator plus 400 B borrowed storage,
  888 B conservative stack, and 3,773 B post-stack margin
- Maximum-composition scenario and proof: the full valid-payload 16-mask
  collision matrix, terminal independent review, and aggregate resource
  measurement pass
- E0 implementation permitted: **yes**
- E0 promotion permitted: **yes**
- E1 promotion permitted: **no; exact-fixture admission remains blocked**
