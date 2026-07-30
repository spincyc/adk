# Lesson 081 terminal resource strain — promotion blocked

Status: implementation complete and correct on branch
`lesson-081-implementation` (`636f0ad`); **promotion blocked** on a hard
resource-gate failure that traces to an inconsistency in the controlling
plan, not to the implementation. Recorded 2026-07-30 under the rule in
[the plan](LESSONS_079_081_COMPONENT_QUALIFICATION_PLAN.md) that architectural
strain is discussed before it is repaired.

## What was built and verified

The E0 core is implemented: the 256-byte record codec, the replay verifier,
and the bench's five-step state machine, with 16 deterministic test functions.
Verified independently of the implementer, in a clean worktree at `636f0ad`:

| Gate | Result |
|---|---|
| `make host-test` (whole battery, including the new test) | pass |
| `make style-check` | pass |
| `make headers-check` | pass |
| `build/host/test_inert_component_qualification_bench` | pass |

## The failure

Measured with the repository's own AVR toolchain
(`avr-g++ 7.3.0`, `-mmcu=atmega2560 -Os -fno-lto`), taking symbol sizes:

| Type | AVR bytes |
|---|---:|
| `InertComponentQualificationBench` | **2919** |
| `ComponentQualificationReplay` | 2069 |
| `ComponentQualificationEnvelope` | 303 |
| `ComponentQualificationSnapshot` | 110 |
| `ComponentQualificationRecord` | 236 |

The plan's coordinator gate is **768 B target, 1024 B hard**. The measured
bench is **2.85× the hard limit**, and the plan states that a hard miss blocks
promotion. At 2919 B the object alone would consume 36% of the Mega 2560's
8 KB of SRAM.

## Root cause: the plan cannot be satisfied as written

The bench is large because it embeds `ComponentQualificationReplay`, which is
five envelopes plus five snapshots. That is not an implementation whim; the
plan leaves no alternative:

- `encodeComponentQualificationRecord (record, replay, image, imageSize)` and
  `ComponentQualificationReplayVerifier::verify (record, replay)` both
  **require** a complete five-entry replay, and the plan states encoding
  "always invokes this full verifier before staging any byte".
- `prepareRecord (uint8_t* image, size_t imageSize) const noexcept` provides
  **no way to pass one in**.

So the bench must retain the replay to honour `prepareRecord`, and retaining
it costs 2069 B against a 1024 B ceiling. Meanwhile the plan's own maximum
composition budgets storage for "one full input envelope and one output
snapshot" — one of each, not five — which is what a 768 B coordinator target
was sized against. The 768/1024 B budget and the five-entry replay requirement
were never reconcilable.

The implementer chose the object over the stack, having found that building
the replay inside `prepareRecord` would put roughly 2 KiB on a 1 KiB stack
budget. Both placements violate a stated budget; there is no compliant
placement available under the current API.

## The decision required

This needs an authoring decision, not a code repair. The options, with what
each costs:

1. **Give `prepareRecord` a replay parameter** —
   `prepareRecord (const ComponentQualificationReplay&, uint8_t*, size_t)`.
   The caller owns the replay, so the bench returns to roughly its two
   children plus state and the 768 B target becomes plausible. Costs: a
   published-API shape change against the plan, and the caller must retain
   2069 B, which must then be budgeted in the maximum composition.
2. **Shrink what the record must prove.** If encode verified against the
   terminal envelope and snapshot rather than all five, the replay type could
   shrink by 4/5. Costs: weakens the "a record cannot be encoded from compact
   fields alone" guarantee, which is a deliberate integrity property.
3. **Raise the coordinator budget** to accommodate a retained replay. Costs:
   a ~2.9 KB object against 8 KB of SRAM, which likely breaks the residual
   SRAM floor once the example and children are included; the residual gate
   would need re-derivation before this could be called safe.
4. **Store the replay as digests rather than values.** The bench would retain
   the eight domain digests plus the terminal envelope, and the verifier would
   compare digests instead of re-deriving them from full values. Costs: the
   verifier can no longer independently recompute from evidence, which is the
   property that makes it a verifier.

Option 1 is the smallest change that keeps every stated integrity property,
and it is the recommendation. Option 4 is the one to avoid: it preserves the
resource budget by removing the guarantee the record exists to provide.

## Disposition

No merge to `main`. The branch is retained. `docs/WORK_QUEUE.md` continues to
show Lesson 081 as queued, and no support claim changes. The Mega example,
HTML and PDF lesson, site page, resource probe, and size baseline remain
unwritten, as they should until the shape is settled.
