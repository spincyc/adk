# Acoustic envelope design stress pass

This record applies the
[component design stress pass](../../templates/component-design-stress-pass.md)
to the hardware-independent Lesson 038 behavior. It evaluates the public
boundary before publication; it does not qualify a microphone specimen or
claim physical acceptance.

## Boundary

- Name and lesson/project: `AcousticEnvelope`, Lesson 038
- Reviewer and date: independent read-only architecture review, 2026-07-28
- Public types and operations: `AcousticPhase`, `AcousticQuality`,
  `AcousticEnvelopeConfig`, `AcousticSample`, `AcousticObservation`, and
  `AcousticEnvelope::{initialize,reset,update,initialized,snapshot}`
- Direct dependencies: `Level`, `Status`, `TimePoint`, `Duration`, and fixed
  width integer types
- Existing decisions and interfaces reconsidered: pure copied-sample
  behaviors; explicit unsigned time; closed status and quality values;
  construction, initialization, reset, and fault recovery; fixed Mega 2560
  memory; endpoint ownership in the lesson adapter; one-update event evidence;
  and the Lesson 039 acoustic-association contract

## Fit review

| Pressure | Evidence and disposition |
|---|---|
| API and layering | **Natural.** The component converts copied analog and optional threshold evidence into relative acoustic windows. It owns no endpoint, pin, clock, callback, display, sounder, or storage. The adapter retains electrical acquisition and lifetime; the composing sequencer consumes only the published observation. Dependencies continue downward to existing value types. `Level` comes from the established digital vocabulary also used by `ContactDynamics`; extracting it during this arc would create wider churn without removing a behavioral special case. |
| Ownership and lifecycle | **Natural.** Construction is inert and the type is non-copyable and non-movable. `initialize()` validates configuration without acquiring resources and is idempotent. `reset()` clears time, calibration, event, disagreement, and completed-event history while preserving initialized state. Latched faults require explicit reset and recalibration. Endpoint rollback, shutdown, and safe electrical state remain adapter responsibilities rather than hidden component ownership. |
| Time and ordering | **Natural.** Every update supplies one explicit `TimePoint`. Identical same-time evidence is idempotent; changed same-time evidence and apparent jumps at or beyond half-range fault without a lower-priority transition. Natural rollover is valid. Calibration, continuous disagreement, event close, event open, and refractory boundaries have exact precedence and boundary rules. Event start and completion flags live for one update, which gives Lesson 039 one deterministic association opportunity without a queue or hidden latency. |
| Errors and status | **Bounded local remediation complete.** Existing `Status` values express configuration, source, hardware-disagreement, and timing outcomes. `AcousticQuality` adds domain interpretation without inventing another transport or diagnostic-string convention. Review exposed one malformed-input pressure: when a threshold is configured and present, an undefined `thresholdLevel` must not be interpreted as merely inactive. It now wins before endpoint-status and analog interpretation, enters `SourceFault` with `InvalidArgument`, and applies no lower-transition or partial raw/threshold mutation. Independent core re-review passed this behavior, including preservation of the preceding `rawThresholdActive=true` evidence rather than replacement by the malformed value. Analog and configured threshold failures otherwise preserve their exact endpoint status. The corrected contract makes evidence retention unambiguous: timing faults may retain the last completed tuple, while clipping, source failure, disagreement, and invalid runtime headroom publish canonical zero event fields. |
| Resource budget | **Natural.** State is fixed-size; update work is bounded integer arithmetic; there is no heap, interrupt, sampling timer, FFT, ADC-reference change, bus, or claim-registry entry. Lesson 038 needs one envelope instance. Lesson 039 composes one instance with four contact behaviors and a sequencer whose hit capacity is separately fixed. Exact Mega flash, SRAM, stack, pin, indicator, and sampling-cadence evidence remains an ordinary example/promotion gate, not an exception in this boundary. |
| Deterministic proof | **Natural.** Host fixtures drive complete timestamped samples with injected analog and threshold statuses. Focused tests cover closed configuration, lifecycle, calibration, positive and negative excursions, intensity endpoints, clipping, source failure, optional-threshold presence and chatter, disagreement recovery, reset recovery, event and refractory boundaries, same-time rules, backward time, half-range, rollover, and deterministic replay. Added fixtures cover malformed threshold-level precedence, preservation of preceding active threshold evidence, absence of partially applied raw/threshold evidence, latched recovery, changed same-time precedence, and identical same-time replay. Independent core re-review and the full gates passed. The behavior has no seed, ambient input, or hardware clock. |
| Packaging and public surface | **Natural.** The declaration is standalone and the implementation is out of line. Native and Arduino packaging discover public `src` headers and sources without a component exception. The standard host-test inventory contains an explicit `AcousticEnvelope` target, and the focused source and tests compile with the repository's strict C++17 warning policy. No component-specific packaging path is required. |
| Example and documentation fit | **Natural.** The planned adapter keeps `setup()` as endpoint and indicator acquisition followed by quiet calibration, and `loop()` as observe, classify, present. Its canonical vocabulary matches the public types. The plan separates the rich pencil-drawing lesson from any electrically authoritative formal schematic and keeps specimen qualification, test points, non-Serial indicators, shutdown evidence, sampling limitations, and physical acceptance explicit. |
| Downstream effects | Lesson 039 consumes the one-update `Refractory + ValidEvent + Ok + eventCompleted` tuple and its bounded interval/intensity directly. No earlier component, example, lesson, status table, physical record, or public contract must change. Future acoustic consumers inherit relative, within-configuration intensity and the explicit prohibition on SPL, frequency, speech, surveillance, or complete-capture claims. |

