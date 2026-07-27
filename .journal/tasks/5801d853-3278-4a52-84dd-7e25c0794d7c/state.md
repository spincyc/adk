---
schema_version: 1
task_uuid: "5801d853-3278-4a52-84dd-7e25c0794d7c"
title: "reconcile the recovered ledger"
status: "queued"
priority: "high"
priority_reason: "Recovered audit prompt 1; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T22:00:43Z"
---

# Goal

Create a high-priority task to reconcile the repository status surfaces with
the 2026-07-27 recovery audit. Correct the stale lesson-PDF target using
direct evidence, update `docs/WORK_QUEUE.md`, `docs/ROADMAP.md`,
`docs/PROJECTS.md`, `docs/SUBAGENTS.md`, and stale lesson/site status prose,
and preserve historical records explicitly rather than silently rewriting
them.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Reconcile only status and documentation; do not implement lesson code or
  claim hardware evidence.
- Validate journal and documentation consistency before completion.

## Original request

Create a high-priority task to reconcile the repository status surfaces with
the 2026-07-27 recovery audit. Correct the stale lesson-PDF target using
direct evidence, update `docs/WORK_QUEUE.md`, `docs/ROADMAP.md`,
`docs/PROJECTS.md`, `docs/SUBAGENTS.md`, and stale lesson/site status prose,
and preserve historical records explicitly rather than silently rewriting
them. Acceptance: every active/queued/deferred claim agrees with
`CURRICULUM.md`; lesson 029 is the only active lesson integration boundary;
030 is queued behind it; 031--081 and all physical/research deferrals remain
visible; documentation checks pass. Do not implement lesson code or claim
hardware evidence.
