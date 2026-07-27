---
schema_version: 1
task_uuid: "3a7d0556-e29c-4a9f-9f71-0cce6c8caf23"
title: "Audit repository and reconstruct lost work queue"
status: "active"
priority: "high"
priority_reason: "The audit restores lost operational scope and must precede further queue-driven development."
parent: null
discovered_by: null
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T21:53:46Z"
updated_at: "2026-07-27T21:53:57Z"
---

# Goal

Audit the entire ADK repository, identify unfinished or lost queued work from durable evidence, and produce an ordered series of journal-ready prompts that safely primes the reconstructed work queue.

## Acceptance criteria

- The canonical journal protocol and helper are installed, validated, and
  committed with fresh ADK-specific state.
- The audit covers tracked first-class source, tests, examples, lessons,
  documentation, build/package/publication machinery, generated deliverables,
  research tracks, Git history, and explicit deferred physical work.
- Findings distinguish confirmed unfinished work, contradictions, stale
  evidence, inferred opportunities, and work that is intentionally deferred.
- Every proposed queue item has bounded scope, dependencies, acceptance
  evidence, priority rationale, and a journal-ready ingestion prompt.
- The proposed prompts are ordered so they can safely prime the journal without
  creating unsupported product, hardware, release, or publication claims.
- Journal state and the authoritative `docs/WORK_QUEUE.md` are reconciled before
  completion, and relevant validation results are recorded.

## Scope and constraints

- Preserve all unrelated repository content and history.
- Do not push, publish, tag, change branches, or claim physical verification.
- Treat `legacy/` as frozen and exclude it from first-class implementation
  recommendations.
- This task audits and proposes prompts; it does not silently implement every
  discovered queue item.

## Original request

this project had a lot of unfinished and queued work that was lost due to context spilling ; audit the entire repo ; propose a series of prompts to feed into the new
  journal system to prime the work queue ; initialize the jouraling system first ; confirm
  understanding

proceed
