---
schema_version: 1
task_uuid: "778a0a46-7969-4a38-b206-8ed54ca72d03"
title: "audit and complete lesson wiring illustrations"
status: "queued"
priority: "normal"
priority_reason: "User explicitly enqueued a repository-wide documentation and wiring-accuracy audit; it is important but does not supersede the active high-priority numeric pre-bench remediation."
parent: null
discovered_by: null
hard_dependencies: []
soft_dependencies: []
related_to: []
superseded_by: null
created_at: "2026-07-28T01:25:42Z"
updated_at: "2026-07-28T02:02:23Z"
---

# Goal

Ensure every published lesson has enough accurate pencil-style drawings to reproduce its complete wiring, with pin positions, component orientation, breadboard topology, rails, and connections matching the canonical example and exact supported layout; establish the same gate for future lessons as they are implemented.

## Acceptance criteria

- Audit every currently published lesson for enough illustrations to assemble
  the complete circuit without inventing omitted connections.
- Verify each drawing against the canonical example, pin/resource tables, and
  lesson wiring text, including actual Mega header positions, breadboard rows
  and rail breaks, component orientation and polarity, resistor placement,
  supply and ground, test points, and all jumpers.
- Add or correct pencil-style drawings where the audit finds a gap; do not use
  a generic conceptual diagram as a substitute for a reproducible layout.
- Preserve exact-specimen gates where module pin order or board revision is
  unresolved; never make a drawing imply an electrically qualified pinout.
- Add the same illustration-accuracy gate to future lessons as they are
  implemented.
- Audit all hands-on projects, with the potato launcher first, and add multiple
  useful pencil drawings wherever they materially improve assembly or
  understanding.
- Treat visual progress indicators as first-class instruction: show the
  expected visible state after each meaningful build stage so learners know
  when they are ready for the next payoff.
- Prefer complementary overview, exact wiring, staged assembly, checkpoint,
  and troubleshooting plates; “more” must add information rather than repeat
  decoration.
- Build and visually inspect affected PDFs, and keep HTML/download references
  consistent.

## Scope assumption pending clarification

Apply the audit to published Lessons 001--033 first. Lessons 034--081 receive
the gate when their implementation boundary creates canonical circuits; do not
invent present-day wiring for lessons that do not yet exist. Audit buildable
projects in parallel, beginning with the potato launcher. Add research-track
visuals only when they clarify a real assembly or signal path.

## Original request

enqueue: make sure every lesson has sufficient pencil drawings to demonstrate wiring everything up ; make sure the pencil drawing accurately reflect the actual layout of pins, breadboard, etc. ; chat for clarity ;
