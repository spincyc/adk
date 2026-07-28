# Lesson progression and learner-flow audit

Date: 2026-07-28
Scope: published Lessons 001--033

## Decision

Learner artifacts teach through a visible sequence:

1. predict the next state;
2. add or configure one dependency;
3. observe a non-Serial result;
4. interpret that result before the next behavior unlocks.

Electrical safety, specimen identity, and honest support boundaries remain in
the lesson where they protect the learner. Repository builds, deterministic
tests, publication gates, physical-acceptance administration, instructor
forms, and command-line maintenance stay outside the learner workflow.

## Completed corrections

- Removed learner-facing acceptance cards, reviewer/sign-off forms, manual
  replay sheets, and repository-maintainer command blocks.
- Added staged, dependent visible synthesis to the early Reaction Timer and
  Simon projects and to Lessons 025--033.
- Added fixed input-independent endings to the later interactive examples.
- Restored immediate next-lesson navigation and cumulative prerequisite
  boundaries where they had drifted.
- Aligned D13 acquisition vocabulary across examples and lessons.
- Corrected false carry-forward claims: Lesson 018 does not reuse the Lesson
  017 servo, and Lesson 021 is explicitly a model-only six-lamp synthesis that
  does not preserve physical range sensing or PWM magnitude.
- Replaced Lesson 022's learner-facing moisture vocabulary with neutral
  control-observation and record language while retaining historical public
  API names only where required.

## Verification

All published examples pass host, style, header, Mega compilation, and measured
size gates. All 33 PDFs pass visual classification, true monochrome, and PDF
policy checks. The complete site builds and validates. Rendered lesson ranges
were reviewed for flow in addition to the automated checks.

Physical observations remain open and are never inferred from these software
or publication results.
