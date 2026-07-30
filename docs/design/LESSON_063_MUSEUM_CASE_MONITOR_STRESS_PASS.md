# Lesson 063 museum-case-monitor architecture stress pass

Status: terminal E0 promotion review; powered composition, persistence,
and relay fixture remain open.

This pass reviews the
[Lessons 061--063 implementation plan](LESSONS_061_063_MUSEUM_CASE_MONITOR_PLAN.md).

`MuseumCaseMonitor` is a natural project composition when it owns only alarm
policy, inert intent, and one copied audit transaction. It is not a security
system, environmental preservation instrument, persistent logger, relay
driver, or life-safety alarm.

| Pressure | Disposition |
|---|---|
| API and layering | Natural: the project consumes qualified copied observations and emits semantic presentation/relay/audit intent. It owns no endpoint or transport and does not requalify child evidence. |
| Ownership and lifecycle | Receive complete copied child observations and retain no child reference or pointer. Inert construction, generation-advancing initialize/reset/shutdown, stale-receipt invalidation, exhaustion fault, canonical inactive outputs, and noncopy/nonmove coordinator. |
| Time and ordering | One supplied timestamp anchors validation, alarm latch, acknowledgement, cooldown, and audit intent. Per-source observation epochs remain visible and are not made simultaneous. |
| Errors and status | Structural failure rejects before semantics. Sensing fault dominates healthy; active hazard dominates acknowledgement; recording failure is additive and cannot mask alarm. |
| Resources | E0 claims none. Ordinary replay is 21,348/1,405 bytes flash/static SRAM. Exact no-LTO evidence is 22,980 flash, 1,405 static, 819 stack, and 709 aggregate objects (528 monitor + 69 Lesson 061 + 112 Lesson 062). The 207-byte caller envelope is fixed static storage; the 72-byte result remains caller-owned. Residual SRAM is 5,840 bytes. The independently reviewed stack target miss remains below the 896-byte hard limit; every hard gate passes. |
| Deterministic proof | Freeze `{health,hazardMask}` as the record key. Prove first complete Healthy/Warning/Alarm/Fault and accepted Qualifying decisions, every reachable health/recovery transition, every individual/combined hazard-bit add/remove, and net-latest successor rules; initial Cooldown is unreachable. Same-key freshness and ineffective acknowledgement do not create records; shutdown does not; reinitialize records its first complete decision. Exact identities, full `MagneticObservation`, literal digest, pre-issue retry rejection, one outstanding plus latest dirty successor, retry/terminal/restart/exhaustion, acceptance/change collision, equal-time derived-field rejection, immediate promotion, rollover, lifecycle, and byte-stable replay pass. |
| Packaging/public surface | One canonical replay and complementary HTML/PDF with pencil drawings. Shared indexes follow only after implementation gates. No physical schematic at E0. |
| Downstream effects | Reuses Lessons 061--062 and copied reed semantics without changing Lessons 034--036. RTC/storage models remain unchanged; relay support and alarm claims are not inherited. |

## Composition pressure

The maximum fixture combines wet drift, thermal uncertainty and disagreement,
radiant pulse/saturation, reed-open/case-open evidence, acknowledgement, audit-write
failure, optional diagnostic failure, rollover, shutdown, and restart while
faults persist. Every hazard remains attributable and output intent remains
inactive after shutdown.

| Pressure | Applicability and evidence |
|---|---|
| Scheduler/time | Applicable. Bound child qualification plus one monitor update, one audit intent, and collision ordering without loops or starvation. |
| Memory/resources | Applicable. Measure the canonical linked maximum-composition sketch, all child objects, caller-owned-once frames/buffers, outstanding/dirty audit slots, compiler-callgraph stack with three-byte retained-return edges, flash/static SRAM, and formula residual under an exact tool/core/flag fingerprint. Machine-readable reviewed target markers must be current; per-buffer, hard, and residual-hard gates follow the plan. |
| Shared bus/transport | Not applicable at E0: no bus or transport call exists. Future LCD/RTC/SD sharing requires exact adapters, ownership, arbitration, rollback, and congestion evidence. |
| Persistence/recovery | Applicable only as an intentionally non-durable copied audit transaction. Prove exact `{health,hazardMask}` creation predicate, initial/changed/recovery matrix, same-key suppression, full witness/digest, exact correlation, deadline/retry, latest qualifying dirty-successor coalescing, acceptance/change ordering, immediate next-sequence promotion, terminal frozen state, lifecycle invalidation, and interruption attribution; durable media and power-loss recovery remain separate blockers. |
| Motion/power/energy | Not applicable at E0 because relay/lamp values are inert intent. E2 requires exact extra-low-voltage load, driver/flyback/opto/contact evidence and independent removal; mains is forbidden. |
| Identity/provenance | Applicable. Config fixes every expected source/configuration/calibration identity. Preserve complete child evidence, complete qualified magnetic contact semantics, acknowledgement revision, and all witness sequences. |
| Diagnostic interference | Applicable. RGB/LCD/sound/result/Serial/audit failure cannot change hazards, clear alarm, or produce healthy. Physical observation paths need aggregate resource evidence. |
| Failure collision | Applicable. Run the canonical record-41/record-42 trace: multiple semantic-key changes coalesce to the latest qualifying successor while pending; same-key sample refresh does not replace it; a failed retry retains both slots; acceptance plus a new key change promotes the latest successor immediately; terminal exhaustion freezes both until explicit lifecycle invalidation. Sensing fault and hazards remain additive; shutdown remains inactive and creates no record. |

## Prior-decision impact

- The project cadence is **preserved**: Lesson 063 composes the preceding two
  components and earlier copied reed semantics without a new hidden adapter.
- Lessons 022/024 persistence decisions are **preserved**: audit intent is not
  durable storage or an RTC/media claim.
- Relay energy rules are **preserved**: E0 is inert, E2 is extra-low-voltage
  only, with no mains or access/safety load.
- Non-Serial observation policy is **extended** to explicit RGB blink,
  LCD-age/fault, sound, and lamp intent; physical proof remains gated.
- Existing magnetic interfaces are **preserved**: the project consumes copied
  reed evidence and does not widen `MagneticObservation`.

## Gate result

- Disposition: `natural fit` after fixing expected identities, complete reed
  evidence, audit witness/recovery, lifecycle generations, and inactive-output
  invariants
- Open risks: persistent recorder semantics; exact LCD/RGB/sound,
  reed, and relay/lamp fixtures; simultaneous current and safe-state evidence
- Required discussion or decision IDs: none if implementation retains only
  copied observations and keeps audit volatile; persistence or shared-bus changes
  require architectural review
- Remediation owner and next action: E0 remediation is complete; E1/E2 owners
  must qualify exact physical fixtures without widening this policy
- Verification commands and results: strict host tests, ASan/UBSan, style,
  headers, canonical Mega replay, exact resource gate, lesson/PDF policy, site,
  and publication checks pass at promotion
- Maximum-composition scenario and proof: the bounded canonical replay,
  collision matrix, and exact linked resource fixture pass with fingerprint
  `1c9a3bcbbf4883f9583ab2b5593cf1e507c5cc476200593d010b5c6ef371aaff`
- Promotion permitted: yes for E0 implementation; no for physical sensing,
  storage, alarm, relay, security, or preservation claims
