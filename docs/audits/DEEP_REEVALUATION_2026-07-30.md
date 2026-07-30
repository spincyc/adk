# Deep project re-evaluation — 2026-07-30

Nineteen parallel read-only auditors swept four tracks: object design across
all of `src/` (nine component groups), lesson content for every published
lesson page (eight blocks), landing-page navigation, and site aesthetics.
They returned 249 findings, 71 high severity. The complete machine-readable
findings are in
[`deep_reevaluation_2026-07-30_findings.json`](deep_reevaluation_2026-07-30_findings.json);
this document records the synthesis, the one immediate correctness fix, and
the remediation program. It does not change any published support claim.

## Immediate correctness fix (landed with this audit)

`StorageRecordSink::append` passed `StableRecord::length` to storage without
validating it against `StableRecord::capacity` (96), so a record with length
97–255 read up to 159 bytes past the end of `text`. The sink now rejects
oversized lengths with `InvalidArgument` before touching storage, and the
record-sink test covers the boundary. No other audited finding is a memory-
safety defect.

## Follow-up finding — eight drawings shipped with unreadable labels (resolved)

**Resolved 2026-07-30.** No font package was installed on the build host, but
TeX Live already ships the DejaVu TrueType family the assets request, so
fontconfig was pointed at
`/usr/share/texmf-dist/fonts/{truetype,opentype}/public` through a user-level
`~/.config/fontconfig/fonts.conf` rather than installing a duplicate copy. All
eight assets were re-rendered and lessons 009, 016, 018, and 069 rebuilt; every
lesson gate passes. Lesson 018 benefits because it reuses lesson 016's keypad
plate.

Two facts surfaced during the repair:

- `021-rover-pencil` is an **orphan**: lesson 021 includes
  `021-rover-layout-pencil`, so the broken asset was never published. Ten
  asset PNGs are referenced by no lesson, overlay, or Make rule —
  `009-night-light-pencil`, `011-timed-traffic-pencil`,
  `018-access-trainer-pencil`, `020-motor-intent-layout`, `021-rover-pencil`,
  and `025-infrared-evidence-pencil`; the four `*-base` files are *not*
  orphans, being inputs to companion overlay documents.
- The repository already has a **font-independent labelling pattern**: a
  standalone TikZ document (`docs/lessons/assets/<name>.tex`) places a
  text-free base PNG and overlays labels using LaTeX's own fonts, as
  `030-process-pencil.tex` does. This is the more robust route for new
  assembly plates than SVG `<text>`, because it cannot regress on a host
  without fontconfig fonts. Neither asset pipeline has a Make rule; both
  produce committed artifacts by hand, which is why the regression went
  unnoticed.

The original finding follows.

## Original finding — eight drawings shipped with unreadable labels

Investigated 2026-07-30 while producing the first new drawing. The pencil
assets are hand-authored SVG (an `feTurbulence` + `feDisplacementMap` filter
over plain shapes) rendered to grayscale PNG by hand; there is no Make pattern
rule, so each PNG is committed as rendered on whatever machine produced it.

Thirty-five of the 72 SVG assets contain `<text>`. Re-rendering each one on a
machine with **no fontconfig fonts installed** and comparing against the
committed PNG identifies eight whose committed bytes match the fontless
render, meaning they were produced without fonts and every label is a
missing-glyph box:

| Asset | Fontless-render difference |
|---|---:|
| `021-rover-pencil` | 0.00000 |
| `069-one-source-session-pencil` | 0.00000 |
| `069-replay-notebook-pencil` | 0.00115 |
| `069-fault-dominance-pencil` | 0.00195 |
| `009-night-light-breadboard` | 0.00752 |
| `009-mega-header-locator` | 0.00769 |
| `016-matrix-keypad-pencil` | 0.00826 |
| `009-night-light-overview` | 0.00888 |

Two are byte-identical to a fontless render, which is conclusive. The other 27
text-bearing assets differ substantially and carry real glyphs.

This reaches readers: these PNGs are embedded in published lesson PDFs, so
lessons 009, 016, and 069 ship boxes where their labels belong — and the three
lesson 009 plates are exactly the assembly drawings the audit recommends
copying. No existing gate catches it, because the monochrome and PDF-policy
checks inspect colour and classification markers, not glyph coverage.

