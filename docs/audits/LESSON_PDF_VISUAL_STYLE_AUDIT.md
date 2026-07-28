# Lesson PDF visual-style audit

Date: 2026-07-28
Scope: published Lessons 001–033
Authority: [PDF publication policy](../PDF_POLICY.md)

## Decision

Every non-schematic lesson visual uses pencil-drawing presentation. Formal
electrical schematics may retain conventional precise drafting only when they
are explicitly identified and electrically authoritative. Filenames,
monochrome output, and rasterization do not establish compliance.

The audit counts physical layouts, orientation plates, progress sequences,
state and timing diagrams, conceptual flows, charts, and troubleshooting art.
It excludes ordinary prose, tables, equations used as notation, code listings,
and decorative rules.

## Acceptance method

For each lesson:

1. inventory `\includegraphics`, visual `\input`, `tikzpicture`, and
   `circuitikz` constructs;
2. classify each visual as `pencil` or `schematic` immediately in the TeX
   source;
3. convert every non-schematic clean-vector or defective raster visual;
4. compare wiring art with canonical pins, connection tables, polarity,
   breadboard topology, and specimen gates;
5. rebuild and visually inspect the PDF;
6. run the visual-classification, PDF policy, and monochrome gates.

## Audit ledger

| Lessons | Verified result |
|---:|---|
| 001–004 | Conceptual, lifecycle, timing, state, and architecture art converted; electrically authoritative circuit art retained or replaced with formal schematics |
| 005–008 | Lesson 005 locator corrected; process/state art converted; Lessons 007–008 pseudo-schematics replaced with formal schematics |
| 009–012 | Lesson 009 formal schematic retained; all other locator, layout, flow, state, and staged-build visuals converted |
| 013–016 | Nine clean TikZ visuals converted; four pencil-treated wiring assets retained and verified |
| 017–020 | Embedded layout and state visuals converted; shared keypad ownership and exact build dependencies reconciled |
| 021–024 | Literal layouts, record/progress plates, state/diagnosis art, and standalone visual sequences converted |
| 025–027 | All plates and flow art converted; corrected infrared/telemetry nets preserved and incomplete labels repaired |
| 028–030 | Defective or unlabeled orientation exports replaced with legible pencil drawings |
| 031–033 | Joystick, Gray-code, console, and evidence-chain art converted; Lesson 031 retains the correct 330-ohm value |

All 33 rebuilt PDFs pass the visual-classification, PDF-policy, and true
monochrome gates. Each range received rendered-output inspection in addition
to the automated checks.

## Future gate

`make lessons-check` runs `scripts/check_lesson_visual_policy.py`. The automated
gate ensures every source visual records one of the two allowed
classifications and prevents plain TikZ from claiming the formal-schematic
exception. Human review remains required for visible pencil treatment,
legibility, electrical correctness, and truthful classification.
