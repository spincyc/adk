# ADK agent contract

Read these before changing first-class code or lessons:

1. `docs/DEVELOPMENT.md` — hierarchy, boundaries, and acceptance gates
2. `docs/STYLE.md` — mandatory C++ layout and naming
3. `docs/CURRICULUM.md` — canonical lesson and project numbers
4. `docs/TESTING.md` — deterministic and hardware evidence
5. `docs/SAFETY_MODEL.md` — electrical and project limits
6. `docs/PACKAGING.md` — Arduino and release layout
7. `docs/PDF_POLICY.md` — printable-document requirements
8. `docs/WORK_QUEUE.md` — authoritative active, queued, deferred, and physical work

Re-read `docs/WORK_QUEUE.md` before assigning work, after every lesson or
project integration, and before any release or publication audit. Update it in
the same boundary whenever work is added, completed, split, deferred, or
removed for a documented reason.

`legacy/` is frozen and unsupported. Do not include it from first-class code,
examples, tests, or release metadata.

Develop in dependency order. Each component needs a clean header, out-of-line
implementation, deterministic tests, Mega 2560 example, HTML reference, rich
PDF lesson, size evidence, and explicit deferred hardware checks. Every lesson
number divisible by three is a multi-component project.

Examples are narrative code. Introduce objects in dependency order; shape
`setup()` as acquire, configure, start; and shape `loop()` as observe, decide,
actuate. Use domain-action helper names, place high-level flow before low-level
mechanics, and keep code and lesson vocabulary identical. Avoid comment-heavy
code and needless one-line helper decomposition.

Every circuit needs a non-Serial observation path: an LED, sounder, display, or
named electrical test point. Lessons state what to predict, where and when to
observe it, and how to interpret it. Resource-acquisition evidence and
safe-state evidence are separate checks. Serial output is optional supporting
evidence, never the only proof.

Never claim physical verification without a recorded bench acceptance result.
Never add pyrotechnic ignition, launcher control, or unknown-protocol replay.