Remediation needs a font installed (`ttf-dejavu` satisfies the
`DejaVu Sans` family the assets request) followed by re-rendering those eight
assets and rebuilding their PDFs; it is a system change, so it is left to the
maintainer. Two durable consequences meanwhile:

1. New drawings carry their wording in the LaTeX caption and keep text out of
   the SVG, which is already the majority pattern for recent lessons
   (`080-*` has no `<text>` at all).
2. A glyph-coverage gate belongs in `lessons-check`, or the SVG-to-PNG render
   belongs in a Make rule so assets cannot be committed from an
   under-provisioned machine.

### Correction — the Lesson 006 assembly-visual finding was overstated

The auditor recorded Lesson 006 as having "three pencil assets that are all
concept diagrams" and needing breadboard plates. With labels rendering again,
`006-simon-pencil` turns out to be a strong full-panel drawing that already
names D22--D25, D30--D33 with their 330\,ohm resistors, the RGB common cathode,
and the 100\,ohm piezo, plus a sequence sidebar. A second TikZ placement plate
was drafted and discarded because it duplicated that content with weaker
artwork.

What Lesson 006 actually lacks is what Lesson 009 Plate B has: **hole-level
breadboard coordinates**. That gap cannot be closed by drawing alone, because
the lesson never specifies a layout — inventing coordinates would fabricate
build guidance the text does not support. Closing it needs a content decision
first (choose and state the layout), then the plate.

Treat the same caution as general: before drawing a plate for any lesson,
check whether the lesson specifies a layout to draw. Where it does not, the
finding is a content gap, not an illustration gap.

### Caption-only text does not substitute for plate labels

Concept drawings survive the caption-only rule because their meaning is
carried by arrangement. Assembly plates do not: their entire purpose is
saying *which* part belongs at *which* position, so `D22` versus `D30` and
`330R` versus `100R` must appear on the plate itself. A first attempt at a
Lesson 006 placement plate was discarded for exactly this reason — the
geometry rendered correctly, but with no labels a learner could not tell the
four button positions from the four cue-LED positions, and shipping a
decorative plate into a published PDF is worse than the acknowledged gap.

Phase 4 therefore depends on labels being renderable. Either install the font,
or make labels font-independent by drawing them as stroked SVG paths from a
small reusable glyph set (digits, `D`, `A`, `k`, `R`, and ohm). The stroke-glyph
route is the stronger option: it removes the font dependency permanently, so
this failure mode cannot recur on another under-provisioned machine, and the
same glyph set repairs the eight broken assets. It is recorded as the
`svg-stroke-labels` tool candidate.

## Correction — seven assets were repaired, not eight

Re-checked 2026-07-30 after the retraction below. `016-matrix-keypad-pencil`
was a **false positive**: its SVG carries `<style>text { display: none; }</style>`,
so all 41 of its labels — including the `A`/`B`/`C`/`D` key legends — are
suppressed by the drawing itself, not by a missing font. Re-rendering it
changed nothing a reader can see. Seven assets were genuinely repaired;
`069-fault-dominance-pencil` and the three lesson 009 plates were verified
legible afterwards.

The detection method's flaw is worth stating, because it was used to justify
the whole repair: comparing a fontless render against the committed PNG finds
assets that *match*, and a match means the text is invisible in both — which
happens when the text was rendered without a font **or** when the text is
hidden, absent, or drawn in the background colour. The test cannot distinguish
those causes. Confirming a tofu render requires looking at the committed image,
which was done for lessons 009 and 069 but not for 016.

Lesson 016's hidden labels are now an open question: a keypad drawing whose key
legends are suppressed is either a deliberate caption-only choice or a mistake,
and lesson 018 reuses the same asset. Resolving it needs the author's intent,
not another re-render.

## Retraction — the Lesson 056 operator-panel plate was withdrawn

A plate drawn for Lesson 056 on 2026-07-30 was published and then reverted the
same day after a self-review checked it against the lesson text rather than
against the gates. Three defects, none of which any gate can detect:

1. **Wrong control model.** The lesson's "one qualified control" means four
   mask bits — `Previous`, `Next`, `Select`, `Acknowledge` — of which exactly
   one may be asserted, so that a chord is rejected. The plate drew a single
   rotary knob, which cannot express a chord at all and so made the lesson's
   central rule look trivial instead of load-bearing.
2. **A fabricated quantity.** The caption asserted "three status cells". The
   word *three* appears nowhere else in the lesson; the count is the
   configurable `selectableCellCount`.
