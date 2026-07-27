---
schema_version: 1
task_uuid: "73b4e704-b483-424e-ab0c-6ffcac540ef3"
title: "reconcile release and lint policy"
status: "done"
priority: "normal"
priority_reason: "Recovered audit prompt 9; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["efdb8701-d7fe-490e-9a85-d0116f37e4fc", "3cbaec90-9c9d-421e-b129-34bbdb477340", "cb197bb3-b7e1-4054-8bfe-92f7f27963f2", "957366fa-d370-4060-89a3-40ec5c4a1722"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T23:32:16Z"
---

# Goal

Create a normal-priority release-policy task, dependent on tooling,
packaging, site, and the intended lesson boundary, to reconcile the documented
Arduino Lint submit/update requirements with local Make and CI. Audit version,
changelog, metadata, archive contents, generated artifacts, status tables,
and release notes.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Record an explicit submit/update lint-policy decision and reproducible
  release gate.
- Do not tag, push, publish, or submit to Library Manager.

## Original request

Create a normal-priority release-policy task, dependent on tooling,
packaging, site, and the intended lesson boundary, to reconcile the documented
Arduino Lint submit/update requirements with local Make and CI. Audit version,
changelog, metadata, archive contents, generated artifacts, status tables,
and release notes. Acceptance: a documented decision names which lint modes
run for initial inclusion and later updates; the complete release gate is
reproducible; failures remain queued with owners. Do not tag, push, publish,
or submit to Library Manager.
