---
schema_version: 1
task_uuid: "764b2584-21f9-40c2-a92d-e1714c45d8e2"
title: "audit lesson progression and verification burden"
status: "queued"
priority: "normal"
priority_reason: "User explicitly queued a curriculum-flow audit; it should inform lesson presentation without weakening active safety or physical-claim gates."
parent: null
discovered_by: null
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-28T01:51:46Z"
updated_at: "2026-07-28T01:54:21Z"
---

# Goal

Ensure lessons build concepts in a natural progression and keep verification proportionate to learning flow, while preserving necessary safety, specimen, and support-claim gates in clearly separated acceptance material.

## Acceptance criteria

- Audit lesson-to-lesson prerequisites, concept order, vocabulary, examples,
  and projects for a coherent increase in difficulty.
- Keep the learner-facing narrative centered on prediction, construction,
  observation, and interpretation; avoid interleaving every implementation
  detail with release-grade acceptance administration.
- Preserve electrical safety, exact-specimen gates, and honest physical
  support boundaries wherever omission could cause harm or a false claim.
- Remove physical acceptance cards from learner-facing lesson PDFs and HTML.
- Replace card-driven learner flow with self-validating staged builds: each
  engaging behavior should depend on the preceding wiring, observation, and
  reasoning being correct, with useful visible failure states.
- Delete the separate instructor/reviewer acceptance workflow; there is no
  instructor audience.
- Keep automated repository tests, build checks, safety review, and
  publication validation behind the scenes so learner artifacts work, but do
  not turn them into learner assignments or forms.
- Make progression resistant to shortcuts through observable dependencies:
  later behavior should only work after the earlier circuit and concept work.
- Use progressive disclosure: essential checks in the main experiment,
  deeper deterministic and physical qualification after the concept is
  established.
- Verify that projects synthesize prior skills rather than introducing several
  unprepared concepts at once.
- Review the rendered PDF and HTML flow with representative beginner tasks,
  not only structural validators.

## Scope assumption pending clarification

Remove physical acceptance cards and instructor/reviewer records entirely.
Retain concise safety and specimen constraints where they protect the family.
Keep automated repository verification behind the scenes unless the user
explicitly directs its removal.

## Original request

enqueue: make sure the lessons have appropriate progressions, not "autistic" verification that will materially impede knowledge flow ; chat to clarify