## Prior-decision impact

- Hardware-neutral behavior over copied observations: **preserved**. Endpoint
  ownership and electrical lifetime do not move into the component.
- Existing `Status`, `TimePoint`, `Duration`, and same-time/half-range
  conventions: **preserved**.
- Inert construction, explicit initialization, non-copyability, and explicit
  reset after a latched behavior fault: **preserved**.
- Fixed-memory, no-exception, no-heap Mega 2560 policy: **preserved**.
- Analog envelope authority with optional comparator evidence:
  **extended** only by the enumerated threshold-disagreement quality; no
  threshold-only mode or silent fallback is added.
- One-update event evidence consumed by deterministic composing projects:
  **extended** by a bounded acoustic interval and relative intensity tuple.
- Lesson 039 pending-group association: **preserved**. It consumes the exact
  legal tuple already defined by Lesson 038 and needs no adapter-specific
  reinterpretation.
- Electrical specimen qualification, non-Serial evidence, pencil-visual
  policy, and open physical acceptance: **preserved**.
- Canonical publication authority: **preserved and out of scope**. This
  component does not resolve or worsen the separately recorded publication
  coupling.

No prior decision is challenged. Rejecting an undefined configured threshold
level extends the existing closed-enum and no-partial-mutation rules at this
unpromoted boundary; it does not add a status convention or affect a prior
consumer. The ten-field configuration is dense, but
its fields form one closed policy, validation rejects every ambiguous
relationship, and the current sole downstream consumer does not need another
configuration abstraction. Splitting it now would add types without reducing
ownership, timing, status, or resource pressure. Reconsider grouping only if
later acoustic components demonstrate a second coherent shared policy.

## Stress disposition

**Bounded local remediation complete; natural fit verified.**
`AcousticEnvelope` adds one hardware-independent classification responsibility
using existing lifecycle, time, status, and fixed-storage conventions. The
review found no compatibility break, inverted dependency, duplicated owner,
hidden resource, or old decision requiring reinterpretation. The malformed
configured threshold-level case was confined to this unpromoted input
boundary. It now rejects the undefined value with deterministic precedence,
preserves preceding active threshold evidence, and does not partially apply
raw or threshold evidence. Independent core re-review and full gates passed.

## Gate result

- Disposition: bounded local remediation complete; natural fit verified
- Open risks: exact microphone electrical identity and E1 evidence remain
  gated; the documented polling interval may miss pulses between updates;
  ordinary Mega size and complete lesson publication gates remain open
- Required discussion or decision IDs: none for the component architecture
- Remediation owner and next action: the AcousticEnvelope core owner completed
  bounded malformed-level validation and fixtures; independent core re-review
  confirmed precedence, preservation of prior `rawThresholdActive=true`
  evidence, no partial mutation, same-time behavior, and reset recovery. No
  architectural remediation is required. The Lesson 038 integration owner
  must complete the example, size, HTML, pencil-visual PDF, and deferred
  physical-acceptance records.
- Verification commands and results:
  - `make build/host/test_acoustic_envelope`: passed
  - `build/host/test_acoustic_envelope`: passed
  - strict focused C++17 build of `status.cpp`, `time.cpp`,
    `acoustic_envelope.cpp`, and `test_acoustic_envelope.cpp`: passed
  - focused `test_acoustic_envelope` executable: passed
  - `git diff --check`: passed before this record
  - independent core re-review of malformed threshold handling: passed
  - full applicable core gates: passed
- Promotion permitted: yes by the architecture stress gate; ordinary
  downstream lesson, size, packaging, publication, and physical-acceptance
  gates remain independently controlling
