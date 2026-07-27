---
schema_version: 1
task_uuid: "3cbaec90-9c9d-421e-b129-34bbdb477340"
title: "repair bootstrap and serial tooling"
status: "queued"
priority: "normal"
priority_reason: "Recovered audit prompt 6; ordered by the repository recovery audit."
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

Create a normal-priority tooling task, parallel-safe with lesson core work
but serialized for shared docs, to add `git` to Arch bootstrap packages, pin
the local AVR core to the CI version, preserve Arduino monitor failures
through `serial-log`, replace the stale `make serial-monitor` command in
lesson 014, and reproducibly rebuild the affected PDF.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Keep shell and Make behavior portable and preserve monitor failures.
- Rebuild only the affected lesson PDF and validate documentation.

## Original request

Create a normal-priority tooling task, parallel-safe with lesson core work
but serialized for shared docs, to add `git` to Arch bootstrap packages, pin
the local AVR core to the CI version, preserve Arduino monitor failures
through `serial-log`, replace the stale `make serial-monitor` command in
lesson 014, and reproducibly rebuild the affected PDF. Acceptance: shell/Make
behavior is portable and failure-tested where practical; local and CI version
guidance agrees; lesson/PDF/site checks pass; unrelated generated PDFs do not
change.
