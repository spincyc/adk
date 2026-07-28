---
schema_version: 1
task_uuid: "1b108ac8-49ec-4e0f-aebd-0d8624d7bef5"
title: "clean up landing page and expose planned work"
status: "queued"
priority: "normal"
priority_reason: "User explicitly queued a publication usability and completeness task; it does not supersede the active high-priority pre-bench remediation."
parent: null
discovered_by: null
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-28T01:45:40Z"
updated_at: "2026-07-28T01:45:40Z"
---

# Goal

Make the published landing page uncluttered, easy to scan and scroll, and complete: show delivered work with valid links and all planned lesson/research work as concise linkless table entries until artifacts exist.

## Acceptance criteria

- Audit the actual published landing page at desktop and narrow/mobile widths.
- Present delivered work with valid links and planned work as plain, linkless
  table entries so the page never implies that an unavailable artifact exists.
- Cover the planned lesson curriculum through Lesson 081 and the retained
  USB/HDMI/shared-fabric research tracks unless the user narrows the scope.
- Keep status, subject, and availability language consistent with
  `docs/WORK_QUEUE.md`, curriculum, roadmap, and release claims.
- Reduce visual clutter, avoid oversized repeated prose, keep tables readable
  and horizontally safe, and make the page easy to scan and scroll.
- Preserve accessibility, heading order, responsive behavior, and print/link
  validation; inspect the rendered result rather than relying on source alone.

## Scope assumption pending clarification

“All planned work” includes both the lesson curriculum and retained research
tracks. Only existing artifacts receive links.

## Original request

enqueue: clean up the landing page ; put in linkless table entries for all planned work ; make sure the page presents well and uncluttered, is easy to scroll, etc. ; chat to clarify
