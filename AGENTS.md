# ADK agent contract

Read these before changing first-class code or lessons:

1. `docs/DEVELOPMENT.md` — hierarchy, boundaries, and acceptance gates
2. `docs/STYLE.md` — mandatory C++ layout and naming
3. `docs/CURRICULUM.md` — canonical lesson and project numbers
4. `docs/TESTING.md` — deterministic and hardware evidence
5. `docs/SAFETY_MODEL.md` — electrical and project limits
6. `docs/PACKAGING.md` — Arduino and release layout
7. `docs/PDF_POLICY.md` — printable-document requirements

`legacy/` is frozen and unsupported. Do not include it from first-class code,
examples, tests, or release metadata.

Develop in dependency order. Each component needs a clean header, out-of-line
implementation, deterministic tests, Mega 2560 example, HTML reference, rich
PDF lesson, size evidence, and explicit deferred hardware checks. Every lesson
number divisible by three is a multi-component project.

Never claim physical verification without a recorded bench acceptance result.
Never add pyrotechnic ignition, launcher control, or unknown-protocol replay.