3. **A contradicting metaphor.** The plate used a mushroom emergency-stop
   actuator for a lesson whose section is titled "Let stop dominate without
   inventing safety" and which states it owns no safety function.

The durable lesson: passing the visual-policy, monochrome, and PDF-policy
gates establishes classification and colour, never that a drawing depicts what
its lesson says. Before any new plate is committed, every drawn element must
trace to a sentence in the lesson, and the caption must assert no quantity the
lesson does not state.

## Track 1 — object design (`src/`, 121 findings)

The library is disciplined about inert construction, idempotent initialize,
snapshot publication, and copied-evidence boundaries. The systemic costs are
repetition and convention drift, not architecture failure:

1. **Init/rollback ceremony is caller-owned everywhere.** ~24 sketches
   hand-write the same initialize-all/unwind-in-reverse staircase, and the
   library itself repeats it inside `RgbLed` and `ShiftRegisterOutput`.
   Remedy: one small initialize/shutdown seam plus a fixed-capacity component
   chain that unwinds on first failure.
2. **The wraparound-safe time guard is copy-pasted into ~35 policies**
   (`halfRange` constant, last-update tracking, forward-time validity), and
   the dwell-qualified state machine appears five times in arc 031–045 alone.
   Remedy: own the modular-chronology contract once in `time.h` and extract a
   shared dwell qualifier.
3. **Evidence-identity headers are re-declared per struct.** The
   lifecycle/session/run/step/request/source/sequence/time/status block
   appears in ~10 sibling structs across lessons 079–081 with inconsistent
   field order, and expected-producer triplets recur in four configs.
   Remedy: a shared correlation-header value type embedded first in each
   evidence struct, plus one `ExpectedProducerIdentity` value type.
4. **Convention drift across four eras.** `shutdown` has three shapes,
   the published-state accessor has four spellings (`snapshot()` by value in
   ~40 types versus Status-out-param and `evidence()` variants),
   `ThresholdInput` follows no sibling convention, and two pin-validity
   authorities coexist (`NUM_DIGITAL_PINS` versus the board capability
   table). Remedy: converge on value-returning `snapshot()` and
   `void shutdown()` (the direction 079/080 already took) and route pin
   validity through the board seam when each type is next touched.
5. **Components that cannot report themselves.** `Button` has no observation
   type (consumers each invent one), `BoundedServo`/`ServoOutput` have no
   composing type, `sampled_signal.h` exports three stages but not the
   pipeline its name promises, and `MoistureSensor` lacks the abstract seam
   its climate siblings have.
6. **Real lifecycle defects (small, bounded):** `InertCueScheduler` cannot be
   re-initialized after shutdown; a failed SPI `endTransaction` self-destructs
   the driver while `SpiBus`/`SpiDevice` still report initialized;
   `EnvironmentalStation` misclassifies out-of-range sensor data as a timing
   fault; `ClueConstraintModel::preflightUpdate` is `const` in name only.

Shared-contract changes require the architecture stress-pass discipline; the
convention convergences are migrate-when-touched, not a big-bang rewrite.

## Track 2 — lesson content (110 findings)

Three failures are systemic across nearly every published page:

1. **The HTML pages embed zero images.** Every pencil drawing ships only in
   the PDF; the "accessible alternative" HTML edition is a wall of text.
   Embedding the existing PNGs (alt text already written in the tex sources)
   is the single highest-leverage lesson fix.
2. **No lesson after ~040 has an assembly-representative drawing**, and many
   earlier ones (006, 010, 012, 029, 030, 032, 033, 039) lack breadboard or
   placement plates for their most wiring-dense builds. Lesson 056 has zero
   assets. E0 arcs may not draw powered wiring, but a pencil plate of the
   intended future bench/desk arrangement (clearly labeled as future E1/E2
   shape) is both honest and needed.
3. **Later arcs read as specification dumps.** From roughly lesson 041
   onward, pages are dominated by contract tables, byte layouts, and API
   listings, with predict/observe tables whose observe column says "in
   executable host tests" without a single runnable command. Learners need
   numbered do-then-observe steps: the exact host-test or make invocation,
   the value to feed, the named cell to inspect, and the expected result.
4. **Standalone assessment sections exist only in the PDFs**: fill-in-blank
   worksheets and "Claim, evidence, reasoning" blanks in 003/006, and
   `\section{Exercises...}` blocks in 055, 058, and 065–070. The site pages
   are already clean. Each exercise folds into the replay stage that
   produces its evidence; the standalone sections are deleted.

