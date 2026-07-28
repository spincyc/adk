---
schema_version: 1
task_uuid: "d6769eef-c69b-457c-bdf1-5aa7498ae5ab"
title: "implement lessons 031--033"
status: "active"
priority: "normal"
priority_reason: "Recovered audit prompt 13; ordered by the repository recovery audit."
parent: null
discovered_by: "f28da18e-494a-428a-a5a6-473f866567c7"
hard_dependencies: ["efdb8701-d7fe-490e-9a85-d0116f37e4fc", "fb9510ee-13cf-4bb7-a7ad-7b6f7bfce3df"]
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-27T22:00:43Z"
updated_at: "2026-07-28T00:00:56Z"
---

# Goal

Create a normal-priority task hard-dependent on lesson 030 promotion and the
relevant exact inventory to implement the already detailed 031--033 input
expansion block in strict dependency order: analog joystick, quadrature
encoder, then calibration console. Use
`docs/design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md` as the contract.

## Acceptance criteria

- Satisfy every acceptance condition in the original request below.
- Implement lessons 031--033 in strict dependency order from the reviewed
  design and exact inventory.
- Advance every shared status surface in the same integration boundary.

## Original request

Create a normal-priority task hard-dependent on lesson 030 promotion and the
relevant exact inventory to implement the already detailed 031--033 input
expansion block in strict dependency order: analog joystick, quadrature
encoder, then calibration console. Use
`docs/design/LESSONS_031_033_INPUT_EXPANSION_PLAN.md` as the contract.
Acceptance: each lesson receives its complete component/test/example/size/
HTML/PDF/open-bench package; lesson 033 proves composition; all earlier gates
remain green; status documents advance together.

## Activation

Both hard dependencies are complete. The authorized Elegoo union establishes
the joystick and rotary-encoder product families; implementation may proceed
with host contracts while exact-revision electrical claims and bench
acceptance remain explicit open gates.
