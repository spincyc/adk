---
schema_version: 1
task_uuid: "f7517e0d-8ca0-4a85-88d5-e7657de132b3"
title: "plan the physical acceptance campaign"
status: "blocked"
priority: "normal"
priority_reason: "Recovered audit prompt 11; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-28T00:48:37Z"
---

# Goal

Create a normal-priority, human-bench-dependent parent task for physical Mega
2560 acceptance of lessons 001--030 in numeric order. Decompose one child per
lesson only when the exact board, specimen, supply, instruments, and operator
are available. Each child must separately record resource acquisition,
primary behavior, non-Serial evidence, shutdown/reset/power-removal safe
state, and any E2/external-power boundary. Software agents may prepare cards
and review evidence but must never fabricate observations or mark a child
complete without the signed bench record.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Decompose physical children only when the named operator and exact equipment
  are available.
- Never fabricate bench observations or infer them from software evidence.

## Original request

Create a normal-priority, human-bench-dependent parent task for physical Mega
2560 acceptance of lessons 001--030 in numeric order. Decompose one child per
lesson only when the exact board, specimen, supply, instruments, and operator
are available. Each child must separately record resource acquisition,
primary behavior, non-Serial evidence, shutdown/reset/power-removal safe
state, and any E2/external-power boundary. Software agents may prepare cards
and review evidence but must never fabricate observations or mark a child
complete without the signed bench record.

## Activation

The software-preparation boundary is active. No per-lesson physical child will
be created until a named operator and exact board, specimens, supply, and
instruments are available.

## Blocker

The preparation audit is complete. Physical child creation requires a named
operator, reviewer, exact board/specimens, supply/current limit, and
instruments. Software/publication defects discovered by the audit are retained
in task `ba1ed653-0cb2-4275-ae36-16c88591ce35`; that runnable work proceeds
without fabricating bench results.
