---
schema_version: 1
task_uuid: "cb197bb3-b7e1-4054-8bfe-92f7f27963f2"
title: "add native C++ consumer packaging"
status: "queued"
priority: "normal"
priority_reason: "Recovered audit prompt 7; ordered by the repository recovery audit."
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

Create a normal-priority packaging task to define and implement a
repository-native C++ archive/export and clean consumer compile/link smoke
test without weakening Arduino packaging. Specify supported headers,
compiled sources, install/archive layout, C++ standard, and absence of
repository-relative dependencies.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Prove a clean native compile/link consumer without weakening Arduino
  packaging.
- Make no installation claim beyond demonstrated archive evidence.

## Original request

Create a normal-priority packaging task to define and implement a
repository-native C++ archive/export and clean consumer compile/link smoke
test without weakening Arduino packaging. Specify supported headers,
compiled sources, install/archive layout, C++ standard, and absence of
repository-relative dependencies. Acceptance: a temporary clean consumer
builds from the exported artifact, Arduino package-smoke still passes, docs
make only the demonstrated installation claim, and no framework or package
manager is introduced.
