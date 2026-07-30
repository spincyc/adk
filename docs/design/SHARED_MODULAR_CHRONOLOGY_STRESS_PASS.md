# Shared modular-chronology architecture stress pass

Status: pre-implementation proposal. This pass exists because the change it
describes alters a contract shared by most published components, and
`docs/WORK_QUEUE.md` requires a recorded stress pass before any such shape is
fixed. No code has been changed.

## Boundary

- Name and lesson/project: shared modular-chronology predicate, extracted into
  `src/time.h`; no lesson number, it is a cross-cutting seam.
- Reviewer and date: self-review, 2026-07-30.
- Public types and operations: proposed additions to the existing `TimePoint`
  and `Duration` contracts only — no new component, no new endpoint.
- Direct dependencies: `src/time.h` alone.
- Existing decisions and interfaces reconsidered: every component that
  currently re-derives the modular forward-time rule privately.

## Measured duplication

Counted 2026-07-30, not estimated:

| Fact | Measurement |
|---|---:|
| `.cpp` files referencing the half-range rule | 42 |
| files defining the constant privately | 33 |
| spelled `UINT32_C (0x80000000)` | 17 |
| spelled `0x80000000UL` | 17 |
| spelled `modularHalfRange` | 1 |
| comparison sites `>= halfRange` | 105 |
| comparison sites `< halfRange` | 58 |

The value is uniform everywhere; only the spelling and the local name vary.
The dominant predicate is "is this delta within the forward modular half
range", written inline at 163 sites.

## Fit review

| Pressure | Disposition |
|---|---|
| API and layering | **Natural.** The rule is a property of `TimePoint`/`Duration`, which already sit at the bottom of the dependency graph. Every current copy points at that same concept, so moving it downward removes an upward leak rather than creating one. |
| Ownership and lifecycle | **Not applicable.** The predicate is pure and owns nothing; no claim, endpoint, or lifetime changes. |
| Time and ordering | **The whole point.** Time still enters explicitly as a supplied argument. The risk to manage is that individual components mean subtly different things: some ask "strictly forward", others "forward or equal", others "unambiguous delta". A single helper must not silently unify three distinct meanings. |
| Errors and status | **Natural.** The predicate returns `bool`; no `Status` semantics change. Components keep publishing their own named discontinuity reasons. |
| Resource budget | **Favourable but unproven.** A `constexpr`/inline predicate should cost nothing at `-Os` and may shrink flash by removing 33 duplicate constants. This must be measured against the exact no-LTO probes for the published lessons, not assumed. |
| Deterministic proof | **Adequate.** The rule is exhaustively testable in isolation: zero, one, half-range minus one, exactly half range, half range plus one, and `UINT32_MAX`, in both directions across rollover. |
| Packaging and public surface | **Widens `time.h`.** `time.h` is in the Arduino archive, the native source scope, and the umbrella header, so any addition is a published API commitment. |
| Example and documentation fit | **Neutral.** No example changes. Lessons that teach the rule keep teaching it; they would cite one authority instead of restating it. |
| Downstream effects | **Broad and the reason for caution.** 42 published, host-verified `.cpp` files inherit this. Each has recorded size evidence, and several have fingerprint-bound resource reviews that would need re-measurement if flash or SRAM moves. |

## The decision this pass actually forces

The duplication is real and larger than the audit claimed. But the pressure
that matters is not "how many copies" — it is **whether the copies mean the
same thing**. 163 comparison sites split across at least two directions
(`>=` and `<`) is evidence that they encode more than one predicate.

Extracting a single helper before establishing that they agree would replace
honest duplication with a false shared contract, which is worse: a component
whose meaning silently shifts during migration would fail in its chronology
handling, the exact area every one of these arcs treats as safety-relevant.

**Disposition: do not extract yet.** The prerequisite is a classification
pass that reads all 163 sites and sorts them into named predicates — likely
"forward or equal", "strictly forward", and "delta is unambiguous" — and
records the count in each class. Only if that pass shows a small closed set
should a helper per meaning be added, each with its own name, so no call site
changes meaning.

Migration, when authorized, must then be incremental: one arc at a time, each
followed by its own resource probe, never a repository-wide sweep. Arcs
carrying fingerprint-bound reviews (067--069, 070--072, 079--080) must
re-measure and re-record before their gates can pass.

## Composition pressure scenario

Not exercised. A composition scenario is premature while the predicate set is
unknown; running one now would test an abstraction this pass declines to fix.

## Gate result

Blocked pending the classification pass described above. No shared contract
changes, no `time.h` addition, and no component migration is authorized by
this record.
