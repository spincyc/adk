---
schema_version: 1
task_uuid: "32f5c784-de10-4f37-ae31-0f611697c8e3"
title: "execute the full clean release-readiness gate"
status: "active"
priority: "high"
priority_reason: "Recovered audit prompt 10; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["efdb8701-d7fe-490e-9a85-d0116f37e4fc", "3cbaec90-9c9d-421e-b129-34bbdb477340", "cb197bb3-b7e1-4054-8bfe-92f7f27963f2", "957366fa-d370-4060-89a3-40ec5c4a1722", "73b4e704-b483-424e-ab0c-6ffcac540ef3"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T23:32:51Z"
---

# Goal

Create a high-priority verification task hard-dependent on completion of the
intended lesson, tooling, packaging, site, and release-policy tasks. From a
clean tree, run the complete host, exception, sanitizer, research-model,
header, style, Mega, size, lesson, monochrome/PDF, site, archive-consumer, and
lint gates. Record exact tool versions, measurements, failures, owners, and
next actions in the journal and work queue.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Run the complete clean release-readiness evidence matrix and durably assign
  every failure.
- Make no hardware, tag, push, release, or live-publication claim.

## Original request

Create a high-priority verification task hard-dependent on completion of the
intended lesson, tooling, packaging, site, and release-policy tasks. From a
clean tree, run the complete host, exception, sanitizer, research-model,
header, style, Mega, size, lesson, monochrome/PDF, site, archive-consumer, and
lint gates. Record exact tool versions, measurements, failures, owners, and
next actions in the journal and work queue. Acceptance: all runnable gates
pass or have durable blockers; no hardware, release, tag, push, or live
publication claim is made.
