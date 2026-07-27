---
schema_version: 1
event_uuid: "400bec8e-a19c-4aa0-81f3-ff3c41cc2aed"
event_type: "ingestion"
scope: "task"
task_ids: ["5801d853-3278-4a52-84dd-7e25c0794d7c"]
agent_instance_uuid: "65af281e-a45f-4392-aa85-5dbfe2b1505a"
created_at: "2026-07-27T22:00:43Z"
---

# Request ingestion

Created through bounded enrichment. Review the task goal, acceptance criteria, dependencies, overlap, and redactions before activation.

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
