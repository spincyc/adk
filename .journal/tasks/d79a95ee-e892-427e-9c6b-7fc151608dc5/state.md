---
schema_version: 1
task_uuid: "d79a95ee-e892-427e-9c6b-7fc151608dc5"
title: "register lesson 029 core"
status: "queued"
priority: "high"
priority_reason: "Recovered audit prompt 3; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["0702f4b1-501a-4fca-a6b0-cac73da74125"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T22:00:43Z"
---

# Goal

Create a high-priority task dependent on the lesson 029 core review to add
`CueAudit` and `InertCueScheduler` to the normal host-test build and supported
umbrella surface. Preserve dependency order and add no example or lesson
prose yet.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Register both focused tests in ordinary and sanitizer paths and export only
  the reviewed public headers.
- Preserve the separation between core registration and lesson packaging.

## Original request

Create a high-priority task dependent on the lesson 029 core review to add
`CueAudit` and `InertCueScheduler` to the normal host-test build and supported
umbrella surface. Preserve dependency order and add no example or lesson
prose yet. Acceptance: both focused tests run under ordinary, exception,
sanitizer, style, and standalone-header gates; all earlier tests pass; public
exports match the reviewed contract; the work queue records the completed
component/test sub-boundary.
