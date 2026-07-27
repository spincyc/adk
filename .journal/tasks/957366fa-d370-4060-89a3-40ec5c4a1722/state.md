---
schema_version: 1
task_uuid: "957366fa-d370-4060-89a3-40ec5c4a1722"
title: "finish site navigation and deployment verification"
status: "active"
priority: "normal"
priority_reason: "Recovered audit prompt 8; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["d6a8e538-7ae1-4522-9b37-ffa79055b36b"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-27T23:11:14Z"
---

# Goal

Create a normal-priority publication-readiness task dependent on the current
active lesson boundary to audit landing-page hierarchy and navigation, then
add post-deployment checks for the live landing page, newest lesson HTML,
newest PDF, and downloadable example. Keep external deployment mutations out
of ordinary tests.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Validate navigation and post-deployment checks for the newest promoted
  lesson boundary.
- Do not deploy or mutate live publication state without separate authority.

## Original request

Create a normal-priority publication-readiness task dependent on the current
active lesson boundary to audit landing-page hierarchy and navigation, then
add post-deployment checks for the live landing page, newest lesson HTML,
newest PDF, and downloadable example. Keep external deployment mutations out
of ordinary tests. Acceptance: local site validation covers the newest
boundary; the Pages workflow fails on bad HTTP responses or missing expected
content; permissions remain least-privilege; live publication itself is not
performed without separate authority.
