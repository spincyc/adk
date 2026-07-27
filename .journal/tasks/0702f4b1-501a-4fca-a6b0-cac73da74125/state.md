---
schema_version: 1
task_uuid: "0702f4b1-501a-4fca-a6b0-cac73da74125"
title: "independently review lesson 029 core"
status: "queued"
priority: "high"
priority_reason: "Recovered audit prompt 2; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["5801d853-3278-4a52-84dd-7e25c0794d7c"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T22:00:43Z"
---

# Goal

Create a high-priority task, dependent on the recovered-ledger task, to
independently audit lesson 029's `CueAudit` and `InertCueScheduler` contracts
against `docs/lessons/029/IMPLEMENTATION_BRIEF.md` and
`docs/lessons/030/design.md`. Cover lifecycle, invalid plans, confirmation
windows, delayed updates, same-time ordering, rollover, stop dominance,
resume, fixed-capacity exhaustion, audit encoding, and shutdown. Fix only
confirmed core/test defects.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Record review evidence for lifecycle, timing, capacity, audit encoding, and
  inert-only safety boundaries.
- Fix only confirmed lesson 029 core or test defects.

## Original request

Create a high-priority task, dependent on the recovered-ledger task, to
independently audit lesson 029's `CueAudit` and `InertCueScheduler` contracts
against `docs/lessons/029/IMPLEMENTATION_BRIEF.md` and
`docs/lessons/030/design.md`. Cover lifecycle, invalid plans, confirmation
windows, delayed updates, same-time ordering, rollover, stop dominance,
resume, fixed-capacity exhaustion, audit encoding, and shutdown. Fix only
confirmed core/test defects. Acceptance: focused strict-warning tests pass,
interfaces remain inert-only and C++11/AVR compatible, no launcher or
energetic semantics appear, and a review event records remaining integration
work.