## Correction to Track 2 — the missing-drawing counts are unreliable

Verified 2026-07-30 by re-profiling every flagged lesson. This repository
places drawings four different ways, and the lesson auditors only recognised
the first:

1. `\includegraphics` of a rendered PNG;
2. `\input` of a standalone TikZ plate from `docs/lessons/assets/`;
3. an **inline** `tikzpicture` written directly in the lesson's `main.tex`; and
4. `\begin{overpic}` — a base PNG with LaTeX-drawn labels overlaid.

Counting `% ADK visual:` markers, which the visual policy requires before every
figure, gives the true count:

| Lesson | Pencil visuals | How they are placed |
|---|---:|---|
| 010 | 2 | both inline TikZ |
| 012 | 2 | one inline, one included |
| 029 | 1 | included with a TikZ overlay |
| 030 | 2 | included with TikZ overlays |
| 032 | 1 | included |
| 033 | 2 | included |
| 039 | 1 (+1 schematic) | `overpic` with LaTeX labels |

The specific claim that Lesson 010 "ships with no pencil drawing at all" is
**false**: it carries two labelled inline TikZ drawings, one naming
`Mega D22`--`D24`, the `74HC595`, the 7-segment display and `DATA / pin 14`,
the other a Mega-and-breadboard layout. What is true is that its
`010-shift-register-pencil.png` asset is unused.

Genuinely unreferenced assets are `009-night-light-pencil`,
`010-shift-register-pencil`, `011-timed-traffic-pencil`,
`018-access-trainer-pencil`, `020-motor-intent-layout`, `021-rover-pencil`,
and `025-infrared-evidence-pencil`. Each is either a superseded draft or an
intended figure someone forgot to place; deciding which, per asset, is the
actual Track 2 task — not drawing replacements for art that already exists.

Lessons 029 and 032 remain genuinely thin at one visual each. Treat every
other "missing drawing" finding in Track 2 as unverified until re-checked with
all four placement mechanisms.

## Track 3 — landing and navigation (14 findings)

There is no single obvious first click: the landing offers four equal-weight
actions, "Open the course" bypasses the course map, and the second element on
the page is a ~59-line per-lesson boundary blockquote that buries the pitch.
The per-lesson scope claims exist in four near-verbatim copies (README,
about, course, lessons index) and are already drifting; the lessons index
simultaneously publishes 080 and lists it as planned. Remediation: one
funnel (start → course map → lesson), a 4–6 line boundary summary on the
landing that links to `about.md#current-status` as the single canonical
boundary statement, and deletion of the stale planned row.

## Track 4 — aesthetics (12 findings)

The intended paper/ink/pencil identity mostly never reaches the page: about
a third of `site.css` targets elements the Bootstrap theme does not render
(`body > header`, `article`), so the 52 rem reading measure and header
styling silently do nothing; dark mode is half-broken because the palette is
hardcoded light while the theme toggle flips `data-bs-theme`; and the
signature pencil drawings — the site's most distinctive asset — appear
nowhere in the HTML. Remediation order: retarget the CSS to the real markup,
restate the palette as Bootstrap variable overrides for both themes, then
let the lesson-page image embedding (track 2) carry the visual identity.

## Remediation program and order

| Phase | Work | Queue |
|---|---|---|
| 1 | Correctness and small lifecycle fixes: record sink (done), cue-scheduler reinitialize, SPI death propagation, station health classification | TASK-7 |
| 2 | Site foundation: CSS retarget + dark mode, landing funnel and canonical boundary statement, stale-row fix | TASK-9, TASK-10 |
| 3 | Lesson pages: embed existing pencil assets across all published pages; convert predict/observe tables to runnable numbered steps arc by arc; dissolve PDF assessment sections | TASK-8 |
| 4 | Assembly plates: per-lesson pencil drawings of physical arrangement, earliest arcs first (006–039), then future-bench plates for E0 arcs. Unblocked — labels render again; prefer the TikZ overlay pattern so plates cannot regress on a host without fontconfig fonts | TASK-8 |
| 5 | Object-design seams and convergence: correlation header, time guard, component chain, then migrate-when-touched convention alignment under stress-pass discipline | TASK-7 |

Lesson 081 continues under its frozen plan; the 079–081 evidence-header
finding feeds its terminal stress pass rather than reopening published
shapes.
